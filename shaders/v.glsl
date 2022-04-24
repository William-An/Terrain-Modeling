#version 330

const int NORMALMODE_FACE = 0;			// Flat normals
const int NORMALMODE_SMOOTH = 1;		// Smooth normals

const int SHADINGMODE_GOURAUD = 2;

layout(location = 0) in vec3 pos;			// Model-space position
layout(location = 1) in vec3 face_norm;		// Model-space face normal
layout(location = 2) in vec3 smooth_norm;	// Model-space smoothed normal
layout(location = 3) in vec2 texture_coord;	// texture coordinate

smooth out vec3 fragPos;	// Interpolated position in world-space
smooth out vec3 fragNorm;	// Interpolated normal in world-space
smooth out vec3 gouraudCol;	// Interpolated frag color
smooth out vec3 localFragPos;	// Local interpolated frag pos
smooth out vec2 fragTextureCoord;

uniform mat4 modelMat;		// Model-to-world transform matrix
uniform mat4 viewProjMat;	// World-to-clip transform matrix
uniform int normalMode;		// Face normals or smooth normals
uniform int shadingMode;	// Shading mode

// For Gourand shading
const int LIGHTTYPE_POINT = 0;			// Point light
const int LIGHTTYPE_DIRECTIONAL = 1;	// Directional light

// Light information
struct LightData {
	bool enabled;	// Whether the light is on
	int type;		// Type of light (0 = point, 1 = directional)
	vec3 pos;		// World-space position/direction of light source
	vec3 color;		// Color of light
};

// Array of lights
const int MAX_LIGHTS = 8;
layout (std140) uniform LightBlock {
	LightData lights [MAX_LIGHTS];
};

uniform vec3 camPos;			// World-space camera position
uniform vec3 objColor;			// Object color
uniform float ambStr;			// Ambient strength
uniform float diffStr;			// Diffuse strength
uniform float specStr;			// Specular strength
uniform float specExp;			// Specular exponent

void main() {
	// Choose which normals to use
	vec3 norm;
	if (normalMode == NORMALMODE_FACE)
		norm = face_norm;
	else if (normalMode == NORMALMODE_SMOOTH)
		norm = smooth_norm;

	// Get world-space position and normal
	fragPos = vec3(modelMat * vec4(pos, 1.0));
	fragNorm = vec3(modelMat * vec4(norm, 0.0));
	localFragPos = pos;
	fragTextureCoord = texture_coord;

	// Output clip-space position
	gl_Position = viewProjMat * vec4(fragPos, 1.0);

	// TODO (Extra credit) =========================================================
	// Implement Gouraud shading
	if (shadingMode == SHADINGMODE_GOURAUD) {
		// Use gouraud shading
		gouraudCol = vec3(0);
		vec3 vertPos = fragPos;
		for (int i = 0; i < MAX_LIGHTS; i++) {
			if (!lights[i].enabled) {
				continue;
			} else {
				// Add light components
				vec3 ambient = ambStr * lights[i].color;

				// Compute light direction and diffuse
				vec3 lightDir = vec3(0);
				if (lights[i].type == LIGHTTYPE_POINT) {
					lightDir = normalize(lights[i].pos - vertPos);
				} else if (lights[i].type == LIGHTTYPE_DIRECTIONAL) {
					lightDir = normalize(lights[i].pos);
				}
				vec3 diffuse = diffStr * max(dot(norm, lightDir), 0) * lights[i].color;

				// Specular component
				vec3 reflection = normalize(reflect(-lightDir, norm));
				vec3 viewDir    = normalize(camPos - vertPos);
				vec3 specular = specStr * pow(max(dot(viewDir, reflection), 0), specExp) * lights[i].color;

				gouraudCol += (ambient + diffuse + specular) * objColor;
			}
		}
	}
}
