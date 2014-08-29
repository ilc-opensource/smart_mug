LD_FLAGS=-L$(ROOT)/bin -lmug -lpthread -lrt

INC_FLAGS=-I$(ROOT)/include -I$(ROOT)/lib/CImg -I$(ROOT)/lib/libuv/include

ifeq ($(Release), 1)
C_FLAGS=-O2 -g $(INC_FLAGS) -fpermissive
else
C_FLAGS=-O0 -g $(INC_FLAGS) -fpermissive
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
