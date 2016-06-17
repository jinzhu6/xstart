// high quality blur shader

varying vec2 TexCoord;
uniform sampler2D tex0;
uniform int FILTER_SIZE;  // size of filter, for more or less blur. set to 0 for no blur.
uniform vec2 FILTER_SCALE;  // usally set to (1.0/width, 1.0/height). may be scaled up for faster+more blur (with smaller filter-size)

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
