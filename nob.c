#define NOBUILD_IMPLEMENTATION
#include "nob.h"

#define STRINGIFY(x) XSTRINGIFY(x)
#define XSTRINGIFY(x) #x

// TODO: maybe make debugging an argument
#define DEBUG 1
#define DEFINE_MACRO(name) "-D" #name "=" STRINGIFY(DEBUG)
#define OPENGL_DEBUG_APP DEFINE_MACRO(OPENGL_DEBUG_APP)

#define BUILD "build"
#define ENGINE "engine"
#define IMPLEMENTATION "implementations"
#define THIRD_PARTY "external"
#define SRC "src"

#define CFLAGS "-Wall", "-Wextra", "-ggdb"
// TODO: not sure if "-I" is an linker flag? but its included in compile_commands.json so ill keep it here for now
// TOOD: maybe use `pkg-config --static --libs glfw3` instead?
#define LDFLAGS "-Lbuild", "-Iexternal", "-Iexternal/include", "-Iexternal/cglm/include", "-Isrc", "-l:glad.o", "-lglfw", "-lGL", "-lm", "-lengine", "-l:stb.o"
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

void build_dep_glad(void) {
    CMD("cc", LDFLAGS, "-c", "-o", PATH(BUILD, "glad.o"), PATH(THIRD_PARTY, "include", "glad", "glad.c"));
}

void build_dep_stb(void) {
    CMD("cc", LDFLAGS, "-c", "-o", PATH(BUILD, "stb.o"), PATH(SRC, ENGINE, IMPLEMENTATION, "stb.c"));
}

#define ENGINE_BUILD(translation_unit) \
    const Cstr translation_unit = PATH(BUILD, #translation_unit ".o"); \
    CMD("cc", CFLAGS, OPENGL_DEBUG_APP, "-c", "-o", translation_unit, PATH(SRC, ENGINE, #translation_unit ".c"), LDFLAGS);

void build_dep_engine() {
    // TODO: simplify further
    ENGINE_BUILD(general);
    ENGINE_BUILD(shader);
    ENGINE_BUILD(camera);
    ENGINE_BUILD(renderer);

    CMD("ar", "rcs", PATH(BUILD, "libengine.a"), general, shader, camera, renderer);
}

void build_dependencies(void) {
    INFO("building glad"); build_dep_glad();
    INFO("building stb"); build_dep_stb();
    INFO("building engine"); build_dep_engine();
}

void build_familiar(void) {
    CMD("cc", CFLAGS, OPENGL_DEBUG_APP, "-o", PATH(BUILD, "familiar"), PATH("src", "main.c"), LDFLAGS);
}

// NOTE: very bare bones just so clangd can pick up on stuff
void create_compile_commands(void) {
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
        // stupid formatting but works cause order of appending `LDFLAGS_DELIM`
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

    INFO("creating \"compile_commands.json\""); create_compile_commands();
    CMD("mkdir", "-p", BUILD);
    INFO("building dependencies"); build_dependencies();
    INFO("building familiar"); build_familiar();

    if(argc >= 2 && strcmp(argv[1], "run") == 0)
        CMD(PATH(BUILD, "familiar"));

    return 0;
}
