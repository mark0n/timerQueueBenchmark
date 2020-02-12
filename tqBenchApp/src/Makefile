TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

PROD_HOST += epicsTimerQueueBench

epicsTimerQueueBench_SRCS += epicsTimerQueueBench.cpp
epicsTimerQueueBench_LIBS += Com
epicsTimerQueueBench_SYS_LIBS += benchmark
epicsTimerQueueBench_SYS_LIBS += pthread

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

