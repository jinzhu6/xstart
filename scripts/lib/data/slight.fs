varying vec2 TexCoord;
uniform sampler2D tex0;

void main(void) {
    vec4 va = texture2D(tex0, TexCoord);
	vec4 vb = vec4(1.0-va.r, 1.0-va.g, 1.0-va.b, va.a);
    gl_FragColor = vec4(va.r * 0.85 + vb.r * 0.3, va.g * 0.85 + vb.g * 0.3, va.b * 0.85 + vb.b * 0.3, va.a);
}
