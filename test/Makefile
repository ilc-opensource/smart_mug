TESTS= show_raw show_raw_N animation show_image motion touch temprature mole image_to_raw fish get_ip draw stop_mcu_flush

all:
	@for i in $(TESTS); do make -C $$i; done

clean:
	@for i in $(TESTS); do make clean -C $$i; done
