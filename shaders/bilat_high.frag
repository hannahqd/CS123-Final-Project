const int MAX_KERNEL_SIZE = 128;
uniform sampler2D tex;
const float M_PI = 3.14159;
const vec3 avgVector = vec3(0.299, 0.587, 0.114);

void main(void) {


    int radius = 5;
    int size = radius * 2 + 1;

float kernel[MAX_KERNEL_SIZE];
float sigma = radius / 3.0f;
float twoSigmaSigma = 2.0f * sigma * sigma;
float rootSigma = sqrt(twoSigmaSigma * M_PI);
float total = 0.0f;
for (int y = -radius, idx = 0; y <= radius; ++y)
{
    for (int x = -radius; x <= radius; ++x,++idx)
    {
        float d = x * x + y * y;
        kernel[idx] = exp(-d / twoSigmaSigma) / rootSigma;
        total += kernel[idx];
    }
}
for (int i = 0; i < size * size; ++i)
{
    kernel[i] /= total;
}

int arraySize = size*size;


    vec4 blurColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 test;
    vec4 curr = texture2D(tex, gl_TexCoord[0].st);

    float cur_lum = dot(avgVector, curr.rgb);
    float blurOff_lum;
    float colordist;
    for(int i = 0; i < arraySize; i++){
        vec4 blurOff = texture2D(tex, gl_TexCoord[0].st + (i - (arraySize/2)));

        blurOff_lum = dot(avgVector, blurOff);

        //If you print out colordist, it is a cool edge detector
        colordist = abs(blurOff_lum - cur_lum);

        colordist = colordist/(colordist+1);
        vec4 convTemp = blurOff*kernel[i]*(1-colordist);

        blurColor = blurColor + convTemp;
    }


    vec4 temp = dot(avgVector, blurColor);
    gl_FragColor = temp;
}
