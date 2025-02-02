const c = @cImport({
    @cInclude("glad/glad.h");
    @cInclude("GLFW/glfw3.h");
});

// pub extern fn init_window([*:0]const u8) c.GLFWwindow;
