#version 330

const int SHADINGMODE_NORMALS = 0;		// Show normals as colors
const int SHADINGMODE_PHONG = 1;		// Phong shading + illumination
const int SHADINGMODE_GOURAUD = 2;		// Gouraud shading

const int LIGHTTYPE_POINT = 0;			// Point light
const int LIGHTTYPE_DIRECTIONAL = 1;	// Directional light

smooth in vec3 fragPos;		// Interpolated position in world-space
smooth in vec3 fragNorm;	// Interpolated normal in world-space
smooth in vec3 gouraudCol;	// Interpolated frag color
smooth in vec3 localFragPos;	// Interpolated local pos

out vec3 outCol;	// Final pixel color

// Light information
struct LightData {
	bool enabled;	// Whether the light is on
	int type;		// Type of light (0 = point, 1 = directional)
	vec3 pos;		// World-space position/direction of light source
	vec3 color;		// Color of light
};

// Layers configuration
const int MAX_LAYERS = 10;
struct PhongConfig {
	float ambient;
	float diffuse;
	float specular;
	float exponent;
	vec3   color;
	int   enable;
	int   drawSurface;
	int   coverBottom;
};

// Array of lights
const int MAX_LIGHTS = 8;
layout (std140) uniform LightBlock {
	LightData lights [MAX_LIGHTS];
};

layout (std140) uniform PhongConfigBlock {
	PhongConfig configs [MAX_LAYERS];
};

uniform int shadingMode;		// Which shading mode
uniform vec3 camPos;			// World-space camera position
uniform float ambStr;			// Ambient strength
uniform float diffStr;			// Diffuse strength
uniform float specStr;			// Specular strength
uniform float specExp;			// Specular exponent
uniform vec3 objColor;			// Object color
uniform bool drawSurface;		// If draw the surface
uniform bool coverBottom;		// If dye the area below it to the config

uniform sampler2D heightMap;

void main() {
	if (shadingMode == SHADINGMODE_NORMALS)
		outCol = normalize(fragNorm) * 0.5 + vec3(0.5);

	else if (shadingMode == SHADINGMODE_PHONG) {
		// TODO ====================================================================
		// Implement Phong illumination
		outCol = vec3(0.0);

		// TODO TESTING
		// vec4 tmp = texture2D(heightMap, vec2(fragPos.x, -fragPos.z));
		// outCol = tmp.r * vec4(0.4, 1.0, 0.8, 1.0).rgb;
		// outCol = 4 * fragPos.x * vec4(0.4, 1.0, 0.8, 1.0).rgb;
		// return;
		// End testing

		vec4 texValue = texture2D(heightMap, vec2(fragPos.x, -fragPos.z));
		
		for (int i = 0; i < MAX_LIGHTS; i++) {
			if (!lights[i].enabled) {
				continue;
			} else {
				// Normalized
				vec3 norm = normalize(fragNorm);
				// Add light components
				// TODO
				// vec3 ambient = ambStr * lights[i].color;
				float height = texValue.r;
				vec3 ambient = vec3(0.0);
				if (localFragPos.y < height) {
					ambient = configs[0].ambient * lights[i].color;
				} else {
					ambient = height * lights[i].color;
				}

				// Compute light direction and diffuse
				vec3 lightDir = vec3(0);
				if (lights[i].type == LIGHTTYPE_POINT) {
					lightDir = normalize(lights[i].pos - fragPos);
				} else if (lights[i].type == LIGHTTYPE_DIRECTIONAL) {
					lightDir = normalize(lights[i].pos);
				}
				// vec3 diffuse = diffStr * max(dot(norm, lightDir), 0) * lights[i].color;
				vec3 diffuse = configs[0].diffuse * max(dot(norm, lightDir), 0) * lights[i].color;

				// Specular component
				vec3 reflection = normalize(reflect(-lightDir, norm));
				vec3 viewDir    = normalize(camPos - fragPos);
				// vec3 specular = specStr * pow(max(dot(viewDir, reflection), 0), specExp) * lights[i].color;
				
				vec3 specular = configs[0].specular * pow(max(dot(viewDir, reflection), 0), configs[0].exponent) * lights[i].color;

				// outCol += (ambient + diffuse + specular) * objColor;
				outCol += (ambient + diffuse + specular) * configs[0].color;
			}
		}
	} else if (shadingMode == SHADINGMODE_GOURAUD) {
		// TODO (Extra credit) =====================================================
		// Use Gouraud shading color
		outCol = gouraudCol;
	}
}
