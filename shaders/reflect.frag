//uniform samplerCube CubeMap;
//varying vec3 normal, lightDir, r;

//void main (void)
//{
//	vec4 final_color = textureCube( CubeMap, r);
//	vec3 N = normalize(normal);
//	vec3 L = normalize(lightDir);
//	float lambertTerm = dot(N,L);
//	if(lambertTerm > 0.0)
//	{
//		// Specular
//		final_color += textureCube( CubeMap, r);
//	}
//       gl_FragColor = final_color;
//}


varying vec3 N;
varying vec3 v;
void main (void)
{
   vec3 L = normalize(gl_LightSource[0].position.xyz - v);
   vec3 E = normalize(-v); // we are in Eye Coordinates, so EyePos is (0,0,0)
   vec3 R = normalize(-reflect(L,N));

   //calculate Ambient Term:
   vec4 Iamb = gl_FrontLightProduct[0].ambient;

   //calculate Diffuse Term:
   vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(N,L), 0.0);
   Idiff = clamp(Idiff, 0.0, 1.0);

   // calculate Specular Term:
   vec4 Ispec = gl_FrontLightProduct[0].specular
                * pow(max(dot(R,E),0.0),0.3*gl_FrontMaterial.shininess);
   Ispec = clamp(Ispec, 0.0, 1.0);
   // write Total Color:
   gl_FragColor = gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec;
}
