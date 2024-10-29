#!/usr/bin/bash

set -euo pipefail

deb_pkgs=(
    http://ftp.uk.debian.org/debian/pool/main/libx/libxkbcommon/libxkbcommon-dev_1.6.0-1+b1_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/libe/libexif/libexif-dev_0.6.24-1+b2_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/f/fontconfig/libfontconfig-dev_2.14.1-4_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/libj/libjpeg-turbo/libjpeg-dev_2.1.5-2_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/libj/libjpeg-turbo/libjpeg62-turbo-dev_2.1.5-2_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/libw/libwebp/libwebp-dev_1.2.4-0.2+deb12u1_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/libw/libwebp/libwebp7_1.2.4-0.2+deb12u1_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/libw/libwebp/libwebpdemux2_1.2.4-0.2+deb12u1_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/libw/libwebp/libwebpmux3_1.2.4-0.2+deb12u1_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/t/tiff/libtiff-dev_4.5.0-6+deb12u1_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/f/freetype/libfreetype-dev_2.12.1+dfsg-5+deb12u3_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/libp/libpng1.6/libpng-dev_1.6.39-2_armhf.deb
    http://ftp.uk.debian.org/debian/pool/main/j/json-c/libjson-c-dev_0.16-2_armhf.deb
)

SYSROOT_TMP=sysroot_deps
SYSROOT="$1"


mkdir -p "$SYSROOT_TMP"

for pkg in "${deb_pkgs[@]}"; do
    pkg_fn="$SYSROOT_TMP/$(basename "$pkg")"
    pkg_extract_dir="$(echo "$pkg_fn" | sed 's/.deb//g')"
    if [ ! -d "$pkg_extract_dir" ]; then
        if [ ! -f "$pkg_fn" ]; then
            echo "$pkg not found, downloading..."
            wget --directory-prefix="$SYSROOT_TMP" "$pkg"
        fi
        echo "Extract $pkg..."
        mkdir -p "$pkg_extract_dir"
        dpkg-deb -R "$pkg_fn" "$pkg_extract_dir"
    fi

    echo "Install $pkg to $SYSROOT"
    if [ -d "$pkg_extract_dir"/usr/include/ ]; then
        sudo cp -r "$pkg_extract_dir"/usr/include/* "$SYSROOT/usr/include/"
    fi
    if [ -d "$pkg_extract_dir"/usr/lib/ ]; then
        sudo cp -r "$pkg_extract_dir"/usr/lib/* "$SYSROOT/usr/lib/"
    fi
    if [ -d "$pkg_extract_dir"/usr/share/ ]; then
        sudo cp -r "$pkg_extract_dir"/usr/share/* "$SYSROOT/usr/share/"
    fi
done

