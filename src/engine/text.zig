const std = @import("std");

const native_endian = @import("builtin").target.cpu.arch.endian();
const FontReader = @This();
// sfnt data types
const FWord = i16;

file: std.fs.File,

// waiting on https://github.com/ziglang/zig/pull/21601
// for something like `try fr.file.reader().readStructEndian(@This(), .big)`
// but for now this will do:
pub fn readPackedStructBigEndian(self: *FontReader, comptime T: type) !T {
    const bytes = try self.file.reader().readBytesNoEof(@divExact(@bitSizeOf(T), 8));
    var result: T = undefined;
    @memcpy(std.mem.asBytes(&result)[0..@divExact(@bitSizeOf(T), 8)], bytes[0..@divExact(@bitSizeOf(T), 8)]);
    if (native_endian != .big) std.mem.byteSwapAllFields(T, &result);
    return result;
}

fn MakeStructParse(comptime T: type) type {
    return struct {
        fn parse(fr: *FontReader) !T {
            // return try fr.file.reader().readStructEndian(@This(), .big);
            return fr.readPackedStructBigEndian(T);
        }
    };
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

    const parse = MakeStructParse(@This()).parse;
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

    const parse = MakeStructParse(@This()).parse;

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

    const SimpleDefinition = packed struct {
        end_contour_points: []u16,
        instruction_length: u16,
        instructions: []u8,
        flags: []u8,

        // TODO: does zig have better bitfields?
        const OutlineFlags = packed struct {
            on_curve: bool,
            x_short_vector: bool,
            y_short_vector: bool,
            repeat: bool,
            special_x: bool,
            special_y: bool,
            reserved: u2
        };
    };

    const CompoundDefinition = packed struct {

    };

    const parse = MakeStructParse(@This()).parse;
};

pub fn parseFont(self: *FontReader) !void {
    std.log.debug("current file offset {d} ({d})", .{try self.file.getPos(), @sizeOf(OffsetSubtable)});
    const ot = try self.parseOffsetSubtable();
    std.log.debug("number of tables: {d}", .{ot.num_tables});
    std.log.debug("current file offset {d}", .{try self.file.getPos()});

    for (0..ot.num_tables) |_| {
        const td = try self.parseTableDirectory();
        std.log.debug("tag: {s}, offset: {d}", .{TableDirectory.tagToString(td.tag), td.offset});
    }
}
