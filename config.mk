LIBALF_PREFIX?=/usr/local
LIBALF_LIBDIR?=$(subst \,/,${LIBALF_PREFIX})/lib
LIBALF_INCLUDE?=$(subst \,/,${LIBALF_PREFIX})/include

JAVA_INCLUDE = $(subst \,/,${JAVA_HOME})/include


ifndef OS
	OS=$(shell uname -s)
endif

ifeq (${OS}, Windows_NT) # Windows
	LIBEXT=dll
	LDFLAGS+=-lws2_32
	JNI_INCLUDE = ${JAVA_INCLUDE}/win32
else ifeq (${OS}, Darwin) # Mac OS
	LIBEXT=dylib
	JNI_INCLUDE = ${JAVA_INCLUDE}/darwin
else ifeq (${OS}, Linux) # Linux
	LIBEXT=so
	JNI_INCLUDE = ${JAVA_INCLUDE}/linux
else
	$(error Unsupported operating system ${OS})
endif

# Skip lib prefix on Windows
ifeq (${OS}, Windows_NT)
	LIBPREFIX=
else
	LIBPREFIX=lib
endif

# Use ginstall in Mac OS X, only use strip -x
ifeq (${OS}, Darwin)
	INSTALL?=ginstall
	STRIPFLAGS=-x
else
	INSTALL?=install
	STRIPFLAGS=--strip-unneeded
endif

# ldconfig needs to be run on Linux
ifeq (${OS}, Linux)
	RUN_LDCONFIG=test `id -u` -eq 0 && /sbin/ldconfig
else
	RUN_LDCONFIG=
endif