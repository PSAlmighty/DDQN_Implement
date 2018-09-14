from copy import deepcopy
import numpy as np
import torch
import random

from torch import nn
from torch.nn.functional import softmax
from torch.autograd import Variable
from agent.forward import DuelingDQN
from agent.access import Access
from collections import deque
from params import *

import sys
sys.path.append("..")

# Ensure values are greater than epsilon to avoid numerical instability.
_EPSILON = 1e-6


class Agent(object):
    def __init__(self, image_shape, output_size, capacity=int(6e6)):
        self.output_size = output_size
        self.access = Access(capacity)
        self.value_net = DuelingDQN(image_shape, output_size)
        self.target_net = DuelingDQN(image_shape, output_size)
        self.value_net = nn.DataParallel(self.value_net)
        self.target_net = nn.DataParallel(self.target_net)
        
        # 自动使用gpu
        self.gpu = torch.cuda.is_available()
        if self.gpu:
            self.value_net.cuda()
            self.target_net.cuda()

        self.optimizer = torch.optim.Adam(self.value_net.parameters())
        
    def get_deterministic_policy(self, x):
        x = Variable(torch.from_numpy(x.astype(np.float32)))
        if not self.gpu:
            out = self.value_net(x).data.numpy()
            return np.argmax(out, axis=1)
        else:
            x = x.cuda()
            out = self.value_net(x)
            out = out.cpu().data.numpy()
            #print(np.argmax(out, axis=1))
            return np.argmax(out, axis=1)    

    def get_epsilon_policy(self, x, epsilon = 0.9):
        if np.random.uniform() > epsilon:
            return np.random.randint(self.output_size)
        else:
            return self.get_deterministic_policy(x)

    def optimize(self):
        batch = self.sample(BATCH_SIZE)
        if self.gpu:
            state, action, reward,  done, next_state = [Variable(torch.from_numpy(np.float32(i))).cuda() for i in batch]
            action = action.type(torch.LongTensor).cuda()
        else:
            state, action, reward,  done, next_state = [Variable(torch.from_numpy(np.float32(i))) for i in batch]
            action = action.type(torch.LongTensor)

        value = self.value_net(state).gather(1, action.unsqueeze(1))
        
        next_value = self.target_net(next_state)
        next_value = next_value.max(1)[0].view([-1, 1])
        
        value = value.squeeze(1)
        next_value = next_value.squeeze(1)
        
        target = done * reward + (1 - done) * (reward + GAMMA * next_value)
        
        #print(target)
        
        loss = (value - target.detach()).pow(2).mean()

        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()
        
        return loss

    def _update_target(self):
        # update target network parameters
        for t, s in zip(self.target_net.parameters(), self.value_net.parameters()):
            t.data.copy_(s.data)

    def append(self, *args):
        self.access.append(*args)

    def sample(self, batch_size = BATCH_SIZE):
        return self.access.sample(batch_size)

    def get_cache_len(self):
        return self.access.length
    
    def save_model(self):
        torch.save(self.value_net, './model/value_net.pkl')
        torch.save(self.target_net, './model/target_net.pkl')

        torch.save(self.value_net.state_dict(), './model/value_net_params.pkl')
        torch.save(self.target_net.state_dict(), './model/target_net_params.pkl')

    def load_model(self, value_path = './model/value_net.pkl', target_path = './model/target_net.pkl'):
        self.value_net = torch.load(value_path)
        self.target_net = torch.load(target_path)

    

