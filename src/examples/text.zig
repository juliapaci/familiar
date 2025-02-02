const std = @import("std");
// TODO: temporary import before we modularize the zig stuff from the c stuff
const text = @import("text");
const familiar = @cImport({
    @cInclude("engine/general.h");
    @cInclude("engine/renderer.h");
});
// const externs = @import("externs");
// const familiar = @import("engine");

pub fn main() !void {
    var fr = try text.init("assets/OpenSans-VariableFont_wdth,wght.ttf");
    defer fr.deinit();
    try fr.parseFont();

    const window = familiar.init_window("Familiar text example (zig)") orelse return;
    defer familiar.glfwTerminate();

    const allocator = std.heap.page_allocator;
    const renderer_a = try allocator.alloc(familiar.Renderer, 1);
    defer allocator.free(renderer_a);
    var renderer = renderer_a[0];
    std.debug.print("renderer zig print: {*}\n", .{&renderer});
    familiar.render_init(&renderer);
    defer familiar.render_free(&renderer);

    while(familiar.glfwWindowShouldClose(window) == 0) {
        familiar.glClearColor(0.1, 0.1, 0.1, 1.0);
        familiar.glClear(familiar.GL_COLOR_BUFFER_BIT);

        familiar.render_frame_begin(&renderer); {
            familiar.render_switch_object(&renderer, familiar.OBJECT_CIRCLE);
            familiar.render_switch_2d(&renderer);

            familiar.render_draw_circle(&renderer, .{
                .x = 0,
                .y = 0,
                .radius = @floatCast(familiar.sin(familiar.glfwGetTime()))
            });
        } familiar.render_frame_end(&renderer);

        familiar.process_general_input(window);
        familiar.glfwSwapBuffers(window);
        familiar.glfwPollEvents();
    }
}
