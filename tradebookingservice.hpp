/**
 * tradebookingservice.hpp
 * Defines the data types and Service for trade booking.
 *
 * @author Breman Thuraisingham
 */
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"
#include "products.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

  // ctor for a trade
  Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);

  // Get the product
  const T& GetProduct() const;

  // Get the trade ID
  const string& GetTradeId() const;

  // Get the mid price
  double GetPrice() const;

  // Get the book
  const string& GetBook() const;

  // Get the quantity
  long GetQuantity() const;

  // Get the side
  Side GetSide() const;

private:
  T product;
  string tradeId;
  double price;
  string book;
  long quantity;
  Side side;

};

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on product identifier.
 * Type T is the product type.
 */

template<typename T>
class TradeBookingService : public Service<string, Trade <T> >
{

public:

	// Book the trade
	virtual void BookTrade(Trade<T> &trade) = 0;

};


class BondTradeBookingService : public TradeBookingService<Bond>
{

public:

	static BondTradeBookingService* instance() {

		static BondTradeBookingService inst; 
		
		return &inst;

	}
	// Book the trade
	void BookTrade(Trade<Bond> &trade) {

		std::cout << "The tradebooking service is feeding " << trade.GetTradeId() << "  to the position service." << std::endl;

		for (auto& listener : listeners) 	listener->ProcessAdd(trade);

	}

	const vector< ServiceListener<Trade<Bond>>* >& GetListeners() const {

		return listeners;

	}

	Trade<Bond>& GetData(std::string cusip) {

		return tradeData.at(cusip);

	}

	void AddListener(ServiceListener<Trade<Bond>> *listener) {

		listeners.push_back(listener);

	}

	void OnMessage(Trade<Bond> &trade) {

		auto cusip = trade.GetProduct().GetProductId();

		tradeData.insert(std::make_pair(cusip, trade));

		BookTrade(trade); 

	}


private:

	std::map<std::string, Trade<Bond>> tradeData;

	BondTradeBookingService() {};

	std::vector<ServiceListener<Trade<Bond>>*> listeners;

};

template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
  product(_product)
{
  tradeId = _tradeId;
  price = _price;
  book = _book;
  quantity = _quantity;
  side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
  return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
  return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
  return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
  return side;
}




// connector
class BondTradeBookingConnector : public Connector<Trade<Bond>> {

public:

	static BondTradeBookingConnector* instance() {

		static BondTradeBookingConnector inst;

		return &inst;

	}


	void Publish(Trade<Bond> &data) {}


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



		ifstream file("trades.txt");

		string line;

		string cusip, tradeId, book, price, quantity, side;

		getline(file, line); 

		while (getline(file, line)) {

			std::vector<std::string> elems = SplitLine(line);

			cusip = elems[0]; tradeId = elems[1]; book = elems[2];

			price = elems[3]; quantity = elems[4]; side = elems[5];

			Bond bond = bondBook->GetData(cusip);

			Trade<Bond> trade(bond, tradeId, String2Price(price), book, std::stol(quantity), (side == "BUY" ? BUY : SELL));

			bondTradeBookingservice->OnMessage(trade);

		}

		std::cout << "The tradingbook service finished subscribing.\n" << std::endl;

	}

	TradeBookingService<Bond>* GetService() {

		return bondTradeBookingservice;

	}

private:

	BondTradeBookingConnector() {

		bondTradeBookingservice = BondTradeBookingService::instance();

		bondBook = BondBook::instance();

	}

	BondTradeBookingService * bondTradeBookingservice;

	BondBook * bondBook;

};


class BondTradeBookingServiceListener : public ServiceListener<Trade<Bond>> {

public:

	static BondTradeBookingServiceListener* instance() {

		static BondTradeBookingServiceListener inst;

		return &inst;

	}

	void ProcessAdd(Trade<Bond> &data) {

		bondTradeBookingService->BookTrade(data);


	}

	void ProcessRemove(Trade<Bond> &data) {}

	void ProcessUpdate(Trade<Bond> &data) {}



	BondTradeBookingService* GetService() {

		return bondTradeBookingService;

	}

private:

	BondTradeBookingService* bondTradeBookingService;

	BondTradeBookingServiceListener() { bondTradeBookingService = BondTradeBookingService::instance(); }

};

#endif
