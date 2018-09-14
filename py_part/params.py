# Three action : long short keep
ACTION_SIZE = 3
MAX_GRAD_NORM = 10
ROLLING_PERIOD_PARAM = 100

# 1st param State Count Days
# 2th param is the number of how much 5min candle in 1 days
# 3th param is the indicator counts
# 4th param is the number of how much days to monitor
# INPUT_SHAPE = [None, ROLLING_PERIOD_PARAM, 24, 1]

MEMORY_SIZE = 320000
# batch size should not be too big for the scenes of indicator ALWAYS causing diffrent return
# this is the problem of trading data 
BATCH_SIZE = 256
SP_BATCH_SIZE = 64

GAMMA = 0.92

TARGET_STEP_SIZE = 256
TRAIN_STEP_SIZE = 128

SP_TARGET_STEP_SIZE = 64
SP_TRAIN_STEP_SIZE = 32

Smp_Start = 2000
Smp_Size = 6200
Full_Size = 12300

Sp_Smp_Size = 3900
Sp_Full_Size = 12300

Full_Smp_Size = 5150

EPSILON_VALUE = 0.85

# ENTROPY_BETA = 0.1
# POLICY_BETA = 1
# VALUE_BETA = 1
# ACTOR_NORM_BETA = 1e-3
# CRITIC_NORM_BETA = 0.1