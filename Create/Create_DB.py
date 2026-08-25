import sqlite3
conn = sqlite3.connect("market_data.db")
cursor = conn.cursor()
cursor.execute(
    "INSERT INTO Trades_Binance (price, quantity, ticker, time) VALUES (?, ?, ?, ?)",
    (100, "5", "BTCUSDT", 1634567890)
)
conn.commit()
conn.close()