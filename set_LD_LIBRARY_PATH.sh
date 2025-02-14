# such a hacky fix that cant even be called from main build system
# TODO: fix root issue or put webgpu lib in /usr/lib or something
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:build
