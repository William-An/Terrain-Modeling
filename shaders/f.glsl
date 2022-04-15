#version 330

const int SHADINGMODE_NORMALS = 0;		// Show normals as colors
const int SHADINGMODE_PHONG = 1;		// Phong shading + illumination
const int SHADINGMODE_GOURAUD = 2;		// Gouraud shading

const int LIGHTTYPE_POINT = 0;			// Point light
const int LIGHTTYPE_DIRECTIONAL = 1;	// Directional light

smooth in vec3 fragPos;		// Interpolated position in world-space
smooth in vec3 fragNorm;	// Interpolated normal in world-space

out vec3 outCol;	// Final pixel color

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

uniform int shadingMode;		// Which shading mode
uniform vec3 camPos;			// World-space camera position
uniform vec3 objColor;			// Object color
uniform float ambStr;			// Ambient strength
uniform float diffStr;			// Diffuse strength
uniform float specStr;			// Specular strength
uniform float specExp;			// Specular exponent

void main() {
	if (shadingMode == SHADINGMODE_NORMALS)
		outCol = normalize(fragNorm) * 0.5 + vec3(0.5);

	else if (shadingMode == SHADINGMODE_PHONG) {
		// TODO ====================================================================
		// Implement Phong illumination
		outCol = vec3(0.0);

	} else if (shadingMode == SHADINGMODE_GOURAUD) {
		// TODO (Extra credit) =====================================================
		// Use Gouraud shading color
		outCol = vec3(0.0);
	}
}
