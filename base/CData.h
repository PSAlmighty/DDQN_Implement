#ifndef _CData
#define _CData

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "./include/ThostFtdcUserApiStruct.h"


using namespace std;

typedef struct _PositionSet
{
	string InstrumentID;
	::map<double, int> mp_PriceMap;
	::map<double, int> mp_PositionDistribution;
	double TotalProfit;
	double TotalValue;
	double CostLine;
	double AveragePositionEntry;
	int NetPosition;
	int Direction_Flag;
}PositionSet;

typedef struct _depth_struct
{
	double bid_price;
	double ask_price;
	bool operator<(const _depth_struct &depth)const
	{
		if (bid_price < depth.bid_price)
			return true;
		else
			return false;
	}
	bool operator>(const _depth_struct &depth)const
	{
		if (ask_price > depth.ask_price)
			return true;
		else
			return false;
	}
	bool operator==(const _depth_struct &depth)const
	{
		if (ask_price == depth.ask_price && bid_price == depth.bid_price)
			return true;
		else
			return false;
	}
	bool operator!=(const _depth_struct &depth)const
	{
		if (ask_price != depth.ask_price || bid_price != depth.bid_price)
			return true;
		else
			return false;
	}
	void operator=(const _depth_struct &depth)
	{
		bid_price = depth.bid_price;
		ask_price = depth.ask_price;
	}
	double operator-(const _depth_struct &depth)
	{
		double bid_gap, ask_gap;
		bid_gap = bid_price - depth.bid_price;
		ask_gap = ask_price - depth.ask_price;
		return (bid_gap + ask_gap) / 2;
	}
}depth_struct;

typedef struct _depth_detail
{
	double InitABR;
	double LastAccuDelta;

	int LastMvt;
	int LastDirSym;
	//Last CrossTime
	int iLCT;
	int iLstRuns;
	int Lst_Max_Delta_Runs;
	double dLABS_AccuDelta, dLMax_AccuDelta;


}depth_detail;

typedef struct _Data_Block
{
	double open, high, low, close;
	double TickCounts;
	double last_bid, last_ask;
	int HorizonCounts;

	bool bEnableTrading;
	bool bEnableStop;
	bool bHighVolatility;

	double dTickCounts_mean;
	double dTickCounts_stdev;

	::vector<double> vc_Ticks;
	string ID;
}Data_Block;

struct ThostFtdcParams{
	TThostFtdcFrontIDType	 FRONT_ID;	    
	TThostFtdcSessionIDType	 SESSION_ID;	
	TThostFtdcOrderRefType	 ORDER_REF;   
	TThostFtdcBrokerIDType	 BROKER_ID;	  
	TThostFtdcInvestorIDType INVESTOR_ID;	 
	TThostFtdcPasswordType   PASSWORD;	

	char *M_FRONT_ADDR;
	char *T_FRONT_ADDR;		
};
#endif
