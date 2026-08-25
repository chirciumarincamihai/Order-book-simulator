import orderbook_engine
import pytest
import pathlib as pl
pl.Path("python_trades.csv").unlink(missing_ok=True)

@pytest.fixture
def logger():
    return orderbook_engine.TradeLogger("python_trades.csv")

@pytest.fixture
def book(logger):
    return orderbook_engine.OrderBook("AAPL", logger)

def test_unfilled_order_remains(book):
    sellorder = orderbook_engine.Order(1571320, "AAPL", False, 100, 100)
    buyorder = orderbook_engine.Order(1271210, "AAPL", True, 100, 60)
    
    book.placeOrder(buyorder) 
    book.placeOrder(sellorder) 
    assert sellorder.getQuantity() == 40


def test_check_order_quantity(book):
    order = orderbook_engine.Order(1571210, "AAPL", True, 100, 10)
    book.placeOrder(order) 

    assert order.getQuantity() == 10

def test_full_fill(book,logger):
    b_order = orderbook_engine.Order(1571210, "AAPL", True, 100, 10)
    s_order = orderbook_engine.Order(1571320, "AAPL", False, 100, 10)

    book.placeOrder(b_order)
    book.placeOrder(s_order)

    history = logger.getTradesForAccount(1571210)

    assert history[-1].sellerID == 1571320 and history[-1].buyerID == 1571210 and history[-1].quantity == 10

def test_check_cancellation(book):
    order = orderbook_engine.Order(15711120, "AAPL", True, 100, 10)
    book.placeOrder(order)
    book.cancelOrder(order.getID())
    s_order = orderbook_engine.Order(1571320, "AAPL", False, 100, 10)
    book.placeOrder(s_order)

    assert s_order.getQuantity() == 10