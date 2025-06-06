#version 330 core


in vec2 TextCoord;
uniform int enableTexture;
uniform sampler2D text;

out vec4 FragColor;

void main()
{
    if (enableTexture == 1) {
        vec4 textureCol = texture(text, TextCoord);
        if (textureCol.w < 0.05)
            discard;
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
    // gl_FragDepth = gl_FragCoord.z;
}