import orderbook_engine
import numpy as np

logger = orderbook_engine.TradeLogger("python_trades.csv")
aapl_book = orderbook_engine.OrderBook("AAPL", logger)


print("Submitting BUY order...")
buy_order = orderbook_engine.Order(59017509, "AAPL", True, 5000000, 2)
aapl_book.placeOrder(buy_order)

print("Submitting SELL order...")
sell_order = orderbook_engine.Order(9017509, "AAPL", False, 5000000, 5)
aapl_book.placeOrder(sell_order)

o = orderbook_engine.Order(1, "AAPL", np.bool_(True), 150250000, 10)
history = logger.getTradesForAccount(59017509)

print(f"\nFound {len(history)} trades for Account 59017509!")
if len(history) > 0:
    first_trade = history[0]
    print(f"Matched Quantity: {first_trade.quantity} at Price: {first_trade.price}")
    print(f"Execution Time: {first_trade.time}")