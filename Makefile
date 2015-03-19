# Makefile for the learnlib-libalf native bridge
# Copyright (c) 2015 TU Dortmund
# This file is part of LearnLib, http://www.learnlib.de/.
# Licensed under LGPLv3, see COPYING.txt for further information.

# Author: Malte Isberner

SRCDIR=src

SOURCES = $(wildcard ${SRCDIR}/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

include ./config.mk

TARGET = ${LIBPREFIX}learnlib-libalf.${LIBEXT}


INCLUDES = include ${LIBALF_INCLUDE} ${JAVA_INCLUDE} ${JNI_INCLUDE}
LIB_DIRS = ${LIBALF_LIBDIR}

CPPFLAGS += $(INCLUDES:%=-I%)
CXXFLAGS += -O3 -fpic

LDFLAGS += -shared
LDFLAGS += $(LIB_DIRS:%=-L%)

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CXX} ${LDFLAGS} -Xlinker ${OBJECTS} ${LIBALF_LIBDIR}/libalf.a -o $@
	strip --strip-unneeded $@

clean:
	-rm -f ${TARGET} ${OBJECTS}

.PHONY: clean
