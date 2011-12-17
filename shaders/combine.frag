uniform sampler2D tex;

const vec3 avgVector = vec3(0.299, 0.587, 0.114);

//this shader is used on a texture that has the hdr luminance from the bilateral filter and the chromiance added together.
//it maps the colors back to the appropriate range and relationship.
void main(void) {
    vec4 sample_color = texture2D(tex, gl_TexCoord[0].st);

    float lum = dot(avgVector, sample_color);
    sample_color.rgb = lum*normalize(sample_color.rgb);

    gl_FragColor = sample_color;
}
