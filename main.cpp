/* Lab 6 base code - transforms using local matrix functions
	to be written by students -
	based on lab 5 by CPE 471 Cal Poly Z. Wood + S. Sueda
	& Ian Dunn, Christian Eckhardt
*/
#include <iostream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "camera.h"
// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;






class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, prog2;

	// Shape to be used (from obj file)
	shared_ptr<Shape> avgNormAlduin, flatNormAlduin, skysphere;
	
	//camera
	camera mycam;

	//texture for sim
	GLuint Texture, TextureSky;
	GLuint Texture2;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{


		GLSL::checkVersion();

		
		// Set background color.
		glClearColor(0.12f, 0.34f, 0.56f, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		//culling:
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		//transparency
		glEnable(GL_BLEND);
		//next function defines how to mix the background color with the transparent pixel in the foreground. 
		//This is the standard:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->init();
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
        prog->addUniform("camPos");
        
        //second prog for skysphere
        
        // Initialize the GLSL program.
        prog2 = make_shared<Program>();
        prog2->setVerbose(true);
        prog2->setShaderNames(resourceDirectory + "/simple_vertsky.glsl", resourceDirectory + "/simple_fragsky.glsl");
        if (!prog2->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        prog2->init();
        prog2->addUniform("P");
        prog2->addUniform("V");
        prog2->addUniform("M");
        prog2->addAttribute("vertPos");
        prog2->addAttribute("vertNor");
        prog2->addAttribute("vertTex");


	}

	void initGeom(const std::string& resourceDirectory)
	{
        int width, height, channels;
        char filepath[1000];
        
        
        // Initialize mesh.
        skysphere = make_shared<Shape>();
        skysphere->loadMesh(resourceDirectory + "/sphere.obj");
        skysphere->resize();
        skysphere->init();
        
        //texture sky
        string str = resourceDirectory + "/sky2.jpg";
        strcpy(filepath, str.c_str());
        unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &TextureSky);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureSky);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        GLuint TexSkyLocation = glGetUniformLocation(prog2->pid, "tex");
        // Then bind the uniform samplers to texture units:
        glUseProgram(prog2->pid);
        glUniform1i(TexSkyLocation, 0);
        
        
		// Initialize mesh.
		avgNormAlduin = make_shared<Shape>();
		avgNormAlduin->loadMesh(resourceDirectory + "/alduin.obj");
		avgNormAlduin->resize();
		avgNormAlduin->init_avg_norms();
        
        flatNormAlduin = make_shared<Shape>();
        flatNormAlduin->loadMesh(resourceDirectory + "/alduin.obj");
        flatNormAlduin->resize();
        flatNormAlduin->init_flat_norms();
        
		
		

		//texture 1
		str = resourceDirectory + "/alduin.jpg";
		strcpy(filepath, str.c_str());		
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader

		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);

	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		auto P = std::make_shared<MatrixStack>();
		auto MV = std::make_shared<MatrixStack>();
		P->pushMatrix();	
		P->perspective(70., width, height, 0.1, 100.0f);
		glm::mat4 mv ,T, R, R2, Ry;

		

		MV->pushMatrix();

		MV->translate(glm::vec3(0,0,-3));

		mv = MV->topMatrix();
        glm::mat4 V = mycam.process();
		
        static float angle = 0.0;
        angle += 0.001;

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw mesh using GLSL
        prog2->bind();
        glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        
        glDisable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);
        float pih = 3.1415926 / 2.0;
        R = glm::rotate(glm::mat4(1), pih, glm::vec3(1, 0, 0));
        T = glm::translate(glm::mat4(1),-mycam.pos);
        mv = T * R;
        glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, &mv[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureSky);
        skysphere->draw(prog2,0);
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CCW);
        prog2->unbind();

		
		// Draw mesh using GLSL
		prog->bind();
        glUniform3f(prog->getUniform("camPos"), mycam.pos.x, mycam.pos.y,mycam.pos.z);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

        Ry = glm::rotate(glm::mat4(1), angle, glm::vec3(0, 0, 1));
		T = glm::translate(glm::mat4(1), glm::vec3(-1, 0, -2));
        R2 = glm::rotate(glm::mat4(1), -1.56f, glm::vec3(1,0,0));
		mv = V * T * R2*Ry;
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &mv[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
		avgNormAlduin->draw(prog, 0);

        R = glm::rotate(glm::mat4(1), 3.14f, glm::vec3(0,0,1));
        T = glm::translate(glm::mat4(1), glm::vec3(1, 0, -2));
        mv = V * T * R2 * R*Ry;
        glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &mv[0][0]);
        flatNormAlduin->draw(prog, 1);
        
		prog->unbind();
	}
};
//*********************************************************************************************************
int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}


	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(600, 600);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
