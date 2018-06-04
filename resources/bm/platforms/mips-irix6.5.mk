COMMON_DFLAGS = $(DBGFLAGS)
LINKER_DFLAGS = $(DBGFLAGS)

ifeq ($(COMPILER),GNU_CC)
	COMMON_CXXFLAGS = $(COMMON_DFLAGS) -D_REENTRANT -DSS_64BIT_SERVER
	COMMON_CFLAGS = $(COMMON_DFLAGS) -D_REENTRANT
	COMMON_LDFLAGS = $(LINKER_DFLAGS)
	COMMON_CLDFLAGS = $(COMMON_LDFLAGS)
	EXTERN_LIBS = $(EXTERN_LIBS_BASE)/lib-gnu
	CXX = g++
	CC = cc
	LD = g++
	CC_PIC_FLAGS = -KPIC
	CXX_PIC_FLAGS = -fpic
	SO_FLAGS = -shared
else
	COMMON_CXXFLAGS =  $(COMMON_DFLAGS) -64 -DMIPS_ABI -DBM64OPT -LANG:std -signed -no_auto_include -woff 1107
	COMMON_CFLAGS = $(COMMON_DFLAGS) -64 -signed
	COMMON_LDFLAGS = $(LINKER_DFLAGS) -64 -LANG:std
	COMMON_CLDFLAGS = $(COMMON_LDFLAGS)
	EXTERN_LIBS = $(EXTERN_LIBS_BASE)/lib
	CXX = CC
	CC = cc
	LD = CC -Wl,-woff,84
	CC_PIC_FLAGS = -KPIC
	CXX_PIC_FLAGS = -KPIC
	SO_FLAGS = -shared
	OPT_FLAGS = -O2

	CXX_CACHE = ii_files
#		SYS_LIBS = 
	AR = $(CXX) -ar
	ARFLAGS = -LANG:std -o

endif

SYS_LIBS += -lpthread -lm -lgen
DAEMON_LIBS = -lelf


INSTALL = /usr/local/bin/install
INSTALLDIR = /usr/local/bin/install -d
AWK = awk
TEST = /bin/test

