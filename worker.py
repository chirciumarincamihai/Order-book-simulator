import redis
import json
import sqlite3
conn = sqlite3.connect("market_data.db")
cursor = conn.cursor()
r = redis.Redis(host='localhost', port=6379, decode_responses=True)


while True: 
    results = r.brpop("ticks") 
    ticks = json.loads(results[1]) 
    cursor.execute(
    "INSERT INTO Trades_Binance (price, quantity, ticker, time) VALUES (?, ?, ?, ?)",
    (ticks['price'], ticks['quantity'], ticks['ticker'], ticks['time'])
)
    conn.commit()
