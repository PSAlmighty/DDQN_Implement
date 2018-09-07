#include "CMdSpi.h"
#include "CTraderSpi.h"
#include <iostream>
#include <fstream>
#include "string.h"

#include "CAgent_DQN.h"

using namespace std;


// USER_API参数
extern CThostFtdcMdApi* pMdApi;
extern CTraderSpi *pTraderSpi;

//全局变量
//extern map<string, CScalper*> mpMomentum;
extern map<string, CAgent*> mpMM;
extern pthread_mutex_t csMMObjectLock;
extern pthread_mutex_t csMainThreadLock;
extern pthread_cond_t hRtnDepMktOutPut;

// 配置参数
extern ThostFtdcParams Citic_Thost_Params;

//extern ThreadParam srThreadParam;
extern char **ppInstrumentID;	
extern int iInstrumentID;

// 请求编号
extern int iRequestID;


void CMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
	cerr << " --->>> "<< "OnRspError" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CMdSpi::OnFrontDisconnected(int nReason)
{
	cerr << " --->>> " << "OnFrontDisconnected" << endl;
	cerr << " --->>> Reason = " << nReason << endl;
}
		
void CMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << " --->>> " << "OnHeartBeatWarning" << endl;
	cerr << " --->>> nTimerLapse = " << nTimeLapse << endl;
}

void CMdSpi::OnFrontConnected()
{
	ReqUserLogin();
}

void CMdSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, Citic_Thost_Params.BROKER_ID);
	strcpy(req.UserID, Citic_Thost_Params.INVESTOR_ID);
	strcpy(req.Password, Citic_Thost_Params.PASSWORD);
	int iResult = pMdApi->ReqUserLogin(&req, ++iRequestID);
	if(iResult==0)
	{
		cerr<<" --->>> Data login BROKER_ID:"<<req.BrokerID<<endl;
		cerr<<" --->>> Data login INVESTOR_ID:"<<req.UserID<<endl;
	}
	else
	{
		cerr<<" --->>> MD Login Failed\n";
	}

}

void CMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << " --->>> " << "OnRspUserLogin" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		// 请求订阅行情
		cerr<<" --->>> Live Data Login "<<endl;
		cerr<<pRspUserLogin->BrokerID<<endl;
		SubscribeMarketData();	
	}
}

void CMdSpi::SubscribeMarketData()
{
	int iResult = pMdApi->SubscribeMarketData(ppInstrumentID, iInstrumentID);
	cerr << " --->>> Subscribe Market Data " << ((iResult == 0) ? "  " : "Failed") << endl;
}

void CMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "OnRspSubMarketData" << endl;
}

void CMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "OnRspUnSubMarketData" << endl;
}

void CMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//unsigned long StartTimer, EndTimer;
	//StartTimer = GetTickCount64();

	//cerr<<pDepthMarketData->LastPrice<<endl;

	CThostFtdcDepthMarketDataField *pDepth = new CThostFtdcDepthMarketDataField;
	strcpy(pDepth->InstrumentID, pDepthMarketData->InstrumentID);
	strcpy(pDepth->TradingDay, pDepthMarketData->TradingDay);
	strcpy(pDepth->UpdateTime, pDepthMarketData->UpdateTime);
	strcpy(pDepth->ExchangeID, pDepthMarketData->ExchangeID);
	pDepth->UpdateMillisec = pDepthMarketData->UpdateMillisec;
	pDepth->BidPrice1 = pDepthMarketData->BidPrice1;
	pDepth->AskPrice1 = pDepthMarketData->AskPrice1;
	pDepth->BidVolume1 = pDepthMarketData->BidVolume1;
	pDepth->AskVolume1 = pDepthMarketData->AskVolume1;
	pDepth->LastPrice= pDepthMarketData->LastPrice;
	pDepth->Volume = pDepthMarketData->Volume;

	pthread_mutex_lock(&csMainThreadLock);

	//cerr<<pDepth->InstrumentID<<endl;
	//cerr<<pDepthMarketData->InstrumentID<<endl;
	//cerr<<mpMM[pDepth->InstrumentID]->InstrumentTickSize<<endl;
	//cerr<<mpMM[pDepth->InstrumentID]->InstrumentLeverage<<endl;
	if (mpMM[pDepth->InstrumentID]->InstrumentTickSize != 0)
	{
		//cerr<<"Add to Worker\n";
		mpMM[pDepth->InstrumentID]->SliceThreadPool(pDepth);
	}
	pthread_cond_signal(&hRtnDepMktOutPut);
	pthread_mutex_unlock(&csMainThreadLock);

	//delete pDepth;
	//EndTimer = GetTickCount64();
	//cerr << " time used " << EndTimer - StartTimer << endl;
	//SetEvent(hRtnDepMktOutPut);
}

bool CMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cerr << " --->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	return bResult;
}



