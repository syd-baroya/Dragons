
#pragma once

#ifndef LAB471_SHAPE_H_INCLUDED
#define LAB471_SHAPE_H_INCLUDED

#include <string>
#include <vector>
#include <memory>

// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Program;

class Shape
{

public:

	void loadMesh(const std::string &meshName);
	void init_avg_norms();
    void init_flat_norms();
    void init();
	void resize();
	void draw(const std::shared_ptr<Program> prog, int val) const;

private:

	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf, bigPosBuf;
	std::vector<float> norBuf, bigNorBuf;
	std::vector<float> texBuf;

	unsigned int eleBufID = 0;
	unsigned int posBufID = 0;
	unsigned int norBufID = 0;
	unsigned int texBufID = 0;
	unsigned int vaoID = 0;
    
    void calc_norms();
    void calc_flat_norms();
    glm::vec3 cross(glm::vec3 a,glm::vec3 b);
    float length(glm::vec3 v);
    glm::vec3 normal_vec(glm::vec3 v);

};

#endif // LAB471_SHAPE_H_INCLUDED
