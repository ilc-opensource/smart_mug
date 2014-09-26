TP_TYPE=-DFOCALTECH=1

LD_FLAGS=-L$(ROOT)/bin -L$(ROOT)/lib/freetype2/bin -lmug -lpthread -lrt -lfreetype

INC_FLAGS=-I$(ROOT)/include -I$(ROOT)/lib/CImg -I$(ROOT)/lib/libuv/include -I$(ROOT)/lib/freetype2/include

C_FLAGS= $(INC_FLAGS) -fpermissive $(TP_TYPE)

ifeq ($(Release), 1)
C_FLAGS +=-O2 
else
C_FLAGS +=-O0 -g 
endif

HAS_LIBUV = $(shell ls -d $(ROOT)/lib/libuv 2>/dev/null)

LIBMUG=$(ROOT)/bin/libmug.a

ifneq ($(HAS_LIBUV), )
C_FLAGS+= -DUSE_LIBUV

ifeq ($(Release), 1)
LD_FLAGS+=-L$(ROOT)/lib/libuv/out/Release -luv
else
LD_FLAGS+=-L$(ROOT)/lib/libuv/out/Debug -luv
endif

else
endif
