# High-Performance Limit Order Book (LOB) Engine
 
## Overview
A high-performance C++ limit order book (LOB) matching engine, bridged to Python via `pybind11`, extended into a distributed, live-data pipeline in the style of a real post-trade system, connecting live market data, routing it through a message queue, processing it across parallel workers, and persisting it for downstream analysis.
 
Built as a portfolio project targeting entry-level quant-developer / backend roles, structured around a real job posting's requirements (async ingest, message queue, parallel workers, time-series store, reconciliation, load testing).
 
## Project Structure
```
Engine/     — C++ matching engine, pybind11 bindings, build files
Pipeline/   — live listener, worker(s), batch CSV ingestion
Data/       — generated output (SQLite store, trade logs) — not source code
Tests/      — pytest suite
```
 
## Core Architecture
 
### C++ Execution Engine (`Engine/`)
- **Price-Time Priority:** `std::map` (Red-Black Tree) for automatic price-level sorting, `std::deque` for fast iteration of resting time-priority orders.
- **Precision Math:** Prices stored as scaled fixed-point integers using `uint64_t` — a fixed-width type, chosen after discovering `unsigned long` is 32-bit on MSVC/Windows but 64-bit on Linux/GCC, a platform-dependent bug that would silently corrupt or overflow prices depending on where the code was compiled.
- **Efficient Memory Management:** Lazy deletion for canceled orders. A real crash bug was found and fixed here during testing: canceling the front order of a price level and popping it left an empty-but-not-erased map entry, causing a later `.front()` call on an empty deque — undefined behavior that produced a hard access-violation crash. Fixed by mirroring the same empty-check/erase pattern the fill branch already used, in both `matchBuyOrder` and `matchSellOrder`.
- **Trade Ledger:** Every executed trade (buyer, seller, price, quantity, ticker, timestamp) is appended to a CSV log via `TradeLogger`.
### Python Integration (`Pipeline/`)
- **`OrderIngestor` (OO batch ingestion):** Reads simulated order flow from CSV via `pandas`, safely converts decimal prices to the engine's scaled integer format using `round()` (not truncation — a real precision bug was traced here, caused by binary floating-point representation of decimals like `0.2`), and routes each order to the correct per-ticker `OrderBook`, created lazily and reused via `dict.setdefault`.
- **Live Market Data Listener (`asyncio`):** Connects to Binance's public trade WebSocket feed, continuously streaming live trades. Each tick is parsed into a `Tick` dataclass (a typed domain object, not a raw dict) — price and quantity, which Binance sends as strings specifically to avoid lossy float parsing, are converted via Python's `Decimal` type to preserve exact precision before scaling.
- **Message Queue (Redis, via WSL2):** The listener pushes each normalized `Tick` onto a Redis list (`LPUSH`), decoupling ingestion from processing. Worker processes consume with `BRPOP` — a blocking pop that waits efficiently rather than busy-looping. Verified with two workers running simultaneously against the same queue: the stream splits cleanly between them with zero duplication.
- **Time-Series Store (SQLite):** Each worker writes consumed ticks into a `Trades_Binance` table (`price`, `quantity` as text to preserve precision, `ticker`, `time`), giving a persistent, queryable record of live market activity.
## Tech Stack
- **Core Engine:** C++17
- **Interoperability:** pybind11
- **Build System:** MSVC / setuptools
- **Data Analytics:** Python 3, pandas
- **Live Data:** `asyncio`, `websockets`, `json`, `decimal`
- **Message Queue:** Redis (via WSL2)
- **Storage:** SQLite (`sqlite3`)
- **Testing:** pytest
## Design Decisions Worth Noting
- **Live ticks are never fed into the matching engine.** A Binance trade tick represents a completed, real-world event with no account ID and no clean buy/sell side — feeding it into `Order()`/`placeOrder()` would mean injecting an already-finished trade as if it were a fresh order. Live data and simulated CSV orders are deliberately kept on separate paths; the live pipeline exists to *observe* the market, not to generate matching activity.
- **`Trade` objects are not shared references.** C++ containers holding `Order`/`Trade` by value make copies on insert — a Python-held object never reflects the engine's internal mutations. State is checked through the trade log (`TradeLogger.getTradesForAccount`), not by re-inspecting the original object, matching how real exchanges report fills via separate notifications rather than mutable shared state.
