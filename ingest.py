import pandas as pd
import orderbook_engine
import pathlib as pl

df = pd.read_csv("market_data.csv")
pl.Path("python_trades.csv").unlink(missing_ok=True)

Price_Scale = 100000000

class OrderIngestor: 
    def __init__(self): 
        self.logger = orderbook_engine.TradeLogger("python_trades.csv") 
        self.dict_of_books = {} 
    
    def process_order(self, row): 
        Price = int(round(row.Price * Price_Scale)) 
        order = orderbook_engine.Order(row.AccountID, row.Ticker,row.IsBuy, Price, row.Quantity)
        default_book =self.dict_of_books.setdefault(order.getTicker(), orderbook_engine.OrderBook(order.getTicker(), self.logger))
        default_book.placeOrder(order)

ingestor = OrderIngestor()

for row in df.itertuples():
    ingestor.process_order(row)