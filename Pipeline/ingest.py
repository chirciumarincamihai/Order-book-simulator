import sys
import os
current_dir = os.path.dirname(__file__)
engine_path = os.path.join(current_dir, "..", "Engine")
sys.path.append(engine_path)
import orderbook_engine
import pandas as pd
import pathlib as pl

csv_path = os.path.join(current_dir, "market_data.csv")
df = pd.read_csv(csv_path)

trades_path = os.path.join(current_dir, "python_trades.csv")
pl.Path(trades_path).unlink(missing_ok=True)

Price_Scale = 100000000

class OrderIngestor: 
    def __init__(self): 
        self.logger = orderbook_engine.TradeLogger(trades_path) 
        self.dict_of_books = {} 
    
    def process_order(self, row): 
        Price = int(round(row.Price * Price_Scale)) 
        order = orderbook_engine.Order(row.AccountID, row.Ticker, row.IsBuy, Price, row.Quantity)
        default_book = self.dict_of_books.setdefault(order.getTicker(), orderbook_engine.OrderBook(order.getTicker(), self.logger))
        default_book.placeOrder(order)

ingestor = OrderIngestor()

for row in df.itertuples():
    ingestor.process_order(row)