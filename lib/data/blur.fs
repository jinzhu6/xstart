varying vec2 TexCoord;
uniform sampler2D tex0;

int FILTER_SIZE = 16;
vec2 FILTER_SCALE = vec2(1.0/800.0, 1.0/600.0);

void main(void) {
	vec4 color = vec4(0.0,0.0,0.0,0.0);
	
	float total = 0.0;
	for(int y = -FILTER_SIZE; y <= FILTER_SIZE; y++) {
		for(int x = -FILTER_SIZE; x <= FILTER_SIZE; x++) {
			color += texture2D(tex0, TexCoord + vec2(x,y) * FILTER_SCALE);
			total += 1.0;
		}
	}
	
	color /= total;
	gl_FragColor = color;
}
