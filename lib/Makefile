EDISON = edison
EDISON_ROOT = ../..
EDISON_TARGET = lib/include lib/bin/libiohub-client.a
EDISON_FILES = $(addprefix $(EDISON)/, $(EDISON_TARGET))

LIBUV = libuv
LIBUV_ROOT = ../..
LIBUV_TARGET = AUTHORS ChangeLog LICENSE README.md include out/Debug/libuv.a out/Release/libuv.a
LIBUV_FILES = $(addprefix $(LIBUV)/, $(LIBUV_TARGET))

all: edison libuv

edison:
	rm -rf $(EDISON)
	cd $(EDISON_ROOT); tar -czvf $(EDISON)_pack.tgz $(EDISON_FILES)
	tar -xzvf $(EDISON_ROOT)/$(EDISON)_pack.tgz
	rm -f $(EDISON_ROOT)/$(EDISON)_pack.tgz

libuv:
	rm -rf $(LIBUV)
	cd $(LIBUV_ROOT); tar -czvf $(LIBUV)_pack.tgz $(LIBUV_FILES)
	tar -xzvf $(LIBUV_ROOT)/$(LIBUV)_pack.tgz
	rm -f $(LIBUV_ROOT)/$(LIBUV)_pack.tgz

clean:
	rm -rf $(EDISON) $(LIBUV)

.PHONY: edison libuv
