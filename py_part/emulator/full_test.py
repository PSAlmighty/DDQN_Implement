import numpy as np
import pandas as pd
import sys
sys.path.append("..")

from emulator.env_data import high2low, fix_data
from emulator.env_factor import get_factors

from params import Smp_Size
from params import ROLLING_PERIOD_PARAM

#Samples_Size = 350

quotes = fix_data('./tickdata/HS300_test.csv')
quotes = high2low(quotes, '15min')
daily_quotes = high2low(quotes, '15min')

Index = quotes.index
High = quotes.high.values
Low = quotes.low.values
Close = quotes.close.values
Open = quotes.open.values
#Volume = quotes.volume.values
#factors = get_factors(Index, Open, Close, High, Low, Volume, rolling=188, drop=True)
print('init quotes shape')
print(quotes.shape)

factors = get_factors(Index, Open, Close, High, Low, rolling=150, drop=True)

print('init factors shape')
print(factors.shape)

daily_quotes['returns'] = np.log(daily_quotes['close'].shift(-1) / daily_quotes['close'])
daily_quotes.dropna(inplace=True)

start_date = pd.to_datetime('2017-03-11')
end_date = pd.to_datetime('2018-12-29')
daily_quotes = daily_quotes.loc[start_date:end_date]

print('init daily shape')
print(daily_quotes.shape)

daily_quotes = daily_quotes.iloc[100:]
factors = factors.loc[start_date:end_date]

print('smp daily shape')
print(daily_quotes.shape)
print('smp factors shape')
print(factors.shape)

ic = 0


fac_list = []
for i in range(len(daily_quotes)):
    s = i 
    e = i + ROLLING_PERIOD_PARAM
    f = np.array(factors.iloc[s:e])
    #print(ic,np.expand_dims(f, axis=0).shape)
    '''ic += 1
    print(ic,factors.iloc[s:e,0:1])
    print('  ')
    print(daily_quotes.iloc[i])'''
    fac_list.append(np.expand_dims(f, axis=0))
    

fac_array = np.concatenate(fac_list, axis=0)
shape = [fac_array.shape[0], 1, ROLLING_PERIOD_PARAM, fac_array.shape[2]]
fac_array = fac_array.reshape(shape)
fac_array = np.transpose(fac_array, [0, 2, 3, 1])

print(fac_array.shape)

DATE_QUOTES = daily_quotes
DATA_FAC = fac_array



class Account(object):
    def __init__(self,_BEGIN = 0):
        self.data_close = DATE_QUOTES['close'][_BEGIN:]
        self.data_open = DATE_QUOTES['open'][_BEGIN:]
        print(DATA_FAC.shape)
        self.data_observation = DATA_FAC[_BEGIN:]
        print(self.data_observation.shape)
        self.action_space = ['long', 'short', 'close']
        self.free = 0
        self.step_counter = 0
        self.cash = 1e5
        self.position = 0
        self.total_value = self.cash + self.position
        self.flags = 0

    def reset(self):
        self.step_counter = 0
        self.cash = 1e5
        self.position = 0
        self.total_value = self.cash + self.position
        self.flags = 0
        self.buffer_reward = []
        self.buffer_value = []
        self.buffer_action = []
        self.buffer_cash = []
        return self._get_initial_state()

    def _get_initial_state(self):
        #print(self.data_observation.shape)
        return np.transpose(self.data_observation[0], [2, 0, 1])

    def get_action_space(self):
        return self.action_space

    def long(self):
        self.flags = 1
        quotes = self.data_open[self.step_counter] * 10
        self.cash -= quotes * (1 + self.free) - 6
        self.position = quotes

    def short(self):
        self.flags = -1
        quotes = self.data_open[self.step_counter] * 10
        self.cash += quotes * (1 - self.free) - 6
        self.position = - quotes

    def keep(self):
        quotes = self.data_open[self.step_counter] * 10
        self.position = quotes * self.flags

    def close_long(self):
        self.flags = 0
        quotes = self.data_open[self.step_counter] * 10
        self.cash += quotes * (1 - self.free) - 6
        self.position = 0

    def close_short(self):
        self.flags = 0
        quotes = self.data_open[self.step_counter] * 10
        self.cash -= quotes * (1 + self.free) - 6
        self.position = 0

    def step_op(self, action, printable, training = True):

        if action == 'long':
            if self.flags == 0:
                self.long()
            elif self.flags == -1:
                self.close_short()
                #self.long()
            else:
                self.keep()

        elif action == 'close':
            if self.flags == 1:
                self.close_long()
            elif self.flags == -1:
                self.close_short()
            else:
                pass

        elif action == 'short':
            if self.flags == 0:
                self.short()
            elif self.flags == 1:
                self.close_long()
                #self.short()
            else:
                self.keep()
        else:
                raise ValueError("action should be elements of ['long', 'short', 'close']")
        
        #position = self.data_close[self.step_counter] * 10 * self.flags
        
        if training == True:
            if self.step_counter < Smp_Size - 5:
                position = self.data_close[self.step_counter + 5] * 10 * self.flags
            else:
                position = self.data_close[self.step_counter] * 10 * self.flags
        else:
            position = self.data_close[self.step_counter] * 10 * self.flags
            
        
        reward = self.cash + position - self.total_value
        
        
        self.step_counter += 1
        self.total_value = position + self.cash
        next_observation = self.data_observation[self.step_counter]
        
        done = False
        if self.total_value < 4000:
            done = True
        if self.step_counter > Smp_Size:
            done = True
            
        if printable == True:
            print(action,self.data_open[self.step_counter-1],self.data_close[self.step_counter-1],reward)
            
        self.buffer_action.append(action)
        self.buffer_reward.append(reward)
        self.buffer_value.append(self.total_value)
        self.buffer_cash.append(self.cash)    
            
        return np.transpose(next_observation, [2, 0, 1]), reward, done

    def step(self, action, printable = False):
        if action == 0:
            return self.step_op('long',printable)
        elif action == 1:
            return self.step_op('short',printable)
        elif action == 2:
            return self.step_op('close',printable)
        else:
            raise ValueError("action should be one of [0,1,2]")
            
    def plot_data(self):
        df = pd.DataFrame([self.buffer_value, self.buffer_reward, self.buffer_cash, self.buffer_action]).T
        length = df.shape[0]
        df.index = self.data_close.index[:length]
        df.columns = ["value", "reward", "cash", "action"]
        return df
    









