varying vec3 vertex;		// The position of the vertex, in eye space
varying vec3 light;		// The normalized vector from the vertex to the light
varying vec3 eye;		// The normalized vector from the vertex to the eye
varying vec3 normal;		// The normal vector of the vertex, in eye space

uniform samplerCube envMap;

void main()
{
    float r0 = 0.4;
    float eta1D = 0.77;
    vec3 eta = vec3(0.75, 0.77, 0.8);
    vec3 n = normalize(normal);
    vec3 e = normalize(vertex);
    vec3 i = normalize(vertex - eye);
   // vec3 l = normalize(light);

    vec3 redVert = gl_ModelViewMatrixInverse * vec4(refract(e, n, eta.r), 0.0);
    vec3 greenVert = gl_ModelViewMatrixInverse * vec4(refract(e, n, eta.g), 0.0);
    vec3 blueVert = gl_ModelViewMatrixInverse * vec4(refract(e, n, eta.b), 0.0);

    float red = clamp(0.0, textureCube(envMap, redVert).r, 1.0);
    float green = clamp(0.0, textureCube(envMap, greenVert).g, 1.0);
    float blue = clamp(0.0, textureCube(envMap, blueVert).b, 1.0);

    vec3 t = vec3(red, green, blue);

    float costheta = dot(n, -i);

    float f = max(0, r0 + (1.0 - r0) * (pow((1.0 - costheta), 5)));

    vec3 incident = (gl_ModelViewMatrixInverse * vec4(reflect(normalize(vertex), n), 0.0)).xyz;
    vec3 refract = (gl_ModelViewMatrixInverse * vec4(normalize(vertex), 0.0)).xyz;

    gl_FragColor = (f* textureCube(envMap, incident)) + ((1-f)*vec4(t.rgb, 1.0));
}
