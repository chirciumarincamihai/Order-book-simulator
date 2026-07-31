# Order Book Matching Engine
 
A price-time priority order matching engine written in C++, modeling the core mechanics of an exchange's matching system. Built as a learning and portfolio project targeting quantitative finance / trading-adjacent software roles.
 
## What It Does
 
- Accepts buy and sell orders for a given ticker
- Matches incoming orders against the best available opposing price, filling partially or fully based on available quantity
- Maintains **price-time priority**: at each price level, the earliest-arrived order is filled first
- Supports **order cancellation** without disrupting the fairness ordering of other resting orders
- Logs every executed trade to a persistent CSV ledger, queryable per account
## Core Concepts
 
### Fixed-Point Pricing
Prices are stored as scaled integers (e.g. cents, or a configurable `PRICE_SCALE`) rather than `float`/`double`. Floating-point types cannot represent most decimal values exactly, and small rounding errors compound across many trades — a well-known, serious problem in real financial systems. Using integers avoids this entirely, at the cost of needing to divide by the scale factor when displaying human-readable prices.
 
### Price-Time Priority
- **Buy orders** are stored in a `std::map<price, std::deque<Order>, std::greater<>>`, sorted highest price first (the best price a buyer offers).
- **Sell orders** are stored in a `std::map<price, std::deque<Order>>`, sorted lowest price first by default (the best price a seller asks).
- Within each price level, a `std::deque<Order>` preserves arrival order, ensuring first-come-first-served matching among orders at the same price.
### Matching Logic
When a new order arrives, the engine walks the opposing book from its best price outward, matching quantity until either the incoming order is fully filled, no more compatible prices remain, or the book is exhausted. Any unfilled remainder is inserted into the order's own side of the book as a resting order.
 
### Cancellation (Lazy Deletion)
`std::queue`/`std::deque` do not support efficient removal from the middle without disturbing element order. Rather than physically removing a cancelled order (which would reverse the arrival order of orders behind it), cancelled orders are flagged via an `isCanceled` state and silently skipped — then popped — the next time the matching engine reaches them.
 
### Trade Logging
Every executed trade is appended to a CSV file via a `TradeLogger` class, recording both parties' account IDs, order IDs, price, quantity, and timestamp. The logger also supports querying full trade history for a specific account.
 
## Design
 
```cpp
class Order {
    // private: accountID, ID, ticker, isBuy, price, quantity, isCanceled
    // public: getters, fillQuantity(), cancel()
};
 
class Trade {
    // buyOrderID, sellOrderID, buyerID, sellerID, price, quantity, time
};
 
class TradeLogger {
    // logTrade(const Trade&)
    // getTradesForAccount(accountID) -> std::vector<Trade>
};
 
class OrderBook {
    // buyOrders, sellOrders (std::map<price, std::deque<Order>>)
    // orderLocations (std::unordered_map<orderID, {price, isBuy}>) — O(1) cancellation lookup
    // placeOrder(Order&), cancelOrder(orderID)
};
```
 
Core logic is encapsulated using proper OOP principles — order state is private, modified only through controlled methods (`fillQuantity()`, `cancel()`), rather than exposed as public fields.
 
## Data Structures Used
 
| Structure | Purpose | Complexity |
|---|---|---|
| `std::map<price, deque<Order>>` | Sorted order book by price level | O(log n) insert/lookup |
| `std::deque<Order>` | FIFO ordering within a price level | O(1) push/pop at ends, supports iteration for cancellation |
| `std::unordered_map<ID, location>` | Direct lookup of an order's price/side for cancellation | O(1) average lookup |
| `std::vector<Trade>` / CSV file | Trade history | O(1) append |
 
## Building and Running
 
```bash
g++ -std=c++17 -O2 order_book.cpp -o order_book
./order_book
```
 
## Example Usage
 
```cpp
TradeLogger myLogger("trades.csv");
OrderBook aaplBook("AAPL", myLogger);
 
Order order1(59017509, "AAPL", true,  5000000, 2); // buy 2 @ $5.00
Order order2(9017509,  "AAPL", false, 5000000, 5); // sell 5 @ $5.00
 
aaplBook.placeOrder(order1);
aaplBook.placeOrder(order2);
 
auto history = myLogger.getTradesForAccount(59017509);
```
 
