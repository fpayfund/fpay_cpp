DEBUG =yes

CC = gcc
CXX = g++
CC32 = gcc


COMM_LIB = ./net/lib/corelib.a ./net/lib/sox.a 

#HIREDIS_LIB = ../../hiredis++/lib/libredisclient.a

CXXFLAGS = -Wall -D__STDC_LIMIT_MACROS

ifeq (yes,${DEBUG})
	CXXFLAGS := ${CXXFLAGS} -O0 -ggdb 
else
	CXXFLAGS := ${CXXFLAGS}
endif

LINK_CXXFLAG = $(CXXFLAGS) -Wl,-rpath,./bin

INCLUDE =  -I./ -I./net -I./helper/ -I./client/ -I./protocol/ -I./server 

SRC_COMM =./cache/RedisClient.cpp \
	./cache/Cache.cpp \
	./client/FPayClientCore.cpp \
	./server/FPayServer.cpp \
	./server/FPayServerCore.cpp \
	./protocol/fpay_protocol.cpp \
	./helper/ecc_helper.cpp \
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
	/usr/lib/libboost_thread.so \
	/usr/local/lib/libthrift.a \
	/usr/local/lib/libthriftnb.a \
	$(HIREDIS_LIB) \
        $(JSON_INCLUDE)/libjson_linux-gcc-4.3.3_libmt.a \
	/usr/lib/libcrypto.a \
        $(METRICS)/libtrace.a \
	-lpthread \
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

