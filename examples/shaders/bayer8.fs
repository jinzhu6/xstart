// BayerRG8 decoder

in vec2 TexCoord;
uniform sampler2D image;
uniform sampler2D mask;
uniform vec2 FILTER_SCALE;  // set to (1.0/width, 1.0/height)

void main(void) {
    vec3 sums = vec3(0.0, 0.0, 0.0);
    vec3 cols = vec3(0.0, 0.0, 0.0);
    
    for(int y = -1; y <= 1; y++) {
        for(int x = 0; x <= 1; x++) {
            vec3 m = texture2D(mask, TexCoord + vec2(x,y) * FILTER_SCALE).rgb;
            sums += m;

            float v = texture2D(image, TexCoord + vec2(x,y) * FILTER_SCALE).r;
            cols += m * v;
        }
    }
    
    cols /= sums;
    gl_FragColor = vec4(cols, 1.0);
}
