#ifndef DATAGENERATING_HPP
#define DATAGENERATING_HPP

#include <iostream>
#include <iomanip>
#include <fstream> 
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "products.hpp"

using namespace std;
using namespace boost::gregorian;


std::vector<std::string> CUSIPS = {

	"9128283H1", "9128283L2", "912828M80",

	"9128283J7", "9128283F5", "912810RZ3" };




void bond_data() {

	std::vector<float> BondCoupon = { static_cast<float>(0), static_cast<float>(0), static_cast<float>(0), static_cast<float>(0), static_cast<float>(0), static_cast<float>(0) };

	std::vector<date> BondMaturity = { date(2019, 11, 30), date(2020, 12, 15), date(2022, 11, 30),

		date(2024, 11, 30), date(2027, 11, 15), date(2047, 11, 15) };

	auto bondBook = BondBook::instance();

	for (int i = 0; i < 6; ++i) {

		Bond bond(CUSIPS[i], CUSIP, "T", BondCoupon[i], BondMaturity[i]);

		bondBook->Add(bond);

	}

}



void trade_data() {

	std::ofstream file;

	file.open("trades.txt", std::ios::out | std::ios::trunc);

	file << "CUSIP,Trade_ID,Book,Price,Quantity,Side\n";

	for (int i = 1; i <= 6; ++i) {

		std::string CUSIP = CUSIPS[i - 1];

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

			file << CUSIP << ",T" << (i - 1) * 10 + j << ",TRSY" << 1 + rand() % 3

				<< "," << str1 + str2 + str3 << "," << (1 + rand() % 9) * 1000000 << ","

				<< (rand() % 2 == 1 ? "BUY" : "SELL") << std::endl;

		}

	}

	file.close();

}

void inquiries_data() {

	std::ofstream file;

	file.open("inquiries.txt", std::ios::out | std::ios::trunc);

	file << "CUSIP, side, quantity, price, state\n";

	for (int i = 0; i < 10; ++i) {

		for (int j = 0; j < 6; ++j) {

			file << CUSIPS[j] + ',' + (rand() % 2 == 0 ? "BUY" : "SELL") + ',' + std::to_string(rand() % 1000 * i) + ',' + "100" + ',' + "RECEIVED" << endl;

		}

	}

}


void market_data() {

	auto Price2String = [](int num) {

		int num1 = num / 256, num2 = num % 256,

			num3 = num2 / 8, num4 = num2 % 8;

		string str1 = std::to_string(99 + num1) + "-",

			str2 = std::to_string(num3), str3 = std::to_string(num4);

		if (num3 < 10) str2 = "0" + str2;

		if (num4 == 4)	str3 = "+";

		return str1 + str2 + str3;

	};


	std::ofstream file;

	file.open("marketdata.txt", std::ios::out | std::ios::trunc);

	file << "CUSIP,bidprice1,quantity,bidprice2,quantity,bidprice3,quantity,bidprice4,quantity,bidprice5,quantity,";

	file << "offerprice1,quantity,offerprice2,quantity,offerprice3,quantity,offerprice4,quantity,offerprice5,quantity,\n";
	
	for (int j = 1; j <= 10; ++j) {

		for (int i = 1; i <= 6; ++i) {

			string cus_ip = CUSIPS[i - 1];

			file << cus_ip << ',';

			int mid_num = rand() % (256 * 2 + 1);

			int bid_num = mid_num - 1;

			for (int k = 1; k <= 5; ++k) {

				int quantity = 1000000 * k;

				file << Price2String(bid_num--) << ',' << quantity << ',';

			}

			int offer_num = mid_num + 1;

			for (int k = 1; k <= 5; ++k) {

				string offer_price = Price2String(offer_num++);

				int quantity = 1000000 * k;

				file << offer_price << ',' << quantity << ',';

			}

			file << endl;

		}

	}

}


void prices_data() {

	auto Price2String = [](int num) {

		int num1 = num / 256, num2 = num % 256,

			num3 = num2 / 8, num4 = num2 % 8;

		string str1 = std::to_string(99 + num1) + "-",

			str2 = std::to_string(num3), str3 = std::to_string(num4);

		if (num3 < 10) str2 = "0" + str2;

		if (num4 == 4)	str3 = "+";

		return str1 + str2 + str3;

	};

	std::ofstream file;

	file.open("prices.txt", std::ios::out | std::ios::trunc);

	file << "CUSIP,mid,bidofferspread\n";

	for (int j = 1; j <= 100; ++j) {

		for (int i = 1; i <= 6; ++i) {

			int mid_num = rand() % (256 * 2 - 8) + 4;

			int tmp = (rand() % 3 + 2);

			std::string osc_str = "0-00" + (tmp == 4 ? "+" : std::to_string(tmp));

			file << CUSIPS[i - 1] << "," << Price2String(mid_num) << ',' << osc_str << endl;

		}

	}

}

#endif
