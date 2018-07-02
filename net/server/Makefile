
include Makefile.inc

CXXFLAGS = $(CXXFLAG) $(INCLUDE)#-pthread -Wall -ggdb -I../../ -I../../common/  $(DEFINE_SEL)  $(DEF_STATUSPROTOCOL)
CXXFLAGS_R = -pthread $(INCLUDE) -Wall -O2 -O3 -I../../ -I../../common/  $(DEFINE_SEL) $(DEF_STATUSPROTOCOL)
#-fomit-frame-pointer

##
# Attention:Fnv Hash compile switch
# (!!!) Please open it when compile imlinkd, imapp, imon2.
##
#CXXFLAGS += -DFNV_HASH

SRCS = DoubleLinkServer.cpp DoubleLinkRouter.cpp  DoubleLinkClient.cpp Client.cpp TrySimpleLock.cpp 


OBJS = $(SRCS:.cpp=.o)
OBJC = $(SRCC:.c=.o)
OBJSS = $(OBJS) $(OBJC) #ntesclient.object

OBJS_R = $(SRCS:.cpp=.ro)
OBJC_R = $(SRCC:.c=.ro)
OBJSS_R = $(OBJS_R) $(OBJC_R)

.SUFFIXES: .o .c
.c.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.SUFFIXES: .ro .c
.c.ro:
	$(CXX32) $(CXXFLAGS_R) -c -o $@ $<

.SUFFIXES: .o .cpp
.cpp.ro:
	$(CXX32) $(CXXFLAGS_R) -c -o $@ $<

all: doublelink.mul.router.a 
release: doublelink.mul.router.ra
doublelink.mul.router.a: $(OBJS) $(OBJC) ../lib/sox.a
	$(ARRU) ../lib/doublelink.mul.router.a $(OBJSS)
	ranlib ../lib/doublelink.mul.router.a

doublelink.mul.router.ra: $(OBJS_R) $(OBJC_R) ../lib/sox.ra
	$(ARRU) ../lib/doublelink.mul.router.ra $(OBJSS_R)
	ranlib ../lib/doublelink.mul.router.ra



depend:
	mkdep $(CXXFLAGS_R) $(SRCS) $(SRCC)

clean:
	rm -f *.o
	rm -f *.ro
	rm -f ../lib/doublelink.mul.router.a

install:

distclean: clean
	rm -f .depend

#	vim: set ts=4 sts=4 syn=make :
#
