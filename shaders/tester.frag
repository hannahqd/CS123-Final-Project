uniform sampler2D tex;

void main(void) {
    vec4 sample = texture2D(tex, gl_TexCoord[0].st);


    gl_FragColor = sample;
}
