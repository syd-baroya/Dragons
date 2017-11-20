#include "Shape.h"
#include <iostream>

#include "GLSL.h"
#include "Program.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


using namespace std;


void Shape::loadMesh(const string &meshName)
{
	// Load geometry
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> objMaterials;
	string errStr;
	bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());

	if (! rc)
	{
		cerr << errStr << endl;
	}
	else if (shapes.size())
	{
		posBuf = shapes[0].mesh.positions;
		norBuf = shapes[0].mesh.normals;
		texBuf = shapes[0].mesh.texcoords;
		eleBuf = shapes[0].mesh.indices;
	}
    
}

void Shape::resize()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float scaleX, scaleY, scaleZ;
	float shiftX, shiftY, shiftZ;
	float epsilon = 0.001f;

	minX = minY = minZ = 1.1754E+38F;
	maxX = maxY = maxZ = -1.1754E+38F;

	// Go through all vertices to determine min and max of each dimension
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		if (posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
		if (posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

		if (posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
		if (posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

		if (posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
		if (posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
	}

	// From min and max compute necessary scale and shift for each dimension
	float maxExtent, xExtent, yExtent, zExtent;
	xExtent = maxX-minX;
	yExtent = maxY-minY;
	zExtent = maxZ-minZ;
	if (xExtent >= yExtent && xExtent >= zExtent)
	{
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent)
	{
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent)
	{
		maxExtent = zExtent;
	}
	scaleX = 2.0f / maxExtent;
	shiftX = minX + (xExtent / 2.0f);
	scaleY = 2.0f / maxExtent;
	shiftY = minY + (yExtent / 2.0f);
	scaleZ = 2.0f / maxExtent;
	shiftZ = minZ + (zExtent / 2.0f);

	// Go through all verticies shift and scale them
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		posBuf[3*v+0] = (posBuf[3*v+0] - shiftX) * scaleX;
		assert(posBuf[3*v+0] >= -1.0f - epsilon);
		assert(posBuf[3*v+0] <= 1.0f + epsilon);
		posBuf[3*v+1] = (posBuf[3*v+1] - shiftY) * scaleY;
		assert(posBuf[3*v+1] >= -1.0f - epsilon);
		assert(posBuf[3*v+1] <= 1.0f + epsilon);
		posBuf[3*v+2] = (posBuf[3*v+2] - shiftZ) * scaleZ;
		assert(posBuf[3*v+2] >= -1.0f - epsilon);
		assert(posBuf[3*v+2] <= 1.0f + epsilon);
	}
}

void Shape::calc_flat_norms(){
    
    bigNorBuf.assign(bigPosBuf.size(), 0.0);

    glm::vec3 v1, v2, nv;
    
    for(size_t m=0; m<bigPosBuf.size()/3; m++){

        v1.x = bigPosBuf[(m+1)*3 + 0] - bigPosBuf[m*3 + 0];
        v1.y = bigPosBuf[(m+1)*3 + 1] - bigPosBuf[m*3 + 1];
        v1.z = bigPosBuf[(m+1)*3 + 2] - bigPosBuf[m*3 + 2];
        
        v2.x = bigPosBuf[(m+2)*3 + 0] - bigPosBuf[m*3 + 0];
        v2.y = bigPosBuf[(m+2)*3 + 1] - bigPosBuf[m*3 + 1];
        v2.z = bigPosBuf[(m+2)*3 + 2] - bigPosBuf[m*3 + 2];
        
        nv = normal_vec(cross(v1,v2));
        
        bigNorBuf[m*3 + 0] = nv.x;
        bigNorBuf[m*3 + 1] = nv.y;
        bigNorBuf[m*3 + 2] = nv.z;
    
        bigNorBuf[(m+1)*3 + 0] = nv.x;
        bigNorBuf[(m+1)*3 + 1] = nv.y;
        bigNorBuf[(m+1)*3 + 2] = nv.z;
    
        bigNorBuf[(m+2)*3 + 0] = nv.x;
        bigNorBuf[(m+2)*3 + 1] = nv.y;
        bigNorBuf[(m+2)*3 + 2] = nv.z;
    }
    
}



void Shape::calc_norms(){
    
    norBuf.assign(posBuf.size(), 0.0);
    
    int t1, t2, t3;
    glm::vec3 v1, v2, nv;
    
    for(size_t m=0; m<eleBuf.size(); m+=3){
        t1 = eleBuf[m];
        t2 = eleBuf[m+1];
        t3 = eleBuf[m+2];
        

        //t1
        v1.x = posBuf[t2*3 + 0] - posBuf[t1*3 + 0];
        v1.y = posBuf[t2*3 + 1] - posBuf[t1*3 + 1];
        v1.z = posBuf[t2*3 + 2] - posBuf[t1*3 + 2];
        
        v2.x = posBuf[t3*3 + 0] - posBuf[t1*3 + 0];
        v2.y = posBuf[t3*3 + 1] - posBuf[t1*3 + 1];
        v2.z = posBuf[t3*3 + 2] - posBuf[t1*3 + 2];
        
        nv = normal_vec(cross(v1,v2));

        norBuf[t1*3 + 0] += nv.x;
        norBuf[t1*3 + 1] += nv.y;
        norBuf[t1*3 + 2] += nv.z;

        norBuf[t2*3 + 0] += nv.x;
        norBuf[t2*3 + 1] += nv.y;
        norBuf[t2*3 + 2] += nv.z;

        norBuf[t3*3 + 0] += nv.x;
        norBuf[t3*3 + 1] += nv.y;
        norBuf[t3*3 + 2] += nv.z;
    }
    
    glm::vec3 sum_vec_norm = glm::vec3(0.0,0.0,0.0);

    for(size_t n=0; n<norBuf.size()/3; n++){
        
        sum_vec_norm.x = norBuf[n*3 + 0];
        sum_vec_norm.y = norBuf[n*3 + 1];
        sum_vec_norm.z = norBuf[n*3 + 2];
        
        sum_vec_norm = normal_vec(sum_vec_norm);
        
        norBuf[n*3 + 0] = sum_vec_norm.x;
        norBuf[n*3 + 1] = sum_vec_norm.y;
        norBuf[n*3 + 2] = sum_vec_norm.z;
    }
}

glm::vec3 Shape::cross(glm::vec3 a,glm::vec3 b)
{
    glm::vec3 r;
    r.x = a.y*b.z - a.z*b.y;
    r.y = a.z*b.x - a.x*b.z;
    r.z = a.x*b.y - a.y*b.x;

    return r;
}
float Shape::length(glm::vec3 v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

glm::vec3 Shape::normal_vec(glm::vec3 v){
    glm::vec3 nv;
    float length_v = length(v);
    nv.x = v.x/length_v;
    nv.y = v.y/length_v;
    nv.z = v.z/length_v;
    
    return nv;
}

void Shape::init_flat_norms()
{
    // Initialize the vertex array object
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
    
    bigPosBuf.assign(eleBuf.size()*3, 0.0);
    
    for(size_t i=0; i<eleBuf.size(); i++){
        bigPosBuf[i*3 + 0] = posBuf[eleBuf[i]*3 + 0];
        bigPosBuf[i*3 + 1] = posBuf[eleBuf[i]*3 + 1];
        bigPosBuf[i*3 + 2] = posBuf[eleBuf[i]*3 + 2];
    }
    
    // Send the position array to the GPU
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, bigPosBuf.size()*sizeof(float), bigPosBuf.data(), GL_STATIC_DRAW);
    
    // Send the normal array to the GPU
    if (bigNorBuf.empty())
        calc_flat_norms();
    
    glGenBuffers(1, &norBufID);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferData(GL_ARRAY_BUFFER, bigNorBuf.size()*sizeof(float), bigNorBuf.data(), GL_STATIC_DRAW);
    
    // Send the texture array to the GPU
    if (texBuf.empty())
    {
        texBufID = 0;
    }
    else
    {
        glGenBuffers(1, &texBufID);
        glBindBuffer(GL_ARRAY_BUFFER, texBufID);
        glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), texBuf.data(), GL_STATIC_DRAW);
    }
    
    
    // Unbind the arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    assert(glGetError() == GL_NO_ERROR);
}

void Shape::init_avg_norms()
{
	// Initialize the vertex array object
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), posBuf.data(), GL_STATIC_DRAW);

	// Send the normal array to the GPU
    if (norBuf.empty())
        calc_norms();

    glGenBuffers(1, &norBufID);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), norBuf.data(), GL_STATIC_DRAW);

	// Send the texture array to the GPU
	if (texBuf.empty())
	{
		texBufID = 0;
	}
	else
	{
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), texBuf.data(), GL_STATIC_DRAW);
	}

	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), eleBuf.data(), GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	assert(glGetError() == GL_NO_ERROR);
}

void Shape::init()
{
    // Initialize the vertex array object
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
    
    // Send the position array to the GPU
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), posBuf.data(), GL_STATIC_DRAW);
    
    // Send the normal array to the GPU
    if (norBuf.empty())
    {
        norBufID = 0;
    }
    else
    {
        glGenBuffers(1, &norBufID);
        glBindBuffer(GL_ARRAY_BUFFER, norBufID);
        glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), norBuf.data(), GL_STATIC_DRAW);
    }
    
    // Send the texture array to the GPU
    if (texBuf.empty())
    {
        texBufID = 0;
    }
    else
    {
        glGenBuffers(1, &texBufID);
        glBindBuffer(GL_ARRAY_BUFFER, texBufID);
        glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), texBuf.data(), GL_STATIC_DRAW);
    }
    
    // Send the element array to the GPU
    glGenBuffers(1, &eleBufID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), eleBuf.data(), GL_STATIC_DRAW);
    
    // Unbind the arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog, int val) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if (h_nor != -1 && norBufID != 0)
	{
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (texBufID != 0)
	{
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if (h_tex != -1 && texBufID != 0)
		{
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}

    if(val==0){
        // Bind element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);

        // Draw
        glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);
    }
    
    else if(val==1){
        glDrawArrays(GL_TRIANGLES, 0, (int)bigPosBuf.size()/3);
    }

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
