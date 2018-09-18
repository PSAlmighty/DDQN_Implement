#coding=utf-8
import os
import zmq
import sys
import time
import numpy as np
import pandas as pd
import DQN_Agent_pb2 

from time import sleep
from IPython.display import clear_output
from agent.main import Agent

from emulator.env_data import high2low, fix_data
from emulator.env_factor import get_factors

from params import Smp_Size
from params import ROLLING_PERIOD_PARAM

sys.path.append("..")

def GetFactorFrame():
    st = time.clock()
    quotes = fix_data('./tickdata/HS300_test.csv')
    quotes = high2low(quotes, '15min')
    daily_quotes = high2low(quotes, '15min')
    ed = time.clock()
    print(ed-st)

    Index = quotes.index
    High = quotes.high.values
    Low = quotes.low.values
    Close = quotes.close.values
    Open = quotes.open.values

    st = time.clock()
    factors = get_factors(Index, Open, Close, High, Low, rolling=150, drop=True)
    ed = time.clock()
    print(ed-st)
    
    print(factors[-100:].shape)
    print(factors.iloc[-100:].index)
    
    factors = np.array(factors.iloc[-100:])
    np.expand_dims(factors, axis=0)
    shape = [1, ROLLING_PERIOD_PARAM, factors.shape[1]]
    factors = factors.reshape(shape)
    
    print(factors.shape)
    
    return factors



context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind('ipc:///tmp/asdf')
AGENT_ACTION = DQN_Agent_pb2.Agent_Info()



image_shape = GetFactorFrame().shape
#agent = Agent(image_shape, 3)


a = 0
while(True):
    
    r_msg = socket.recv()
    #print(r_msg)
    
    if a % 10 == 0 :
        clear_output(True)
    
    a += 1
    a = a % 10
    print(a)
    AGENT_ACTION.Agent_Action = a
    AGENT_ACTION.Current_Position = 1
    AGENT_ACTION.Agent_Trading_Instrument = 'cu1812'
    AGENT_ACTION.Action_Timestamp = "123"
    AGENT_ACTION.LatestResult = 1
    AGENT_ACTION.msg_pipe_init = 1
    
    
    s_msg = AGENT_ACTION.SerializeToString()
    socket.send(s_msg)