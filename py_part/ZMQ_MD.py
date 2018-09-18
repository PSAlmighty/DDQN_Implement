#coding=utf-8
import os
import zmq
import sys
import time
import numpy as np
import pandas as pd
from time import sleep
import DQN_Agent_pb2 


context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.connect('ipc:///tmp/cumd')
socket.setsockopt_string(zmq.SUBSCRIBE,'')
price_raw_data = []
volume_raw_data = []
timestamp_raw_data = []

freq = 15
resample_output_enable = True
last_time_stamp_mod = 0

AGENT_ACTION = DQN_Agent_pb2.Agent_Info()
MD =DQN_Agent_pb2.MD_Info()

def IsTradingTime(Time_Clock):
    TIME = int(time.strftime("%H%M",Time_Clock))
    Ret = False
    if TIME >= 900 and TIME <= 1015:
        Ret = True
    if TIME >= 1030 and TIME <= 1130:
        Ret = True
    if TIME >= 1330 and TIME <= 1500:
        Ret = True
    if TIME >= 2100 and TIME <= 2400:
        Ret = True
    if TIME >= 0 and TIME <= 100:
        Ret = True
    return Ret

while(True):
    r_msg = socket.recv()
    MD.ParseFromString(r_msg)
    
    timestamp_raw_data.append(MD.MD_Timestamp)
    volume_raw_data.append(MD.volume)
    price_raw_data.append(MD.last_price)
    date = MD.MD_TradingDay
    
    if int(time.strftime('%M',time.localtime(time.time()))) % freq != last_time_stamp_mod:
        resample_output_enable = True
        last_time_stamp_mod = int(time.strftime('%M',time.localtime(time.time()))) % freq
        
    pd_md_data = pd.DataFrame()
    pd_md_data['TradeTime'] = timestamp_raw_data
    pd_md_data['last_price'] = price_raw_data
    pd_md_data['volume'] = volume_raw_data
    pd_md_data.set_index(pd_md_data.TradeTime)
    paths = MD.MD_Instrument + '_' + str(date) +'.csv'
        
    if os.path.exists(paths) == True:
        pd_md_data.to_csv(paths,index = 0,mode = 'a',header = False)
    else:
        pd_md_data.to_csv(paths,index = 0,mode = 'a',header = True)
            
    if int(time.strftime('%M',time.localtime(time.time()))) % 15 == 0 \
        and resample_output_enable == True \
        and IsTradingTime(time.localtime(time.time())) == True:
                
        pd_resample_data = pd.read_csv(paths, encoding="gbk", engine='c')
        time_index = pd.to_datetime(pd_resample_data.TradeTime)
        pd_resample_data = pd_resample_data.set_index(time_index)
            
        pd_ohlc = pd_resample_data['last_price'].resample('15min').ohlc()
        pd_ohlc.dropna()
        pd_vol = pd_resample_data['volume'].resample('15min').sum()
        pd_vol.dropna(inplace=True)
            
        pd_resample_data = pd.concat([pd_ohlc,pd_vol], axis = 1)
        pd_resample_data.dropna()
            
        path_resample = MD.MD_Instrument +'_resample.csv'
        pd_resample_data.to_csv(path_resample,mode = 'w',header = True)
                
        resample_output_enable = False
            
    price_raw_data = []
    volume_raw_data = []
    timestamp_raw_data = []