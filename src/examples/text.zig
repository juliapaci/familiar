const std = @import("std");
// TODO: temporary import before we modularize the zig stuff from the c stuff
const text = @import("text");
const familiar = @cImport({
    @cInclude("engine/shader.h");
    @cInclude("engine/general.h");
    @cInclude("engine/renderer.h");
});
// const externs = @import("externs");
// const familiar = @import("engine");

pub fn main() !void {
    var fr = try text.init("assets/OpenSans-VariableFont_wdth,wght.ttf");
    defer fr.deinit();

    const window = familiar.init_window("Familiar text example (zig)");
    if(window == null) return;

    try fr.parseFont();
}
