LD_FLAGS=-L$(ROOT)/bin -lmug -lpthread -lrt

INC_FLAGS=-I$(ROOT)/include -I$(ROOT)/lib/CImg -I$(ROOT)/lib/libuv/include

C_FLAGS=-O0 -g $(INC_FLAGS)

HAS_LIBUV = $(shell ls -d $(ROOT)/lib/libuv 2>/dev/null)

LIBMUG=$(ROOT)/bin/libmug.a

ifneq ($(HAS_LIBUV), )
C_FLAGS+= -DUSE_LIBUV
LD_FLAGS+=-L$(ROOT)/lib/libuv/out/Debug -L$(ROOT)/lib/libuv/out/Release -luv
else
endif
