# hackswayimg

Hacked [Swayimg](https://github.com/artemsen/swayimg) for my [homeboard project](https://nicolasbrailo.github.io/blog/projects_texts/24homeboard.html). Hackswayimg is a fork of Swayimg, and has different features. (TODO, list a few)

## Building

1. git clone this repo, including submodules
1. `make xcompile-start` to mount or create and mount a new [cross-compiler environment for RpiW](https://github.com/nicolasbrailo/rpiz-xcompile).
1. `make xcompile-rebuild-sysrootdeps` to download to the xcompiler env all the packages needed to build Hackswayimg.
1. `make hackimg`. If everything worked, the binary hackimg should be built. `file hackimg` should say it's an ARM binary.
1. scp to your target (or, optionally, modify the `deploytgt` target, and just `make deploytgt`)

## Running

TODO

