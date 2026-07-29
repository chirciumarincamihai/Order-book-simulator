#include <map>
#include <queue>
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

const unsigned long PRICE_SCALE = 1000000;
unsigned int idGenerator = 1;

struct Order {
    unsigned int ID;
    std::string ticker;
    bool isBuy;
    unsigned long price;
    unsigned int quantity;
    bool isCanceled=false;
};

std::map<unsigned long, std::deque<Order>, std::greater<unsigned long>> buyOrders;
std::map<unsigned long, std::deque<Order>> sellOrders;
std::unordered_map<unsigned int, std::pair<unsigned long, bool>> orderLocations;

void matchBuyOrder(Order& newOrder) {
    while (!sellOrders.empty() && newOrder.price >= sellOrders.begin()->first && newOrder.quantity > 0) {

        Order& bestSeller = sellOrders.begin()->second.front(); //begin takes 1st element from the map(lowest price) -> secound takes the 2nd part of map (the deque of orders at that price)
 
        if(bestSeller.isCanceled){
            sellOrders.begin()->second.pop_front();
            continue;
        }

        unsigned int tradedQuantity = std::min(newOrder.quantity, bestSeller.quantity);
        newOrder.quantity -= tradedQuantity;
        bestSeller.quantity -= tradedQuantity;

        if (bestSeller.quantity == 0) {
            sellOrders.begin()->second.pop_front();
        }
        if (sellOrders.begin()->second.empty()) {
            sellOrders.erase(sellOrders.begin());
        }
    }
        if(newOrder.quantity > 0){
            buyOrders[newOrder.price].push_back(newOrder);
            orderLocations[ newOrder.ID ] = { newOrder.price , newOrder.isBuy };
            }
}

void matchSellOrder(Order& newOrder) {
    while (!buyOrders.empty() && newOrder.price <= buyOrders.begin()->first && newOrder.quantity > 0) {

        Order& bestBuyer = buyOrders.begin()->second.front(); 

        if(bestBuyer.isCanceled){ 
            buyOrders.begin()->second.pop_front(); 
            continue; 
        }

        unsigned int tradedQuantity = std::min(newOrder.quantity, bestBuyer.quantity);
        newOrder.quantity -= tradedQuantity;
        bestBuyer.quantity -= tradedQuantity;

        if (bestBuyer.quantity == 0) {
            buyOrders.begin()->second.pop_front();
        }
        if (buyOrders.begin()->second.empty()) {
            buyOrders.erase(buyOrders.begin());
        }
    }
        if(newOrder.quantity > 0){
            sellOrders[newOrder.price].push_back(newOrder);
            orderLocations[ newOrder.ID ] = { newOrder.price , newOrder.isBuy };
}
}

void placeOrder(Order& newOrder)
{
    if(newOrder.isBuy)
        matchBuyOrder(newOrder);
    else
        matchSellOrder(newOrder);
}
Order generateOrder(std::string ticker, bool isBuy, unsigned long price, unsigned int quantity) {
    Order newOrder;
    newOrder.ID = idGenerator++;
    newOrder.ticker = ticker;
    newOrder.isBuy = isBuy;
    newOrder.price = price;
    newOrder.quantity = quantity;
    return newOrder;
}
void cancelOrder(unsigned int orderID){
    auto it = orderLocations.find(orderID);
    if (it != orderLocations.end()) {
        unsigned long price = it->second.first;
        bool isBuy = it->second.second;

        if (isBuy) {
            for (auto& order : buyOrders[price]) {
                if (order.ID == orderID) {
                    order.isCanceled = true;
                    break;
                }
            }
        } else {
            for (auto& order : sellOrders[price]) {
                if (order.ID == orderID) {
                    order.isCanceled = true;
                    break;
                }
            }
        }
        orderLocations.erase(it);
    }
}

int main() {
    // Example usage
    Order order1 = generateOrder("AAPL", true, 5000000, 10); // Buy order
    Order order2 = generateOrder("AAPL", false, 5000000, 5); // Sell order

    std::cout << "Placing order 1..." << std::endl;
    placeOrder(order1);
    std::cout << "Placing order 2..." << std::endl;
    placeOrder(order2);

    return 0;
}