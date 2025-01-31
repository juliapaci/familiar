const std = @import("std");

const FontReader = @This();
file: std.fs.File,

pub fn init(ttf_path: []const u8) !FontReader {
    return .{
        .file = try std.fs.cwd().openFile(ttf_path, .{})
    };
}

pub fn deinit(self: *FontReader) void {
    self.file.close();
}

const OffsetTable = struct {
    scalar_type: u32,
    num_tables: u16,
    search_range: u16,
    entry_selector: u16,
    range_shift: u16,

    fn parse(fr: FontReader) !@This() {
        return try fr.file.reader().skipBytes(0, .{}).readStructEndian(@This(), std.builtin.Endian.big);
    }
};
