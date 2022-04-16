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

        for (int i = 0; i < width; i++)
            for (int j = 0; j < length; j++)
                matrix[i][j] = 0;

        // Fill in values for matrix
        double vars[3];
        vars[2] = 0;
        for (auto func_it = functions.begin(); func_it != functions.end(); func_it++) {
            // Evaluaten function and add to terrain height map
            std::string func_string = *func_it;
            terrainParser.Parse(func_string, "x,y,N");
            // terrainParser.Optimize();
            for (int row = 0; row < width; row++) {
                for (int col = 0; col < length; col++) {
                    // Get xy coordinate by mapping x and y to [-1, 1]
                    double x = 2 * ((double) row / (double) width) - 1;
                    double y = 2 * ((double) col / (double) length) - 1;
                    
                    // Put variables for functions here
                    vars[0] = x;
                    vars[1] = y;

                    // Evaluate functions
                    matrix[row][col] += terrainParser.Eval(vars);
                }
            }
            // Increase count of layer, N
            vars[2]++;
        }

        // Finish generating one layer, push to vector
        raw_layers.push_back(std::pair(matrix, color));
    }
}

void Terrain::generate() {
    // TODO from raw_layers, generate faces, flat norms, and smooth norms
    // TODO Just plot the terrain first, layer of sea should be done later
    int num_triangles = (length - 1) * (width - 1) * 2;
	std::vector<Vertex> vertices(num_triangles * 3);
    std::vector<std::vector<glm::vec3>> accumulated_normals(width, std::vector<glm::vec3>(length, glm::vec3(0))); // For each vertex

    // TODO Just plotting the first layer
    std::pair first_layer = raw_layers[0];
    double **heightmap = first_layer.first;
    glm::vec3 color = first_layer.second;

    for (int row = 0; row < width - 1; row++) {
        for (int col = 0; col < length - 1; col++) {
            // Add two triangle in the sqaure formed by
            // matrix[row][col], matrix[row + 1][col], matrix[row + 1][col + 1], matrix[row, col + 1]

            // col   col + 1
            // c1 --- c2  row + 1
            //  |  /  |
            // c4 --- c3  row

            // Prepare indices for corner vertices
            glm::vec<2, int> c1_indx(row + 1, col    );
            glm::vec<2, int> c2_indx(row + 1, col + 1);
            glm::vec<2, int> c3_indx(row    , col + 1);
            glm::vec<2, int> c4_indx(row    , col    );
            // Scale to [-1, 1]
            glm::vec3 corner1(2 * ((double) c1_indx.x / width) - 1, 2 * ((double) c1_indx.y / length) - 1, heightmap[c1_indx.x][c1_indx.y]);
            glm::vec3 corner2(2 * ((double) c2_indx.x / width) - 1, 2 * ((double) c2_indx.y / length) - 1, heightmap[c2_indx.x][c2_indx.y]);
            glm::vec3 corner3(2 * ((double) c3_indx.x / width) - 1, 2 * ((double) c3_indx.y / length) - 1, heightmap[c3_indx.x][c3_indx.y]);
            glm::vec3 corner4(2 * ((double) c4_indx.x / width) - 1, 2 * ((double) c4_indx.y / length) - 1, heightmap[c4_indx.x][c4_indx.y]);
            // Correct axe with height as z to height as y, and y to -z
            double tmp;
            tmp = corner1.z;
            corner1.z = -corner1.y;
            corner1.y = tmp;

            tmp = corner2.z;
            corner2.z = -corner2.y;
            corner2.y = tmp;

            tmp = corner3.z;
            corner3.z = -corner3.y;
            corner3.y = tmp;

            tmp = corner4.z;
            corner4.z = -corner4.y;
            corner4.y = tmp;

            // Calculating top triangle face norm and smooth norm
            // formed by c4, c1, c2
            glm::vec3 top_v1 = glm::normalize(corner1 - corner2);
            glm::vec3 top_v2 = glm::normalize(corner4 - corner2);
            glm::vec3 face_norm_top  = -glm::normalize(glm::cross(top_v1, top_v2));

            // Calculating botton triangle
            // formed by c2, c3, c4
            glm::vec3 bot_v1 = glm::normalize(corner3 - corner4);
            glm::vec3 bot_v2 = glm::normalize(corner2 - corner4);
            glm::vec3 face_norm_bot  = -glm::normalize(glm::cross(bot_v1, bot_v2));

            // Push back triangle vertices and norms data into vector
            vertices[(row * (length - 1) + col) * 6 + 0].pos = corner4;
            vertices[(row * (length - 1) + col) * 6 + 1].pos = corner1;
            vertices[(row * (length - 1) + col) * 6 + 2].pos = corner2;
            vertices[(row * (length - 1) + col) * 6 + 3].pos = corner2;
            vertices[(row * (length - 1) + col) * 6 + 4].pos = corner3;
            vertices[(row * (length - 1) + col) * 6 + 5].pos = corner4;

            vertices[(row * (length - 1) + col) * 6 + 0].face_norm = face_norm_top;
            vertices[(row * (length - 1) + col) * 6 + 1].face_norm = face_norm_top;
            vertices[(row * (length - 1) + col) * 6 + 2].face_norm = face_norm_top;
            vertices[(row * (length - 1) + col) * 6 + 3].face_norm = face_norm_bot;
            vertices[(row * (length - 1) + col) * 6 + 4].face_norm = face_norm_bot;
            vertices[(row * (length - 1) + col) * 6 + 5].face_norm = face_norm_bot;

            // corner1: one 90 degree for top normal
            // corner2: two 45 degree for top and bot normal
            // corner3: one 90 degree for bot normal
            // corner4: two 45 degree for top and bot normal
            accumulated_normals[c1_indx.x][c1_indx.y] += face_norm_top;
            accumulated_normals[c1_indx.x][c1_indx.y] += face_norm_top + face_norm_bot;
            accumulated_normals[c1_indx.x][c1_indx.y] += face_norm_top;
            accumulated_normals[c1_indx.x][c1_indx.y] += face_norm_top + face_norm_bot;
        }
    }

    // Normalize acculated normals
    for (auto row_it = accumulated_normals.begin(); row_it != accumulated_normals.end(); row_it++) {
        auto row = *row_it;
        for (auto cell_it = row.begin(); cell_it != row.end(); cell_it++) {
            *cell_it = glm::normalize(*cell_it);
        }
    }

    // Assigning smooth normals
    for (int row = 0; row < width - 1; row++) {
        for (int col = 0; col < length - 1; col++) {
            // Same order as c4->c1->c2; c2->c3->c4; two triangles
            vertices[(row * (length - 1) + col) * 6 + 0].smooth_norm = accumulated_normals[row][col];
            vertices[(row * (length - 1) + col) * 6 + 1].smooth_norm = accumulated_normals[row + 1][col];
            vertices[(row * (length - 1) + col) * 6 + 2].smooth_norm = accumulated_normals[row + 1][col + 1];
            vertices[(row * (length - 1) + col) * 6 + 3].smooth_norm = accumulated_normals[row + 1][col + 1];
            vertices[(row * (length - 1) + col) * 6 + 4].smooth_norm = accumulated_normals[row + 0][col + 1];
            vertices[(row * (length - 1) + col) * 6 + 5].smooth_norm = accumulated_normals[row][col];
        }
    }

    std::cout << glm::to_string(vertices[0].pos) << std::endl;
    std::cout << glm::to_string(vertices[0].face_norm) << std::endl;
    std::cout << glm::to_string(vertices[1].pos) << std::endl;
    std::cout << glm::to_string(vertices[1].face_norm) << std::endl;std::cout << glm::to_string(vertices[2].pos) << std::endl;
    std::cout << glm::to_string(vertices[2].face_norm) << std::endl;

    // Load into OpenGL
	vcount = (GLsizei)vertices.size();
    // Load vertices into OpenGL
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(glm::vec3)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Terrain::draw() {
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vcount);
	glBindVertexArray(0);
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
    return perlin_device.noise2D(x * f, y * f);
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
        glm::vec3 v3 = apex - p1;

        // Projected to zero-plane for v1 and v2
        glm::vec3 point = glm::vec3(x, y, 0) - glm::vec3(p2.x, p2.y, 0);
        glm::vec3 t1 = glm::cross(glm::vec3(v1.x, v1.y, 0), point);
        glm::vec3 t2 = glm::cross(point, glm::vec3(v2.x, v2.y, 0));
        point = glm::vec3(x, y, 0) - glm::vec3(p1.x, p1.y, 0);
        glm::vec3 t3 = glm::cross(glm::vec3(v3.x, v3.y, 0), point);

        // Same sign, point in triangle
        if ((t1.z <= 0 && t2.z <= 0 && t3.z <= 0) || (t1.z >= 0 && t2.z >= 0 && t3.z >= 0)) {
            // Compute z 
            glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
            z = (-normal.x * (x - p2.x) - normal.y * (y - p2.y))/normal.z + p2.z;
            break;
        }
    }
    return z;
}
