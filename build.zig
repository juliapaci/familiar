// TODO: shared constants for linkage, files, etc. with nob build system

const std = @import("std");

// TODO: some linkijng issues so we arent using this directly via a static/dynamic library but rather we are using the c libraries directly
fn linkGraphics(b: *std.Build, compile: *std.Build.Step.Compile) void {
    compile.linkSystemLibrary("glfw");
    compile.linkSystemLibrary("GL");
    compile.addSystemIncludePath(b.path("external").path(b, "gl"));
    compile.addLibraryPath(b.path("build"));
    compile.linkLibC();
    compile.addCSourceFile(.{ .file = b.path("external/gl/glad/glad.c") });
}

fn linkEngineSource(b: *std.Build, compile: *std.Build.Step.Compile) void {
    // "-Lbuild", "-Iexternal", "-Iexternal/gl", "-Iexternal/cglm/include", "-Isrc", "-l:glad.o", "-lglfw", "-lGL", "-lm", "-lengine", "-l:stb.o"
    compile.addIncludePath(b.path("external"));
    compile.addIncludePath(b.path("external/gl"));
    compile.addIncludePath(b.path("external/cglm/include"));
    compile.addIncludePath(b.path("src"));

    compile.addLibraryPath(b.path("build"));

    compile.linkSystemLibrary2("glad", .{ .preferred_link_mode = .static });
    compile.linkSystemLibrary2("glfw", .{ .preferred_link_mode = .dynamic });
    compile.linkSystemLibrary2("GL", .{ .preferred_link_mode = .dynamic });
    compile.linkSystemLibrary2("m", .{ .preferred_link_mode = .dynamic });
    // compile.linkSystemLibrary2("engine", .{ .preferred_link_mode = .static, .weak = true }); // zig seems to link the individual object files generated by zig before the entire archive so we get duplicate symbols
    compile.linkSystemLibrary2("stb", .{ .preferred_link_mode = .static });

    // compile.linkLibC();

    compile.addCSourceFiles(.{
        .root = b.path("src/engine"),
        .files = &[_][]const u8{ "general.c", "shader.c", "camera.c", "renderer.c", "animation.c", "utilities.c" },
        // also we already warn from the c compiling step in nob
        .flags = &[_][]const u8{"-ggdb"}
    });
}

pub fn build(b: *std.Build) !void {
    const alloc = std.heap.page_allocator;
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});


    var engine_parts = std.ArrayList(struct { lpath: std.Build.LazyPath, name: []const u8, module: *std.Build.Module }).init(alloc);
    defer engine_parts.deinit();

    {
        var dir = try std.fs.cwd().openDir("src/engine", .{ .iterate = true });

        var walker = try dir.walk(b.allocator);
        defer walker.deinit();

        while (try walker.next()) |entry| {
            if (!std.mem.eql(u8, std.fs.path.extension(entry.basename), ".zig"))
                continue;

            const name = entry.basename[0..entry.basename.len - ".zig".len];
            const root_source_file = b.path("src/engine").path(b, entry.path);

            // TODO: normal object files would be better because we create the archive later but static libraries have a default installation step so for now its fine
            const obj = b.addStaticLibrary(.{
                .name = name,
                .root_source_file = root_source_file,
                .target = target,
                .optimize = optimize
            });
            // linkGraphics(b, obj);
            linkEngineSource(b, obj);
            b.installArtifact(obj);

            const shared_name = try alloc.alloc(u8, name.len);
            std.mem.copyForwards(u8, shared_name, name);
            try engine_parts.append(.{
                .lpath = obj.getEmittedBin(),
                .name = shared_name,
                .module = b.addModule(name, .{
                    .root_source_file = root_source_file,
                    .target = target,
                    .optimize = optimize
                })
            });
        }
    }

    {
        var dir = try std.fs.cwd().openDir("src/examples", .{ .iterate = true });

        var walker = try dir.walk(b.allocator);
        defer walker.deinit();

        while (try walker.next()) |entry| {
            if (!std.mem.eql(u8, std.fs.path.extension(entry.basename), ".zig"))
                continue;

            var name = try std.ArrayList(u8).initCapacity(std.heap.page_allocator,  "example_".len + entry.basename.len - ".zig".len);
            defer name.deinit();
            try name.appendSlice("example_");
            try name.appendSlice(entry.basename[0..entry.basename.len - ".zig".len]);

            const exec = b.addExecutable(.{
                .name = try name.toOwnedSlice(),
                .root_source_file = b.path("src/examples").path(b, entry.path),
                .target = target,
                .optimize = optimize
            });

            // linkGraphics(b, exec);
            linkEngineSource(b, exec);

            for (engine_parts.items) |part| {
                exec.addLibraryPath(part.lpath);
                // TODO: link library with compile step from engine object binary
                exec.root_module.addImport(part.name, part.module);

                alloc.free(part.name);
            }

            // link Familiar
            // cant statically link? https://github.com/ziglang/zig/issues/9053
            // exec.linkSystemLibrary2("engine", .{ .preferred_link_mode = .static });
            // exec.addObjectFile(b.path("build/libengine.a"));

            b.installArtifact(exec);
        }
    }
}
