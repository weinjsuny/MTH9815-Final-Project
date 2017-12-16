/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

  // ctor for a position
	Position() {};
	Position(const T &_product);

  // Get the product
	const T& GetProduct() const;

  // Get the position quantity
	long GetPosition(std::string &book);

  // Get the aggregate position
	long GetAggregatePosition();

	void AddPosition(const std::string &book, long amount) {

		auto it = positions.find(book);

		if (it == positions.end()) {

			positions.insert(std::make_pair(book, amount));
		
		}

		else 
			
			positions[book] += amount;
	
	}



private:
	T product;
	map<std::string,long> positions;

};

/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */

template<typename T>
class PositionService : public Service<string, Position <T> >
{

public:

	// Add a trade to the service
	virtual void AddTrade(const Trade<T> &trade) = 0;

};

class BondPositionService : PositionService<Bond>
{

public:

	static BondPositionService* instance() {

		static BondPositionService inst; 
		
		return &inst;

	}

	void Add(Position<Bond> &position) {

		positionData.insert(std::make_pair(position.GetProduct().GetProductId(), position));

	}



	void OnMessage(Position<Bond> &data) {

		for (auto& listener : listeners) 	listener->ProcessAdd(data);

	}

	void AddTrade(const Trade<Bond> &trade)  {

		std::cout << "The position service is taking trade " << trade.GetTradeId() << " from trading book service." << std::endl;

		Bond thisBond = trade.GetProduct();

		string product_ID = thisBond.GetProductId();

		long quantity = (trade.GetSide() == BUY ? trade.GetQuantity() : -trade.GetQuantity());

		auto it = positionData.find(product_ID);

		if (it == positionData.end()) {

			Position<Bond> pb(thisBond);

			Add(pb);

		}

		positionData[product_ID].AddPosition(trade.GetBook(), quantity);

		Position<Bond> pb = positionData[product_ID];

		quantity = pb.GetAggregatePosition();

		std::cout << "The updated position of product " << product_ID << " is " << quantity << std::endl;

		OnMessage(pb);

	}

	Position<Bond>& GetData(std::string cusip) {

		return positionData.at(cusip);

	}


	void AddListener(ServiceListener<Position<Bond>> *listener) {

		listeners.push_back(listener);

	}


	const vector< ServiceListener<Position<Bond>>* >& GetListeners() const {

		return listeners;

	}

private:

	std::map<std::string, Position<Bond>> positionData;
	
	std::vector<ServiceListener<Position<Bond>>*> listeners;

	BondPositionService() { positionData = std::map<std::string, Position<Bond>>(); };

};

class BondPositionServiceListener : public ServiceListener<Trade<Bond>> {

public:

	static BondPositionServiceListener* instance() {

		static BondPositionServiceListener inst;

		return &inst;

	}

	void ProcessAdd(Trade<Bond> &data) {

		bondPositionService->AddTrade(data);

	}


	void ProcessRemove(Trade<Bond> &data) {}


	void ProcessUpdate(Trade<Bond> &data) {}



	BondPositionService* GetService() {

		return bondPositionService;

	}

private:

	BondPositionService * bondPositionService;

	BondPositionServiceListener() { bondPositionService = BondPositionService::instance(); }

};

template<typename T>
Position<T>::Position(const T &_product) :
  product(_product)
{
	positions = std::map<std::string, long>();

}

template<typename T>
const T& Position<T>::GetProduct() const
{
  return product;
}

template<typename T>
long Position<T>::GetPosition(string &book)
{
  return positions[book];
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
	long result = 0;

	for (auto pair : positions) 	result += pair.second;

	return result;
}

#endif
