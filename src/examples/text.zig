const std = @import("std");
// TODO: temporary import before we modularize the zig stuff from the c stuff
const text = @import("text");
const familiar = @cImport({
    @cInclude("engine/general.h");
    @cInclude("engine/renderer.h");
});

pub fn main() !void {
    var fr = try text.init("assets/OpenSans-VariableFont_wdth,wght.ttf");
    defer fr.deinit();
    var glyphs = try fr.parseFont();
    defer glyphs[1].deinit();

    const window = familiar.init_window("Familiar text example (zig)") orelse return;
    defer familiar.glfwTerminate();
    familiar.glEnable(familiar.GL_LINE_SMOOTH);

    // TODO: for some reason the renderer pointer breaks when we pass it to a extern (c) function with a segfault because for some reason we cant access half the struct?? it only happens on certain computers and only sometimes (like sometimes it will run fine, others will crash on render_init, and others will crash half way through when we try to push a vertex). very weird behaviour
    const allocator = std.heap.page_allocator;
    const renderer = try allocator.create(familiar.Renderer);
    defer allocator.destroy(renderer);
    familiar.render_init(renderer);
    defer familiar.render_free(renderer);

    while(familiar.glfwWindowShouldClose(window) == 0) {
        familiar.glClearColor(0.1, 0.1, 0.1, 1.0);
        familiar.glClear(familiar.GL_COLOR_BUFFER_BIT);

        familiar.render_frame_begin(renderer); {
            {
                familiar.render_switch_object(renderer, familiar.OBJECT_CIRCLE);
                familiar.render_switch_2d(renderer);

                for(
                    glyphs[1].x_coords.items,
                    glyphs[1].y_coords.items
                ) |x, y| familiar.render_draw_circle(renderer, .{
                    .x = @floatFromInt(@divFloor(x, @divFloor(glyphs[0].x_max, 10)*2)),
                    .y = @floatFromInt(@divFloor(y, @divFloor(glyphs[0].y_max, 10)*2)),
                    .radius = 0.1
                });
            }

            {
                familiar.render_switch_object(renderer, familiar.OBJECT_LINE);
                familiar.render_switch_2d(renderer);

                familiar.render_draw_line(renderer, .{
                    .start_x = 0,
                    .start_y = 0,
                    .start_z = 0,
                    .end_x = 1,
                    .end_y = @floatCast(std.math.sin(familiar.glfwGetTime())),
                    .end_z = 0,

                    .thickness = 4.0
                });

                familiar.render_draw_lined_rectangle(renderer, .{
                    .x = 0.0,
                    .y = 0.0,
                    .width = 10.0,
                    .height = 10.0
                }, 4.0);

                // const contour_ends = glyphs[1].end_contour_points.items;
                // for(1..contour_ends.len) |c| {
                //     for(
                //         glyphs[1].x_coords.items[contour_ends[c - 1] .. contour_ends[c]],
                //         glyphs[1].y_coords.items[contour_ends[c - 1] .. contour_ends[c]]
                //     ) |x, y| familiar.render_draw_circle(renderer, .{
                //         .x = @floatFromInt(@divFloor(x, @divFloor(glyphs[0].x_max, 10)*2)),
                //         .y = @floatFromInt(@divFloor(y, @divFloor(glyphs[0].y_max, 10)*2)),
                //         .radius = 0.1
                //     });
                // }
            }
        } familiar.render_frame_end(renderer);

        familiar.process_general_input(window);
        familiar.glfwSwapBuffers(window);
        familiar.glfwPollEvents();
    }
}
