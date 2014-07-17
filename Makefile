SHELL := bash

tiny-yandex-disk-uploader: main.cpp Makefile
	clang++ -std=c++11 -fPIE -O0 -ggdb3 -Weverything -Wno-c++98-compat $(shell pkg-config --cflags --libs Qt5Gui Qt5Widgets Qt5Network | sed 's/-I\//-isystem\ \//g') $< -o $@

#upx --best --ultra-brute --lzma -qqq $@

clean:
	rm -f tiny-yandex-disk-uploader
