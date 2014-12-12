PACKS=fish temperature motion get_ip touch_trace mole show_id mug_shut_down player tile battery drink
TOOLS=stop_mcu_flush mug_shut_down

TESTS= $(PACKS) show_raw show_raw_N animation show_image touch image_to_raw stop_mcu_flush text mug_shut_down show_id tile battery

PACK_BIN=app_packs.tgz
TOOL_BIN=mug_tools.tgz
all:
	@for i in $(TESTS); do make -C $$i; done

clean:
	@for i in $(TESTS); do make clean -C $$i; done
	rm -f $(PACK_BIN) 
	rm -rf $(TOOL_BIN)

pack:
	rm -f *.tgz
	tar --exclude *.tgz --exclude build --exclude *.cpp --exclude *.h --exclude Makefile -czvf $(PACK_BIN) $(PACKS)
	tar --exclude *.tgz --exclude build --exclude *.cpp --exclude *.h --exclude Makefile -czvf $(TOOL_BIN) $(TOOLS)

.PHONY: pack
