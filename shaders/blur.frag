const int MAX_KERNEL_SIZE = 128;
uniform sampler2D tex;
uniform int arraySize;
uniform vec2 offsets[MAX_KERNEL_SIZE];
uniform float kernel[MAX_KERNEL_SIZE];
void main(void) {
    int i;
    vec4 blurColor = vec4(0, 0, 0, 0);
    for(i = 0; i < arraySize; i++){
        vec4 blurOff = texture2D(tex, gl_TexCoord[0].st + offsets[i]);
        vec4 conv = kernel[i] * blurOff;
        blurColor = blurColor + conv;
    }

    gl_FragColor = blurColor;
}
