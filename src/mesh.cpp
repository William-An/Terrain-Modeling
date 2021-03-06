#define NOMINMAX
#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include "terrain.hpp"

// Helper functions
int indexOfNumberLetter(std::string& str, int offset);
int lastIndexOfNumberLetter(std::string& str);
std::vector<std::string> split(const std::string &s, char delim);

// Vertex constructor
Mesh::Vertex::Vertex() :
	pos(glm::vec3(0.0f, 0.0f, 0.0f)),
	face_norm(glm::vec3(1.0f, 0.0f, 0.0f)),
	smooth_norm(glm::vec3(0.0f, 1.0f, 0.0f)) {}

// Constructor - load mesh from file
Mesh::Mesh(std::string filename, bool keepLocalGeometry) {
	minBB = glm::vec3(std::numeric_limits<float>::max());
	maxBB = glm::vec3(std::numeric_limits<float>::lowest());

	vao = 0;
	vbuf = 0;
	vcount = 0;
	load(filename, keepLocalGeometry);
}

// Draw the mesh
void Mesh::draw() {
	terrain.draw();
	// glBindVertexArray(vao);
	// glDrawArrays(GL_TRIANGLES, 0, vcount);
	// glBindVertexArray(0);
}

// Load a wavefront OBJ file
void Mesh::load(std::string filename, bool keepLocalGeometry) {
	// Release resources
	// release();

	// TODO Temp: testing generator
	terrain.setSize(100, 100);
	terrain.setSeed(1);
	std::string name("Test");
	terrain.setName(name);
	std::vector<std::string> functions;
	glm::vec3 color(12, 323, 23);

	functions.push_back("pyramid(x, y, 0, 0, 0, 0, 0.5, -0.3, 1)");
	functions.push_back("pyramid(x, y, 0, 0, 0, 0, -0.5, 0.3, 1)");
	functions.push_back("pyramid(x, y, 0, 0, 0, 0.3, -0.3, 0, -1)");
	functions.push_back("perlin(x, y, 2^(N/2)) * 0.5");
	functions.push_back("perlin(x, y, 2^(N/2)) * 0.05");
	functions.push_back("perlin(x, y, 2^(N/2)) * 0.01");
	functions.push_back("perlin(x, y, 2^(N/2)) * 0.005");


	// terrain.pushLayer(std::pair(functions, color));
	terrain.evaluate();
	// terrain.printMatrix(0);
	terrain.generate();
	// terrain.draw();

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::stringstream ss;
		ss << "Error reading " << filename << ": failed to open file";
		throw std::runtime_error(ss.str());
	}

	// Store vertex data while reading
	std::vector<glm::vec3> raw_vertices;	// Vertices data
	std::vector<unsigned int> v_elements;	// Vertices indices forming faces

	std::string line;
	while (getline(file, line)) {
		if (line.substr(0, 2) == "v ") {
			// Read position data
			int index1 = indexOfNumberLetter(line, 2);
			int index2 = lastIndexOfNumberLetter(line);
			std::vector<std::string> values = split(line.substr(index1, index2 - index1 + 1), ' ');
			glm::vec3 vert(stof(values[0]), stof(values[1]), stof(values[2]));
			raw_vertices.push_back(vert);

			// Update bounding box
			minBB = glm::min(minBB, vert);
			maxBB = glm::max(maxBB, vert);

		} else if (line.substr(0, 2) == "f ") {
			// Read face data
			int index1 = indexOfNumberLetter(line, 2);
			int index2 = lastIndexOfNumberLetter(line);
			std::vector<std::string> values = split(line.substr(index1, index2 - index1 + 1), ' ');
			for (int i = 0; i < int(values.size()) - 2; i++) {
				// Split up vertex indices
				std::vector<std::string> v1 = split(values[0], '/');		// Triangle fan for ngons
				std::vector<std::string> v2 = split(values[i+1], '/');
				std::vector<std::string> v3 = split(values[i+2], '/');

				// Store position indices
				v_elements.push_back(stoul(v1[0]) - 1);
				v_elements.push_back(stoul(v2[0]) - 1);
				v_elements.push_back(stoul(v3[0]) - 1);
			}
		}
	}
	file.close();

	// Check if the file was invalid
	if (raw_vertices.empty() || v_elements.empty()) {
		std::stringstream ss;
		ss << "Error reading " << filename << ": invalid file or no geometry";
		throw std::runtime_error(ss.str());
	}

	// TODO ========================================================================
	// Calculate face and smoothed normals
	std::vector<glm::vec3> face_normals(v_elements.size());
	std::vector<glm::vec3> accumulated_normals(raw_vertices.size(), glm::vec3(0.0f));

	// Compute face normals first
	for (int i = 0; i< int(face_normals.size()); i += 3) {
		// All three vertices in a triangle shared the same face normal
		// ? Order of the two edges?
		int A = v_elements[i];
		int B = v_elements[i + 1];
		int C = v_elements[i + 2];
		glm::vec3 AB = glm::normalize(raw_vertices[B] - raw_vertices[A]);
		glm::vec3 AC = glm::normalize(raw_vertices[C] - raw_vertices[A]);
		glm::vec3 n  = glm::normalize(glm::cross(AB, AC));
		face_normals[i] = n;
		face_normals[i + 1] = n;
		face_normals[i + 2] = n;

		// For angles
		glm::vec3 BA = -AB;
		glm::vec3 BC = glm::normalize(raw_vertices[C] - raw_vertices[B]);
		glm::vec3 CA = -AC;
		glm::vec3 CB = -BC;

		// Weighted accumlate
		accumulated_normals[A] += n * glm::acos(glm::dot(AB, AC));
		accumulated_normals[B] += n * glm::acos(glm::dot(BA, BC));
		accumulated_normals[C] += n * glm::acos(glm::dot(CA, CB));
	}

	// Based on face normals, compute smoothed normal for each vertices
	for (int i = 0; i < int(accumulated_normals.size()); i++) {
		accumulated_normals[i] = glm::normalize(accumulated_normals[i]);
	}

	// Create vertex array
	vertices = std::vector<Vertex>(v_elements.size());
	for (int i = 0; i < int(v_elements.size()); i += 3) {
		// Store positions
		vertices[i+0].pos = raw_vertices[v_elements[i+0]];
		vertices[i+1].pos = raw_vertices[v_elements[i+1]];
		vertices[i+2].pos = raw_vertices[v_elements[i+2]];

		// Store face and smoothed normals in each vertex
		vertices[i+0].face_norm = face_normals[i+0];
		vertices[i+1].face_norm = face_normals[i+1];
		vertices[i+2].face_norm = face_normals[i+2];

		vertices[i+0].smooth_norm = accumulated_normals[v_elements[i+0]];
		vertices[i+1].smooth_norm = accumulated_normals[v_elements[i+1]];
		vertices[i+2].smooth_norm = accumulated_normals[v_elements[i+2]];
	}
	// vcount = (GLsizei)vertices.size();

	// Load vertices into OpenGL
	// glGenVertexArrays(1, &vao);
	// glBindVertexArray(vao);

	// glGenBuffers(1, &vbuf);
	// glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	// glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	// glEnableVertexAttribArray(0);
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	// glEnableVertexAttribArray(1);
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
	// glEnableVertexAttribArray(2);
	// glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(glm::vec3)));

	// glBindVertexArray(0);
	// glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Delete local copy of geometry
	if (!keepLocalGeometry)
		vertices.clear();
}

// Release resources
void Mesh::release() {
	minBB = glm::vec3(std::numeric_limits<float>::max());
	maxBB = glm::vec3(std::numeric_limits<float>::lowest());

	vertices.clear();
	if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
	if (vbuf) { glDeleteBuffers(1, &vbuf); vbuf = 0; }
	vcount = 0;
}

int indexOfNumberLetter(std::string& str, int offset) {
	for (int i = offset; i < int(str.length()); ++i) {
		if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-' || str[i] == '.') return i;
	}
	return (int)str.length();
}
int lastIndexOfNumberLetter(std::string& str) {
	for (int i = int(str.length()) - 1; i >= 0; --i) {
		if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-' || str[i] == '.') return i;
	}
	return 0;
}
std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;

	std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }

    return elems;
}
