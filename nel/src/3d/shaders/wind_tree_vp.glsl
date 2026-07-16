// wind_tree_vp.glsl
// Hierarchical wind tree vertex program with per-vertex Gouraud lighting.
// GLSL 330 version for GL3/GLES3 driver.
//
// Two variants exist at runtime:
//
// 1. LEGACY (16-variant path):
//    Uses preprocessor defines for light count/specular/normalize.
//    Lighting in object space with pre-multiplied light×material colors.
//    Source: WindTreeVPCodeGLSL_Body (meshvp_wind_tree.cpp)
//
// 2. UBO (single program, shown below):
//    Uses NlCamera, NlLightTable, NlModel UBOs for eye-space lighting.
//    All variants folded (light count via indices, specular via material=black).
//    Source: WindTreeVPCodeGLSL_UBO_Body (meshvp_wind_tree.cpp)
//
// This file shows the UBO variant for reference.
// Requires: #version 330 and GL_ARB_separate_shader_objects
// (prepended by the constructor before this source)
// UBO blocks (NlCamera, NlLightTable, NlModel) are inserted by insertBuiltinHeaders.

// Inputs
layout (location = 0) in vec4 vposition;
layout (location = 2) in vec4 vnormal;
layout (location = 3) in vec4 vprimaryColor;
layout (location = 8) in vec4 vtexCoord0;

// Outputs
out gl_PerVertex { vec4 gl_Position; };
layout(location = 3) smooth out vec4 vertexColor;
layout(location = 4) smooth out vec4 specularColor;
layout(location = 8) smooth out vec4 texCoord0;
layout(location = 0) smooth out vec4 ecPos;

// Wind animation (individual uniforms)
// TODO: Wind and material uniforms will move to user VP UBO (binding 5) via CUniformBuffer when IDriver bind API is available.
uniform vec3 windLevel1;
uniform vec3 windLevel2[4];
uniform vec3 windLevel3[4];

// Material (individual uniforms)
uniform vec4 nlMaterialDiffuse;
uniform vec4 nlMaterialSpecular;
uniform float nlMaterialShininess;

void computeLight(int lightMode, vec3 dirOrPos, vec4 colDiff, vec4 colSpec, float shininess,
                   float constAttn, float linAttn, float quadAttn,
                   vec3 spotDir, float spotCutoff, float spotExp,
                   vec3 normal3, vec3 ecPos3, vec3 eyeDir,
                   inout vec4 lightDiffuse, inout vec4 lightSpecular)
{
  if (lightMode < 0) return;

  vec3 lightDir;
  float attnFactor = 1.0;

  if (lightMode == 0) { // DirectionalLight
    lightDir = normalize(mat3(viewMatrix) * dirOrPos);
  } else {
    // Point or spot light
    vec4 lightPos4 = viewMatrix * vec4(dirOrPos, 1.0);
    vec3 lightPos = lightPos4.xyz / lightPos4.w;
    vec3 lightVec = lightPos - ecPos3;
    float lightDistance = length(lightVec);
    lightDir = lightVec / lightDistance;
    float attenuation = constAttn + linAttn * lightDistance + quadAttn * lightDistance * lightDistance;
    attnFactor = 1.0 / attenuation;
    if (lightMode == 2) { // SpotLight
      vec3 spotDirView = normalize(mat3(viewMatrix) * spotDir);
      float spotDot = dot(-lightDir, spotDirView);
      attnFactor *= (spotDot >= spotCutoff) ? pow(spotDot, spotExp) : 0.0;
    }
  }

  float diffAngle = max(0.0, dot(lightDir, normal3));
  lightDiffuse += diffAngle * attnFactor * colDiff;

  vec3 halfVector = normalize(lightDir + eyeDir);
  float specAngle = max(0.0, dot(normal3, halfVector));
  float specPow = pow(specAngle, shininess);
  lightSpecular += specPow * attnFactor * colSpec;
}

void main()
{
  // --- Wind Animation (object space) ---
  vec3 factors = clamp(vprimaryColor.xxx * 3.0 + vec3(0.0, -1.0, -2.0), 0.0, 1.0);
  vec4 pos = vposition;
  pos.xyz += windLevel1 * factors.x;
  vec2 phase = vprimaryColor.yz * 3.99;
  int idx2 = int(phase.x);
  pos.xyz += windLevel2[idx2] * factors.y;
  int idx3 = int(phase.y);
  pos.xyz += windLevel3[idx3] * factors.z;

  // --- Transform ---
  gl_Position = modelViewProjection * pos;
  vec4 ecPos4 = modelView * pos;
  ecPos = ecPos4;

  // --- Lighting (eye space, light table) ---
  vec3 normal3 = normalize(normalMatrix * vnormal.xyz);
  vec3 ecPos3 = ecPos4.xyz / ecPos4.w;
  vec3 eyeDir = normalize(-ecPos3);
  vec4 diffuseVertex = vec4(0.0);
  vec4 specularVertex = vec4(0.0);

  // Unrolled 8 light slots from NlModel UBO
  { int idx = nlLightIndices01.x;
    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors01.x;
      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;
      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,
        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);
  } }
  { int idx = nlLightIndices01.y;
    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors01.y;
      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;
      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,
        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);
  } }
  { int idx = nlLightIndices01.z;
    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors01.z;
      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;
      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,
        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);
  } }
  { int idx = nlLightIndices01.w;
    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors01.w;
      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;
      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,
        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);
  } }
  { int idx = nlLightIndices45.x;
    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors45.x;
      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;
      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,
        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);
  } }
  { int idx = nlLightIndices45.y;
    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors45.y;
      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;
      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,
        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);
  } }
  { int idx = nlLightIndices45.z;
    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors45.z;
      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;
      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,
        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);
  } }
  { int idx = nlLightIndices45.w;
    if (idx >= 0) { NlLightInfo li = nlLights[idx]; float factor = nlLightFactors45.w;
      vec3 adjDirOrPos = (li.mode == 0) ? -li.dirOrPos : li.dirOrPos - pzbCameraPos;
      computeLight(li.mode, adjDirOrPos, li.diffuse * factor * nlMaterialDiffuse, li.specular * factor * nlMaterialSpecular, nlMaterialShininess,
        li.constAttn, li.linAttn, li.quadAttn, li.spotDir, li.spotCutoff, li.spotExp, normal3, ecPos3, eyeDir, diffuseVertex, specularVertex);
  } }

  // Self-illumination from NlModel UBO
  diffuseVertex.rgb += selfIllumination.rgb;
  diffuseVertex.a = 1.0;

  vertexColor = clamp(diffuseVertex, 0.0, 1.0);
  specularColor = clamp(vec4(specularVertex.rgb, 0.0), 0.0, 1.0);
  texCoord0 = vtexCoord0;
}
