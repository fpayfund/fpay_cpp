CXXFLAGS1 = $(CXXFLAGS) -DXML_NULL -DTIXML_USE_STL
CXXFLAGS1_R = $(CXXFLAGS_R) -DXML_NULL -DTIXML_USE_STL
SRCS =  tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp 
ARRU = ar -ru

OBJS = $(SRCS:.cpp=.o)
OBJC = $(SRCC:.c=.o)
OBJSS = $(OBJS) $(OBJC) #ntesclient.object

OBJS_R = $(SRCS:.cpp=.ro)
OBJC_R = $(SRCC:.c=.ro)
OBJSS_R = $(OBJS_R) $(OBJC_R)

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) -g $(CXXFLAGS1) $(INCLUDE) -c -o $@ $<

.SUFFIXES: .ro .c
.c.ro:
	$(CXX32) $(CXXFLAGS1_R)  $(INCLUDE) -c -o $@ $<

.SUFFIXES: .o .cpp
.cpp.ro:
	$(CXX32) $(CXXFLAGS1_R) -O3 $(INCLUDE)  -c -o $@ $<

all: tinyxml.a 
release: tinyxml.ra

tinyxml.a: $(OBJS) $(OBJC)
	mkdir -p ./lib
	$(ARRU) ./lib/libtinyxml.a $(OBJSS)
	ranlib ./lib/libtinyxml.a

tinyxml.ra: $(OBJS_R) $(OBJC_R)
	$(ARRU) ../tinyxml.ra $(OBJSS_R)
	ranlib ../tinyxml.ra

test: $(OBJS) $(OBJC)
	$(CXX) -g -o $@ $(LINK_CXXFLAG) $(INCLUDE) $(OBJSS) $(COMM_LIB)
depend:
	mkdep $(CXXFLAGS_R) $(SRCS) $(SRCC)

clean:
	rm -f *.o
	rm -f *.ro

install:
	mkdir -p ./include/tinyxml/
	cp tinyxml.h ./include/tinyxml

distclean: clean
	rm -f .depend

#	vim: set ts=4 sts=4 syn=make :
#
