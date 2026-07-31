#include <map>
#include <queue>
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <fstream>
#include <vector>
#include <sstream>

const unsigned long PRICE_SCALE = 1000000;
unsigned int idGenerator = 1;

class Order
{
private:
    unsigned int accountID;
    unsigned int ID;
    std::string ticker;
    bool isBuy;
    unsigned long price;
    unsigned int quantity;
    bool isCanceled = false;

public:
    Order(unsigned int accID, const std::string &tick, // constructor
          bool buy, unsigned long prc, unsigned int qty)
        : accountID(accID), ID(idGenerator++), ticker(tick),
          isBuy(buy), price(prc), quantity(qty), isCanceled(false)
    {
    }

    unsigned int getAccountID() const { return accountID; }
    unsigned int getID() const { return ID; }
    const std::string &getTicker() { return ticker; }
    bool getIsBuy() const { return isBuy; }
    unsigned long getPrice() const { return price; }
    unsigned int getQuantity() const { return quantity; }
    bool getIsCanceled() const { return isCanceled; }
    void fillQuantity(unsigned int tradedQty)
    {
        if (tradedQty <= quantity)
            quantity -= tradedQty;
    }
    void cancel() { isCanceled = true; }
};

std::string getCurrentTimeString()
{
    auto myTimeVariable = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timeString = std::ctime(&myTimeVariable);
    return timeString;
}

class Trade
{
public:
    unsigned int buyOrderID, buyerID;
    unsigned int sellOrderID, sellerID;
    unsigned long price;
    unsigned int quantity;
    std::string time;
};

std::vector<Trade> tradeLog;

class TradeLogger {
private:
    std::string filename;

public:
    explicit TradeLogger(const std::string& file) : filename(file) {}

    void logTrade(const Trade& trade) {
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) return;

        file << trade.buyerID << "," 
             << trade.sellerID << "," 
             << trade.sellOrderID << "," 
             << trade.buyOrderID << ","
             << trade.price << "," 
             << trade.quantity << ","
             << trade.time; 
    }

    std::vector<Trade> getTradesForAccount(unsigned int targetAccountID) {
        std::vector<Trade> accountHistory;
        std::ifstream file(filename);
        
        if (!file.is_open()) {
            std::cerr << "Could not open log file for reading." << std::endl;
            return accountHistory;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string token;
            Trade trade;

            std::getline(ss, token, ','); trade.buyerID = std::stoul(token);
            std::getline(ss, token, ','); trade.sellerID = std::stoul(token);
            std::getline(ss, token, ','); trade.sellOrderID = std::stoul(token);
            std::getline(ss, token, ','); trade.buyOrderID = std::stoul(token);
            std::getline(ss, token, ','); trade.price = std::stoul(token);
            std::getline(ss, token, ','); trade.quantity = std::stoul(token);
            std::getline(ss, token);      trade.time = token; 


            if (trade.buyerID == targetAccountID || trade.sellerID == targetAccountID) {
                accountHistory.push_back(trade);
            }
        }
        return accountHistory;
    }
};

class OrderBook
{
private:
    std::string ticker;
    TradeLogger& logger;
    std::map<unsigned long, std::deque<Order>, std::greater<unsigned long>> buyOrders;
    std::map<unsigned long, std::deque<Order>> sellOrders;
    std::unordered_map<unsigned int, std::pair<unsigned long, bool>> orderLocations;

    void matchBuyOrder(Order &newOrder)
    {
        while (!sellOrders.empty() && newOrder.getPrice() >= sellOrders.begin()->first && newOrder.getQuantity() > 0)
        {

            Order &bestSeller = sellOrders.begin()->second.front(); // begin takes 1st element from the map(lowest price) -> secound takes the 2nd part of map (the deque of orders at that price)

            if (bestSeller.getIsCanceled())
            {
                sellOrders.begin()->second.pop_front();
                continue;
            }

            unsigned int tradedQuantity = std::min(newOrder.getQuantity(), bestSeller.getQuantity());
            newOrder.fillQuantity(tradedQuantity);
            bestSeller.fillQuantity(tradedQuantity);

            if (bestSeller.getQuantity() == 0)
            {
                sellOrders.begin()->second.pop_front();
            }
            if (sellOrders.begin()->second.empty())
            {
                sellOrders.erase(sellOrders.begin());
            }
            Trade trade;
            trade.buyOrderID = newOrder.getID();
            trade.sellOrderID = bestSeller.getID();
            trade.buyerID = newOrder.getAccountID();
            trade.sellerID = bestSeller.getAccountID();
            trade.price = bestSeller.getPrice();
            trade.quantity = tradedQuantity;
            trade.time = getCurrentTimeString();
            logger.logTrade(trade);
        }
        if (newOrder.getQuantity() > 0)
        {
            buyOrders[newOrder.getPrice()].push_back(newOrder);
            orderLocations[newOrder.getID()] = {newOrder.getPrice(), newOrder.getIsBuy()};
        }
    }

    void matchSellOrder(Order &newOrder)
    {
        while (!buyOrders.empty() && newOrder.getPrice() <= buyOrders.begin()->first && newOrder.getQuantity() > 0)
        {

            Order &bestBuyer = buyOrders.begin()->second.front();

            if (bestBuyer.getIsCanceled())
            {
                buyOrders.begin()->second.pop_front();
                continue;
            }

            unsigned int tradedQuantity = std::min(newOrder.getQuantity(), bestBuyer.getQuantity());
            newOrder.fillQuantity(tradedQuantity);
            bestBuyer.fillQuantity(tradedQuantity);

            if (bestBuyer.getQuantity() == 0)
            {
                buyOrders.begin()->second.pop_front();
            }
            if (buyOrders.begin()->second.empty())
            {
                buyOrders.erase(buyOrders.begin());
            }
            Trade trade;
            trade.sellOrderID = newOrder.getID();
            trade.buyOrderID = bestBuyer.getID();
            trade.sellerID = newOrder.getAccountID();
            trade.buyerID = bestBuyer.getAccountID();
            trade.price = bestBuyer.getPrice();
            trade.quantity = tradedQuantity;
            trade.time = getCurrentTimeString();
            logger.logTrade(trade);
        }
        if (newOrder.getQuantity() > 0)
        {
            sellOrders[newOrder.getPrice()].push_back(newOrder);
            orderLocations[newOrder.getID()] = {newOrder.getPrice(), newOrder.getIsBuy()};
        }
    }

public:
    explicit OrderBook(const std::string &symbol, TradeLogger& logRef) : ticker(symbol), logger(logRef) {}

    void placeOrder(Order &newOrder)
    {
        if (newOrder.getIsBuy())
            matchBuyOrder(newOrder);
        else
            matchSellOrder(newOrder);
    }

    void cancelOrder(unsigned int orderID)
    {
        auto it = orderLocations.find(orderID);
        if (it != orderLocations.end())
        {
            unsigned long price = it->second.first;
            bool isBuy = it->second.second;

            if (isBuy)
            {
                for (auto &order : buyOrders[price])
                {
                    if (order.getID() == orderID)
                    {
                        order.cancel();
                        break;
                    }
                }
            }
            else
            {
                for (auto &order : sellOrders[price])
                {
                    if (order.getID() == orderID)
                    {
                        order.cancel();
                        break;
                    }
                }
            }
            orderLocations.erase(it);
        }
    }
};

int main() {
    TradeLogger myLogger("trades.csv");
    OrderBook aaplBook("AAPL", myLogger);

    Order order1(59017509, "AAPL", true,  5000000, 2);
    Order order2(9017509,  "AAPL", false, 5000000, 5);

    std::cout << "Placing order 1..." << std::endl;
    aaplBook.placeOrder(order1);
    
    std::cout << "Placing order 2..." << std::endl;
    aaplBook.placeOrder(order2); 

    std::cout << "\nFetching ledger for Account 59017509..." << std::endl;
    auto history = myLogger.getTradesForAccount(59017509);
    std::cout << "Found " << history.size() << " trades for this account!" << std::endl;

    return 0;
}