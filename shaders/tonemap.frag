uniform sampler2D tex;
uniform float exposure;

const vec3 avgVector = vec3(0.299, 0.587, 0.114);
void main(void) {
    vec4 sample = texture2D(tex, gl_TexCoord[0].st);

    float luminance = max(0.0, dot(avgVector, sample.rgb));


    sample.rgb = exposure*luminance/(luminance +1);

    gl_FragColor = sample;
}
