#include "CAgent_DQN.h"
#include "CTraderSpi.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include "CThreadPool.h"
#include "pthread.h"
#include "string.h"
#include <sstream> 

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "pthread.h"
#include "sys/types.h"
#include "assert.h"
#include "fstream"

extern CTraderSpi *pTraderSpi;
extern CThostFtdcTraderApi* pTraderApi;
extern CThread_Pool *_ThreadPool;

extern int iRequestID;

extern ThostFtdcParams Citic_Thost_Params;

//¶©µ¥Ïß³Ì¿ØÖÆ×é±äÁ¿
extern vector<int> vcOrderRef;
extern queue<CThostFtdcInputOrderField> qInsertOrderQueue;
extern queue<CThostFtdcInputOrderActionField> qCancelOrderQueue;
extern pthread_mutex_t csInsert;
extern pthread_mutex_t csCancel;
extern pthread_cond_t hInsertEvent;
extern pthread_cond_t hCancelEvent;

extern int PULL_ORDER_LIMIT;
extern bool ORDER_OVER_POSITION;

//void *SliceCallBack(void *pvContext);

CAgent::CAgent(string _ID,string _md_pipe_name,string _ag_pipe_name){
	memset(Timer, 0, sizeof(Timer));

	InstrumentID = _ID;
	instrument_md_pipe_name = _md_pipe_name;
	instrument_ag_pipe_name = _ag_pipe_name;

	Hours = 0;
	Minutes = 0;
	Seconds = 0;

	InitVolume = 0;
	TotalTrades = 0;

	bMktClosing = false;
	bStopTrading = false;
	bMD_PyReady = false;
	bAG_PyReady = false;

	Posi.mp_PriceMap.clear();
	Posi.NetPosition = 0;
	Posi.TotalProfit = 0;
	Posi.TotalValue = 0;
	Posi.Direction_Flag = 0;

	InstrumentTickSize = 0;
	InstrumentLeverage = 0;

	pthread_mutex_init(&csInstrumentInternalLock,NULL);
	pthread_mutex_init(&csInstrumentUnfillFlagLock,NULL);
	pthread_mutex_init(&csInstrumentUnfillOrderLock,NULL);

	Start_MD_ZMQ_Server();
	Start_AG_ZMQ_Server();
}

CAgent::~CAgent(){
	pthread_mutex_destroy(&csInstrumentInternalLock);
	pthread_mutex_destroy(&csInstrumentUnfillFlagLock);
	pthread_mutex_destroy(&csInstrumentUnfillOrderLock);
}

void CAgent::SliceThreadPool(CThostFtdcDepthMarketDataField *DepthData){
	if (InstrumentID == DepthData->InstrumentID){
		this->SprDepthData = DepthData;
		Adding_Worker(_ThreadPool,SliceCallBack,this);
	};
}


void *CAgent::SliceCallBack(void *arg){
	CAgent *pThis = (CAgent*)arg;

	pthread_mutex_lock(&pThis->csInstrumentInternalLock);

	string tempID = pThis->SprDepthData->InstrumentID;

	//record time stamp
	pThis->TradingDATE = pThis->SprDepthData->TradingDay;
	pThis->TradingTIME = pThis->SprDepthData->UpdateTime;

	string TempTimingStr = pThis->SprDepthData->UpdateTime;
	int pos = 0;

	pos = TempTimingStr.find(":");
	pThis->Hours = atoi(TempTimingStr.substr(0, pos).c_str());
	TempTimingStr.erase(0, pos+1);
	pos = TempTimingStr.find(":");
	pThis->Minutes = atoi(TempTimingStr.substr(0, pos).c_str());
	TempTimingStr.erase(0, pos+1);
	pThis->Seconds = atoi(TempTimingStr.c_str());

	if (!pThis->bMktClosing){
		if ((pThis->Hours == 14 && pThis->Minutes == 48 && pThis->Seconds > 30)
			|| (pThis->Hours == 10 && pThis->Minutes == 13 && pThis->Seconds > 30)
			|| (pThis->Hours == 11 && pThis->Minutes == 27 && pThis->Seconds > 30)
			|| (pThis->Hours == 23 && pThis->Minutes == 27 && pThis->Seconds > 30)
			|| (pThis->Hours == 0 && pThis->Minutes == 55 && pThis->Seconds > 30))
			pThis->bMktClosing = true;
	}
	else{
		if ((pThis->Hours == 9 && pThis->Minutes == 25 && pThis->Seconds > 30)
			|| (pThis->Hours == 10 && pThis->Minutes == 32 && pThis->Seconds > 30)
			|| (pThis->Hours == 13 && pThis->Minutes == 32 && pThis->Seconds > 30)
			|| (pThis->Hours == 23 && pThis->Minutes == 32 && pThis->Seconds > 30)){
			pThis->bMktClosing = false;
		}
	}
	
	if (!pThis->bMktClosing
		&& !pThis->bStopTrading){
		//pThis->DQN_Action(pThis->SprDepthData);
		cerr<<"Heartbeat Working\n";
		cerr<<pThis->SprDepthData->UpdateTime<<endl;
		cerr<<pThis->SprDepthData->Volume<<endl;
		cerr<<pThis->SprDepthData->LastPrice<<endl;
		pThis->Broadcast_MD(pThis->SprDepthData);
		pThis->Req_DQN_Action(pThis->SprDepthData);
	}

	pThis->CalculatePnL(pThis->SprDepthData);

	pthread_mutex_unlock(&pThis->csInstrumentInternalLock);
	return NULL;
}

void CAgent::Req_DQN_Action(CThostFtdcDepthMarketDataField * DepthData)
{
	char CloseOffSetFlag;
	if (InstrumentExchangeID == "SHFE")
		CloseOffSetFlag = THOST_FTDC_OF_CloseToday;
	else
		CloseOffSetFlag = THOST_FTDC_OF_Close;


	char buff[200];
	zmq_msg_t ag_recv_msg;
	zmq_msg_t ag_req_msg;
	int ret_recv = zmq_msg_init_size(&ag_recv_msg,200);
	int ret_req = zmq_msg_init_size(&ag_req_msg,200);
	
	memcpy(zmq_msg_data(&ag_req_msg),"ActionRequest",30);
	zmq_send(&ag_req_msg,AG_ZMQ_Subscriber,0);

	int size = zmq_msg_recv(&ag_recv_msg,AG_ZMQ_Subscriber,ZMQ_DONTWAIT);
	memset(buff,0,200*sizeof(char));
	memcpy(buff,zmq_msg_data(&ag_recv_msg),200);
	ptAG_Info.ParseFromArray(buff,200);
	cerr<<"Agent Action: "<<ptAG_Info.agent_action()<<endl;
	memset(buff,0,100*sizeof(char));
}

void CAgent::PendingOrder(CThostFtdcDepthMarketDataField *DepthData, double price, int lots, char offlag, char direction)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));

	//if (offlag == THOST_FTDC_OF_Close
	//	|| offlag == THOST_FTDC_OF_CloseToday){
	//	if (direction == THOST_FTDC_D_Buy){
	//		if (lots <= LongAvailable)
	//			req.CombOffsetFlag[0] = offlag;
	//		else
	//			req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	//	}

	//	if (direction == THOST_FTDC_D_Sell){
	//		if (lots <= ShortAvailable)
	//			req.CombOffsetFlag[0] = offlag;
	//		else
	//			req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	//	}
	//}	

	strcpy(req.BrokerID,Citic_Thost_Params.BROKER_ID);
	strcpy(req.InvestorID, Citic_Thost_Params.INVESTOR_ID);
	strcpy(req.InstrumentID, DepthData->InstrumentID);
	strcpy(req.OrderRef, Citic_Thost_Params.ORDER_REF);
	//	TThostFtdcUserIDType	UserID;
	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;

	req.Direction = direction;

	req.CombOffsetFlag[0] = offlag;

	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	req.LimitPrice = price;
	req.VolumeTotalOriginal = abs(lots);
	//req.TimeCondition = THOST_FTDC_TC_GFD;
	req.TimeCondition = THOST_FTDC_TC_IOC;
	//	TThostFtdcDateType	GTDDate;
	req.VolumeCondition = THOST_FTDC_VC_AV;
	req.MinVolume = 1;
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	//	TThostFtdcPriceType	StopPrice;
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	req.IsAutoSuspend = 0;
	//	TThostFtdcBusinessUnitType	BusinessUnit;
	//	TThostFtdcRequestIDType	RequestID;
	req.UserForceClose = 0;

	req.CombOffsetFlag[0] = offlag;

	pthread_mutex_lock(&csInsert);

	qInsertOrderQueue.push(req);

	pthread_cond_signal(&hInsertEvent);
	pthread_mutex_unlock(&csInsert);

	//SetEvent(hInsertEvent);
}

void CAgent::SetPositionStatus(CThostFtdcTradeField * pTrade){
	//Output
	string OffSetFlag, Direction, Date, OrderSysID;
	Date = pTrade->TradeDate;
	string sz_path = "./TradeLog_" + Posi.InstrumentID + "_" + Date + ".csv";
	ofstream TradeLog(sz_path.c_str(), ios::app);

	int tempLots = 0;
	int tempTotalLots = 0;
	Posi.InstrumentID = pTrade->InstrumentID;
	OrderSysID = pTrade->OrderSysID;
	if (pTrade->Direction == '0'){
		tempLots = pTrade->Volume;
		//LongLimitPrice = 0;
	}
	else{
		tempLots = -1 * pTrade->Volume;
		//ShortLimitPrice = 0;
	}

	if (Posi.mp_PriceMap.find(pTrade->Price) == Posi.mp_PriceMap.end())
		Posi.mp_PriceMap[pTrade->Price] = tempLots;
	else
		Posi.mp_PriceMap[pTrade->Price] = Posi.mp_PriceMap[pTrade->Price] + tempLots;

	for (map<double, int>::iterator mpit = Posi.mp_PriceMap.begin(); mpit != Posi.mp_PriceMap.end(); mpit++){
		//cerr <<Posi.InstrumentID << " " << mpit->first << " " << mpit->second<<endl;
		tempTotalLots += mpit->second;
	}
	Posi.NetPosition = tempTotalLots;

	if (pTrade->OffsetFlag == '0'){
		OffSetFlag = "OPEN";
		//DiretionType is a char '0'=buy '1'=sell
		//OffsetFlag is a char '0'=open '1'=close
		if (pTrade->Direction == '0'){
			Direction = "LONG";
		}
		else{
			Direction = "SHORT";
		}

		if (Posi.mp_PositionDistribution.find(pTrade->Price) == Posi.mp_PositionDistribution.end())
			Posi.mp_PositionDistribution[pTrade->Price] = tempLots;
		else
			Posi.mp_PositionDistribution[pTrade->Price] += tempLots;

		double tempTotalPosiValue = 0;
		double tempTotolLots = 0;
		for (map<double, int>::iterator mpit = Posi.mp_PositionDistribution.begin(); 
		     mpit != Posi.mp_PositionDistribution.end();
		     mpit++){
			tempTotalPosiValue += mpit->first*mpit->second;
			tempTotolLots += mpit->second;
		}
		Posi.AveragePositionEntry = tempTotalPosiValue / tempTotolLots;

		if (pTrade->Volume != 0){
			TotalTrades += pTrade->Volume;
			if (TradeLog.is_open())
				TradeLog << pTrade->InstrumentID << "," << OffSetFlag << "," << Direction << "," << pTrade->TradeTime << "," << pTrade->Price << "," << pTrade->Volume << "," << TotalTrades << endl;
			TradeLog.close();
		}
	}
	else{
		OffSetFlag = "CLOSE";
		//DiretionType is a char '0'=buy '1'=sell
		//OffsetFlag is a char '0'=open '1'=close
		if (pTrade->Direction == '0'){
			Direction = "LONG";
		}
		else{
			Direction = "SHORT";
		}

		Posi.mp_PositionDistribution.clear();
		Posi.AveragePositionEntry = 0;


		if (pTrade->Volume != 0){
			if (TradeLog.is_open())
				TradeLog << pTrade->InstrumentID << "," << OffSetFlag << "," << Direction << "," << pTrade->TradeTime << "," << pTrade->Price << "," << pTrade->Volume << "," << "," << Posi.TotalProfit << endl << endl;
			TradeLog.close();
		}

	}
}


void CAgent::CalculatePnL(CThostFtdcDepthMarketDataField *DepthData){
	Posi.InstrumentID = DepthData->InstrumentID;
	Posi.TotalValue = 0;
	double total_lots = 0;
	for (map<double, int>::iterator mpit = Posi.mp_PriceMap.begin(); mpit != Posi.mp_PriceMap.end(); mpit++){
		//cerr <<Posi.InstrumentID << " " << mpit->first << " " << mpit->second<<endl;
		Posi.TotalValue = Posi.TotalValue + mpit->first*mpit->second;
		total_lots += mpit->second;
	}
	Posi.TotalProfit = (DepthData->LastPrice*Posi.NetPosition - Posi.TotalValue) / InstrumentTickSize;
	Posi.CostLine = Posi.TotalValue / total_lots;
}


string CAgent::GetTimer(){
	return SprDepthData->UpdateTime;
}

void CAgent::Start_MD_ZMQ_Server(){
	MD_ZMQ_Context = zmq_ctx_new();
	MD_ZMQ_Publisher = zmq_socket(MD_ZMQ_Context, ZMQ_PUB);
	string IPC_pipe_name = "ipc:///tmp/" + instrument_md_pipe_name;
	cerr<<IPC_pipe_name<<endl;
	int rc = zmq_bind(MD_ZMQ_Publisher, IPC_pipe_name.c_str());
	assert(rc == 0);
}

void CAgent::Start_AG_ZMQ_Server(){
	AG_ZMQ_Context = zmq_ctx_new();
	AG_ZMQ_Subscriber = zmq_socket(AG_ZMQ_Context, ZMQ_REQ);
	string IPC_pipe_name = "ipc:///tmp/" + instrument_ag_pipe_name;
	cerr<<IPC_pipe_name<<endl;
	int rc = zmq_connect(AG_ZMQ_Subscriber, IPC_pipe_name.c_str());
	//zmq_setsockopt(AG_ZMQ_Subscriber,ZMQ_SUBSCRIBE,"",0);
	assert(rc == 0);
}

void CAgent::Broadcast_MD(CThostFtdcDepthMarketDataField *pDepth){
	ptMD_Info.set_last_price(pDepth->LastPrice);
	ptMD_Info.set_volume(pDepth->Volume);
	ptMD_Info.set_md_timestamp(pDepth->UpdateTime);
	ptMD_Info.set_md_instrument(pDepth->InstrumentID);
	ptMD_Info.set_md_tradingday(pDepth->TradingDay);

	char buff[100];
	//zmq_msg_t request;
	zmq_msg_t md_msg;

	string proto_buffer;
	ptMD_Info.SerializeToString(&proto_buffer);
	
	int md_msg_size = proto_buffer.size();
	int ret = zmq_msg_init_size(&md_msg,200);
	memcpy(zmq_msg_data(&md_msg),proto_buffer.c_str(),md_msg_size + 10);
	zmq_msg_send(&md_msg,MD_ZMQ_Publisher,0);

	//zmq_msg_init_size(&request,100);
	//int size = zmq_msg_recv(&request,MD_ZMQ_Responder,0);
}












