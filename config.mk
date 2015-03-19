LIBALF_PREFIX?=/usr/local
LIBALF_LIBDIR?=${LIBALF_PREFIX}/lib
LIBALF_INCLUDE?=${LIBALF_PREFIX}/include

JAVA_INCLUDE = ${JAVA_HOME}/include
JNI_INCLUDE = ${JAVA_INCLUDE}/darwin


ifndef OS
	OS=$(shell uname -s)
endif

ifeq (${OS}, Windows_NT) # Windows
	LIBEXT=dll
	LDFLAGS+=-lws2_32
else ifeq (${OS}, Darwin) # Mac OS
	LIBEXT=dylib
else ifeq (${OS}, Linux) # Linux
	LIBEXT=so
else
	$(error Unsupported operating system ${OS})
endif

# Skip lib prefix on Windows
ifeq (${OS}, Windows_NT)
	LIBPREFIX=
else
	LIBPREFIX=lib
endif

# Use ginstall in Mac OS X
ifeq (${OS}, Darwin)
	INSTALL?=ginstall
else
	INSTALL?=install
endif

# ldconfig needs to be run on Linux
ifeq (${OS}, Linux)
	RUN_LDCONFIG=test `id -u` -eq 0 && /sbin/ldconfig
else
	RUN_LDCONFIG=
endif