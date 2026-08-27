import asyncio
import sys
import os
import json
current_dir = os.path.dirname(__file__)
engine_path = os.path.join(current_dir, "..", "Engine")
sys.path.append(engine_path)
import orderbook_engine
from websockets.asyncio.client import connect
from decimal import Decimal
import redis
from dataclasses import dataclass

r = redis.Redis(host='localhost', port=6379, decode_responses=True)

@dataclass
class Tick:
    price: int
    quantity: str
    ticker: str
    time: int

Price_Scale = 100000000

async def getting_data():
    async with connect("wss://stream.binance.com:9443/ws/btcusdt@trade") as websocket:
        async for message in websocket:
            data = json.loads(message)
            price = int(Decimal(data['p']) * Price_Scale)
            quantity = str(Decimal(data['q']))
            ticker = data['s']
            time = data['E']
            tick = Tick(price=price, quantity=quantity, ticker=ticker, time=time)
            print(tick)
            r.lpush("ticks", json.dumps(tick.__dict__))

if __name__ == "__main__":
    asyncio.run(getting_data())