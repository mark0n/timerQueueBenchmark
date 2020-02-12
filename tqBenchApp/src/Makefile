TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += tqBench

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install tqBench.dbd into <top>/dbd
DBD += tqBench.dbd

# specify all source files to be compiled and added to the library
#tqBench_SRCS += xxx

tqBench_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

