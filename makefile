
BASEDIR = .

BINARY = FindKey

SOURCES = main.cpp DES.cpp 

OBJECTS = ${SOURCES:.cpp=.o} 

INCLUDES = -I .  -I /usr/include 

LOCATIONS =  -L/usr/local/lib  -L/usr/lib 

LIBRARIES =   -lpthread

CXXFLAGS = -ggdb 
#CXXFLAGS = -O2  
CXX = g++ ${CXXFLAGS} -DREENTRANT -D_REENTRANT 

.SUFFIXES:      .cpp .o

.cpp.o:
		@echo
		@echo Building $@
		${CXX} ${INCLUDES} -c -o $@ $<          

all:            ${OBJECTS} ${BINARY} 

${BINARY}:      ${OBJECTS}
		@echo
		@echo Building ${BINARY} Executable
		${CXX} -o $@ \
		${OBJECTS}  \
		${LIBRARIES} \
		${LOCATIONS}
                         
clean:
		rm -f ${BINARY} *.o



