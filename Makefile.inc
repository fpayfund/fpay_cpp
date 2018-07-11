CC = gcc
CXX32 = g++
CC32 = gcc

ARRU = ar -ru

DEFINE_SEL=-DHAVE_EPOLL

#DEF_STATUSPROTOCOL=-DSTATUS_OLDPROTOCOL
DEF_STATUSPROTOCOL=-DSTATUS_NEWPROTOCOL

DEF_DBTHREAD=-DDB_MULTITHREAD

COMM_LIB = ../lib/daemon.a ../lib/corelib.a ../lib/config.a ../lib/sox.a /usr/lib/libcrypto.a /usr/lib/libuuid.a
COMM_LIB_R = ../lib/corelib.ra ../lib/sox.ra /usr/lib/libcrypto.a ../lib/daemon.ra ../lib/config.ra /usr/lib/libuuid.a 

VERSION=
CXXFLAG = -Wall -fPIC -ggdb -D_REENTRANT $(DEFINE_SEL) -DXML_NULL  $(DEF_STATUSPROTOCOL)
CXXFLAG_R = -Wall -fPIC -O2 -O3 -D_REENTRANT $(DEFINE_SEL) -DXML_NULL $(DEF_STATUSPROTOCOL)

LINK_CXXFLAG = $(CXXFLAG) -Wl,-rpath,../bin
LINK_CXXFLAG_R = $(CXXFLAG_R) -Wl,-rpath,../bin

INCLUDE =  -I../ -I../common/  -I../protocol/ -I../common/occi -I../service/open_service_common/include/
#	vim: ft=make


.ver:
	export LC_ALL=C && sed -i 's/[0-9][0-9]*/'`svn info|grep Revision|awk '{print $$2}'`'/' ../helper/version.h
