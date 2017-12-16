/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "tradebookingservice.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

  // ctor for an inquiry
  Inquiry() {};
  Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);

  // Get the inquiry ID
  const string& GetInquiryId() const;

  // Get the product
  const T& GetProduct() const;

  // Get the side on the inquiry
  Side GetSide() const;

  // Get the quantity that the client is inquiring for
  long GetQuantity() const;

  // Get the price that we have responded back with
  double GetPrice() const;

  void Set(double _price, InquiryState _state) {

	  price = _price;

	  state = _state;

  }

  // Get the current state on the inquiry
  InquiryState GetState() const;

private:
  string inquiryId;
  T product;
  Side side;
  long quantity;
  double price;
  InquiryState state;

};

/**
 * Service for customer inquirry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string,Inquiry <T> >
{

public:

  // Send a quote back to the client
  virtual void SendQuote(const string &inquiryId, double price) = 0;

  // Reject an inquiry from the client
  virtual void RejectInquiry(const string &inquiryId) = 0;

};

template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state) :
  product(_product)
{
  inquiryId = _inquiryId;
  side = _side;
  quantity = _quantity;
  price = _price;
  state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
  return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
  return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
  return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
  return price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
  return state;
}


class BondInquiryService : public InquiryService<Bond> {

public:

	static BondInquiryService* instance() {

		static BondInquiryService inst;

		return &inst;

	}

	void SendQuote(const string &inquiryId, double price) {}


	void RejectInquiry(const string &inquiryId) {}



	void OnMessage(Inquiry<Bond> &trade)  {

		trade.Set(trade.GetPrice(), DONE);

		std::cout << "The inquiry service is feeding " << trade.GetProduct().GetProductId() << "  to the inquiry history service." << std::endl;

		for (auto& listener : listeners) 	listener->ProcessAdd(trade);

	}



	Inquiry<Bond>& GetData(std::string _cusip) {

		return inquiryData.at(_cusip);

	}



	void AddListener(ServiceListener<Inquiry<Bond>> *listener)  {

		listeners.push_back(listener);

	}



	const vector< ServiceListener<Inquiry<Bond>>* >& GetListeners() const  {

		return listeners;

	}

private:

	std::map<std::string, Inquiry<Bond>> inquiryData;

	std::vector<ServiceListener<Inquiry<Bond>>*> listeners;

	BondInquiryService() {}

};


class BondInquiryConnector : public Connector<Inquiry<Bond>> {

public:

	static BondInquiryConnector* instance() {

		static BondInquiryConnector inst;

		return &inst;

	}



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



		static int inquiryId = 1; inquiryId++;

		ifstream file("inquiries.txt");

		string line;

		getline(file, line);

		while (getline(file, line))

		{

			std::vector<std::string> elems = SplitLine(line);

			std::string cusip, side, quantity, price, s;

			cusip = elems[0]; side = elems[1]; quantity = elems[2];

			price = elems[3]; s = elems[4];

			InquiryState state;

			if (s == "RECEIVED") state = InquiryState::QUOTED;

			Bond bond = bondBook->GetData(cusip);

			Inquiry<Bond> inq(std::to_string(inquiryId), bond, (side == "BUY" ? Side::BUY : Side::SELL),

				static_cast<long>(std::stod(quantity)), std::stod(price), state);

			bondInquiryservice->OnMessage(inq);

		}

		std::cout << "The inquiry service finished subscribing.\n" << std::endl;

	}



	void Publish(Inquiry<Bond> &data) {}

	InquiryService<Bond>* GetService() {

		return bondInquiryservice;

	}

private:

	BondInquiryConnector() {

		bondInquiryservice = BondInquiryService::instance();

		bondBook = BondBook::instance();

	}

	BondInquiryService * bondInquiryservice;

	BondBook * bondBook;

};

#endif
