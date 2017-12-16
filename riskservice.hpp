/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"

/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{

public:

  // ctor for a PV01 value
	PV01() {};
	PV01(const T &_product, double _pv01, long _quantity);

  // Get the product on this PV01 value
	const T& GetProduct() const;

  // Get the PV01 value
	double GetPV01() const;

  // Get the quantity that this risk value is associated with
	long GetQuantity() const;

	void AddQuantity(long q) {
		
		quantity += q;

	}

private:
	T product;
	double pv01;
	long quantity;

};

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{

public:

  // ctor for a bucket sector
  BucketedSector(const vector<T> &_products, string _name);

  // Get the products associated with this bucket
  const vector<T>& GetProducts() const;

  // Get the name of the bucket
  const string& GetName() const;

private:
  vector<T> products;
  string name;

};

/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */

template<typename T>
class RiskService : public Service<string, PV01 <T> >
{

public:

	// Add a position that the service will risk
	virtual void AddPosition(Position<T> &position) = 0;

	// Get the bucketed risk for the bucket sector
	virtual PV01<BucketedSector<T>> GetBucketedRisk(const BucketedSector<T> &sector) = 0;

};


class BondRiskService : public RiskService<Bond>
{

public:

	static BondRiskService* instance() {

		static BondRiskService inst;

		return &inst;

	}

	void AddBusketedSector(BucketedSector<Bond> &sector) {

		sectorData.push_back(sector);

	}

	void AddPosition(Position<Bond> &position) {

		std::cout << "The risk service is taking position of " << position.GetProduct().GetProductId() << " from position service." << std::endl;

		Bond thisBond = position.GetProduct();

		string product_ID = thisBond.GetProductId();

		long quantity = position.GetAggregatePosition();

		riskData[product_ID].AddQuantity(quantity);
		
		PV01<Bond> pb = riskData[product_ID];

		std::cout << "The risk of the product is " << pb.GetPV01() << ".\n" << std::endl;

		OnMessage(pb);

		/*for (auto iter = sectorData.begin(); iter != sectorData.end(); iter++) {

			BucketedSector<Bond> sector = *iter;

			vector<Bond> products = sector.GetProducts();

			if (std::find(products.begin(), products.end(), thisBond) != products.end()) {

				PV01< BucketedSector<Bond> > pb = GetBucketedRisk(sector);

				OnMessage(pb);
			}

		}*/

	}

	PV01< BucketedSector<Bond> > GetBucketedRisk(const BucketedSector<Bond> &sector) {
		
		double sectorPV01 = 0;

		for (auto&b : sector.GetProducts()) sectorPV01 += riskData.at(b.GetProductId()).GetPV01();

		PV01< BucketedSector<Bond> > result = PV01< BucketedSector<Bond> >(sector, sectorPV01, 1);

		return result;
	}

	PV01<Bond>& GetData(std::string _cusip) {

		return riskData.at(_cusip);

	}


	void AddListener(ServiceListener<PV01<Bond>> *listener) {

		risklisteners.push_back(listener);

	}


	const vector< ServiceListener<PV01<Bond>>* >& GetListeners() const {

		return risklisteners;

	}


	void OnMessage(PV01<Bond> &trade) {

		for (auto& listener : risklisteners) 	listener->ProcessAdd(trade);
	
	}

	void Add(PV01<Bond> &risk) {

		riskData.insert(std::make_pair(risk.GetProduct().GetProductId(), risk));

	}

private:

	std::map<std::string, PV01<Bond>> riskData;

	vector<BucketedSector<Bond>> sectorData;

	std::vector<ServiceListener<PV01<Bond>>*>  risklisteners;

	BondRiskService() {}

};

class BondRiskServiceListener : public ServiceListener<Position<Bond>> {

public:


	static BondRiskServiceListener* instance() {

		static BondRiskServiceListener inst;

		return &inst;

	}


	void ProcessAdd(Position<Bond> &data) {

		bondRiskService->AddPosition(data);

	}



	void ProcessRemove(Position<Bond> &data) {}

	void ProcessUpdate(Position<Bond> &data) {}



	BondRiskService* GetService() {

		return bondRiskService;

	}

private:

	BondRiskService* bondRiskService;

	BondRiskServiceListener() { bondRiskService = BondRiskService::instance(); }

};


template<typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) :
  product(_product)
{
  pv01 = _pv01;
  quantity = _quantity;
}

template<typename T>
const T& PV01<T>::GetProduct() const
{
	return product;
}

template<typename T>
long PV01<T>::GetQuantity() const
{
	return quantity;
}


template<typename T>
double PV01<T>::GetPV01() const
{
	return pv01;
}


template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
  products(_products)
{
  name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
  return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
  return name;
}

#endif
