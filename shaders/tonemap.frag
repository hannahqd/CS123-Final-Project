uniform sampler2D tex;
uniform float exposure;
uniform float luminance;

void main(void) {
    vec4 sample = texture2D(tex, gl_TexCoord[0].st);

    gl_FragColor = sample;
}
