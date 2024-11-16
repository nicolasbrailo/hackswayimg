#!/usr/bin/bash

set -euo pipefail

deb_pkgs=(
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

# Path missing .so's
if [ ! -f "$SYSROOT"/usr/lib/arm-linux-gnueabihf/libzstd.so ]; then
  sudo ln -s "$SYSROOT/usr/lib/arm-linux-gnueabihf/libzstd.so.1" "$SYSROOT"/usr/lib/arm-linux-gnueabihf/libzstd.so
fi
if [ ! -f "$SYSROOT"/usr/lib/arm-linux-gnueabihf/libwayland-client.so ]; then
  sudo ln -s "$SYSROOT/usr/lib/arm-linux-gnueabihf/libwayland-client.so.0" "$SYSROOT"/usr/lib/arm-linux-gnueabihf/libwayland-client.so
fi


