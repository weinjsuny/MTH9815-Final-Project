/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "products.hpp"

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:

  // ctor for a price
  Price() {};
  Price(const T &_product, double _mid, double _bidOfferSpread);

  // Get the product
  const T& GetProduct() const;

  // Get the mid price
  double GetMid() const;

  // Get the bid/offer spread around the mid
  double GetBidOfferSpread() const;

private:
  const T& product;
  double mid;
  double bidOfferSpread;

};

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string,Price <T> >
{
};

template<typename T>
Price<T>::Price(const T &_product, double _mid, double _bidOfferSpread) :
  product(_product)
{
  mid = _mid;
  bidOfferSpread = _bidOfferSpread;
}

template<typename T>
const T& Price<T>::GetProduct() const
{
  return product;
}

template<typename T>
double Price<T>::GetMid() const
{
  return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
  return bidOfferSpread;
}


class BondPricingService : public Service<string, Price<Bond>>

{

public:

	static BondPricingService* instance() {

		static BondPricingService instance;

		return &instance;

	}

	void OnMessage(Price<Bond> &p)
	{
		auto cusip = p.GetProduct().GetProductId();

		PriceData.insert(std::make_pair(cusip, p));

		for (auto& listener : listeners) 	listener->ProcessAdd(p);

	}

	Price<Bond>& GetData(std::string _cusip) {

		return PriceData.at(_cusip);

	}

	void AddListener(ServiceListener<Price<Bond>> *listener) {

		listeners.push_back(listener);

	}

	const vector< ServiceListener<Price<Bond>>* >& GetListeners() const {

		return listeners;

	}



private:

	std::map<std::string, Price<Bond> > PriceData;

	std::vector<ServiceListener<Price<Bond> >*> listeners; 

	BondPricingService() {}

};



class BondPricingServiceConnector : public Connector<Price<Bond>> {

public:

	static BondPricingServiceConnector* instance() {

		static BondPricingServiceConnector inst;

		return &inst;

	}

	void Publish(Price<Bond> &data) {}


	void Subscribe ()
	{

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



		ifstream file("prices.txt");

		string line;

		getline(file, line);

		string cusip, mid, bidofferspread;

		while (getline(file, line)) {

			std::vector<std::string> elems = SplitLine(line);

			cusip = elems[0]; mid = elems[1]; bidofferspread = elems[2];

			double mid_price = String2Price(mid);

			double spread = String2Price(bidofferspread);

			auto bond = bondBook->GetData(cusip);

			Price<Bond> price(bond, mid_price, spread);

			bondPricingService->OnMessage(price);

		}

		std::cout << "The pricing service finished subscribing.\n" << std::endl;

	}

	BondPricingService* GetService() {

		return bondPricingService;

	}



private:

	BondPricingServiceConnector()

	{

		bondPricingService = BondPricingService::instance();

		bondBook = BondBook::instance();

	}



	BondPricingService * bondPricingService;;

	BondBook * bondBook;

};



#endif
