PACKS=fish temperature motion get_ip touch_trace mole

TESTS= $(PACKS) show_raw show_raw_N animation show_image touch image_to_raw stop_mcu_flush text

PACK_BIN=app_packs.tgz

all:
	@for i in $(TESTS); do make -C $$i; done

clean:
	@for i in $(TESTS); do make clean -C $$i; done
	rm -f $(PACK_BIN) 

pack:
	tar --exclude *.tgz --exclude build --exclude *.cpp --exclude *.h --exclude Makefile -czvf $(PACK_BIN) $(PACKS)

.PHONY: pack
