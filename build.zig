// TODO: shared constants for linkage, files, etc. with nob build system

const std = @import("std");

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
                .root_source_file = b.path(try std.fs.path.join(std.heap.page_allocator, &.{"src/examples", entry.path})),
                .target = target,
                .optimize = optimize
            });
            exec.linkSystemLibrary("glfw");
            exec.linkSystemLibrary("GL");
            exec.addIncludePath(b.path("external/gl"));
            exec.linkLibC();
            exec.addCSourceFile(.{ .file = b.path("external/gl/glad/glad.c") });
            for (engine_parts.items) |part| {
                exec.addLibraryPath(part.lpath);
                // TODO: link library with compile step from engine object binary
                exec.root_module.addImport(part.name, part.module);

                alloc.free(part.name);
            }
            b.installArtifact(exec);
        }
    }
}
