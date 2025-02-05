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
    std.debug.print("size:\n\tzig @sizeOf: {}\n\t@divExact(@bitSizeOf, 8): {}\n\tc sizeof: {}\n", .{@sizeOf(familiar.Renderer), @divExact(@bitSizeOf(familiar.Renderer), 8), familiar.size_of_renderer});

    // TODO: for some reason the renderer pointer breaks when we pass it to a extern (c) function with a segfault because for some reason we cant access half the struct?? it only happens on certain computers and only sometimes (like sometimes it will run fine, others will crash on render_init, and others will crash half way through when we try to push a vertex). very weird behaviour
    const allocator = std.heap.page_allocator;
    const renderer = try allocator.create(familiar.Renderer);
    defer allocator.destroy(renderer);

    familiar.render_init(renderer);
    defer familiar.render_free(renderer);
    familiar.set_camera_callback(window, &renderer.camera);

    while(familiar.glfwWindowShouldClose(window) == 0) {
        familiar.glClearColor(0.1, 0.1, 0.1, 1.0);
        familiar.glClear(familiar.GL_COLOR_BUFFER_BIT);

        familiar.render_frame_begin(renderer); {
            familiar.render_switch_2d(renderer);

            {
                familiar.render_switch_object(renderer, familiar.OBJECT_CIRCLE);

                for(
                    glyphs[1].x_coords.items,
                    glyphs[1].y_coords.items
                ) |x, y| familiar.render_draw_circle(renderer, .{
                    .x = @floatFromInt(x),
                    .y = @floatFromInt(y),
                    .radius = 0.1
                });

                for(glyphs[1].end_contour_points.items) |p| {
                    familiar.render_draw_circle(renderer, .{
                        .x = @floatFromInt(glyphs[1].x_coords.items[p]),
                        .y = @floatFromInt(glyphs[1].y_coords.items[p]),
                        .radius = 0.2
                    });
                }
            }

            {
                familiar.render_switch_object(renderer, familiar.OBJECT_LINE_SIMPLE);
                var last = [2]f32{
                    @floatFromInt(glyphs[1].x_coords.items[0]),
                    @floatFromInt(glyphs[1].y_coords.items[0]),
                };

                const contour_ends = glyphs[1].end_contour_points.items;
                for(0..contour_ends.len) |c| {
                    for(
                        glyphs[1].x_coords.items[if (c == 0) 0 else contour_ends[c - 1] .. contour_ends[c] - 1],
                        glyphs[1].y_coords.items[if (c == 0) 0 else contour_ends[c - 1] .. contour_ends[c] - 1]
                    ) |x, y| {
                        const current = [2]f32{
                            @floatFromInt(x),
                            @floatFromInt(y),
                        };

                        familiar.render_draw_line(renderer, .{
                            .start = .{.raw = [3]f32{ last[0], last[1], 0 }},
                            .end = .{.raw = [3]f32{ current[0], current[1], 0 }},
                            .thickness = 2.0
                        });

                        last = current;
                    }

                    familiar.render_draw_line(renderer, .{
                        .start = .{.raw = [3]f32 { last[0], last[1], 0 }},
                        .end = .{.raw = [3]f32 {
                            @floatFromInt(glyphs[1].x_coords.items[contour_ends[c]]),
                            @floatFromInt(glyphs[1].x_coords.items[contour_ends[c]]),
                            0
                        }},
                        .thickness = 2.0
                    });
                }
            }
        } familiar.render_frame_end(renderer);

        familiar.process_general_input(window);
        familiar.process_camera_input(window, &renderer.camera);
        familiar.glfwSwapBuffers(window);
        familiar.glfwPollEvents();
    }
}
