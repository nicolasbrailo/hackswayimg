# Use SYSROOT=/ for local build includes
SYSROOT=/
SYSROOT=/home/batman/src/xcomp-rpiz-env/mnt/

XCOMPILE=-target arm-linux-gnueabihf \
	 -mcpu=arm1176jzf-s \
	 --sysroot $(SYSROOT)

CFLAGS= \
			 $(XCOMPILE) \
       -I./src/ \
			 -isystem ./build \
       -isystem $(SYSROOT)/usr/include/freetype2 \
			 -isystem $(SYSROOT)/usr/include/json-c \
       -Wall -Werror -Wextra -Wpedantic \
       -Wundef \
       -Wmissing-include-dirs \
       -Wpointer-arith \
       -Winit-self \
       -Wfloat-equal \
       -Wredundant-decls \
       -Wimplicit-fallthrough \
       -Wendif-labels \
       -Wstrict-aliasing=2 \
       -Woverflow \
       -Wformat=2 \
			 -Winvalid-pch \
	 	   -O3 \
	     -std=c99 \
       -fdiagnostics-color=always \
	     -D_FILE_OFFSET_BITS=64 \
	     -D_POSIX_C_SOURCE=200809 \
	     -pthread

build/xdg-shell-protocol.c: /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml 
	mkdir -p build
	wayland-scanner private-code $< $@
build/xdg-shell-protocol.h: /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml 
	mkdir -p build
	wayland-scanner client-header $< $@
build/xdg-shell-protocol.o: build/xdg-shell-protocol.c build/xdg-shell-protocol.h
	clang $(CFLAGS) $< -c -o $@

build/src/%.o: src/%.c src/%.h
	mkdir -p build/src/formats
	clang $(CFLAGS) $< -c -o $@

build/src/formats/%.o: src/formats/%.c
	mkdir -p build/src/formats
	clang $(CFLAGS) $< -c -o $@

hackimg: \
		 build/src/action.o \
		 build/src/config.o \
		 build/src/event.o \
		 build/src/exif.o \
		 build/src/font.o \
		 build/src/formats/jpeg.o \
		 build/src/gallery.o \
	   build/xdg-shell-protocol.o \
     build/src/application.o \
     build/src/fetcher.o \
     build/src/formats/bmp.o \
     build/src/formats/png.o \
     build/src/formats/pnm.o \
     build/src/formats/qoi.o \
     build/src/formats/tga.o \
     build/src/formats/tiff.o \
     build/src/formats/webp.o \
     build/src/image.o \
     build/src/imagelist.o \
     build/src/info.o \
     build/src/keybind.o \
     build/src/loader.o \
     build/src/main.o \
     build/src/memdata.o \
     build/src/pixmap.o \
     build/src/sway.o \
     build/src/ui.o \
     build/src/viewer.o
	clang $(CFLAGS) -o $@ $^ \
					-Wl,--as-needed \
					-Wl,--no-undefined \
					-Wl,-O1 \
					-Wl,--start-group \
					-lrt \
					-pthread \
					-lexif \
					-lfontconfig \
					-lfreetype \
					-ljpeg \
					-ljson-c \
					-lpng16 \
					-ltiff \
					-lwayland-client \
					-lwebp \
					-lwebpdemux \
					-lxkbcommon \
					-Wl,--end-group

.PHONY: clean distclean

clean:
	rm -rf build hackimg

distclean: clean
	rm -rf sysroot_deps

.PHONY: xcompile-start xcompile-end xcompile-rebuild-sysrootdeps deploytgt

xcompile-start:
	./rpiz-xcompile/mount_rpy_root.sh ~/src/xcomp-rpiz-env

xcompile-end:
	./rpiz-xcompile/umount_rpy_root.sh ~/src/xcomp-rpiz-env

xcompile-rebuild-sysrootdeps:
	./sysroot_build.sh ~/src/xcomp-rpiz-env/mnt

deploytgt: hackimg
	scp ./hackimg batman@10.0.0.146:/home/batman/picframe/hackimg

