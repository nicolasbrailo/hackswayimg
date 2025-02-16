.PHONY: clean deploytgt

all: hackimg

clean:
	rm -rf build hackimg

deploytgt: hackimg
	scp ./hackimg batman@10.0.0.146:/home/batman/picframe/hackimg


SYSROOT=/home/batman/src/xcomp-rpiz-env/mnt/
XCOMPILE=\
	 -target arm-linux-gnueabihf \
	 -mcpu=arm1176jzf-s \
	 --sysroot $(SYSROOT)

# Use SYSROOT=/ for local build includes
#SYSROOT=/
#XCOMPILE=

CFLAGS= \
       $(XCOMPILE) \
       -I./src/ \
       -isystem ./build \
       -isystem $(SYSROOT)/usr/include/freetype2 \
       -isystem $(SYSROOT)/usr/include/json-c \
       -Wall -Werror -Wextra -Wpedantic \
       -Wno-strict-prototypes \
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
       -ggdb -O0 \
       -std=c99 \
       -fdiagnostics-color=always \
       -D_FILE_OFFSET_BITS=64 \
       -D_POSIX_C_SOURCE=200809 \
       -pthread \

# static linking like this seems to mess up loader, so run with /lib/ld-linux-armhf.so.3 ./hackimg
LDFLAGS=\
				  -L $(SYSROOT)/usr/lib/arm-linux-gnueabihf \
					-static \
					-Wl,--as-needed \
					-Wl,--no-undefined \
					-Wl,-O1 \
					-Wl,--start-group \
					-lrt \
					-ljson-c \
					-lwebp \
					-lwebpdemux \
					-ljpeg \
	        -lexpat \
					-lfreetype \
					-lfontconfig \
					-ltiff \
					-lpng16 \
					-lexif \
	        -lz \
          -llzma \
					-lzstd \
	        -ldeflate \
          -ljbig \
          -lLerc \
          -lbrotlicommon \
          -lbrotlidec \
					-lm \
					-pthread \
					-lstdc++ \
					-Wl,-Bdynamic \
					-lcurl \
					-lwayland-client \
					-lxkbcommon \
					-Wl,--end-group \
					-v

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
		 build/xdg-shell-protocol.o \
     build/src/formats/bmp.o \
     build/src/formats/png.o \
     build/src/formats/jpeg.o \
     build/src/formats/pnm.o \
     build/src/formats/tga.o \
     build/src/formats/tiff.o \
     build/src/formats/webp.o \
		 build/src/formats/loader.o \
		 build/src/canvas.o \
		 build/src/config.o \
		 build/src/exif.o \
		 build/src/font.o \
		 build/src/image.o \
		 build/src/imagedownloader.o \
		 build/src/imagelist.o \
		 build/src/imageprefetcher.o \
		 build/src/info.o \
		 build/src/keybind.o \
		 build/src/main.o \
		 build/src/pixmap.o \
		 build/src/str.o \
		 build/src/sway.o \
		 build/src/ui.o \
		 build/src/viewer.o
	clang $(CFLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: xcompile-start xcompile-end xcompile-rebuild-sysrootdeps

xcompile-start:
	./rpiz-xcompile/mount_rpy_root.sh ~/src/xcomp-rpiz-env

xcompile-end:
	./rpiz-xcompile/umount_rpy_root.sh ~/src/xcomp-rpiz-env

install_sysroot_deps:
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/c/curl/libcurl4-openssl-dev_7.88.1-10+rpi1+deb12u8_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://archive.raspberrypi.com/debian/pool/main/w/wayland/libwayland-dev_1.22.0-2.1~bpo12+rpt1_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libx/libxkbcommon/libxkbcommon-dev_1.5.0-1_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libe/libexif/libexif12_0.6.24-1+b2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libe/libexif/libexif-dev_0.6.24-1+b2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/f/fontconfig/fontconfig_2.14.1-4_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/f/fontconfig/libfontconfig-dev_2.14.1-4_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/f/freetype/libfreetype-dev_2.12.1+dfsg-5+deb12u3_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/f/freetype/libfreetype6_2.12.1+dfsg-5+deb12u3_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libj/libjpeg-turbo/libjpeg-dev_2.1.5-2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libj/libjpeg-turbo/libjpeg62-turbo-dev_2.1.5-2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libj/libjpeg-turbo/libjpeg62-turbo_2.1.5-2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libp/libpng1.6/libpng-dev_1.6.39-2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libp/libpng1.6/libpng-tools_1.6.39-2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libp/libpng1.6/libpng16-16_1.6.39-2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/t/tiff/libtiff6_4.5.0-6+deb12u2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/t/tiff/libtiff-dev_4.5.0-6+deb12u2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libw/libwebp/libwebp-dev_1.2.4-0.2+deb12u1_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/j/json-c/libjson-c-dev_0.16-2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/b/brotli/libbrotli1_1.0.9-2+b3_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/b/brotli/libbrotli-dev_1.0.9-2+b3_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/x/xz-utils/liblzma-dev_5.4.1-0.2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libz/libzstd/libzstd-dev_1.5.4+dfsg2-5_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/libd/libdeflate/libdeflate-dev_1.14-1_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/l/lerc/liblerc-dev_4.0.0+ds-2_armhf.deb
	./rpiz-xcompile/add_sysroot_pkg.sh ~/src/xcomp-rpiz-env http://raspbian.raspberrypi.com/raspbian/pool/main/j/jbigkit/libjbig-dev_2.1-6.1_armhf.deb

