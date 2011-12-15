varying vec3 N;
varying vec3 v;
void main (void)
{
   vec3 l0 = normalize(gl_LightSource[0].position.xyz - v);
 //  vec3 l1 = normalize(gl_LightSource[1].position.xyz - v);
   vec3 E = normalize(-v); // we are in Eye Coordinates, so EyePos is (0,0,0)
   vec3 r0 = normalize(-reflect(l0, N));
 //  vec3 r1 = normalize(-reflect(l1, N));

   //calculate Ambient Term:
   vec4 amb0 = gl_FrontLightProduct[0].ambient;
 //  vec4 amb1 = gl_FrontLightProduct[1].ambient;
 //  vec4 amb = 0.5 * (amb0 + amb1);

   //calculate Diffuse Term:
   vec4 diff0 = gl_FrontLightProduct[0].diffuse * max(dot(N, l0), 0.0);
 //  vec4 diff1 = gl_FrontLightProduct[1].diffuse * max(dot(N, l0), 0.0);
 //  vec4 diff = diff0 + diff1;
   vec4 diff = clamp(diff0, 0.0, 1.0);

   // calculate Specular Term:
   vec4 spec0 = gl_FrontLightProduct[0].specular
                * pow(max(dot(r0, E),0.0),0.3*gl_FrontMaterial.shininess);
//   vec4 spec1 = gl_FrontLightProduct[1].specular
//                * pow(max(dot(r1, E),0.0),0.3*gl_FrontMaterial.shininess);
//   vec4 spec = spec0 + spec1;
   vec4 spec = clamp(spec0, 0.0, 1.0);
   // write Total Color:
   gl_FragColor = gl_FrontLightModelProduct.sceneColor + amb0 + diff + spec;
}
