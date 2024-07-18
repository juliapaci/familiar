#version 330 core
out vec4 fragColour;
in vec3 colour;

void main() {
   fragColour = vec4(colour, 1.0);
};
