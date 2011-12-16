const int MAX_KERNEL_SIZE = 128;
uniform samplerCube tex;
//uniform int arraySize;
//uniform vec2 offsets[MAX_KERNEL_SIZE];
uniform float kernel[MAX_KERNEL_SIZE];
void main(void) {
    int i;
    vec4 blurColor = vec4(1, 0, 0, 0);
    int arraySize = 8;
    for(i = 0; i < arraySize; i++){
        vec4 blurOff = texture2D(tex, gl_TexCoord[0].st + (i - (arraySize/2)));
        vec4 curr = texture2D(tex, gl_TexCoord[0].st);
        int jtemp = ((abs(blurOff.r - curr.r) + abs(blurOff.g - curr.g) + abs(blurOff.b - curr.b))/3);
        int j = (min(jtemp, arraySize/2)) + (arraySize/2);
        vec4 convTemp = kernel[i] * blurOff;
        vec4 conv = vec4(0.0, 1.0, 0.0, 1.0);//kernel[j] * convTemp;
        blurColor = blurColor + conv;
    }

    gl_FragColor = blurColor;//vec4(1.0, 1.0, 0.0, 1.0);//
}
