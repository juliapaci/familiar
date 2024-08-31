#define NOBUILD_IMPLEMENTATION
#include "nob.h"
#include <string.h>

#define BUILD "build"
#define CFLAGS "-Wall", "-Wextra", "-ggdb"
// TODO: not sure if "-I" is an linker flag? but its included in compile_commands.json so ill keep it here for now
// TOOD: maybe use `pkg-config --static --libs glfw3` instead?
#define LDFLAGS "-Lbuild", "-Iexternal/include", "-Iexternal/cglm/include", "-Isrc/render_base", "-Isrc/engine", "-l:glad.o", "-lglfw", "-lGL", "-lm"
#define LDFLAGS_DELIM "\", \""

Cstr all_c_files_in_dir(const char *dir_path) {
    char *files = malloc(1);
    files[0] = 0;
    size_t size = 0;
    if(!ENDS_WITH(dir_path, PATH_SEP))
        dir_path = CONCAT(dir_path, PATH_SEP);
    const size_t path_size = strlen(dir_path);

    FOREACH_FILE_IN_DIR(file, dir_path, {
        if(ENDS_WITH(file, ".c")) {
            size += strlen(file) + path_size + 1;
            files = realloc(files, size);
            files = strcat(files, CONCAT(dir_path, file, " "));
        }
    });

    return files;
}

void build_deps(void) {
    CMD("cc", LDFLAGS, "-c", "-o", PATH(BUILD, "glad.o"), PATH("external", "include", "glad", "glad.c"));
    // TODO: make render_base here aswell and statically link
}

void build_familiar(void) {
    CMD("cc", "-o", PATH(BUILD, "familiar"), CFLAGS, LDFLAGS,
        PATH("src", "main.c"),
        PATH("src", "render_base", "general.c"),
        PATH("src", "render_base", "shader.c"),
        PATH("src", "render_base", "camera.c"),
        PATH("src", "engine", "raycaster.c")
        /* all_c_files_in_dir("src"), all_c_files_in_dir(PATH("src", "render_base")), all_c_files_in_dir(PATH("src", "engine")) */
    );
}

// NOTE: very bare bones just so clangd can pick up on stuff
void create_compile_commands(void) {
    CMD("touch", "compile_commands.json");
    FILE *json = fopen("compile_commands.json", "w");

    const char *ldflags_array[] = {LDFLAGS};
    const size_t ldflags_amount = sizeof(ldflags_array)/sizeof(char *);
    size_t ldflags_size = 0;
    for(size_t i = 0; i < ldflags_amount; i++)
        ldflags_size += strlen(ldflags_array[i]) + strlen(LDFLAGS_DELIM);

    char *ldflags_args = malloc(ldflags_size);
    ldflags_args[0] = '\0';
    for(size_t i = 0; i < ldflags_amount; i++) {
        ldflags_args = strcat(ldflags_args, LDFLAGS_DELIM);
        ldflags_args = strcat(ldflags_args, ldflags_array[i]);
    }

    fprintf(json,
        "["                                     "\n"
        "\t"    "{"                             "\n"
        "\t\t"      "\"directory\": \"%s\","    "\n"
        // stupid formatting but works cause order of appending LDFLAGS_DELIM
        "\t\t"      "\"arguments\": [\"/usr/bin/cc%s\"],"   "\n"
        "\t\t"      "\"file\": \"N/A\""         "\n"
        "\t"    "}"                             "\n"
        "]",
        GETCWD(),
        ldflags_args
    );

    free(ldflags_args);
    fclose(json);
}

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);

    all_c_files_in_dir("src/render_base");
    CMD("mkdir", "-p", BUILD);
    INFO("building dependencies"); build_deps();
    INFO("building familiar"); build_familiar();
    INFO("creating \"compile_commands.json\""); create_compile_commands();

    return 0;
}