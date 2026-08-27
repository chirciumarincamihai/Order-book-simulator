import sqlite3
conn = sqlite3.connect("market_data.db")
cursor = conn.cursor()
cursor.execute('''
CREATE TABLE IF NOT EXISTS Trades_Binance (
price INTEGER, quantity TEXT, ticker TEXT, time INTEGER
)''')
conn.commit()
cursor.execute(
    "INSERT INTO Trades_Binance (price, quantity, ticker, time) VALUES (?, ?, ?, ?)",
    (100, "5", "BTCUSDT", 1634567890)
)
conn.commit()
conn.close()