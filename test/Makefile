TESTS= show_raw show_raw_N animation show_image motion touch temprature mole

all:
	@for i in $(TESTS); do make -C $$i; done

clean:
	@for i in $(TESTS); do make clean -C $$i; done
