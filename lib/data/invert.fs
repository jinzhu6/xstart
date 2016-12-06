varying vec2 TexCoord;
uniform sampler2D tex0;

void main(void) {
    vec4 v = texture2D(tex0, TexCoord);
    gl_FragColor = vec4(1.0-v.r, 1.0-v.g, 1.0-v.b, v.a);
}
