varying vec2 TexCoord;
uniform sampler2D tex0;

void main(void) {
	gl_FragColor = texture2D(tex0, TexCoord);
}
