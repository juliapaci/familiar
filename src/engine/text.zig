const std = @import("std");

const NoEofError = std.fs.File.ReadError || error{
    EndOfStream,
};
const native_endian = @import("builtin").target.cpu.arch.endian();

const FontReader = @This();
// sfnt data types
const FWord = i16;

file: std.fs.File,

// waiting on https://github.com/ziglang/zig/pull/21601
// for something like `try fr.file.reader().readStructEndian(@This(), .big)`
// but for now this will do:
pub fn readPackedStructBigEndian(self: *FontReader, comptime T: type) NoEofError!T {
    const bytes = try self.file.reader().readBytesNoEof(@divExact(@bitSizeOf(T), 8));
    var result: T = undefined;
    @memcpy(std.mem.asBytes(&result)[0..@divExact(@bitSizeOf(T), 8)], bytes[0..@divExact(@bitSizeOf(T), 8)]);
    if (native_endian != .big) std.mem.byteSwapAllFields(T, &result);
    return result;
}

inline fn MakeStructParse(comptime T: type) fn (fr: *FontReader) NoEofError!T {
    return struct {
        fn parse(fr: *FontReader) !T {
            // return try fr.file.reader().readStructEndian(@This(), .big);
            return fr.readPackedStructBigEndian(T);
        }
    }.parse;
}

pub fn init(ttf_path: []const u8) !FontReader {
    return .{
        .file = try std.fs.cwd().openFile(ttf_path, .{})
    };
}

pub fn deinit(self: *FontReader) void {
    self.file.close();
}

pub const OffsetSubtable = packed struct {
    scalar_type: u32,
    num_tables: u16,
    search_range: u16,
    entry_selector: u16,
    range_shift: u16,

    const parse = MakeStructParse(@This());
};

// TODO: better way than wrapper functions to call nested methods with parent structure (`self`) as first arg?
pub inline fn parseOffsetSubtable(self: *FontReader) @typeInfo(@TypeOf(OffsetSubtable.parse)).Fn.return_type.? {
    return OffsetSubtable.parse(self);
}

pub const TableDirectory = packed struct {
    tag: u32,
    checksum: u32,
    offset: u32,
    length: u32,

    const parse = MakeStructParse(@This());

    pub fn tagToString(tag: u32) [4]u8 {
        return if (native_endian == .little) .{
            @truncate(tag >> 24),
            @truncate(tag >> 16),
            @truncate(tag >> 8),
            @truncate(tag >> 0)
        } else .{
            @truncate(tag >> 0),
            @truncate(tag >> 8),
            @truncate(tag >> 16),
            @truncate(tag >> 24)
        };
    }

    // TODO: checksum calc
};

pub inline fn parseTableDirectory(self: *FontReader) !TableDirectory {
    return TableDirectory.parse(self);
}

pub const GlyphDescription = packed struct {
    contour_amount: i16,
    x_min: FWord,
    y_min: FWord,
    x_max: FWord,
    y_max: FWord,

    const SimpleDefinition = struct {
        end_contour_points: std.ArrayList(u16),
        instruction_length: u16,
        instructions: std.ArrayList(u8),
        flags: std.ArrayList(OutlineFlags),
        // storing as i16, half of it will just be zeroed out if the x/y-short flag is set
        x_coords: std.ArrayList(i16),
        y_coords: std.ArrayList(i16),

        // TODO: does zig have better bitfields?
        const OutlineFlags = packed struct(u8) {
            on_curve: bool,
            x_short_vector: bool,
            y_short_vector: bool,
            repeat: bool,
            special_x: bool,
            special_y: bool,
            _reserved: u2 = 0b00,

            pub fn parse(fr: *FontReader) !@This() {
                const data = try fr.file.reader().readInt(u8, .big);
                var flags: OutlineFlags = undefined;
                @memcpy(std.mem.asBytes(&flags), std.mem.asBytes(&data));
                return flags;
            }
        };

        // NOTE: should be deinitialised
        fn parse(fr: *FontReader, description: GlyphDescription) !@This() {
            std.debug.print("{any}\n", .{description});
            const allocator = std.heap.page_allocator;
            const contour_amount_usize = @as(usize, @intCast(description.contour_amount));
            const reader = fr.file.reader();
            var data: @This() = .{
                .end_contour_points = try std.ArrayList(u16).initCapacity(allocator, contour_amount_usize),
                .instruction_length = undefined,
                .instructions       = undefined,
                .flags              = try std.ArrayList(OutlineFlags).initCapacity(allocator, contour_amount_usize),
                .x_coords           = try std.ArrayList(i16).initCapacity(allocator, contour_amount_usize),
                .y_coords           = try std.ArrayList(i16).initCapacity(allocator, contour_amount_usize)
            };

            for(0..contour_amount_usize) |_| try data.end_contour_points.append(reader.readInt(u16, .big) catch unreachable);
            data.instruction_length = reader.readInt(u16, .big) catch unreachable;
            data.instructions = try std.ArrayList(u8).initCapacity(allocator, data.instruction_length);
            for(0..@intCast(data.instruction_length)) |_| try data.instructions.append(reader.readByte() catch unreachable);


            var all_flags = try std.ArrayList(OutlineFlags).initCapacity(allocator, contour_amount_usize);
            while (all_flags.items.len < description.contour_amount) {
                const flags = OutlineFlags.parse(fr) catch unreachable;
                try all_flags.append(flags);
                if (flags.repeat) {
                    const amount = reader.readByte() catch unreachable;
                    for (0..amount) |_| try all_flags.append(flags);
                }
            }

            for (all_flags.items) |flags| {
                var x_coord = data.x_coords.getLastOrNull() orelse 0;

                if(!flags.special_x) {
                    if(!flags.x_short_vector) {
                        x_coord += reader.readInt(i16, .big) catch unreachable;
                    } else {
                        const offset = reader.readByte() catch unreachable;
                        const signedness: i16 = @as(i16, @intFromBool(flags.special_x)) * 2 - 1;
                        x_coord += offset * signedness;
                    }
                }

                try data.x_coords.append(x_coord);
            }

            for (all_flags.items) |flags| {
                var y_coord = data.y_coords.getLastOrNull() orelse 0;

                if(!flags.special_y) {
                    if(!flags.y_short_vector) {
                        y_coord += reader.readInt(i16, .big) catch unreachable;
                    } else {
                        const offset = reader.readByte() catch unreachable;
                        const signedness: i16 = @as(i16, @intFromBool(flags.special_y)) * 2 - 1;
                        y_coord += offset * signedness;
                    }
                }

                try data.y_coords.append(y_coord);
            }

            return data;
        }

        pub fn deinit(self: *@This()) void {
            self.end_contour_points.deinit();
            self.instructions.deinit();
            self.flags.deinit();
            self.x_coords.deinit();
            self.y_coords.deinit();
        }
    };

    const CompoundDefinition = packed struct {

    };

    const parse = MakeStructParse(@This());
};

pub fn parseFont(self: *FontReader) !void {
    std.log.debug("current file offset {d} ({d})", .{try self.file.getPos(), @sizeOf(OffsetSubtable)});
    const ot = try self.parseOffsetSubtable();
    std.log.debug("number of tables: {d}", .{ot.num_tables});
    std.log.debug("current file offset {d}", .{try self.file.getPos()});

    const allocator = std.heap.page_allocator;
    var tables = std.StringHashMap(u32).init(allocator);
    defer tables.deinit();
    for (0..ot.num_tables) |_| {
        const td = try self.parseTableDirectory();
        std.log.debug("tag: {s}, offset: {d}", .{TableDirectory.tagToString(td.tag), td.offset});

        const tag = TableDirectory.tagToString(td.tag);
        var tagh = try allocator.alloc(u8, 4);
        @memcpy(std.mem.asBytes(tagh[0..4]), std.mem.asBytes(&tag));
        try tables.putNoClobber(tagh, td.offset);
    }

    self.file.seekTo(tables.get("glyf").?) catch unreachable;
    var simple = try GlyphDescription.SimpleDefinition.parse(self, GlyphDescription.parse(self) catch unreachable);
    defer simple.deinit();

    for(simple.x_coords.items, simple.y_coords.items) |x, y| {
        std.log.debug("{d}, {d}", .{x, y});
    }
}
