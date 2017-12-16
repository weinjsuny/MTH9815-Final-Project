/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include <fstream>
#include "soa.hpp"
#include "products.hpp"

using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:

  // ctor for an order
  Order(double _price, long _quantity, PricingSide _side);

  // Get the price on the order
  double GetPrice() const;

  // Get the quantity on the order
  long GetQuantity() const;

  // Get the side on the order
  PricingSide GetSide() const;

private:
  double price;
  long quantity;
  PricingSide side;

};

/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

  // ctor for bid/offer
  BidOffer(const Order &_bidOrder, const Order &_offerOrder);

  // Get the bid order
  const Order& GetBidOrder() const;

  // Get the offer order
  const Order& GetOfferOrder() const;

private:
  Order bidOrder;
  Order offerOrder;

};

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

  // ctor for the order book
  OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);

  // Get the product
  const T& GetProduct() const;

  // Get the bid stack
  const vector<Order>& GetBidStack() const;

  // Get the offer stack
  const vector<Order>& GetOfferStack() const;

private:
  T product;
  vector<Order> bidStack;
  vector<Order> offerStack;

};

/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{

public:

  // Get the best bid/offer order
  virtual void GetBestBidOffer(const string &productId) = 0;

  // Aggregate the order book
  virtual void AggregateDepth(const string &productId) = 0;

};

Order::Order(double _price, long _quantity, PricingSide _side)
{
  price = _price;
  quantity = _quantity;
  side = _side;
}

double Order::GetPrice() const
{
  return price;
}
 
long Order::GetQuantity() const
{
  return quantity;
}
 
PricingSide Order::GetSide() const
{
  return side;
}

BidOffer::BidOffer(const Order &_bidOrder, const Order &_offerOrder) :
  bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
  return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
  return offerOrder;
}

template<typename T>
OrderBook<T>::OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) :
  product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
  return product;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetBidStack() const
{
  return bidStack;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetOfferStack() const
{
  return offerStack;
}



class BondMarketDataService : MarketDataService<Bond> {

public:

	static BondMarketDataService* instance() {

		static BondMarketDataService inst;

		return &inst;

	}



	virtual void OnMessage(OrderBook <Bond> &data) {

		for (auto listener : listeners) {

			listener->ProcessAdd(data);

		}

	}

	//BidOffer GetBestBidOffer(const string &productId) {};

	//OrderBook<Bond> AggregateDepth(const string &productId) {};

	void GetBestBidOffer(const string &productId) {};

	void AggregateDepth(const string &productId) {};


	OrderBook<Bond>& GetData(std::string _cusip) {

		return marketData.at(_cusip);

	}



	void AddListener(ServiceListener<OrderBook<Bond>> *listener) {

		listeners.push_back(listener);

	}

	const vector< ServiceListener<OrderBook<Bond>>* >& GetListeners() const {

		return listeners;

	}

private:

	std::map<std::string, OrderBook<Bond>> marketData;

	std::vector<ServiceListener<OrderBook<Bond>>*> listeners;

	BondMarketDataService() {}

};



class BondMarketDataConnector : public Connector<OrderBook <Bond>> {

public:

	static BondMarketDataConnector* instance() {

		static BondMarketDataConnector inst;

		return &inst;

	}

	void Publish(OrderBook<Bond> &data) {}

	void Subscribe() {

		auto SplitLine = [](std::string& line) {

			stringstream enter_line(line);

			std::string item;

			std::vector<std::string> tmp;

			while (getline(enter_line, item, ',')) 	tmp.push_back(item);

			return tmp;

		};

		auto String2Price = [](std::string& str) {

			size_t idx = str.find_first_of('-');

			double result = std::stoi(str.substr(0, idx));

			int num1 = std::stoi(str.substr(idx + 1, 2));

			char ch = str[str.size() - 1];

			if (ch == '+') ch = '4';

			int num2 = ch - '0';

			result += (num1 * 8 + num2) / 256.0;

			return result;

		};



		ifstream file("marketdata.txt");

		string line, cusip;

		getline(file, line);

		for (int i = 0; i < 12; ++i) {

			getline(file, line);

			std::vector<std::string> elems = SplitLine(line);

			cusip = elems[0];

			PricingSide side;

			vector<Order> bid_stack, offer_stack;

			double price;	long quantity;

			int idx = 1;

			for (int k = 1; k <= 5; ++k) {

				price = String2Price(elems[idx++]);

				quantity = std::stol(elems[idx++]);

				side = BID;

				Order bid_order(price, quantity, side);

				bid_stack.push_back(bid_order);

			}

			for (int k = 1; k <= 5; ++k) {

				price = String2Price(elems[idx++]);

				quantity = std::stol(elems[idx++]);

				side = OFFER;

				Order offer_order(price, quantity, side);

				offer_stack.push_back(offer_order);

			}



			auto bond = bondBook->GetData(cusip);

			OrderBook<Bond> order_book(bond, bid_stack, offer_stack);

			bondMarketDataService->OnMessage(order_book);

		}

		std::cout << "The marketdata service finished subscribing.\n" << std::endl;

	}



	BondMarketDataService* GetService() {

		return bondMarketDataService;

	}



private:

	BondMarketDataConnector() {

		bondMarketDataService = BondMarketDataService::instance();

		bondBook = BondBook::instance();

	}

	BondMarketDataService * bondMarketDataService;

	BondBook * bondBook;



};

#endif
