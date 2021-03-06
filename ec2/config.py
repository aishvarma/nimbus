#!/usr/bin/env python

# Author: Omid Mashayekhi <omidm@stanford.edu>

# ssh -i ~/.ssh/omidm-sing-key-pair-us-west-2.pem -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no ubuntu@<ip>

# US West (Northern California) Region
# EC2_LOCATION = 'us-west-1'
# NIMBUS_AMI = 'ami-50201815'
# UBUNTU_AMI = 'ami-660c3023'
# KEY_NAME = 'omidm-sing-key-pair-us-west-1'
# SECURITY_GROUP = 'nimbus_sg_uswest1'

# EC2 configurations
# US West (Oregon) Region
EC2_LOCATION                    = 'us-west-2'
UBUNTU_AMI                      = 'ami-fa9cf1ca'
NIMBUS_AMI                      = 'ami-4f5c392f'
CONTROLLER_INSTANCE_TYPE        = 'c3.4xlarge'
WORKER_INSTANCE_TYPE            = 'c3.2xlarge'
PLACEMENT                       = 'us-west-2a' # None
PLACEMENT_GROUP                 = 'nimbus-cluster' # None / '*'
SECURITY_GROUP                  = 'nimbus_sg_uswest2'
KEY_NAME                        = 'sing-key-pair-us-west-2'
PRIVATE_KEY                     = '/home/omidm/.ssh/' + KEY_NAME + '.pem'
CONTROLLER_NUM                  = 1
WORKER_NUM                      = 100


#Environment variables
DBG_MODE                        = 'warn' # 'error'
TTIMER_LEVEL                    = 'l1'  # 'l0'


# Controller configurations
ASSIGNER_THREAD_NUM             = 8
BATCH_ASSIGN_NUM                = 200
COMMAND_BATCH_SIZE              = 10000
DEACTIVATE_CONTROLLER_TEMPLATE  = False
DEACTIVATE_COMPLEX_MEMOIZATION  = False
DEACTIVATE_BINDING_MEMOIZATION  = False
DEACTIVATE_WORKER_TEMPLATE      = False
DEACTIVATE_MEGA_RCR_JOB         = False
DEACTIVATE_CASCADED_BINDING     = False
ACTIVATE_LB                     = False
ACTIVATE_FT                     = False
LB_PERIOD                       = 60
FT_PERIOD                       = 600
FIRST_PORT                      = 5800
SPLIT_ARGS                      = str(WORKER_NUM) + ' 1 1' # '2 2 2' '4 4 4'


# Worker configurations
OTHREAD_NUM                     = 8
APPLICATION                     = 'lr' # 'lr' 'k-means' 'water' 'heat'
DEACTIVATE_EXECUTION_TEMPLATE   = False
DEACTIVATE_CACHE_MANAGER        = False
DEACTIVATE_VDATA_MANAGER        = False
DISABLE_HYPERTHREADING          = False
RUN_WITH_TASKSET                = False
WORKER_TASKSET                  = '0-1,4-5' # '0-3,8-11'


# Application configurations
# lr and k-means
DIMENSION                       = 10
CLUSTER_NUM                     = 2
ITERATION_NUM                   = 30
PARTITION_PER_CORE              = 10
PARTITION_NUM                   = WORKER_NUM * OTHREAD_NUM * PARTITION_PER_CORE # 8000
REDUCTION_PARTITION_NUM         = WORKER_NUM
SAMPLE_NUM_M                    = 544 # PARTITION_NUM / 1000000.0
SPIN_WAIT_US                    = 0 # 4000 # 4000 * (100 / WORKER_NUM)
DEACTIVATE_AUTOMATIC_REDUCTION  = True
DEACTIVATE_REDUCTION_COMBINER   = False

# water
SIMULATION_SCALE                = 512
PART_X                          = 4
PART_Y                          = 4
PART_Z                          = 4
PROJ_PART_X                     = 4
PROJ_PART_Y                     = 4
PROJ_PART_Z                     = 4
FRAME_NUMBER                    = 1
ITERATION_BATCH                 = 1
MAX_ITERATION                   = 100
WATER_LEVEL                     = 0.35
PROJECTION_SMART_LEVEL          = 0
NO_PROJ_BOTTLENECK              = False
GLOBAL_WRITE                    = False

# heat
ITER_NUM                        = 30
NX                              = 800
NY                              = 800
NZ                              = 800
PNX                             = 2
PNY                             = 2
PNZ                             = 2
BW                              = 1
SW_US                           = 0


