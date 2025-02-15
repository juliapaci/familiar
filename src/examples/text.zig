const std = @import("std");
// TODO: temporary import before we modularize the zig stuff from the c stuff
const text = @import("text");
const familiar = @cImport({
    @cInclude("engine/general.h");
    @cInclude("engine/renderer.h");
});

fn draw_glyph(
    renderer: *familiar.Renderer,
    glyph: *struct {
        text.GlyphDescription,
        text.GlyphDescription.SimpleDefinition
    },
    offset: [2]f32
) void {
    const glyph_helper = struct {
        const spread = 10;

        glyph: @TypeOf(glyph),
        offset: @TypeOf(offset),
        fn new(g: @TypeOf(glyph), o: @TypeOf(offset)) @This() {
            return .{
                .glyph = g,
                .offset = o
            };
        }

        fn get_x_ord(self: *const @This(), y: i16) f32 {
            return self.glyph[0].clip_ord_x(y)*spread + self.offset[0];
        }

        fn get_y_ord(self: *const @This(), y: i16) f32 {
            return self.glyph[0].clip_ord_y(y)*spread + self.offset[1];
        }

        fn get_x_ord_index(self: *const @This(), index: usize) f32 {
            return get_x_ord(self, self.glyph[1].x_coords.items[index]);
        }

        fn get_y_ord_index(self: *const @This(), index: usize) f32 {
            return get_y_ord(self, self.glyph[1].y_coords.items[index]);
        }
    };
    const gh = glyph_helper.new(glyph, offset);

    { // draw the circle end points
        familiar.render_switch_object(renderer, familiar.OBJECT_CIRCLE);

        for(
            glyph[1].x_coords.items,
            glyph[1].y_coords.items
        ) |x, y| familiar.render_draw_circle(renderer, .{
            .x = gh.get_x_ord(x),
            .y = gh.get_y_ord(y),
            .radius = 0.1
        });

        for(glyph[1].end_contour_points.items) |p| {
            familiar.render_draw_circle(renderer, .{
                .x = gh.get_x_ord_index(p),
                .y = gh.get_y_ord_index(p),
                .radius = @floatCast(0.2*std.math.sin(familiar.glfwGetTime()))
            });
        }
    }

    {
        familiar.render_switch_object(renderer, familiar.OBJECT_LINE_SIMPLE);

        var s: usize = 0; // start
        for(glyph[1].end_contour_points.items) |end| {
            const e = end + 1; // end (+1 for wrap around)
            for(s + 1..e + 1) |p| // (+1 for an inclusive range)
                familiar.render_draw_line(renderer, .{
                    .start = .{.raw = [3]f32{
                        gh.get_x_ord_index(@max(@as(isize, @intCast(p)) - 1, 0)),
                        gh.get_y_ord_index(@max(@as(isize, @intCast(p)) - 1, 0)),
                        0
                    }},
                    .end = .{.raw = [3]f32{
                        gh.get_x_ord_index(if (p == e) s else p),
                        gh.get_y_ord_index(if (p == e) s else p),
                        0
                    }},
                    .thickness = 1.0
                });

            s = e;
        }
    }
}

pub fn main() !void {
    var args = try std.process.argsWithAllocator(std.heap.page_allocator);
    defer args.deinit();
    _ = args.skip();
    var fr = try text.init(args.next() orelse unreachable);
    defer fr.deinit();
    var glyphs = try fr.parseFont();
    defer glyphs[1].deinit();

    const window = familiar.init_window("Familiar text example (zig)") orelse return;
    defer familiar.glfwTerminate();
    familiar.glEnable(familiar.GL_LINE_SMOOTH);

    const allocator = std.heap.page_allocator;
    const renderer = try allocator.create(familiar.Renderer);
    defer allocator.destroy(renderer);

    familiar.render_init(renderer);
    defer familiar.render_free(renderer);
    familiar.set_camera_callback(window, &renderer.camera);

    while(familiar.glfwWindowShouldClose(window) == 0) {
        familiar.glClearColor(0.1, 0.1, 0.1, 1.0);
        familiar.glClear(familiar.GL_COLOR_BUFFER_BIT | familiar.GL_DEPTH_BUFFER_BIT);

        familiar.render_frame_begin(renderer); {
            familiar.render_switch_3d(renderer);
            draw_glyph(renderer, &glyphs, [2]f32{0.0, 0.0});
        } familiar.render_frame_end(renderer);

        familiar.process_camera_input(window, &renderer.camera);
        familiar.process_general_input(window);
        familiar.glfwSwapBuffers(window);
        familiar.glfwPollEvents();
    }
}
