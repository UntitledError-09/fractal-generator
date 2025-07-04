#version 450

/**
 * @file fullscreen.vert
 * @brief Fullscreen quad vertex shader
 * 
 * This vertex shader creates a fullscreen quad without requiring
 * vertex buffers. It uses gl_VertexIndex to generate vertex positions.
 */

layout(location = 0) out vec2 fragTexCoord;

// Fullscreen quad vertices (triangle strip)
vec2 positions[4] = vec2[](
    vec2(-1.0, -1.0),  // Bottom-left
    vec2( 1.0, -1.0),  // Bottom-right
    vec2(-1.0,  1.0),  // Top-left
    vec2( 1.0,  1.0)   // Top-right
);

// Corresponding texture coordinates (normal orientation)
vec2 texCoords[4] = vec2[](
    vec2(0.0, 0.0),    // Bottom-left
    vec2(1.0, 0.0),    // Bottom-right
    vec2(0.0, 1.0),    // Top-left
    vec2(1.0, 1.0)     // Top-right
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragTexCoord = texCoords[gl_VertexIndex];
}
