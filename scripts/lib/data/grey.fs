varying vec2 TexCoord;
uniform sampler2D tex0;

void main(void) {
    vec4 v = texture2D(tex0, TexCoord);
    float grey = (v.r + v.g + v.b) * 0.33;
	gl_FragColor = vec4(grey, grey, grey, v.a);
}
