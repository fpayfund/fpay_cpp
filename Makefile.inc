DEBUG =yes

CC = gcc
CXX = g++
CC32 = gcc


COMM_LIB = ./net/lib/corelib.a ./net/lib/sox.a 

HIREDIS_LIB = ./thirdparts/hiredis/libhiredis.a

CXXFLAGS = -Wall -Wwrite-strings -D__STDC_LIMIT_MACROS -DHAVE_EPOLL -DTIXML_USE_STL

ifeq (yes,${DEBUG})
	CXXFLAGS := ${CXXFLAGS} -O0 -ggdb 
else
	CXXFLAGS := ${CXXFLAGS}
endif

LINK_CXXFLAG = $(CXXFLAGS) -Wl,-rpath,./bin

INCLUDE =  -I./ -I./net -I./net/common -I./helper -I./client/ -I./protocol/ -I./thirdparts -I./thirdparts/hiredis/ -Ithirdparts/hiredis/tinyxml -I./server 


