import asyncio
import pandas as pd
import json
import orderbook_engine
import websockets
import pathlib as pl
from websockets.asyncio.client import connect
from decimal import Decimal
import redis
r = redis.Redis(host='localhost', port=6379, decode_responses=True)

df = pd.read_csv("market_data.csv")
pl.Path("python_trades.csv").unlink(missing_ok=True)

Price_Scale = 100000000

class OrderIngestor: 
    def __init__(self): 
        self.logger = orderbook_engine.TradeLogger("python_trades.csv") 
        self.dict_of_books = {} 
    
    def process_order(self, datas): 

        order = orderbook_engine.Order(datas['ticker'], datas['is_buy'], datas['price'], datas['quantity'])
        default_book =self.dict_of_books.setdefault(order.getTicker(), orderbook_engine.OrderBook(order.getTicker(), self.logger))
        default_book.placeOrder(order)

ingestor = OrderIngestor()


async def getting_data():
    async with connect("wss://stream.binance.com:9443/ws/btcusdt@trade") as websocket:

        async for message in websocket:

            data=json.loads(message)
            price=int(Decimal(data['p'])*Price_Scale)
            quantity=str((Decimal(data['q'])))
            ticker=data['s']
            time=data['E']
            datas={'price': price, 'quantity': quantity, 'ticker': ticker, 'time': time}
            print(f"Price: {price}, Quantity: {quantity}, Ticker: {ticker}, Time: {time}")

            r.lpush("ticks", json.dumps(datas))
        

if __name__ == "__main__": 
    asyncio.run(getting_data())