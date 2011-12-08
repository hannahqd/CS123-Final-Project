//varying vec3 normal, lightDir, r;
//const vec3 L = vec3(0.,0.,1.);
//void main()
//{
//	gl_Position = ftransform();
//	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);
//	lightDir = vec3(L - vVertex);
//	vec4 eyeVec = gl_ProjectionMatrixInverse*vec4(0,0,-1,0);
//	normal = normalize( gl_NormalMatrix * gl_Normal );
//	vec3 I = normalize(vVertex - eyeVec.xyz); // Eye to vertex
//  r = reflect(I,normal);
//}


varying vec3 N;
varying vec3 v;
void main(void)
{
   v = vec3(gl_ModelViewMatrix * gl_Vertex);
   N = normalize(gl_NormalMatrix * gl_Normal);
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

