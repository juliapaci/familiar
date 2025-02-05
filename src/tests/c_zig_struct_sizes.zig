const std = @import("std");

const c = @cImport({
    @cInclude("engine/general.h");
    @cInclude("engine/renderer.h");
});

test "c renderer struct is the same size as the zig generated one" {
    std.debug.print("size:\n\tzig @sizeOf: {}\n\t@divExact(@bitSizeOf, 8): {}\n\tc sizeof: {}\n", .{
        @sizeOf(c.Renderer),
        @divExact(@bitSizeOf(c.Renderer), 8),
        c.size_of_renderer
    });

    try std.testing.expect(@sizeOf(c.Renderer) == c.size_of_renderer);
}

test "cant access lower fields of the struct from c" {
    // `c.do_something_with_the_entire_struct` will probably fail at runtime, im pretty sure its UB so it might not
    // if its still not panicking then try the "text" example a couple times (`./nob example text`) for a real world example

    _ = c.init_window("just creating this to init glad. not really related to the test") orelse return;
    defer c.glfwTerminate();

    const allocator = std.heap.page_allocator;
    const renderer_h = try allocator.create(c.Renderer);
    defer allocator.destroy(renderer_h);
    c.do_something_with_the_entire_struct(renderer_h);

    var renderer_s: c.Renderer = undefined;
    c.do_something_with_the_entire_struct(&renderer_s);
}
