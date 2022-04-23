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

uniform sampler2DArray heightMap;
uniform int originalPhongIndx;	// The initial phong config to use for the terrainbool

void main() {
	if (shadingMode == SHADINGMODE_NORMALS)
		outCol = normalize(fragNorm) * 0.5 + vec3(0.5);

	else if (shadingMode == SHADINGMODE_PHONG) {
		// TODO ====================================================================
		// Implement Phong illumination
		outCol = vec3(0.0);

		// TODO TESTING
		// int i = 1;
		// if (configs[i].enable == 1)
		// 	outCol = configs[i].color;
		// else
		// 	outCol = vec3(1.0, 0.3, 0.2);
		// return;
		// End testing

		// Determine which region this frag lies in and use the 
		// corresponding config
		int configIdx = originalPhongIndx;

		// Only plot regions if this shape is the terrain
		// which is the first surface
		// we don't want ocean to be separated by forest
		if (originalPhongIndx == 0) {
			bool foundPhong2Use = false;
			// Starts from the bottom, as we treat the frag below the surface
			// to be the corresponding Phong Config
			for (configIdx = MAX_LAYERS - 1; configIdx >= 0; configIdx--) {
				PhongConfig config = configs[configIdx];

				// Skip disable surface
				if (configIdx == originalPhongIndx || config.enable == 0)
					continue;
				
				// Below the surface, choose this config
				vec4 texValue = texture(heightMap, vec3(localFragPos.x * 0.5 + 0.5, (-localFragPos.z) * 0.5 + 0.5, configIdx));
				float height = texValue.r;
				if (localFragPos.y < height) {
					foundPhong2Use = true;
					break;
				}
			}

			// If not found, use own texture
			if (!foundPhong2Use)
				configIdx = originalPhongIndx;
		}

		for (int i = 0; i < MAX_LIGHTS; i++) {
			if (!lights[i].enabled) {
				continue;
			} else {
				// Normalized
				vec3 norm = normalize(fragNorm);
				// Add light components
				vec3 ambient = configs[configIdx].ambient * lights[i].color;

				// Compute light direction and diffuse
				vec3 lightDir = vec3(0);
				if (lights[i].type == LIGHTTYPE_POINT) {
					lightDir = normalize(lights[i].pos - fragPos);
				} else if (lights[i].type == LIGHTTYPE_DIRECTIONAL) {
					lightDir = normalize(lights[i].pos);
				}
				// vec3 diffuse = diffStr * max(dot(norm, lightDir), 0) * lights[i].color;
				vec3 diffuse = configs[configIdx].diffuse * max(dot(norm, lightDir), 0) * lights[i].color;

				// Specular component
				vec3 reflection = normalize(reflect(-lightDir, norm));
				vec3 viewDir    = normalize(camPos - fragPos);
				// vec3 specular = specStr * pow(max(dot(viewDir, reflection), 0), specExp) * lights[i].color;
				
				vec3 specular = configs[configIdx].specular * pow(max(dot(viewDir, reflection), 0), configs[configIdx].exponent) * lights[i].color;

				// outCol += (ambient + diffuse + specular) * objColor;
				outCol += (ambient + diffuse + specular) * configs[configIdx].color;
			}
		}
	} else if (shadingMode == SHADINGMODE_GOURAUD) {
		// TODO (Extra credit) =====================================================
		// Use Gouraud shading color
		outCol = gouraudCol;
	}
}
