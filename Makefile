include Makefile.inc

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
    ./thirdparts/tinyxml/lib/libtinyxml.a \
	-lssl \
	-lcrypto \
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

