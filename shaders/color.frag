uniform sampler2D tex;

//this normalizes color values for a color without intesity image
void main(void) {
    vec4 sample = texture2D(tex, gl_TexCoord[0].st);

    sample.rgb = normalize(sample.rgb);
    gl_FragColor = sample;
}
