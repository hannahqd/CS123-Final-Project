uniform samplerCube envMap;
varying vec3 normal, lightDir, r;

uniform float r0;		// The R0 value to use in Schlick's approximation
uniform float eta1D;		// The eta value to use initially, ratio of IORs ( n2 / n1 in Snell's law )
uniform float etaR;
uniform float etaG;
uniform float etaB;

void main (void)
{
        vec4 final_color = textureCube(envMap, r);
        vec3 N = normalize(normal);
        vec3 L = normalize(lightDir);
        float lambertTerm = dot(N,L);
        if(lambertTerm > 0.0)
        {
                // Specular
                final_color += textureCube(envMap, r);
        }
        gl_FragColor = final_color;
}




//varying vec3 vertex;		// The position of the vertex, in eye space
//varying vec3 light;		// The normalized vector from the vertex to the light
//varying vec3 eye;		// The normalized vector from the vertex to the eye
//varying vec3 normal;		// The normal vector of the vertex, in eye space

//uniform float r0;		// The R0 value to use in Schlick's approximation
//uniform float eta1D;		// The eta value to use initially, ratio of IORs ( n2 / n1 in Snell's law )
//uniform float etaR;
//uniform float etaG;
//uniform float etaB;

//uniform samplerCube envMap;

//void main()
//{
//    vec3 n = normalize(normal);
//    vec3 e = normalize(vertex);
//    vec3 i = normalize(vertex - eye);
   // vec3 l = normalize(light);

//    vec3 redVert = gl_ModelViewMatrixInverse * vec4(refract(e, n, etaR), 0.0);
//    vec3 greenVert = gl_ModelViewMatrixInverse * vec4(refract(e, n, etaG), 0.0);
//    vec3 blueVert = gl_ModelViewMatrixInverse * vec4(refract(e, n, etaB), 0.0);

//    float red = textureCube(envMap, redVert).r;
//    float green = textureCube(envMap, greenVert).g;
//    float blue = textureCube(envMap, blueVert).b;

//    vec3 t = vec3(red, green, blue);

//    float costheta = dot(n, -i);

//    float f = max(0, r0 + (1.0 - r0) * (pow((1.0 - costheta), 5)));

//    vec3 incident = (gl_ModelViewMatrixInverse * vec4(reflect(normalize(vertex), n), 0.0)).xyz;
//    vec3 refract = (gl_ModelViewMatrixInverse * vec4(normalize(vertex), 0.0)).xyz;

//    float incClampR = clamp(0.0, incident.r, 1.0);
//    float incClampG = clamp(0.0, incident.g, 1.0);
//    float incClampB = clamp(0.0, incident.b, 1.0);

//    vec3 incClamp = vec3(incClampR, incClampG, incClampB);

//    float refClampR = clamp(0.0, refract.r, 1.0);
//    float refClampG = clamp(0.0, refract.g, 1.0);
//    float refClampB = clamp(0.0, refract.b, 1.0);
//
//    vec3 refClamp = vec3(refClampR, refClampG, refClampB);

//    gl_FragColor = (f* textureCube(envMap, incident)) + ((1-f)*vec4((t * refClamp).rgb, 1.0));
//}
