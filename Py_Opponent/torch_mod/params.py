# Three action : long short keep
ACTION_SIZE = 3
MAX_GRAD_NORM = 10
ROLLING_PERIOD_PARAM = 56

# State Count Days
# 50 is the number of how much 5min candle in 1 days
# 51 is the indicator counts
# 5 is the number of how much days to monitor
INPUT_SHAPE = [None, ROLLING_PERIOD_PARAM, 42, 1]

MEMORY_SIZE = 320000
BATCH_SIZE = 1024
GAMMA = 0.82

TARGET_STEP_SIZE = 256
TRAIN_STEP_SIZE = 128

Smp_Size = 8000
Full_Size = 12300

TEST_SMP = 3350

# ENTROPY_BETA = 0.1
# POLICY_BETA = 1
# VALUE_BETA = 1
# ACTOR_NORM_BETA = 1e-3
# CRITIC_NORM_BETA = 0.1