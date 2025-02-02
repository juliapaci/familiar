// TODO: upgrade to nob (new tsoding build system. successor to nobuild)

#define NOBUILD_IMPLEMENTATION
#include "nob.h"

#include <ctype.h>

#define STRINGIFY(x) XSTRINGIFY(x)
#define XSTRINGIFY(x) #x

// TODO: maybe make debugging an argument
#define DEBUG 1
#if DEBUG==1
#define OPTIMISATION "-ggdb"
#define DEFINE_MACRO(name) "-D" #name "=" STRINGIFY(DEBUG)
#define OPENGL_DEBUG_APP DEFINE_MACRO(OPENGL_DEBUG_APP)
#else
#define OPTIMISATION "-O2"
#define OPENGL_DEBUG_APP ""
#endif // DEBUG


#define SRC "src"
#define ENGINE "engine"
#define EXAMPLE "examples"
#define THIRD_PARTY "external"
#define BUILD "build"
#define ZIGLIB "lib"
#define ZIGBIN "bin"

#define CFLAGS "-Wall", "-Wextra", "-Wno-missing-braces", OPTIMISATION
// TODO: not sure if "-I" is an linker flag? but its included in compile_commands.json so ill keep it here for now
// TOOD: maybe use `pkg-config --static --libs glfw3` instead?
#define LDFLAGS "-L"BUILD, "-I"THIRD_PARTY, CONCAT("-I", PATH(THIRD_PARTY, "gl")), CONCAT("-I", PATH(THIRD_PARTY, "cglm", "include")), "-I"SRC, "-lglad", "-lglfw", "-lGL", "-lm", "-lengine", "-lstb"
#define LDFLAGS_DELIM "\", \""

void build_dep_glad(void) {
    CMD("cc", LDFLAGS, "-c", "-o", PATH(BUILD, "glad.o"), PATH(THIRD_PARTY, "gl", "glad", "glad.c"));
    CMD("ar", "rcs", PATH(BUILD, "libglad.a"), PATH(BUILD, "glad.o"));
}

void build_dep_stb(void) {
    FILE *impl = fopen(PATH(BUILD, "stb_implementations.c"), "w");
    if(impl == NULL)
        return;

    FOREACH_FILE_IN_DIR(file, PATH(THIRD_PARTY, "stb"), {
        if(file[0] == '.') // ".", ".."
            continue;

        const char *header = CONCAT("#include ", "<", file, ">\n");

        char *name = (char *)file;
        *(name + (strlen(name) - strlen(".h"))) = '\0';

        for(size_t i = 0; i < strlen(name); i++)
            name[i] = toupper(name[i]);

        fputs(CONCAT("#define ", name, "_IMPLEMENTATION\n"), impl);
        fputs(header, impl);
    });
    fclose(impl);

    CMD("cc", CONCAT("-I", PATH(THIRD_PARTY, "stb")), "-c", "-o", PATH(BUILD, "stb.o"), PATH(BUILD, "stb_implementations.c"));
    CMD("ar", "rcs", PATH(BUILD, "libstb.a"), PATH(BUILD, "stb.o"));
}

#define ENGINE_BUILD_STATIC(translation_unit) \
    const Cstr translation_unit = PATH(BUILD, #translation_unit ".o"); \
    CMD("cc", CFLAGS, OPENGL_DEBUG_APP, "-c", "-o", translation_unit, PATH(SRC, ENGINE, #translation_unit ".c"), LDFLAGS);

#define ENGINE_BUILD_DYNAMIC(translation_unit) \
    const Cstr translation_unit = PATH(BUILD, #translation_unit ".o"); \
    CMD("cc", CFLAGS, "-fpic", OPENGL_DEBUG_APP, "-c", "-o", translation_unit, PATH(SRC, ENGINE, #translation_unit ".c"), LDFLAGS);

void build_dep_engine() {
    // TODO: simplify further
    INFO("ENGINE: building c parts");
    {
        ENGINE_BUILD_STATIC(general);
        ENGINE_BUILD_STATIC(shader);
        ENGINE_BUILD_STATIC(camera);
        ENGINE_BUILD_STATIC(renderer);
        ENGINE_BUILD_STATIC(animation);
        ENGINE_BUILD_STATIC(utilities);

        CMD("ar", "rcs", PATH(BUILD, "libengine.a"), general, shader, camera, renderer, animation, utilities);

        INFO("ENGINE: buliding zig parts");
        CMD("zig", "build", "--prefix", "build");

        CMD("ar", "rcs", PATH(BUILD, "libengine.a"), PATH(BUILD, ZIGLIB, "libtext.a"));
    }

#if 0
    {
        ENGINE_BUILD_DYNAMIC(general);
        ENGINE_BUILD_DYNAMIC(shader);
        ENGINE_BUILD_DYNAMIC(camera);
        ENGINE_BUILD_DYNAMIC(renderer);
        ENGINE_BUILD_DYNAMIC(animation);
        ENGINE_BUILD_DYNAMIC(utilities);
        CMD("cc", "-shared", "-o", PATH(BUILD, "libengine.so"), general, shader, camera, renderer, animation, utilities, PATH(BUILD, ZIGLIB, "libtext.a"));
    }
#endif
}

void build_dependencies(void) {
    INFO("building glad"); build_dep_glad();
    INFO("building stb"); build_dep_stb();
    INFO("building engine"); build_dep_engine();
}

void build_examples(void) {
    FOREACH_FILE_IN_DIR(file, PATH(SRC, EXAMPLE), {
        if(ENDS_WITH(file, ".c")) {
            INFO("building example \"%s\"", file);
            CMD("cc", CFLAGS, "-o", PATH(BUILD, CONCAT("example_", NOEXT(file))), PATH(SRC, EXAMPLE, file), LDFLAGS);
        }
    });
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
        "["                                                 "\n"
        "\t"    "{"                                         "\n"
        "\t\t"      "\"directory\": \"%s\","                "\n"
        // stupid formatting but works cause order of appending `LDFLAGS_DELIM`
        "\t\t"      "\"arguments\": [\"/usr/bin/cc%s\"],"   "\n"
        "\t\t"      "\"file\": \"N/A\""                     "\n"
        "\t"    "}"                                         "\n"
        "]\n",
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
    INFO("building examples"); build_examples();

    if(argc >= 3 && strcmp(argv[1], "example") == 0) {
        Cstr prog = CONCAT("example_", argv[2]);
        // this doesnt work because the first one can panic with CMD
        // CMD(PATH(BUILD, prog), "||", PATH(BUILD, ZIGBIN, prog));
        if(path_exists(PATH(BUILD, prog))) CMD(PATH(BUILD, prog));
        else if(path_exists(PATH(BUILD, ZIGBIN, prog))) CMD(PATH(BUILD, ZIGBIN, prog));
    } else
        INFO("note that you can run examples with \"%s example <example name>\"", argv[0]);

    return 0;
}
