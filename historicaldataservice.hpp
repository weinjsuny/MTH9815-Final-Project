/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Breman Thuraisingham
 * Defines the data types and Service for historical data.
 *
 * @author Breman Thuraisingham
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP
#include "datagenerating.hpp"
#include "products.hpp"
#include "tradebookingservice.hpp"
#include "positionservice.hpp"
#include "riskservice.hpp"
#include "inquiryservice.hpp"
#include "marketdataservice.hpp"
#include "executionservice.hpp"
#include "pricingservice.hpp"
#include "streamingservice.hpp"


/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataService : Service<string,T>
{

public:

  // Persist data to a store
	virtual void PersistData(string persistKey, const T& data) {};

};

#endif


class BondHistoricalPV01Connector : public Connector<PV01<Bond>> {

public:

	static BondHistoricalPV01Connector* instance() {

		static BondHistoricalPV01Connector inst;

		return &inst;

	}

	void Publish(PV01<Bond>& data) {

		ofstream os("risk.txt", ios_base::app);

		string msg = "PV01 of " + data.GetProduct().GetProductId() + " is " + std::to_string(data.GetPV01());

		os << msg << endl;

	}

	void Subscribe() {};

private:

	BondHistoricalPV01Connector() {}

};



class BondHistoricalPV01Service : public HistoricalDataService<PV01<Bond> > {


public:

	static BondHistoricalPV01Service* instance() {

		static BondHistoricalPV01Service inst;

		return &inst;

	}

	PV01<Bond> & GetData(string persistKey) {

		return Data.at(persistKey);

	}

	void OnMessage(PV01<Bond> &b) {}



	void AddListener(ServiceListener<PV01<Bond> > *listener) {

		listeners.push_back(listener);

	}

	const vector<ServiceListener<PV01<Bond> > *>& GetListeners() const {

		return listeners;

	}

	void PersistData(string persistKey, PV01<Bond>& data) {

		bondHistoricalPV01Connector->Publish(data);

	}

private:

	vector<ServiceListener<PV01<Bond> >*> listeners;    

	map<string, PV01 <Bond> > Data;      

	BondHistoricalPV01Connector* bondHistoricalPV01Connector;

	BondHistoricalPV01Service() { bondHistoricalPV01Connector = BondHistoricalPV01Connector::instance(); }

};



class BondHistoricalPV01ServiceListener : public ServiceListener<PV01<Bond>> {

public:

	static BondHistoricalPV01ServiceListener* instance() {

		static BondHistoricalPV01ServiceListener inst;

		return &inst;

	}

	void ProcessAdd(PV01<Bond> &data) {

		bondHistoryPV01Service->PersistData(data.GetProduct().GetProductId(), data); 

	}

	void ProcessRemove(PV01<Bond> &data) {}

	void ProcessUpdate(PV01<Bond> &data) {}

private:

	BondHistoricalPV01Service * bondHistoryPV01Service;

	BondHistoricalPV01ServiceListener() { bondHistoryPV01Service = BondHistoricalPV01Service::instance(); }

};


class BondHistoricalExecutionConnector : public Connector<ExecutionOrder<Bond>> {

public:

	static BondHistoricalExecutionConnector* instance() {

		static BondHistoricalExecutionConnector inst;

		return &inst;

	}

	void Publish(ExecutionOrder<Bond>& data) {

		ofstream os("executions.txt", ios_base::app);

		std::string msg = "Executing the order of bond " + data.GetProduct().GetProductId();

		os << msg << endl;

	}

	void Subscribe() {}  

private:

	BondHistoricalExecutionConnector() {}

};



class BondHistoricalExecutionService : public HistoricalDataService<ExecutionOrder<Bond> > {

public:

	static BondHistoricalExecutionService* instance() {

		static BondHistoricalExecutionService inst;

		return &inst;

	}

	ExecutionOrder<Bond> & GetData(string persistKey) {

		return Data.at(persistKey);

	}

	void OnMessage(ExecutionOrder<Bond> &b) {

		auto persistKey = b.GetProduct().GetProductId();

		Data[persistKey] = b;

		for (auto& lp : listeners) lp->ProcessAdd(b); 

	}


	void AddListener(ServiceListener<ExecutionOrder<Bond> > *listener) {

		listeners.push_back(listener);

	}

	const vector<ServiceListener<ExecutionOrder<Bond> > *>& GetListeners() const {

		return listeners;

	}

	void PersistData(string persistKey, ExecutionOrder<Bond>& data) {

		bondHistoricalExecutionConnector->Publish(data);

	}

private:

	vector<ServiceListener<ExecutionOrder<Bond> >*> listeners;      

	map<string, ExecutionOrder <Bond> > Data;                       

	BondHistoricalExecutionConnector* bondHistoricalExecutionConnector; 

	BondHistoricalExecutionService() { bondHistoricalExecutionConnector = BondHistoricalExecutionConnector::instance(); }

};



class BondHistoricalExecutionServiceListener : public ServiceListener<ExecutionOrder<Bond>> {

public:

	static BondHistoricalExecutionServiceListener* instance() {

		static BondHistoricalExecutionServiceListener inst;

		return &inst;

	}


	void ProcessAdd(ExecutionOrder<Bond> &data) {

		bondHistoryExecutionService->OnMessage(data);

		bondHistoryExecutionService->PersistData(data.GetProduct().GetProductId(), data); // to write.

	}

	void ProcessRemove(ExecutionOrder<Bond> &data) {}

	void ProcessUpdate(ExecutionOrder<Bond> &data) {}



private:

	BondHistoricalExecutionService * bondHistoryExecutionService;



	BondHistoricalExecutionServiceListener() { bondHistoryExecutionService = BondHistoricalExecutionService::instance(); }

};




class BondHistoricalStreamingConnector : public Connector<PriceStream<Bond>> {

public:

	static BondHistoricalStreamingConnector* instance() {

		static BondHistoricalStreamingConnector inst;

		return &inst;

	}



	void Publish(PriceStream<Bond>& data) {

		ofstream os("streaming.txt", ios_base::app);



		double bid = data.GetBidOrder().GetPrice();

		double offer = data.GetOfferOrder().GetPrice();

		std::string cus_id = data.GetProduct().GetProductId();

		std::string msg = "The bond " + cus_id + " has bid price " + std::to_string(bid) +

			" and offer price " + std::to_string(offer);

		os << msg << endl;

	}

	void Subscribe() {} 



private:

	BondHistoricalStreamingConnector() {}
};



class BondHistoricalStreamingService : public HistoricalDataService<PriceStream<Bond> > {

public:

	static BondHistoricalStreamingService* instance() {

		static BondHistoricalStreamingService inst;

		return &inst;

	}


	PriceStream<Bond> & GetData(string persistKey) {

		return Data.at(persistKey);

	}

	void OnMessage(PriceStream<Bond> &b) {

		auto persistKey = b.GetProduct().GetProductId();

		Data[persistKey] = b;		

		for (auto& lp : listeners) lp->ProcessAdd(b); 

	}



	void AddListener(ServiceListener<PriceStream<Bond> > *listener) {

		listeners.push_back(listener);

	}

	const vector<ServiceListener<PriceStream<Bond> > *>& GetListeners() const {

		return listeners;

	}

	void PersistData(string persistKey, PriceStream<Bond>& data) {

		bondHistoricalStreamingConnector->Publish(data);

	}



private:

	vector<ServiceListener<PriceStream<Bond> >*> listeners;      

	map<string, PriceStream <Bond> > Data;                       

	BondHistoricalStreamingConnector* bondHistoricalStreamingConnector; 

	BondHistoricalStreamingService() { bondHistoricalStreamingConnector = BondHistoricalStreamingConnector::instance(); }

};



class BondHistoricalStreamingServiceListener : public ServiceListener<PriceStream<Bond>> {

public:

	static BondHistoricalStreamingServiceListener* instance() {

		static BondHistoricalStreamingServiceListener inst;

		return &inst;

	}


	void ProcessAdd(PriceStream<Bond> &data) {

		bondHistoryStreamingService->OnMessage(data);

		bondHistoryStreamingService->PersistData(data.GetProduct().GetProductId(), data); 

	}

	void ProcessRemove(PriceStream<Bond> &data) {}

	void ProcessUpdate(PriceStream<Bond> &data) {}



private:

	BondHistoricalStreamingService * bondHistoryStreamingService;

	BondHistoricalStreamingServiceListener() { bondHistoryStreamingService = BondHistoricalStreamingService::instance(); }

};




class BondHistoricalInquiryConnector : public Connector<Inquiry<Bond>> {

public:

	static BondHistoricalInquiryConnector* instance() {

		static BondHistoricalInquiryConnector inst;

		return &inst;

	}

	void Publish(Inquiry<Bond>& data) {

		ofstream os("allinquiries.txt", ios_base::app);

		std::string msg;

		msg += "The inquire ID is " + data.GetInquiryId();

		data.GetSide() == Side::BUY ? msg += " and BUY Side " : msg += " and SELL Side ";

		msg += ", the product is " + data.GetProduct().GetProductId();

		msg += ", the quantity is " + std::to_string(data.GetQuantity());

		msg += ", the price is " + std::to_string(data.GetPrice());

		os << msg << endl;

	}

	void Subscribe() {}  

private:

	BondHistoricalInquiryConnector() {}

};



class BondHistoricalInquiryService : public HistoricalDataService<Inquiry<Bond> > {



public:

	static BondHistoricalInquiryService* instance() {

		static BondHistoricalInquiryService inst;

		return &inst;

	}


	Inquiry<Bond> & GetData(string persistKey) {

		return inquriyData.at(persistKey);

	}

	void OnMessage(Inquiry<Bond> &b) {

		auto persistKey = b.GetProduct().GetProductId();

		inquriyData[persistKey] = b;

		for (auto& lp : listeners) lp->ProcessAdd(b);

	}



	void AddListener(ServiceListener<Inquiry<Bond> > *listener) {

		listeners.push_back(listener);

	}

	const vector<ServiceListener<Inquiry<Bond> > *>& GetListeners() const {

		return listeners;

	}

	void PersistData(string persistKey, Inquiry<Bond>& data) {

		bondHistoricalInquiryConnector->Publish(data);

	}



private:

	vector<ServiceListener<Inquiry<Bond> >*> listeners;      

	map<string, Inquiry <Bond> > inquriyData;                     

	BondHistoricalInquiryConnector* bondHistoricalInquiryConnector; 

	BondHistoricalInquiryService() { bondHistoricalInquiryConnector = BondHistoricalInquiryConnector::instance(); }

};


class BondHistoricalInquiryServiceListener : public ServiceListener<Inquiry<Bond>> {

public:

	static BondHistoricalInquiryServiceListener* instance() {

		static BondHistoricalInquiryServiceListener inst;

		return &inst;

	}


	void ProcessAdd(Inquiry<Bond> &data) {

		bondHistoryInquiryService->OnMessage(data);

		bondHistoryInquiryService->PersistData(data.GetProduct().GetProductId(), data); 

	}

	void ProcessRemove(Inquiry<Bond> &data) {}

	void ProcessUpdate(Inquiry<Bond> &data) {}



private:

	BondHistoricalInquiryService * bondHistoryInquiryService;

	BondHistoricalInquiryServiceListener() { bondHistoryInquiryService = BondHistoricalInquiryService::instance(); }

};