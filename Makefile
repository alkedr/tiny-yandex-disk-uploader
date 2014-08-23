SHELL := bash

tiny-yandex-disk-uploader: main.c Makefile
	clang -std=c11 -O0 -ggdb3 -Weverything -Wno-c++98-compat -Wno-documentation -Wno-padded  `pkg-config --cflags --libs gtk+-3.0` -lssl -lcrypto -lcairo -lX11 $< -o $@

tiny-screenshooter: main2.c Makefile
	clang -std=c11 -Oz -Weverything -Wno-padded -Wno-unused-parameter -lX11 -lGL -lGLU -lglut $< -o $@
	upx --best --ultra-brute --lzma -qqq $@

clean:
	rm -f tiny-yandex-disk-uploader
