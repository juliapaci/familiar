const std = @import("std");

const c = @cImport({ @cInclude("engine/renderer.h"); });

// for https://github.com/ziglang/zig/issues/22766

test "sizeof" {
    try std.testing.expectEqual(@sizeOf(c.Renderer), c.renderer_sizeof);
}

test "alignof" {
    try std.testing.expectEqual(@alignOf(c.Renderer), c.renderer_alignof);
}
