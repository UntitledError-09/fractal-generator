#version 450

/**
 * @file fractal_display.frag
 * @brief Fragment shader for displaying computed fractal data
 * 
 * This fragment shader samples the computed fractal texture and
 * displays it on screen. The fractal data is computed by the
 * compute shader and transferred to this texture.
 */

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D fractalTexture;

void main() {
    // Sample the fractal texture
    vec4 fractalColor = texture(fractalTexture, fragTexCoord);
    
    // The compute shader outputs RGBA fractal data
    outColor = fractalColor;
}
