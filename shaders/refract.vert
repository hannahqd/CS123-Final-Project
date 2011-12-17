varying vec3 normal, lightDir, r;
const vec3 L = vec3(0.,0.,0.);
void main()
{
        gl_Position = ftransform();
        vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);
        lightDir = vec3(L - vVertex);
        vec4 eyeVec = gl_ProjectionMatrixInverse*vec4(0,0,-1,0);

        normal = normalize( gl_NormalMatrix * gl_Normal );
        vec3 I = normalize(vVertex - eyeVec.xyz); // Eye to vertex
  r = refract(I,normal, 0.9);
}




//varying vec3 vertex;		// The position of the vertex, in eye space
//varying vec3 eye;		// The normalized vector from the vertex to the eye
//varying vec3 normal;		// The normal vector of the vertex, in eye space

//void main()
//{
//    vertex = (gl_ModelViewMatrix * gl_Vertex).xyz;
//    normal = normalize(gl_NormalMatrix * gl_Normal);
//    eye = -vertex;
//
//    gl_Position = ftransform();
//}
