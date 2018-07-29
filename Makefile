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

SRC_COMM =./client/FPayClientCore.cpp \
	./server/FPayServer.cpp \
	./server/FPayServerCore.cpp \
	./server/FPayBlockService.cpp \
	./server/FPayConfig.cpp \
	./server/FPayTXService.cpp \
	./protocol/fpay_protocol.cpp \
	./helper/ecc_helper.cpp \
    ./cache/Cache.cpp \
	./cache/RedisClient.cpp \
    ./flags.cpp \
	./main.cpp 
  

OBJ_COMM = $(SRC_COMM:.cpp=.o)

LIB = $(COMM_LIB)

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<


PROGRAM = fpay_d #serviced
all: fpay_d #serviced

fpay_d:  $(OBJ_COMM) $(LIB) $(SVC_COMN_LIB) $@ 
	$(CXX) -o $@ $(LINK_CXXFLAG) $(INCLUDE) $(OBJ_COMM) $(SVC_COMN_LIB) $(LIB) \
	$(HIREDIS_LIB) \
	-lssl \
	-lrt -ldl -lz

depend:
	mkdep $(INCLUDE) $(SRC_COMM) $(CXXFLAGS)

install:
	install $(PROGRAM) ../bin/
clean:
	$(RM) -r $(PROGRAM) 
	rm -f *.o
	rm -f ./client/*.o
	rm -f ./server/*.o
	rm -f ./protocol/*.o
	rm -f ./cache/*.o
	rm -f ./helper/*.o
rebuild:clean all

