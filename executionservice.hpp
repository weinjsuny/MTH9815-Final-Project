/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham
 */
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "marketdataservice.hpp"
#include "datagenerating.hpp"
#include "products.hpp"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

template<typename T>

class ExecutionOrder
{

public:

	ExecutionOrder() {};

	// ctor for an order

	ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
		product(_product)
	{
		side = _side;

		orderId = _orderId;

		orderType = _orderType;

		price = _price;

		visibleQuantity = _visibleQuantity;

		hiddenQuantity = _hiddenQuantity;

		parentOrderId = _parentOrderId;

		isChildOrder = _isChildOrder;

	}

	const T& GetProduct() const

	{

		return product;

	}


	const string& GetOrderId() const

	{

		return orderId;

	}

	OrderType GetOrderType() const

	{

		return orderType;

	}

	double GetPrice() const

	{

		return price;

	}

	long GetVisibleQuantity() const

	{

		return static_cast<long>(visibleQuantity);

	}

	long GetHiddenQuantity() const

	{

		return static_cast<long>(hiddenQuantity);

	}

	const string& GetParentOrderId() const

	{

		return parentOrderId;

	}


	bool IsChildOrder() const

	{

		return isChildOrder;

	}



private:

	T product;

	PricingSide side;

	string orderId;

	OrderType orderType;

	double price;

	double visibleQuantity;

	double hiddenQuantity;

	string parentOrderId;

	bool isChildOrder;

};


template<typename T>

class AlgoExecution {

public:
	AlgoExecution() {};

	ExecutionOrder<T> GetExecutionOrder() const {

		return executionOrder;

	}

	AlgoExecution(OrderBook<T>& o) { 
	
		auto product = o.GetProduct();

		int id = 0;

		string orderID = "O" + to_string(id);

		PricingSide side = ((id % 2) ? BID : OFFER);

		OrderType orderType;

		switch (id % 5) {

		case 0: orderType = FOK; break;

		case 1: orderType = MARKET; break;

		case 2: orderType = LIMIT; break;

		case 3: orderType = STOP; break;

		case 4: orderType = IOC;

		}

		auto ask = o.GetOfferStack().begin();

		auto bid = o.GetBidStack().begin();

		double price = (side == BID ? ask->GetPrice() : bid->GetPrice());

		long vQ = (side == BID ? ask->GetQuantity() : bid->GetQuantity());

		long hQ = 0;

		string parentID = orderID + "P";

		bool isChild = false;

		ExecutionOrder<Bond> Ex_tmp(product, side, orderID, orderType, price, vQ, hQ, parentID, isChild);

		executionOrder = Ex_tmp;
	
	}

private:

	ExecutionOrder<T> executionOrder;

};


	

class BondAlgoExecutionService : public Service<string, AlgoExecution<Bond>> {

public:

	static BondAlgoExecutionService* instance() {

		static BondAlgoExecutionService inst;

		return &inst;

	}

	AlgoExecution<Bond> & GetData(string product_ID) {

		return algoExeData.at(product_ID);

	}

	void OnMessage(AlgoExecution<Bond> &b) {} 

	void AddListener(ServiceListener<AlgoExecution<Bond>> *listener) {

		listeners.push_back(listener);

	}

	const vector<ServiceListener<AlgoExecution<Bond>> *>& GetListeners() const {

		return listeners;

	}

	void AddBook(OrderBook<Bond>& od) {

		std::cout << "The algoexecution service is feeding order book of " << od.GetProduct().GetProductId() << " to the excution service." << std::endl;

		Bond thisBond = od.GetProduct();

		string product_ID = thisBond.GetProductId();

		auto it = algoExeData.find(product_ID);

		if (it == algoExeData.end()) {

			AlgoExecution<Bond> newExecution(od);

			algoExeData.insert(std::make_pair(product_ID, newExecution));

		}

		AlgoExecution<Bond> pb = algoExeData[product_ID];

		for (auto& listener : listeners) 	listener->ProcessAdd(pb);

	}



private:

	vector<ServiceListener<AlgoExecution<Bond> >*> listeners;     

	std::map<std::string, AlgoExecution<Bond> > algoExeData;    

	BondAlgoExecutionService() {}

};




class BondAlgoExecutionServiceListener : public ServiceListener<OrderBook<Bond>> {

public:

	static BondAlgoExecutionServiceListener* instance() {

		static BondAlgoExecutionServiceListener inst;

		return &inst;

	}

	void ProcessAdd(OrderBook<Bond> &data) {

		bondAlgoExecutionService->AddBook(data);

	}

	void ProcessRemove(OrderBook<Bond> &data) {} 

	void ProcessUpdate(OrderBook<Bond> &data) {} 


	BondAlgoExecutionService* GetService() {

		return bondAlgoExecutionService;

	}



private:

	BondAlgoExecutionService* bondAlgoExecutionService;

	BondAlgoExecutionServiceListener() { bondAlgoExecutionService = BondAlgoExecutionService::instance(); }

};




template<typename T>

class ExecutionService : public Service<string, ExecutionOrder <T> >

{

public:


	virtual void ExecuteOrder(const ExecutionOrder<T>& order, Market market) = 0;

};



class BondExecutionService : public ExecutionService<Bond> {

public:

	static BondExecutionService* instance() {

		static BondExecutionService inst;

		return &inst;

	}



	ExecutionOrder<Bond> & GetData(string product_ID) {

		return executionData.at(product_ID);

	}

	void OnMessage(ExecutionOrder<Bond> &b) {};

	void AddListener(ServiceListener<ExecutionOrder<Bond> > *listener) {

		listeners.push_back(listener);

	}

	void AddListener(ServiceListener<Trade<Bond> > *listener) {

		tradelisteners.push_back(listener);

	}

	const vector<ServiceListener<ExecutionOrder<Bond> > *>& GetListeners() const {

		return listeners;

	}



	void ExecuteOrder(const ExecutionOrder<Bond> &order, Market market) {


		Bond thisBond = order.GetProduct();

		string product_ID = thisBond.GetProductId();



		auto it = executionData.find(product_ID);

		if (it == executionData.end()) {

			ExecutionOrder<Bond> newExecution(order);

			executionData.insert(std::make_pair(product_ID, newExecution));

		}

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

		ExecutionOrder<Bond> pb = executionData[product_ID];

		for (auto& listener : listeners) 	listener->ProcessAdd(pb);

		std::cout << "The bond execution service is generating the trade of " << product_ID << "." << std::endl;

		for (int j = 1; j <= 10; ++j) {

			int num, num1, num2, num3, num4;

			std::string str1, str2, str3;

			num = rand() % (256 * 2 + 1);

			num1 = num / 256; num2 = num % 256;

			num3 = num2 / 8; num4 = num2 % 8;

			str1 = std::to_string(99 + num1) + "-";

			str2 = std::to_string(num3);

			str3 = std::to_string(num4);

			if (num4 == 4)	str3 = "+";

			if (num3 < 10) str2 = "0" + str2;

			string cusip, tradeId, book, price, quantity, side;

			price = str1 + str2 + str3;

			tradeId = "T_" + product_ID + std::to_string(j);

			book = "TSRY" + std::to_string(1 + rand() % 3);

			quantity = std::to_string((1 + rand() % 9) * 1000000);

			side = (rand() % 2 == 1 ? "BUY" : "SELL");

			Trade<Bond> trade(thisBond, tradeId, String2Price(price), book, std::stol(quantity), (side == "BUY" ? BUY : SELL));
		
			for (auto& listener : tradelisteners) 	listener->ProcessAdd(trade);
		}


	}


	void AddAlgoExecution(const AlgoExecution<Bond>& algo) {

		auto eo = algo.GetExecutionOrder();

		string product_ID = eo.GetProduct().GetProductId();

		executionData[product_ID] = eo;

		std::cout << "The bond execution service is receiving the execution order of " << product_ID << " from the algoexecution service." << std::endl;

		ExecuteOrder(eo, CME);

	}


private:

	std::vector<ServiceListener<ExecutionOrder<Bond> >*> listeners;

	std::vector<ServiceListener<Trade<Bond> >*> tradelisteners;

	std::map<std::string, ExecutionOrder<Bond> > executionData;

	BondExecutionService() {}

};




class BondExecutionServiceListener : public ServiceListener<AlgoExecution<Bond>> {

public:

	static BondExecutionServiceListener* instance() {

		static BondExecutionServiceListener inst;

		return &inst;

	}

	void ProcessAdd(AlgoExecution<Bond> &data) {

		auto eo = data.GetExecutionOrder();

		bondExecutionService->AddAlgoExecution(data);


	}

	void ProcessRemove(AlgoExecution<Bond> &data) {} 

	void ProcessUpdate(AlgoExecution<Bond> &data) {}



	BondExecutionService* GetService() {

		return bondExecutionService;

	}

private:

	BondExecutionService* bondExecutionService;

	BondExecutionServiceListener() { bondExecutionService = BondExecutionService::instance(); }

};

#endif
