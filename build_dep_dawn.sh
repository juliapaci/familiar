# https://dawn.googlesource.com/dawn/+/HEAD/docs/building.md

cd external/dawn

[ ! -f DEPS ] && python tools/fetch_dawn_dependencies.py --use-test-deps

mkdir -p out/Debug
cd out/Debug
cmake -GNinja ../..
ninja # or autoninja

cd ../.. # not really needed but whatever
