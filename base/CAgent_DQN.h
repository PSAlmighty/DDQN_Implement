#ifndef _CMM
#define _CMM

#include <iostream>
#include <string>
#include "zmq.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "pthread.h"
#include "sys/types.h"
#include "assert.h"
#include "CData.h"
#include "string.h"
#include "./include/ThostFtdcUserApiStruct.h"
#include "./include/ThostFtdcTraderApi.h"
#include "./DQN_Agent.pb.h"

//typedef struct ThreadParam
//{
//	CAgent *CMMObject;
//	CThostFtdcDepthMarketDataField *DepthDataRtn;
//}ThreadParam;


class CAgent 
{
public:

	CAgent(string,string);
	~CAgent();

	void Start_ZMQ_Server();

	void SliceThreadPool(CThostFtdcDepthMarketDataField *DepthData);

 	string GetTimer();

	void SetPositionStatus(CThostFtdcTradeField *pTrade);

	//static void NTAPI SliceCallBack(PTP_CALLBACK_INSTANCE pInstance, PVOID pvContext);
	//Kernel data dueling
	static void *SliceCallBack(void *pvContext);

	string InstrumentExchangeID;
	double InstrumentTickSize;
	int InstrumentLeverage;

	int TotalTrades;

	PositionSet Posi;

	pthread_mutex_t csInstrumentUnfillFlagLock;
	pthread_mutex_t csInstrumentUnfillOrderLock;

private:

	void CalculatePnL(CThostFtdcDepthMarketDataField *DepthData);
	void DQN_Action(CThostFtdcDepthMarketDataField *DepthData);
	void CleanPosition(CThostFtdcDepthMarketDataField *DepthData);
	void PendingOrder(CThostFtdcDepthMarketDataField *DepthData, double, int, char, char);

	double Factorial(double fc);

	TThostFtdcTimeType Timer;
	int Hours;
	int Minutes;
	int Seconds;

	double DailyStopLimit;
	bool bStopTrading;
	bool bMktClosing;

	depth_struct DepthAB;

	string InstrumentID;
	string instrument_pipe_name;
	string TradingDATE;
	string TradingTIME;
	string Previous_Action_Timestamp;

	int InitVolume;

	void *ZMQ_Context;
	void *ZMQ_Responder;
	Agent_Info ptAgInfo;

	pthread_mutex_t csInstrumentInternalLock;

	CThostFtdcDepthMarketDataField *SprDepthData;
};




#endif

