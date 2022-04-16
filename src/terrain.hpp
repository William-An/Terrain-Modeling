#ifndef __TERRAIN_HPP__
#define __TERRAIN_HPP__

#include <string>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <PerlinNoise.hpp>
#include "gl_core_3_3.h"
#include "fparser.hh"

// Class of procedural modeling terrain configuration
// Get configuration from parameter passing or via importing config file
// Able to evaluate the configuration and generate mesh
// for rendering
// Also able to export config as a file
class Terrain {
public:
    Terrain();
    Terrain(std::string& config_file_path);
    // Terrain(std::ifstream& config_file); // TODO
	~Terrain(){}

    // Load config file
    void load(std::string& config_file_path);
    void load(std::ifstream& config_file);

    // Export config file
    void dump(std::string& out_name);
    void dump(std::ofstream& out_file);

    // Evaluate configuration and generate mesh data for draw
    void evaluate();

    // Generate vertices and Load into opengl
    void generate();

    // Draw the mesh
    void draw();

    // Print the matrix
    void printMatrix(int indx);

    // TODO Need setter and getter for UI interactions
    void setSeed(int64_t s) {seed = s;};
    int64_t getSeed() {return seed;};

    void setName(std::string& n) {name = n;};
    std::string& getName() {return name;};

    void setSize(uint32_t w, uint32_t l) {width = w; length = l;};
    uint32_t getWidth() {return width;};
    uint32_t getLength() {return length;};

    void insertLayer(int pos, std::pair<std::vector<std::string>, glm::vec3> layer) {
        auto it = layers_functions.begin();
        layers_functions.insert(it + pos, layer);
    };
    void pushLayer(std::pair<std::vector<std::string>, glm::vec3> layer) {
        layers_functions.push_back(layer);
    };
    void eraseLayer(int pos) {
        auto it = layers_functions.begin();
        layers_functions.erase(it + pos);
    };

protected:
    // Member variables storing the terrain specifications
    int64_t seed = 0;       // Random generator seed
    std::string name;   // Name of the terrain

    // Width (x) and length (y) of the terrain, in number of points to generate
    // ->: x, ^: y
    ///       |
    // TODO want ratio? not just square terrain?
    uint32_t width;
    uint32_t length;
    
    // Layers of terrain
    // double[][]: layer height
    // glm::vec3 : layer color
    // first one is the terrain and color is ignored
    std::vector<std::pair<double**, glm::vec3>> raw_layers;

    // Function controlling each layer
    std::vector<std::pair<std::vector<std::string>, glm::vec3>> layers_functions;

    // TODO Use thread to speed up?
    class TerrainFuncParser : public FunctionParser {
        public:
            TerrainFuncParser() {
                AddFunction("perlin", perlinNoise, 3);
                AddFunction("plane", plane, 5);
                AddFunction("pyramid", pyramid, 9);
            };

            static void setSeed(int64_t s) {seed = s;};
            static void setSize(uint32_t w, uint32_t l) {
                width = w;
                length = l;
            };
            static void configNoiseGnerators() {
                perlin_device.reseed(seed);
            }

            static inline int64_t seed;
            static inline uint32_t width, length;
            static inline siv::PerlinNoise perlin_device;

            static double perlinNoise(const double* xyf);

            // Specify the corner height in clockwise direction starting
            // from left top corner, with the corners' (x, y) at (-1, 1)
            // (1, 1), (1, -1) respectively
            static double plane(const double* xyc1c2c3);

            // xy: point position for height
            // c1c2c3c4: corner height in CW direction start from left top
            // xyz: pyramid apex position
            static double pyramid(const double* xyc1c2c3c4xyz);

            // TODO Allow loading object file?
    };

    TerrainFuncParser terrainParser;

    // Vertex structure for rendering
    struct Vertex {
		glm::vec3 pos;			// Position
		glm::vec3 face_norm;	// Face normal
		glm::vec3 smooth_norm;	// Smoothed normal
		// Vertex();
	};

    void release();		// Release OpenGL resources

	// Bounding box
	glm::vec3 minBB;
	glm::vec3 maxBB;

	// OpenGL resources
	GLuint vao;		// Vertex array object
	GLuint vbuf;	// Vertex buffer
	GLsizei vcount;	// Number of vertices
};

#endif // !__TERRAIN_HPP__
