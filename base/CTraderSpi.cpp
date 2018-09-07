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

// USER_API����

extern CThostFtdcTraderApi* pTraderApi;
extern CThread_Pool *_ThreadPool;

// ���ò���

extern char T_FRONT_ADDR[];		// ǰ�õ�ַ

extern ThostFtdcParams Citic_Thost_Params;

extern TThostFtdcPriceType	LIMIT_PRICE;	// �۸�
extern TThostFtdcDirectionType	DIRECTION;	// �������� 

extern pthread_mutex_t csOperatingOrderSysID;
extern pthread_mutex_t csMainThreadLock;
//extern map<string, pthread_mutex_t> mp_csOrderDoneFlag;

extern char **ppInstrumentID;	// ��Լ����
extern int iInstrumentID;

extern string TradingDate;

extern int PULL_ORDER_LIMIT;

extern vector<int> vcOrderRef;
extern vector<string> vcSysIDRef;

// ������
extern int iRequestID;

extern map<string, CAgent*> mpMM;
extern pthread_mutex_t csMMObjectLock;
extern pthread_mutex_t csInsert;

extern pthread_cond_t hTradingThreadDone;
extern pthread_cond_t hInstrumentSetupDone;


// �����ж�
bool IsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}

void CTraderSpi::OnFrontConnected()
{
	cerr << " --->>> " << "OnFrontConnected" << endl;
	///�û���¼����
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
		// ����Ự����
		Citic_Thost_Params.FRONT_ID = pRspUserLogin->FrontID;
		Citic_Thost_Params.SESSION_ID = pRspUserLogin->SessionID;
		sprintf(Citic_Thost_Params.ORDER_REF, "%d", atoi(pRspUserLogin->MaxOrderRef));
		///��ȡ��ǰ������
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
		///Ͷ���߽�����ȷ��
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
		///�����ѯ��Լ
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
			//InstrumentInfo << " --->>> Trading IntrumentID��" << pInstrument->InstrumentID << endl;
			//InstrumentInfo << " --->>> Trading Intrument Name��" << pInstrument->InstrumentName << endl;
			//InstrumentInfo << " --->>> Intrument Tick Size��" << pInstrument->PriceTick << endl;
			//InstrumentInfo << " --->>> Instrument Leverage��" << pInstrument->VolumeMultiple << endl;
			//InstrumentInfo << " --->>> ExchangeID��" << pInstrument->ExchangeID << endl;
			//InstrumentInfo << "-----------------------------------------" << endl;
			pthread_mutex_lock(&csMMObjectLock);

			mpMM[tempID]->InstrumentTickSize = pInstrument->PriceTick;
			mpMM[tempID]->InstrumentLeverage = pInstrument->VolumeMultiple;
			mpMM[tempID]->InstrumentExchangeID = pInstrument->ExchangeID;

			pthread_mutex_unlock(&csMMObjectLock);
		}
		if (InstrumentInfo.is_open())
		{
			InstrumentInfo << " --->>> Trading IntrumentID��" << pInstrument->InstrumentID << endl;
			InstrumentInfo << " --->>> Trading Intrument Name��" << pInstrument->InstrumentName << endl;
			InstrumentInfo << " --->>> Intrument Tick Size��" << pInstrument->PriceTick << endl;
			InstrumentInfo << " --->>> Instrument Leverage��" << pInstrument->VolumeMultiple << endl;
			InstrumentInfo << " --->>> ExchangeID��" << pInstrument->ExchangeID << endl;
			InstrumentInfo << "-----------------------------------------" << endl;
		}
	}
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///�����ѯ�˻�
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
	cerr << " --->>> Static  Balance�� " <<setprecision(2)<<setiosflags(ios::fixed)<< preBalance << endl;
	cerr << " --->>> Current Balance�� " <<setprecision(2)<<setiosflags(ios::fixed)<< current << endl;
	cerr << "-----------------------------------------"<<endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///�����ѯͶ���ֲ߳�
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
			PosiInfo << " --->>> Instrument       �� " << pInvestorPosition->InstrumentID << endl;
			PosiInfo << " --->>> Realized   Profit�� " << pInvestorPosition->CloseProfit << endl;
			PosiInfo << " --->>> Unrealized Profit�� " << pInvestorPosition->PositionProfit << endl;
			PosiInfo << " --->>> Net Position�� " << pInvestorPosition->Position << endl;
			//entry cost
			//cerr << " --->>> OpenCost�� " << pInvestorPosition->OpenCost << endl;
			//holding posiion value
			//cerr << " --->>> PositionCost�� " << pInvestorPosition->PositionCost << endl;
			//cerr << " --->>> OpenAmount�� " << pInvestorPosition->OpenAmount << endl;
			//cerr << " --->>> OpenVolume�� " << pInvestorPosition->OpenVolume << endl;
			pthread_mutex_lock(&csMMObjectLock);
			string ID = pInvestorPosition->InstrumentID;
			if (pInvestorPosition->PosiDirection == '2')
			{
				PosiInfo << " --->>> Direciton��Long " << endl;
				double cost = pInvestorPosition->OpenCost;
			}
			else if (pInvestorPosition->PosiDirection == '3')
			{
				PosiInfo << " --->>> Direciton��Short " << endl;
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
//����������
//�պ���Ҫ���һ����ȥ�����ⲿ�ֲ���
//Ȼ�������̳����������������н���
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

///����֪ͨ
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

///�ɽ�֪ͨ
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

//�ж��Ƿ��Լ��ĵ���
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

//�ж��Ƿ��Լ��Ľ���
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

//�жϵ����Ƿ����ڽ���״̬�����������������ڽ���״̬1�����ֳɽ����ڶ��У�2�����ֳɽ����ڶ��У�3��û�гɽ�
bool CTraderSpi::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus == THOST_FTDC_OST_PartTradedQueueing)          //���ֳɽ����ڶ���
			||(pOrder->OrderStatus == THOST_FTDC_OST_PartTradedNotQueueing)     //���ֳɽ����ڶ���
			||(pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)			//��ȫδ�ɽ����ڶ���
			||(pOrder->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing));      //��ȫδ�ɽ����ڶ���
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
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
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
