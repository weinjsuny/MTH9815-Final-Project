// BondTradingSystem.cpp : Defines the entry point for the console application.
//

#include "historicaldataservice.hpp"

int main()
{
	std::vector<std::string> CUSIPS = {

		"9128283H1", "9128283L2", "912828M80",

		"9128283J7", "9128283F5", "912810RZ3" };

	std::vector<float> BondCoupon = { static_cast<float>(0), static_cast<float>(0), static_cast<float>(0), static_cast<float>(0), static_cast<float>(0), static_cast<float>(0) };

	std::vector<date> BondMaturity = { date(2019, 11, 30), date(2020, 12, 15), date(2022, 11, 30),

		date(2024, 11, 30), date(2027, 11, 15), date(2047, 11, 15) };

	std::vector<float> BondYield = { static_cast<float>(1.765 / 100.), static_cast<float>(1.932 / 100.), 
		
		static_cast<float>(2.066 / 100.), static_cast<float>(2.230 / 100.),

		static_cast<float>(2.384 / 100.), static_cast<float>(2.801 / 100.) };


	auto bondRiskServiceListener = BondRiskServiceListener::instance();

	//auto bondTradeBookingServiceConnector = BondTradeBookingConnector::instance();

	auto bondPositionServiceListener = BondPositionServiceListener::instance();

	auto bondRiskService = bondRiskServiceListener->GetService();

	auto bondPositionService = bondPositionServiceListener->GetService();

	//auto bondTradeBookingService = bondTradeBookingServiceConnector->GetService();

	auto bondInquiryServiceConnector = BondInquiryConnector::instance();

	auto bondInquiryService = bondInquiryServiceConnector->GetService();

	auto bondMarketDataServiceConnector = BondMarketDataConnector::instance();

	auto bondMarketDataService = bondMarketDataServiceConnector->GetService();

	auto bondAlgoExecutionServiceListener = BondAlgoExecutionServiceListener::instance();

	auto bondAlgoExecutionService = bondAlgoExecutionServiceListener->GetService();

	auto bondExecutionServiceListener = BondExecutionServiceListener::instance();

	auto bondExecutionService = bondExecutionServiceListener->GetService();

	auto bondTradeBookingServiceListener = BondTradeBookingServiceListener::instance();

	auto bondTradeBookingService = bondTradeBookingServiceListener->GetService();

	auto bondPricingServiceConnector = BondPricingServiceConnector::instance();

	auto bondPricingService = bondPricingServiceConnector->GetService();

	auto bondAlgoStreamingServiceListener = BondAlgoStreamingServiceListener::instance();

	auto bondAlgoStreamingService = bondAlgoStreamingServiceListener->GetService();

	auto bondStreamingServiceListener = BondStreamingServiceListener::instance();

	auto bondStreamingService = bondStreamingServiceListener->GetService();

	auto bondGUIServiceListener = BondGUIServiceListener::instance();

	auto bondGUIService = bondGUIServiceListener->GetService();

	auto bondHistoricalPV01ServiceListener = BondHistoricalPV01ServiceListener::instance();

	auto bondHistoricalExecutionServiceListener = BondHistoricalExecutionServiceListener::instance();

	auto bondHistoricalStreamingServiceListener = BondHistoricalStreamingServiceListener::instance();

	auto bondHistoricalInquriyServiceListener = BondHistoricalInquiryServiceListener::instance();

	bondInquiryService->AddListener(bondHistoricalInquriyServiceListener);

	bondStreamingService->AddListener(bondHistoricalStreamingServiceListener);

	bondExecutionService->AddListener(bondHistoricalExecutionServiceListener);

	bondRiskService->AddListener(bondHistoricalPV01ServiceListener);

	bondAlgoStreamingService->AddListener(bondStreamingServiceListener);

	bondPricingService->AddListener(bondAlgoStreamingServiceListener);

	bondMarketDataService->AddListener(bondAlgoExecutionServiceListener);

	bondAlgoExecutionService->AddListener(bondExecutionServiceListener);

	bondTradeBookingService->AddListener(bondPositionServiceListener);

	bondPositionService->AddListener(bondRiskServiceListener);

	bondExecutionService->AddListener(bondTradeBookingServiceListener);

	bondPricingService->AddListener(bondGUIServiceListener);

	for (int i = 0; i < 6; ++i) {

		Bond bond(CUSIPS[i], CUSIP, "T", BondCoupon[i], BondMaturity[i]);

		Position<Bond> position(bond);

		PV01<Bond> pv01(bond, BondYield[i], position.GetAggregatePosition());

		bondRiskService->Add(pv01);

	}
	
	std::vector<Bond> bonds = { Bond(CUSIPS[0], CUSIP, "T", BondCoupon[0], BondMaturity[0]),  Bond(CUSIPS[1], CUSIP, "T", BondCoupon[1], BondMaturity[1]) };
	BucketedSector<Bond> sector(bonds, "sector1");
	bondRiskService->AddBusketedSector(sector);

	bonds = { Bond(CUSIPS[2], CUSIP, "T", BondCoupon[2], BondMaturity[2]),  Bond(CUSIPS[3], CUSIP, "T", BondCoupon[3], BondMaturity[3]), Bond(CUSIPS[4], CUSIP, "T", BondCoupon[4], BondMaturity[4]) };
	sector = BucketedSector<Bond>(bonds, "sector2");
	bondRiskService->AddBusketedSector(sector);

	bonds = { Bond(CUSIPS[5], CUSIP, "T", BondCoupon[5], BondMaturity[5]) };
	sector = BucketedSector<Bond>(bonds, "sector3");
	bondRiskService->AddBusketedSector(sector);

	trade_data();

	bond_data();

	inquiries_data();

	market_data();

	prices_data();

	//bondTradeBookingServiceConnector->Subscribe();


	bondInquiryServiceConnector->Subscribe();

	bondMarketDataServiceConnector->Subscribe();

	bondPricingServiceConnector->Subscribe();


    
	return 0;
}

