SHELL := bash

tiny-yandex-disk-uploader: main.c Makefile
	clang -std=c11 -O0 -ggdb3 -Weverything -Wno-c++98-compat -lssl -lcrypto -lcairo -lX11 $< -o $@

#upx --best --ultra-brute --lzma -qqq $@

clean:
	rm -f tiny-yandex-disk-uploader
