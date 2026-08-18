import asyncio
import pandas as pd
from streamlit import json
import orderbook_engine
import websocket
import websockets
import pathlib as pl
from websockets.asyncio.client import connect
import json
from decimal import Decimal

df = pd.read_csv("market_data.csv")
pl.Path("python_trades.csv").unlink(missing_ok=True)

Price_Scale = 100000000

class OrderIngestor: 
    def __init__(self): 
        self.logger = orderbook_engine.TradeLogger("python_trades.csv") 
        self.dict_of_books = {} 
    
    def process_order(self, row): 

        order = orderbook_engine.Order(row.Ticker,row.IsBuy, row.Price, row.Quantity)
        default_book =self.dict_of_books.setdefault(order.getTicker(), orderbook_engine.OrderBook(order.getTicker(), self.logger))
        default_book.placeOrder(order)

ingestor = OrderIngestor()


async def getting_data():
    async with connect("wss://stream.binance.com:9443/ws/btcusdt@trade") as websocket:

        async for message in websocket:

            data=json.loads(message)
            price=int(Decimal(data['p'])*Price_Scale)
            quantity=Decimal(data['q'])
            ticker=data['s']
            time=data['E']
            data={price,quantity,ticker,time}
            ingestor.process_order(data)
            print(f"Price: {price}, Quantity: {quantity}, Ticker: {ticker}, Time: {time}")

    if __name__ == "__main__": 
        asyncio.run(getting_data())