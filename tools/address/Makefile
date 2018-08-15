include ../../Makefile.inc

COMM_LIB = ../../net/lib/corelib.a ../../net/lib/sox.a 
HIREDIS_LIB = ../../thirdparts/hiredis/libhiredis.a
SRC_COMM =../../helper/ecc_helper.cpp \
	./createAddressInfo.cpp 
  
INCLUDE_SELF = -I../../ -I../../thirdparts -I../../protocol -I../../cache -I../../helper -I../../net -I../../net/common -I../../net/core/sox -I../../net/core/corelib
OBJ_COMM = $(SRC_COMM:.cpp=.o)

LIB = $(COMM_LIB)

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(INCLUDE_SELF) -c -o $@ $<


PROGRAM = tool_create_address #serviced
all: tool_create_address #serviced

tool_create_address:  $(OBJ_COMM) $(LIB) $(SVC_COMN_LIB) $@ 
	$(CXX) -o $@ $(LINK_CXXFLAG) $(INCLUDE) $(INCLUDE_SELF) $(OBJ_COMM) $(SVC_COMN_LIB) $(LIB) \
	$(HIREDIS_LIB) \
    ../../thirdparts/tinyxml/lib/libtinyxml.a \
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
	rm -f ./protocol/*.o
	rm -f ./cache/*.o
	rm -f ./helper/*.o
rebuild:clean all

