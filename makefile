.PHONY: all, clean

all:
	make -C src/decoder
	make -C src/encoder

clean:
	rm -rf build/
