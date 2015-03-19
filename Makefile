
SRCDIR=src

SOURCES = $(wildcard ${SRCDIR}/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

TARGET = liblearnlib-libalf.dylib

LIBALF_PREFIX?=/usr/local
LIBALF_LIBDIR?=${LIBALF_PREFIX}/lib
LIBALF_INCLUDE?=${LIBALF_PREFIX}/include

JAVA_INCLUDE = ${JAVA_HOME}/include
JNI_INCLUDE = ${JAVA_INCLUDE}/darwin

INCLUDES = include ${LIBALF_INCLUDE} ${JAVA_INCLUDE} ${JNI_INCLUDE}
LIB_DIRS = ${LIBALF_LIBDIR}

CPPFLAGS += $(INCLUDES:%=-I%)
CXXFLAGS += -O3 -fpic

LDFLAGS += -shared
LDFLAGS += $(LIB_DIRS:%=-L%)

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CXX} ${LDFLAGS} -Xlinker ${OBJECTS} ${LIBALF_LIBDIR}/libalf.a -o $@

clean:
	-rm -f ${TARGET} ${OBJECTS}

.PHONY: clean
