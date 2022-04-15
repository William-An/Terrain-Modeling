#include "terrain.hpp"
#include <fstream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

Terrain::Terrain() {

}

Terrain::Terrain(std::string& config_file_path) {
    this->load(config_file_path);
}

void Terrain::load(std::string& config_file_path) {
    std::ifstream config(config_file_path);
    load(config);
}

void Terrain::load(std::ifstream& config_file) {
    // TODO Performan parsing
}

void Terrain::dump(std::string& out_name) {
    std::ofstream out(out_name);
    dump(out);
}

void Terrain::dump(std::ofstream& out_file) {
    // TODO Dump the configuration
}

void Terrain::evaluate() {
    // Iterate through layer functions and generate terrain and other layers
    
    // Reconfigure function parser
    terrainParser.setSeed(seed);
    terrainParser.setSize(width, length);
    terrainParser.configNoiseGnerators();
    
    for (auto it = layers_functions.begin(); it != layers_functions.end(); it++) {
        std::pair<std::vector<std::string>, glm::vec3> layer_functions = *it;
        glm::vec3 color = layer_functions.second;
        std::vector<std::string> functions = layer_functions.first;

        // Initialize 2D matrix holding terrain height
        double** matrix = new double*[width];
        for (int i = 0; i < width; i++)
            matrix[i] = new double[length];

        // Fill in values for matrix
        for (int row = 0; row < width; row++) {
            for (int col = 0; col < length; col++) {
                // Initialize height to 0
                matrix[row][col] = 0;

                // Get xy coordinate by mapping x and y to [-1, 1]
                double x = 2 * ((double) row / (double) width) - 1;
                double y = 2 * ((double) col / (double) length) - 1;
                
                // Put variables for functions here
                double vars[3];
                vars[0] = x;
                vars[1] = y;
                vars[2] = 0;

                // Evaluate functions
                for (auto func_it = functions.begin(); func_it != functions.end(); func_it++) {
                    // Evaluaten function and add to terrain height map
                    std::string func_string = *func_it;
                    terrainParser.Parse(func_string, "x,y,N");
                    matrix[row][col] += terrainParser.Eval(vars);

                    // Increase count of layer, N
                    vars[2]++;
                }
            }
        }

        // Finish generating one layer, push to vector
        raw_layers.push_back(std::pair(matrix, color));
    }
}

void Terrain::draw() {
    // TODO
}

void Terrain::printMatrix(int indx) {
    printf("Printing info for %s\n", name.c_str());
    if (indx < 0 || indx >= raw_layers.size()) {
        std::cout << "Fatal error: index of matrix out of bound\n";
        return;
    }

    std::pair raw_layer = raw_layers[indx];
    double **matrix = raw_layer.first;
    glm::vec3 color = raw_layer.second;

    std::cout << "Color of layer: " << glm::to_string(color) << std::endl;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < length; j++) {
            printf("%.4f ", matrix[i][j]);
        }
        printf("\n");
    }
}

double Terrain::TerrainFuncParser::perlinNoise(const double* xyf)  {
    double x = xyf[0];
    double y = xyf[1];
    double f = xyf[2];
    double freq_x = f / width;
    double freq_y = f / length;
    return perlin_device.noise2D(x * freq_x, y * freq_y);
}

double Terrain::TerrainFuncParser::plane(const double* xyc1c2c3) {
    /**
     * c1 ----- c2
     * |         |
     * |         |
     *    ----- c3
     * */
    double x = xyc1c2c3[0];
    double y = xyc1c2c3[1];
    double c1 = xyc1c2c3[2];
    double c2 = xyc1c2c3[3];
    double c3 = xyc1c2c3[4];

    glm::vec3 corner1(-1, 1, c1);
    glm::vec3 corner2( 1, 1, c2);
    glm::vec3 corner3( 1,-1, c3);
    glm::vec3 normal = glm::normalize(glm::cross(corner1 - corner2, corner3 - corner2));

    // Plane expression for z given (x, y)
    double z = (-normal.x * (x - corner2.x) - normal.y * (y - corner2.y))/normal.z + corner2.z;

    return z;
}

double Terrain::TerrainFuncParser::pyramid(const double* xyc1c2c3c4xyz) {
     /**
     * c1 ----- c2
     * | \     / |
     * |   mid   |
     * | /     \ |
     * c4 ----- c3
     * */

    // Identify which plane the point is located and use the
    // plane expression for that plane

    double x        = xyc1c2c3c4xyz[0];
    double y        = xyc1c2c3c4xyz[1];
    double c1       = xyc1c2c3c4xyz[2];
    double c2       = xyc1c2c3c4xyz[3];
    double c3       = xyc1c2c3c4xyz[4];
    double c4       = xyc1c2c3c4xyz[5];
    double apex_x   = xyc1c2c3c4xyz[6];
    double apex_y   = xyc1c2c3c4xyz[7];
    double apex_z   = xyc1c2c3c4xyz[8];

    glm::vec3 corner1(-1, 1, c1);
    glm::vec3 corner2( 1, 1, c2);
    glm::vec3 corner3( 1,-1, c3);
    glm::vec3 corner4(-1,-1, c4);
    glm::vec3 apex(apex_x, apex_y, apex_z);

    // Get the potential points (x,y) might lies in
    glm::vec3 point_sequence[5];
    point_sequence[0] = corner1;
    point_sequence[1] = corner2;
    point_sequence[2] = corner3;
    point_sequence[3] = corner4;
    point_sequence[4] = corner1;

    double z = 0;
    for (int i = 0; i < 4; i++) {
        // Check if the (x,y) lies in the triangle projection
        // formed by p1 and p2 and apex onto the z = 0 plane
        glm::vec3 p1 = point_sequence[i];
        glm::vec3 p2 = point_sequence[i + 1];
        glm::vec3 v1 = p1 - p2;
        glm::vec3 v2 = apex - p2;

        // Projected to zero-plane for v1 and v2
        glm::vec3 point = glm::vec3(x, y, 0) - glm::vec3(p2.x, p2.y, 0);
        glm::vec3 t1 = glm::cross(glm::vec3(v1.x, v1.y, 0), point);
        glm::vec3 t2 = glm::cross(point, glm::vec3(v2.x, v2.y, 0));

        // Same sign, point in triangle
        if ((t1.z <= 0 && t2.z <= 0) || (t1.z >= 0 && t2.z >= 0)) {
            // Compute z 
            glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
            z = (-normal.x * (x - p2.x) - normal.y * (y - p2.y))/normal.z + p2.z;
            break;
        }
    }

    return z;
}
