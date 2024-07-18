# using a shell script for a build system for this no make and cmake since we are keeping it simple
# TODO: ill switch to a custom build system later like nakst essence project nakst/essence instead of my usual tosding/nob

# TODO: build cglm libraries instead of just including the header in the repo
mkdir -p build;
gcc -c glad/glad.c -Iinclude -o build/glad.o
gcc -Wall -Wextra -ggdb src/*.c src/render_base/*.c -Iinclude -Icglm/include -Lbuild -l:glad.o -lglfw -lGL -lm -o build/program

# maybe use `pkg-config --static --libs glfw3` instead?
