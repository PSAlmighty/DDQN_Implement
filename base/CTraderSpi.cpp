#ifndef _CTRADER
#define _CTRADER
#endif


#include <iostream>
#include <iomanip>
#include <queue>
#include "CAgent_DQN.h"
#include "CTraderSpi.h"
#include "CThreadPool.h"
#include "string.h"
#include "pthread.h"


using namespace std;

// USER_API参数

extern CThostFtdcTraderApi* pTraderApi;
extern CThread_Pool *_ThreadPool;

// 配置参数

extern char T_FRONT_ADDR[];		// 前置地址

extern ThostFtdcParams Citic_Thost_Params;

extern TThostFtdcPriceType	LIMIT_PRICE;	// 价格
extern TThostFtdcDirectionType	DIRECTION;	// 买卖方向 

extern pthread_mutex_t csOperatingOrderSysID;
extern pthread_mutex_t csMainThreadLock;
//extern map<string, pthread_mutex_t> mp_csOrderDoneFlag;

extern char **ppInstrumentID;	// 合约代码
extern int iInstrumentID;

extern string TradingDate;

extern int PULL_ORDER_LIMIT;

extern vector<int> vcOrderRef;
extern vector<string> vcSysIDRef;

// 请求编号
extern int iRequestID;

extern map<string, CAgent*> mpMM;
extern pthread_mutex_t csMMObjectLock;
extern pthread_mutex_t csInsert;

extern pthread_cond_t hTradingThreadDone;
extern pthread_cond_t hInstrumentSetupDone;


// 流控判断
bool IsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}

void CTraderSpi::OnFrontConnected()
{
	cerr << " --->>> " << "OnFrontConnected" << endl;
	///用户登录请求
	ReqUserLogin();
	//cerr<<Citic_Thost_Params.BROKER_ID<<endl;
	//cerr<<Citic_Thost_Params.INVESTOR_ID<<endl;
	//cerr<<Citic_Thost_Params.PASSWORD<<endl;
}

void CTraderSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,Citic_Thost_Params.BROKER_ID);
	cerr<<" --->>> Trader login BROKER_ID:"<<req.BrokerID<<endl;
	strcpy(req.UserID, Citic_Thost_Params.INVESTOR_ID);
	cerr<<" --->>> Trader login INVESTOR_ID:"<<req.UserID<<endl;
	strcpy(req.Password, Citic_Thost_Params.PASSWORD);
	int iResult = pTraderApi->ReqUserLogin(&req, ++iRequestID);
	cerr << " --->>> Request Trader Login " << ((iResult == 0) ? "  " : " Failed") << endl;
}

void CTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << " --->>> " << "OnRspUserLogin" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		// 保存会话参数
		Citic_Thost_Params.FRONT_ID = pRspUserLogin->FrontID;
		Citic_Thost_Params.SESSION_ID = pRspUserLogin->SessionID;
		sprintf(Citic_Thost_Params.ORDER_REF, "%d", atoi(pRspUserLogin->MaxOrderRef));
		///获取当前交易日
		cerr << " --->>> Current Date = " << pTraderApi->GetTradingDay() << endl;

		string *DelPath=new string[iInstrumentID];
		for (int i = 0; i < iInstrumentID; i++)
		{
			string ID = ppInstrumentID[i];
			DelPath[i] = "rm -f Detail_" + ID + "_" + pTraderApi->GetTradingDay() + ".csv";
			system(DelPath[i].c_str());
		}
		TradingDate = pTraderApi->GetTradingDay();

		pthread_mutex_lock(&csMainThreadLock);
		pthread_cond_signal(&hTradingThreadDone);
		pthread_mutex_unlock(&csMainThreadLock);
		///投资者结算结果确认
		ReqSettlementInfoConfirm();
	}
}

void CTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, Citic_Thost_Params.BROKER_ID);
	strcpy(req.InvestorID, Citic_Thost_Params.INVESTOR_ID);
	int iResult = pTraderApi->ReqSettlementInfoConfirm(&req, ++iRequestID);
	cerr << " --->>> Settlement Info Confirm " << ((iResult == 0) ? "  " : " Failed") << endl;
}

void CTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << " --->>> " << "OnRspSettlementInfoConfirm" << endl;
    //cerr << " --->>> InvestorID " << pSettlementInfoConfirm->InvestorID << endl;
	cerr << "-----------------------------------------"<<endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询合约
		ReqQryInstrument();
	}
}

void CTraderSpi::ReqQryInstrument()
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));

	pthread_mutex_lock(&csMainThreadLock);
	pthread_cond_wait(&hInstrumentSetupDone,&csMainThreadLock);

	//int i = 0;
	while (true)
	{
		int iResult = pTraderApi->ReqQryInstrument(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			cerr << " --->>> ReqQryInstrument " << ((iResult == 0) ? "  " : " Failed") << endl;
			break;
		}
		else
		{
			cerr << " --->>> ReqQryInstrument " << ", Flow Control" << endl;
			usleep(1500000);
		}
	}

	pthread_mutex_unlock(&csMainThreadLock);
}

void CTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	string tempID = pInstrument->InstrumentID;
	ofstream InstrumentInfo("ContractList", ios::app);
	for (int i = 0; i < iInstrumentID; i++)
	{
		if (strcmp(pInstrument->InstrumentID, ppInstrumentID[i]) == 0)
		{
			//InstrumentInfo << " --->>> Trading IntrumentID：" << pInstrument->InstrumentID << endl;
			//InstrumentInfo << " --->>> Trading Intrument Name：" << pInstrument->InstrumentName << endl;
			//InstrumentInfo << " --->>> Intrument Tick Size：" << pInstrument->PriceTick << endl;
			//InstrumentInfo << " --->>> Instrument Leverage：" << pInstrument->VolumeMultiple << endl;
			//InstrumentInfo << " --->>> ExchangeID：" << pInstrument->ExchangeID << endl;
			//InstrumentInfo << "-----------------------------------------" << endl;
			pthread_mutex_lock(&csMMObjectLock);

			mpMM[tempID]->InstrumentTickSize = pInstrument->PriceTick;
			mpMM[tempID]->InstrumentLeverage = pInstrument->VolumeMultiple;
			mpMM[tempID]->InstrumentExchangeID = pInstrument->ExchangeID;

			pthread_mutex_unlock(&csMMObjectLock);
		}
		if (InstrumentInfo.is_open())
		{
			InstrumentInfo << " --->>> Trading IntrumentID：" << pInstrument->InstrumentID << endl;
			InstrumentInfo << " --->>> Trading Intrument Name：" << pInstrument->InstrumentName << endl;
			InstrumentInfo << " --->>> Intrument Tick Size：" << pInstrument->PriceTick << endl;
			InstrumentInfo << " --->>> Instrument Leverage：" << pInstrument->VolumeMultiple << endl;
			InstrumentInfo << " --->>> ExchangeID：" << pInstrument->ExchangeID << endl;
			InstrumentInfo << "-----------------------------------------" << endl;
		}
	}
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询账户
		ReqQryTradingAccount();
		InstrumentInfo.close();
	}
}

void CTraderSpi::ReqQryTradingAccount()
{
	
	CThostFtdcQryTradingAccountField req;
	CThostFtdcTradingAccountField TA;
	memset(&req, 0, sizeof(req));
	memset(&TA, 0, sizeof(TA));
	strcpy(req.BrokerID, Citic_Thost_Params.BROKER_ID);
	strcpy(req.InvestorID, Citic_Thost_Params.INVESTOR_ID);
	while (true)
	{
		int iResult = pTraderApi->ReqQryTradingAccount(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			cerr << " --->>> Request Query Account " << ((iResult == 0) ? "  " : " Failed") << endl;
			break;
		}
		else
		{
			cerr << " --->>> Request Query Account " << iResult << ", Flow Control" << endl;
			usleep(1000000);
		}
	} // while
}

void CTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	double preBalance=pTradingAccount->PreBalance-pTradingAccount->Withdraw+pTradingAccount->Deposit;
	double current=preBalance+pTradingAccount->CloseProfit+pTradingAccount->PositionProfit-pTradingAccount->Commission;
	cerr << " --->>> " << "OnRspQryTradingAccount" << endl;
	cerr << " --->>> Static  Balance： " <<setprecision(2)<<setiosflags(ios::fixed)<< preBalance << endl;
	cerr << " --->>> Current Balance： " <<setprecision(2)<<setiosflags(ios::fixed)<< current << endl;
	cerr << "-----------------------------------------"<<endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询投资者持仓
		ReqQryInvestorPosition();
	}
}

void CTraderSpi::ReqQryInvestorPosition()
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, Citic_Thost_Params.BROKER_ID);
	strcpy(req.InvestorID, Citic_Thost_Params.INVESTOR_ID);
	//strcpy(req.InstrumentID, *ppInstrumentID);
	while (true)
	{
		int iResult = pTraderApi->ReqQryInvestorPosition(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			cerr << " --->>> Request Query Position " << ((iResult == 0) ? "  " : " Failed") << endl;
			break;
		}
		else
		{
			cerr << " --->>> Request Query Position " << iResult << ", Flow Control" << endl;
			usleep(1000000);
		}
	} // while
}

void CTraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

	cerr << " --->>> " << "OnRspQryInvestorPosition" << endl;
	ofstream PosiInfo("InvestorPosition", ios::app);
	if(pInvestorPosition!=NULL)
	{
		if (PosiInfo.is_open())
		{
			PosiInfo << " --->>> Instrument       ： " << pInvestorPosition->InstrumentID << endl;
			PosiInfo << " --->>> Realized   Profit： " << pInvestorPosition->CloseProfit << endl;
			PosiInfo << " --->>> Unrealized Profit： " << pInvestorPosition->PositionProfit << endl;
			PosiInfo << " --->>> Net Position： " << pInvestorPosition->Position << endl;
			//entry cost
			//cerr << " --->>> OpenCost： " << pInvestorPosition->OpenCost << endl;
			//holding posiion value
			//cerr << " --->>> PositionCost： " << pInvestorPosition->PositionCost << endl;
			//cerr << " --->>> OpenAmount： " << pInvestorPosition->OpenAmount << endl;
			//cerr << " --->>> OpenVolume： " << pInvestorPosition->OpenVolume << endl;
			pthread_mutex_lock(&csMMObjectLock);
			string ID = pInvestorPosition->InstrumentID;
			if (pInvestorPosition->PosiDirection == '2')
			{
				PosiInfo << " --->>> Direciton：Long " << endl;
				double cost = pInvestorPosition->OpenCost;
			}
			else if (pInvestorPosition->PosiDirection == '3')
			{
				PosiInfo << " --->>> Direciton：Short " << endl;
				string ID = pInvestorPosition->InstrumentID;
				double cost = pInvestorPosition->OpenCost;
			}
			else
				PosiInfo << " --->>> No Position " << endl;
		}
		pthread_mutex_unlock(&csMMObjectLock);
		PosiInfo << "-----------------------------------------"<<endl;
	}
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{

	}
			
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//订单处理部分
//日后需要设计一个类去集成这部分操作
//然后策略类继承这个订单处理类进行交易
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr << " --->>> " << "OnRspOrderInsert" << endl;
	IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr << " --->>> " << "OnRspOrderAction" << endl;
	IsErrorRspInfo(pRspInfo);
}

///报单通知
void CTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	//cerr << " --->>> OnRtnOrder"  << endl;
	//system("PAUSE");
	CThostFtdcOrderField *pParam = new CThostFtdcOrderField;
	
	//copy pOrder
	strcpy(pParam->InsertTime, pOrder->InsertTime);
	strcpy(pParam->OrderRef, pOrder->OrderRef);
	strcpy(pParam->OrderSysID, pOrder->OrderSysID);
	strcpy(pParam->InstrumentID, pOrder->InstrumentID);
	strcpy(pParam->CombOffsetFlag, pOrder->CombOffsetFlag);
	strcpy(pParam->BrokerID, pOrder->BrokerID);
	strcpy(pParam->InvestorID, pOrder->InvestorID);
	strcpy(pParam->InstrumentID, pOrder->InstrumentID);
	strcpy(pParam->ExchangeID, pOrder->ExchangeID);

	pParam->Direction = pOrder->Direction;
	pParam->LimitPrice = pOrder->LimitPrice;
	pParam->VolumeTotalOriginal = pOrder->VolumeTotalOriginal;
	pParam->VolumeTotal = pOrder->VolumeTotal;
	pParam->VolumeTraded = pOrder->VolumeTraded;
	pParam->VolumeCondition = pOrder->VolumeCondition;
	pParam->OrderStatus = pOrder->OrderStatus;


	if (IsMyOrder(pOrder))
	{
		AddToThreadPool(pParam);
		//string RESULT;
	}
}

///成交通知
void CTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	//cerr << " --->>> " << "OnRtnTrade"  << endl;
	if (IsMyTrades(pTrade))
	{
		pthread_mutex_lock(&csMMObjectLock);
		mpMM[pTrade->InstrumentID]->SetPositionStatus(pTrade);
		pthread_mutex_unlock(&csMMObjectLock);
	}
}

//判断是否自己的单子
bool CTraderSpi::IsMyOrder(CThostFtdcOrderField *pOrder)
{
	pthread_mutex_lock(&csInsert);

	bool REF = false;
	vector<int>::iterator it = find(vcOrderRef.begin(), vcOrderRef.end(), atoi(pOrder->OrderRef));
	if (it != vcOrderRef.end())
		REF = true;

	pthread_mutex_unlock(&csInsert);

	return ((pOrder->FrontID == Citic_Thost_Params.FRONT_ID) &&
			(pOrder->SessionID == Citic_Thost_Params.SESSION_ID) &&
			REF);
}

//判断是否自己的交易
bool CTraderSpi::IsMyTrades(CThostFtdcTradeField *pTrade)
{
	pthread_mutex_lock(&csOperatingOrderSysID);

	bool REF;
	vector<string>::iterator it = find(vcSysIDRef.begin(), vcSysIDRef.end(), pTrade->OrderSysID);
	if (it != vcSysIDRef.end())
		REF = true;
	else
		REF = false;

	pthread_mutex_unlock(&csOperatingOrderSysID);

	return REF;
}

//判断单子是否仍在交易状态，如何情况均当作处于交易状态1、部分成交仍在队列；2、部分成交不在队列；3、没有成交
bool CTraderSpi::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus == THOST_FTDC_OST_PartTradedQueueing)          //部分成交仍在队列
			||(pOrder->OrderStatus == THOST_FTDC_OST_PartTradedNotQueueing)     //部分成交不在队列
			||(pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)			//完全未成交仍在队列
			||(pOrder->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing));      //完全未成交不在队列
}

void CTraderSpi::OnFrontDisconnected(int nReason)
{
	cerr << " --->>> " << "OnFrontDisconnected" << endl;
	cerr << " --->>> Reason = " << nReason << endl;
}

void CTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << " --->>> " << "OnHeartBeatWarning" << endl;
	cerr << " --->>> nTimerLapse = " << nTimeLapse << endl;
}

void CTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << " --->>> " << "OnRspError" << endl;
	IsErrorRspInfo(pRspInfo);
}

bool CTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
	{
		string sz_path = "ErrorLog_" + TradingDate + ".txt";
		ofstream ErrorLog(sz_path.c_str(), ios::app);
		if (ErrorLog.is_open())
			ErrorLog << " --->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
		ErrorLog.close();
	}
	return bResult;
}

void CTraderSpi::AddToThreadPool(CThostFtdcOrderField *pOrder)
{
	Adding_Worker(_ThreadPool,OrderRtnCallBack,pOrder);
}

void *CTraderSpi::OrderRtnCallBack(void *pvContext)
{
	CThostFtdcOrderField *pParam = (CThostFtdcOrderField*) pvContext;
	string RESULT;
	string TradingDate = pParam->InsertDate;

	//cerr << "OrderRtnCallBack\n";

	pthread_mutex_lock(&csMMObjectLock);
	//////////////////////////////////////////////////////////
	if (pParam->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
	{
		//pthread_mutex_lock(&mp_csOrderDoneFlag[pParam->InstrumentID]);
		mpMM[pParam->InstrumentID]->SetCancelField(pParam, pParam->VolumeTotal);

		RESULT = "Order Inserted";
		//cerr << RESULT << endl;

	}
	else if (pParam->OrderStatus == THOST_FTDC_OST_PartTradedQueueing)
	{
		mpMM[pParam->InstrumentID]->SetUnfilledLots(pParam->OrderSysID, pParam->VolumeTotal, pParam->LimitPrice);

		RESULT = "Order Partly Filled";
		//cerr << RESULT << endl;

	}
	else if (pParam->OrderStatus == THOST_FTDC_OST_Canceled)
	{
		mpMM[pParam->InstrumentID]->ResetOrderDoneFlag(pParam);
		mpMM[pParam->InstrumentID]->DeleteLimitOrderCancel(pParam->LimitPrice, pParam->OrderSysID, pParam->Direction, pParam->CombOffsetFlag[0], pParam->VolumeTotal);

		//Operating vcOrderRef
		pthread_mutex_lock(&csInsert);
		vector<int>::iterator OrderRefIt = find(vcOrderRef.begin(), vcOrderRef.end(), atoi(pParam->OrderRef));
		if (OrderRefIt != vcOrderRef.end())
			vcOrderRef.erase(OrderRefIt);
		pthread_mutex_unlock(&csInsert);

		RESULT = "Order Canceled";
		//cerr << RESULT << endl;

	}
	else if (pParam->OrderStatus == THOST_FTDC_OST_AllTraded)
	{
		mpMM[pParam->InstrumentID]->ResetOrderDoneFlag(pParam);
		mpMM[pParam->InstrumentID]->DeleteLimitOrderFilled(pParam->LimitPrice, pParam->OrderSysID, pParam->Direction, pParam->VolumeTotal);

		//Operating vcOrderRef
		pthread_mutex_lock(&csInsert);
		vector<int>::iterator OrderRefIt = find(vcOrderRef.begin(), vcOrderRef.end(), atoi(pParam->OrderRef));
		if (OrderRefIt != vcOrderRef.end())
			vcOrderRef.erase(OrderRefIt);
		pthread_mutex_unlock(&csInsert);

		RESULT = "Order All Filled";
		//cerr << RESULT << endl;

	}
	//else if (pParam->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing)
	//{
	//	mpMM[pParam->InstrumentID]->ResetOrderDoneFlag(pParam);
	//	mpMM[pParam->InstrumentID]->DeleteLimitOrderCancel(pParam->LimitPrice, pParam->OrderSysID, pParam->Direction, pParam->CombOffsetFlag[0], pParam->VolumeCondition);

	//	//Operating vcOrderRef
	//	pthread_mutex_lock(&csInsert);
	//	vector<int>::iterator OrderRefIt = find(vcOrderRef.begin(), vcOrderRef.end(), atoi(pParam->OrderRef));
	//	if (OrderRefIt != vcOrderRef.end())
	//		vcOrderRef.erase(OrderRefIt);
	//	pthread_mutex_unlock(&csInsert);
	//
	//	RESULT = "Order Error";
	//}
	//////////////////////////////////////////////////////////
	pthread_mutex_unlock(&csMMObjectLock);

	pthread_mutex_lock(&csOperatingOrderSysID);
	//////////////////////////////////////////////////////////
	string TempSysID = pParam->OrderSysID;
	vcSysIDRef.push_back(TempSysID);
	//////////////////////////////////////////////////////////
	pthread_mutex_unlock(&csOperatingOrderSysID);

	//delete pParam;

	return NULL;
}
