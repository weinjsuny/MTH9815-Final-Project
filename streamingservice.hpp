/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham
 */
#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "marketdataservice.hpp"
#include "pricingservice.hpp"
#include <chrono>
#include <ctime>
#include <time.h>


/**
 * A price stream order with price and quantity (visible and hidden)
 */
class PriceStreamOrder
{

public:

  // ctor for an order
  PriceStreamOrder() {};
  PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

  // The side on this order
  PricingSide GetSide() const {
	  
	  return side;
  
  }

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity on this order
  long GetHiddenQuantity() const;

private:
  double price;
  long visibleQuantity;
  long hiddenQuantity;
  PricingSide side;

};

/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
template<typename T>
class PriceStream
{

public:

  // ctor
  PriceStream() {};
  PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the bid order
  const PriceStreamOrder& GetBidOrder() const;

  // Get the offer order
  const PriceStreamOrder& GetOfferOrder() const;

private:
  T product;
  PriceStreamOrder bidOrder;
  PriceStreamOrder offerOrder;

};

/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string,PriceStream <T> >
{

public:

  // Publish two-way prices
  virtual void PublishPrice(const PriceStream<T>& priceStream) = 0;

};


template<typename T>
class GUIService : public Service<string, Price <T> >
{

public:

	virtual void PublishPrice(const Price<T>& price) = 0;

};

PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  side = _side;
}

double PriceStreamOrder::GetPrice() const
{
  return price;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
  return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

template<typename T>
PriceStream<T>::PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder) :
  product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
  return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
  return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
  return offerOrder;
}


template <typename T>

class AlgoStream {

public:

	AlgoStream() {}

	AlgoStream(const Price<T> &p) {

		Bond thisBond = p.GetProduct();

		double mid = p.GetMid();

		double spread = p.GetBidOfferSpread();

		double bid = mid - spread / 2;

		double ask = mid + spread / 2;

		long bid_visibleQuantity = 1000000; 

		long bid_hiddenQuantity = 2000000;

		long ask_visibleQuantity = 1000000;

		long ask_hiddenQuantity = 2000000;

		PriceStreamOrder ps_bid(bid, bid_visibleQuantity, bid_hiddenQuantity, BID);

		PriceStreamOrder ps_ask(ask, ask_visibleQuantity, ask_hiddenQuantity, OFFER);

		PriceStream<T> ps(thisBond, ps_bid, ps_ask);

		priceStream = ps;

	}

	PriceStream<T>  GetPriceStream() const {

		return priceStream;

	}

private:

	PriceStream<T> priceStream;

};



class BondAlgoStreamingService : public Service<string, AlgoStream<Bond>> {

public:

	static BondAlgoStreamingService * instance() {

		static BondAlgoStreamingService  inst;

		return &inst;

	}



	AlgoStream<Bond> & GetData(string product_ID) {

		return algoExeData.at(product_ID);

	}



	void OnMessage(AlgoStream<Bond> &b) {} 

	void AddListener(ServiceListener<AlgoStream<Bond>> *listener) {

		listeners.push_back(listener);

	}



	const vector<ServiceListener<AlgoStream<Bond>> *>& GetListeners() const {

		return listeners;

	}

	void AddPrice(Price<Bond> &price) {

		std::cout << "The bond algostreaming service is feeding bid/offer prices of " << price.GetProduct().GetProductId() << " to the bond streaming service." << std::endl;

		Bond thisBond = price.GetProduct();

		string product_ID = thisBond.GetProductId();

		auto it = algoExeData.find(product_ID);

		if (it == algoExeData.end()) {

			AlgoStream<Bond> newStream(price);

			algoExeData.insert(std::make_pair(product_ID, newStream));

		}


		AlgoStream<Bond> pb = algoExeData[product_ID];

		for (auto& listener : listeners) 	listener->ProcessAdd(pb);
	}


private:

	vector<ServiceListener<AlgoStream<Bond> >*> listeners;  

	std::map<std::string, AlgoStream<Bond> > algoExeData;   

	BondAlgoStreamingService() {}

};




class BondAlgoStreamingServiceListener : public ServiceListener<Price<Bond>> {

public:

	static BondAlgoStreamingServiceListener* instance() {

		static BondAlgoStreamingServiceListener inst;

		return &inst;

	}

	void ProcessAdd(Price<Bond> &data) {

		bondAlgoStreamingService->AddPrice(data);

	}

	void ProcessRemove(Price<Bond> &data) {}  

	void ProcessUpdate(Price<Bond> &data) {}



	BondAlgoStreamingService* GetService() {

		return bondAlgoStreamingService;

	}



private:

	BondAlgoStreamingService* bondAlgoStreamingService;

	BondAlgoStreamingServiceListener() { bondAlgoStreamingService = BondAlgoStreamingService::instance(); }

};


class BondStreamingService : public StreamingService<Bond> {

public:

	static BondStreamingService* instance() {

		static BondStreamingService inst;

		return &inst;

	}



	PriceStream<Bond>& GetData(string product_ID) {

		return streamingData.at(product_ID);

	}

	void OnMessage(PriceStream<Bond> &b) {} 

	void AddListener(ServiceListener<PriceStream<Bond> > *listener) {

		listeners.push_back(listener);

	}

	const vector<ServiceListener<PriceStream<Bond> > *>& GetListeners() const {

		return listeners;

	}

	void PublishPrice(const PriceStream<Bond>& priceStream) {

		Bond thisBond = priceStream.GetProduct();

		string product_ID = thisBond.GetProductId();



		auto it = streamingData.find(product_ID);

		if (it == streamingData.end()) {

			PriceStream<Bond> newStream(priceStream);

			streamingData.insert(std::make_pair(product_ID, newStream));

		}


		PriceStream<Bond> pb = streamingData[product_ID];

		for (auto& listener : listeners) 	listener->ProcessAdd(pb);



	}

	void AddAlgoStream(const AlgoStream<Bond>& algo) {

		auto eo = algo.GetPriceStream();

		string product_ID = eo.GetProduct().GetProductId();

		streamingData[product_ID] = eo;

		std::cout << "The bond streaming service is receiving the bid/offer prices of " << product_ID << " from the bond algostreaming service." << std::endl;

		for (auto& listener : listeners) listener->ProcessAdd(eo);

	}



private:

	std::vector<ServiceListener<PriceStream<Bond> >*> listeners;

	std::map<std::string, PriceStream<Bond>> streamingData;

	BondStreamingService() {}

};





class BondStreamingServiceListener : public ServiceListener<AlgoStream<Bond>> {

public:

	static BondStreamingServiceListener* instance() {

		static BondStreamingServiceListener inst;

		return &inst;

	}

	void ProcessAdd(AlgoStream<Bond> &data) {

		auto eo = data.GetPriceStream();

		bondStreamingService->AddAlgoStream(data);

		bondStreamingService->PublishPrice(eo);

	}

	void ProcessRemove(AlgoStream<Bond> &data) {}  

	void ProcessUpdate(AlgoStream<Bond> &data) {} 



	BondStreamingService* GetService() {

		return bondStreamingService;

	}

private:

	BondStreamingService* bondStreamingService;

	BondStreamingServiceListener() { bondStreamingService = BondStreamingService::instance(); }

};





class BondGUIService : public GUIService<Bond> {

public:

	static BondGUIService* instance() {

		static BondGUIService inst;

		return &inst;

	}



	Price<Bond>& GetData(string product_ID) {

		return priceData.at(product_ID);

	}

	void OnMessage(Price<Bond> &b) {}

	void AddListener(ServiceListener<Price<Bond> > *listener) {

		listeners.push_back(listener);

	}

	const vector<ServiceListener<Price<Bond> > *>& GetListeners() const {

		return listeners;

	}

	void PublishPrice(const Price<Bond>& price) {

		auto Price2String = [](double num) {


			int num1 = int(std::floor(num)), num2 = int((num - num1) * 256),

				num3 = num2 / 8, num4 = num2 % 8;

			string str1 = std::to_string(num1) + "-",

				str2 = std::to_string(num3), str3 = std::to_string(num4);

			if (num3 < 10) str2 = "0" + str2;

			if (num4 == 4)	str3 = "+";

			return str1 + str2 + str3;

		};

		double mid_price = price.GetMid();

		double spread = price.GetBidOfferSpread();

		auto bond = price.GetProduct().GetProductId();

		std::string osc_str = Price2String(spread);

		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() > 299) {
			
			file.open("gui.txt", std::ios::out | std::ios::app);

			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

			std::time_t now_c = std::chrono::system_clock::to_time_t(now);

			file << std::ctime(&now_c) << bond << "," << Price2String(mid_price) << ',' << osc_str << endl;

			file.close();
			
			start = std::chrono::system_clock::now();
		
		}

	}




private:

	std::vector<ServiceListener<Price<Bond> >*> listeners;

	std::map<std::string, Price<Bond>> priceData;

	BondGUIService() {

		start = std::chrono::system_clock::now();

		file.open("gui.txt", std::ios::out | std::ios::trunc);

		file << "timestamp,CUSIP,mid,bidofferspread\n";

		file.close();
	}

	std::chrono::system_clock::time_point  start;

	std::ofstream file;

};





class BondGUIServiceListener : public ServiceListener<Price<Bond>> {

public:

	static BondGUIServiceListener* instance() {

		static BondGUIServiceListener inst;

		return &inst;

	}

	void ProcessAdd(Price<Bond> &data) {

		bondGUIService->PublishPrice(data);

	}

	void ProcessRemove(Price<Bond> &data) {}

	void ProcessUpdate(Price<Bond> &data) {}



	BondGUIService* GetService() {

		return bondGUIService;

	}

private:

	BondGUIService* bondGUIService;

	BondGUIServiceListener() { bondGUIService = BondGUIService::instance(); }

};

#endif
