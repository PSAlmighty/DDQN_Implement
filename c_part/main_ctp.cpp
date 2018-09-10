// testTraderApi.cpp : 定义控制台应用程序的入口点。
// Design By ZhaomingHu



#include "./include/ThostFtdcTraderApi.h"
#include "./include/ThostFtdcMdApi.h"
#include "CMdSpi.h"
#include "CTraderSpi.h"
#include "CData.h"
#include <iostream>
#include <signal.h>
#include <queue>
#include "string.h"

#include "CAgent_DQN.h"
#include "CThreadPool.h"




using namespace std;

CThostFtdcMdApi *pMdApi;
CThostFtdcTraderApi *pTraderApi;

CMdSpi *pMdSpi = new CMdSpi();
CTraderSpi *pTraderSpi = new CTraderSpi();
CThread_Pool *_ThreadPool;


char  LIVE_M_FRONT_ADDR[] = "tcp://211.144.195.157 :41213";
char  LIVE_T_FRONT_ADDR[] = "tcp://172.31.10.142 :41205";		// 前置地址Live

ThostFtdcParams Citic_Thost_Params;

// 交易品种数组
char **ppInstrumentID;
int iInstrumentID;


//交易品种文件路径
char *InstrumentParseFile;

string TradingDate;


//订单线程控制组变量
vector<int> vcOrderRef;
vector<string> vcSysIDRef;

queue<CThostFtdcInputOrderField> qInsertOrderQueue;
queue<CThostFtdcInputOrderActionField> qCancelOrderQueue;

// 交易品种默认交易量
map<string, CAgent*> mpMM;
map<string, int> mpInstrumentClipSize;
map<string, double> mpInstrumentDailyStopLimit;
map<string, string> mpInstrumentMDPipeName;
map<string, string> mpInstrumentAGPipeName;

pthread_mutex_t csOperatingOrderSysID;
pthread_mutex_t csInsert;
pthread_mutex_t csCancel;
pthread_mutex_t csMMObjectLock;
pthread_mutex_t csMainThreadLock;

pthread_cond_t hInsertEvent;
pthread_cond_t hCancelEvent;

pthread_cond_t hRtnDepMktOutPut;
pthread_cond_t hTradingThreadDone;
pthread_cond_t hInstrumentSetupDone;

// 请求编号
int iRequestID = 0;
int PULL_ORDER_LIMIT;


void *Trading(void *lpParammeter);                 //交易回报线程
void *MarketData(void *lpParameter);               //市场数据线程 
void *InsertOrderThread(void *lpParameter);        //订单发送处理线程
void *CancelOrderThread(void *lpParameter);        //撤单发送处理线程


int main(int argc, char *argv[]){

	_ThreadPool=(CThread_Pool*)malloc(sizeof(CThread_Pool));
	Pool_Init(_ThreadPool,66);

	mpMM.clear();
	mpInstrumentClipSize.clear();	
	mpInstrumentDailyStopLimit.clear();
	mpInstrumentMDPipeName.clear();
	mpInstrumentAGPipeName.clear();
	
	//第二参数FALSE 事件触发后将自动复位
	pthread_cond_init(&hRtnDepMktOutPut, NULL);
	pthread_cond_init(&hTradingThreadDone, NULL);
	pthread_cond_init(&hInstrumentSetupDone, NULL);
	pthread_cond_init(&hInsertEvent, NULL);
	pthread_cond_init(&hCancelEvent, NULL);

	if (argc < 4)
	{
		printf(" run MM BrokerID InvestorID pwd \n");
		system("PAUSE");
		exit(0);
	}
	else
	{
		printf(" Live Trading Setup\n");
		Citic_Thost_Params.M_FRONT_ADDR = new char[sizeof(LIVE_M_FRONT_ADDR)];
		Citic_Thost_Params.T_FRONT_ADDR = new char[sizeof(LIVE_T_FRONT_ADDR)];
		InstrumentParseFile = new char[20];
		strcpy(Citic_Thost_Params.M_FRONT_ADDR, LIVE_M_FRONT_ADDR);
		strcpy(Citic_Thost_Params.T_FRONT_ADDR, LIVE_T_FRONT_ADDR);
		strcpy(Citic_Thost_Params.BROKER_ID, argv[1]);
		strcpy(Citic_Thost_Params.INVESTOR_ID, argv[2]);
		strcpy(Citic_Thost_Params.PASSWORD, argv[3]);
		strcpy(InstrumentParseFile, "./Product_Live.list");
		PULL_ORDER_LIMIT = 495;
	}

	iInstrumentID = 0;
	ppInstrumentID = (char**)malloc(sizeof(char*)  *20);
	FILE *fp;
	int ID_num = 0;
	TThostFtdcInstrumentIDType temp_ID;
	memset(temp_ID,0,sizeof(TThostFtdcInstrumentIDType));
	char InputBuff[100];
	if ((fp = fopen(InstrumentParseFile, "r")) == NULL)
	{
		printf("File Open Failed\n");
		printf("Get and key to exit\n");
		//system("PAUSE");
		exit(0);
	}
	else
	{
		ppInstrumentID = (char**)malloc(sizeof(char*) * 20);
		while (fgets(InputBuff, 100, fp) != NULL){
			string tempStr = InputBuff;
			string Substr;

			cerr<<InputBuff<<endl;
			int pos = tempStr.find("|");
			if (pos != -1)
				//InstrumentID
				Substr = tempStr.substr(0, pos);
			else
			{
				printf("No ClipSize\n");
				system("PAUSE");
				exit(0);
			}
			strcpy(temp_ID, Substr.c_str());
			ppInstrumentID[iInstrumentID] = (char*)malloc(sizeof(temp_ID));
			temp_ID[strlen(temp_ID)] = '\0';
			cerr<<temp_ID<<endl;
			strcpy(ppInstrumentID[iInstrumentID], temp_ID);
			tempStr.erase(0, pos + 1);

			pos = tempStr.find(",");
			if (pos != -1)
				//clip_size
				Substr = tempStr.substr(0, pos);
			else
			{
				printf("No StopLimit\n");
				system("PAUSE");
				exit(0);
			}
			mpInstrumentClipSize[ppInstrumentID[iInstrumentID]] = atoi(Substr.c_str());
			tempStr.erase(0, pos + 1);

			pos = tempStr.find(",");
			if (pos != -1)
				//Daily_stop_limit
				Substr = tempStr.substr(0, pos);
			else
			{
				printf("No MD_Pipe_name\n");
				system("PAUSE");
				exit(0);
			}
			mpInstrumentDailyStopLimit[ppInstrumentID[iInstrumentID]] = atof(Substr.c_str());
			tempStr.erase(0, pos + 1);

			pos = tempStr.find(",");
			if (pos != -1)
				//MD_PipeName
				Substr = tempStr.substr(0, pos);
			else
			{
				printf("No AGENT_Pipe_name\n");
				system("PAUSE");
				exit(0);
			}
			mpInstrumentMDPipeName[ppInstrumentID[iInstrumentID]] = Substr.c_str();
			tempStr.erase(0, pos + 1);

			mpInstrumentAGPipeName[ppInstrumentID[iInstrumentID]] = tempStr.c_str();
			iInstrumentID++;
		}
		fclose(fp);
	}

	printf(" PULL_ORDER_LIMIT = %d\n", PULL_ORDER_LIMIT);

	pthread_mutex_init(&csInsert,NULL);
	pthread_mutex_init(&csCancel,NULL);
	pthread_mutex_init(&csOperatingOrderSysID,NULL);
	pthread_mutex_init(&csMMObjectLock,NULL);
	pthread_mutex_init(&csMainThreadLock,NULL);

	//start strategy thread
	pthread_t hThreadMD;
	pthread_t hThreadTrader;
	pthread_t hInsertOrderThread;
	pthread_t hCancelOrderThread;

	if(pthread_create(&hThreadTrader,NULL, Trading, NULL))
		printf("Failed create Trader Thread\n");
	pthread_mutex_lock(&csMainThreadLock);
	pthread_cond_wait(&hTradingThreadDone,&csMainThreadLock);
	pthread_mutex_unlock(&csMainThreadLock);

	//Initialize mpAgent
	for (int i = 0; i < iInstrumentID; i++){
		cerr<<" ID "<<ppInstrumentID[i]<<endl;
		cerr<<" ClipSize "<<mpInstrumentClipSize[ppInstrumentID[i]]<<endl;
		cerr<<" StopLimit "<<mpInstrumentDailyStopLimit[ppInstrumentID[i]]<<endl;
		cerr<<" Pipe_name "<<mpInstrumentMDPipeName[ppInstrumentID[i]]<<endl;
		cerr<<" Pipe_name "<<mpInstrumentAGPipeName[ppInstrumentID[i]]<<endl;
	}

	for (int i = 0; i < iInstrumentID; i++){
		mpMM[ppInstrumentID[i]] = new CAgent(ppInstrumentID[i],
				mpInstrumentMDPipeName[ppInstrumentID[i]],
				mpInstrumentAGPipeName[ppInstrumentID[i]]);
	}
	sleep(5);

	//SetEvent(hInstrumentSetupDone);
	pthread_mutex_lock(&csMainThreadLock);
	pthread_cond_signal(&hInstrumentSetupDone);
	pthread_mutex_unlock(&csMainThreadLock);

	if(pthread_create(&hCancelOrderThread, NULL,CancelOrderThread, &qCancelOrderQueue))
		printf("Failed create Cancel Thread\n");

	if(pthread_create(&hInsertOrderThread, NULL, InsertOrderThread, &qInsertOrderQueue))
		printf("Failed create Insert Thread\n");

	sleep(2);
	if(pthread_create(&hThreadMD, NULL, MarketData, NULL))
		printf("Failed Create MD Thread");

	int OrderLots = 0;
	double OrderPrice = 0;
	//getchar();
	while (1)
	{
		pthread_mutex_lock(&csMainThreadLock);
		pthread_cond_wait(&hRtnDepMktOutPut,&csMainThreadLock);
		pthread_mutex_unlock(&csMainThreadLock);

		//system("clear");
		pthread_mutex_lock(&csMMObjectLock);

		pthread_mutex_unlock(&csMMObjectLock);
	}

	pthread_mutex_destroy(&csInsert);
	pthread_mutex_destroy(&csCancel);
	pthread_mutex_destroy(&csOperatingOrderSysID);
	pthread_mutex_destroy(&csMMObjectLock);
	pthread_mutex_destroy(&csMainThreadLock);

	pthread_cond_destroy(&hInsertEvent);
	pthread_cond_destroy(&hCancelEvent);
	pthread_cond_destroy(&hRtnDepMktOutPut);
	pthread_cond_destroy(&hTradingThreadDone);
	pthread_cond_destroy(&hInstrumentSetupDone);


 	return 0;
}

void *Trading(void *lpParameter)
{
	pTraderApi=CThostFtdcTraderApi::CreateFtdcTraderApi("./CTP_FLow/Trader");  // 创建TraderApi并指定生成文件名以同时调用两个DLL
	pTraderApi->RegisterSpi(pTraderSpi);
	pTraderApi->RegisterFront(Citic_Thost_Params.T_FRONT_ADDR);
	pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
	pTraderApi->Init();
	pTraderApi->Join();

	return 0;
}

void *MarketData(void *lpParameter)
{
	pMdApi = CThostFtdcMdApi::CreateFtdcMdApi("./CTP_FLow/MarketData");	// 创建MdApi并指定生成文件名以同时调用两个DLL
	pMdApi->RegisterSpi(pMdSpi);	                            // 注册事件类
	pMdApi->RegisterFront(Citic_Thost_Params.M_FRONT_ADDR);						// connect
    pMdApi->Init();
	pMdApi->Join();

	return 0;
}

void *InsertOrderThread(void *lpParameter)
{
	cerr << " Sending Order Thread Begin \n";

	queue<CThostFtdcInputOrderField> *p_qToPush = (queue<CThostFtdcInputOrderField>*)lpParameter;

	string OffSetFlag, Direction, Date, ID;

	while (true)
	{
		pthread_mutex_lock(&csInsert);
		pthread_cond_wait(&hInsertEvent,&csInsert);

		while (!p_qToPush->empty())
		{
			ID = p_qToPush->front().InstrumentID;
			string sz_path = "PendingLog_" + ID + ".csv";
			ofstream PendingLog(sz_path.c_str(), ios::app);

			CThostFtdcInputOrderField req;
			memset(&req, 0, sizeof(req));

			///经纪公司代码
			strcpy(req.BrokerID, Citic_Thost_Params.BROKER_ID);
			///投资者代码
			strcpy(req.InvestorID, Citic_Thost_Params.INVESTOR_ID);
			///合约代码 
			strcpy(req.InstrumentID, p_qToPush->front().InstrumentID);
			///报单引用
			strcpy(req.OrderRef, Citic_Thost_Params.ORDER_REF);
			///用户代码
			//	TThostFtdcUserIDType	UserID;
			///报单价格条件: 限价
			req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;

			///买卖方向: 
			req.Direction = p_qToPush->front().Direction;

			///组合开平标志: 开仓
			req.CombOffsetFlag[0] = p_qToPush->front().CombOffsetFlag[0];

			///组合投机套保标志
			req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
			///价格
			req.LimitPrice = p_qToPush->front().LimitPrice;
			///数量
			req.VolumeTotalOriginal = p_qToPush->front().VolumeTotalOriginal;
			///有效期类型: 当日有效
			req.TimeCondition = p_qToPush->front().TimeCondition;
			///GTD日期
			//	TThostFtdcDateType	GTDDate;
			///成交量类型: 任何数量
			req.VolumeCondition = THOST_FTDC_VC_AV;
			///最小成交量: 1
			req.MinVolume = 1;
			///触发条件: 立即
			req.ContingentCondition = THOST_FTDC_CC_Immediately;
			///止损价
			//	TThostFtdcPriceType	StopPrice;
			///强平原因: 非强平
			req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
			///自动挂起标志: 否
			req.IsAutoSuspend = 0;
			///业务单元
			//	TThostFtdcBusinessUnitType	BusinessUnit;
			///请求编号
			//	TThostFtdcRequestIDType	RequestID;
			///用户强评标志: 否
			req.UserForceClose = 0;

			//报单引用存储
			vcOrderRef.push_back(atoi(Citic_Thost_Params.ORDER_REF));
			//刷新报单引用
			int iNextOrderRef = atoi(Citic_Thost_Params.ORDER_REF);
			++iNextOrderRef;
			sprintf(Citic_Thost_Params.ORDER_REF, "%d", iNextOrderRef);

			int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID);

			//cerr << req.InstrumentID << " " << req.LimitPrice << endl;
			
			if (PendingLog.is_open())
			{
				PendingLog << p_qToPush->front().InstrumentID << ",";
				PendingLog << p_qToPush->front().CombOffsetFlag << ",";
				PendingLog << p_qToPush->front().Direction << ",";
				PendingLog << p_qToPush->front().LimitPrice << ",";
				PendingLog << p_qToPush->front().VolumeTotalOriginal << ",";
				PendingLog << p_qToPush->front().OrderRef << endl;
			}
			PendingLog.close();

			p_qToPush->pop();
		}
		//printf("All Order Sent\n");
		
		pthread_mutex_unlock(&csInsert);
	}

	return 0;
}

void *CancelOrderThread(void *lpParameter)
{
	cerr << " Pulling Order Thread Begin \n";

	string OffSetFlag, Direction, Date, ID;

	queue<CThostFtdcInputOrderActionField> *p_qToPull = (queue<CThostFtdcInputOrderActionField>*)lpParameter;

	while (true)
	{
		//WaitForSingleObject(hCancelEvent, INFINITE);
		//EnterCriticalSection(&csCancel);
		pthread_mutex_lock(&csCancel);
		pthread_cond_wait(&hCancelEvent,&csCancel);

		while (!p_qToPull->empty())
		{
			ID = p_qToPull->front().InstrumentID;
			string sz_path = "PullingLog_" + ID + ".csv";
			ofstream PullingLog(sz_path.c_str(), ios::app);

			//cerr << "Order Queue Count " << p_qToPull->size() << endl;
			//system("PAUSE");
			static bool ORDER_ACTION_SENT;		//是否发送修改订单请求

			CThostFtdcInputOrderActionField req;
			memset(&req, 0, sizeof(req));
			///经纪公司代码
			strcpy(req.BrokerID, p_qToPull->front().BrokerID);
			///投资者代码
			strcpy(req.InvestorID, p_qToPull->front().InvestorID);
			///报单操作引用
			//	TThostFtdcOrderActionRefType	OrderActionRef;
			///报单引用
			strcpy(req.OrderRef, p_qToPull->front().OrderRef);
			///请求编号
			//	TThostFtdcRequestIDType	RequestID;
			///前置编号
			req.FrontID = Citic_Thost_Params.FRONT_ID;
			///会话编号
			req.SessionID = Citic_Thost_Params.SESSION_ID;
			///交易所代码
			//	TThostFtdcExchangeIDType	ExchangeID;
			strcpy(req.ExchangeID, p_qToPull->front().ExchangeID);
			///报单编号
			//	TThostFtdcOrderSysIDType	OrderSysID;
			strcpy(req.OrderSysID, p_qToPull->front().OrderSysID);
			///操作标志
			//  THOST_FTDC_AF_Delete 撤单
			req.ActionFlag = THOST_FTDC_AF_Delete;
			///价格
			//	TThostFtdcPriceType	LimitPrice;
			///数量变化
			//	TThostFtdcVolumeType	VolumeChange;
			///用户代码
			//	TThostFtdcUserIDType	UserID;
			///合约代码
			strcpy(req.InstrumentID, p_qToPull->front().InstrumentID);

			int iResult = pTraderApi->ReqOrderAction(&req, ++iRequestID);

			if (iResult != 0)
				cerr << "Pulled Error\n";

			if (PullingLog.is_open()
					&& iResult == 0)
			{
				PullingLog << p_qToPull->front().InstrumentID << ",";
				PullingLog << p_qToPull->front().LimitPrice << ",";
				PullingLog << p_qToPull->front().OrderRef << endl;
			}
			PullingLog.close();

			p_qToPull->pop();
		}
        
		pthread_mutex_unlock(&csCancel);
		//LeaveCriticalSection(&csCancel);
	}


	return 0;
}


