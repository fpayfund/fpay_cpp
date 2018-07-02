
include ../Makefile.inc

#depends: sox."int_types.h"
#-fomit-frame-pointer

SRCS = InnerConn.cpp hash_method.cpp  AbstractConn.cpp MultiConnManagerImp.cpp MultiConnManagerVecImp.cpp \
		selfip.cpp  Request.cpp IConn.cpp ConnManagerLinkImp.cpp Sender.cpp \
		 WrapServerStart.cpp  IBaseConnFactory.cpp UdpConnImp.cpp WriterImp.cpp \
		 MfcAppContext.cpp BackLinkHandler.cpp MAbstractServer.cpp AStatLinkHandler.cpp  RedisProtocol.cpp \
                 RedisSubscribeLinkHandler.cpp  RedisSubscribeClient.cpp  RedisAsynResponseLinkHandler.cpp  RedisPublishClient.cpp TrySimpleLock.cpp  RedisAsynWriteClient.cpp  

# UinfoIdx.cpp Birthday.cpp po_signal.cpp

SRCC = #rc4.c

OBJS = $(SRCS:.cpp=.o)
OBJC = $(SRCC:.c=.o)
OBJSS = $(OBJS) $(OBJC) #ntesclient.object

OBJS_R = $(SRCS:.cpp=.ro)
OBJC_R = $(SRCC:.c=.ro)
OBJSS_R = $(OBJS_R) $(OBJC_R)

.SUFFIXES: .o .c
.c.o:
	$(CXX) $(CXXFLAG) ${INCLUDE} -c -o $@ $<

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAG) ${INCLUDE} -c -o $@ $<

.SUFFIXES: .ro .c
.c.ro:
	$(CXX32) $(CXXFLAG_R) ${INCLUDE} -c -o $@ $<

.SUFFIXES: .o .cpp
.cpp.ro:
	$(CXX32) $(CXXFLAG_R) ${INCLUDE} -c -o $@ $<

all: corelib 

release: corelib.ra

corelib: $(OBJS) $(OBJC) ../../lib/sox.a | ../../lib
	$(ARRU) ../../lib/corelib.a $(OBJSS)
	ranlib ../../lib/corelib.a

../../lib:
	mkdir ../../lib

corelib.ra: $(OBJS_R) $(OBJC_R) ../../lib/sox.ra 
	$(ARRU) ../../lib/corelib.ra $(OBJSS_R)
#	ranlib ../../lib/corelib.ra


depend:
	mkdep $(CXXFLAGS_R) $(SRCS) $(SRCC)

clean:
	rm -f *.o
	rm -f *.ro
	rm -f ../../lib/corelib.a ../../lib/corelib.ra

install:

distclean: clean
	rm -f .depend

#	vim: set ts=4 sts=4 syn=make :
