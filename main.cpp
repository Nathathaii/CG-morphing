#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "ShaderUtil.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLuint LoadTexture(const char* filePath)
{
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
	if (!data) {
		std::cerr << "Failed to load texture" << std::endl;
		return 0;
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	return texture;
}


float* calCenter(const float facePoints[4][3]) {
	float cx = 0.0f, cy = 0.0f, cz = 0.0f;
	for (int i = 0; i < 4; i++) {
		cx += facePoints[i][0];
		cy += facePoints[i][1];
		cz += facePoints[i][2];
	}
	cx /= 4;
	cy /= 4;
	cz /= 4;

	float* center = new float[3];
	center[0] = cx;
	center[1] = cy;
	center[2] = cz;
	return center;
}

int createCubePlane(const float facePoints[4][3], float* ver, int ind) {
	float* center = calCenter(facePoints);
	
	//4*(3+2) = 20
	int i = ind;
	float shader_x[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
	float shader_y[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	while (i < ind + 20) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 3; k++) {
				ver[i] = facePoints[j][k];
				i++;
			}	
			//add shader
			ver[i] = shader_x[j]; i++;
			ver[i] = shader_y[j]; i++;

			for (int k = 0; k < 3; k++) {
				ver[i] = facePoints[(j + 1) % 4][k];
				i++;
			}
			//add shader
			ver[i] = shader_x[(j + 1) % 4]; i++;
			ver[i] = shader_y[(j + 1) % 4]; i++;

			for (int k = 0; k < 3; k++) {
				ver[i] = center[k];
				i++;
			}
			ver[i] = 0.5f; i++;
			ver[i] = 0.5f; i++;
		}
	};      
	return i;
}

float* createCubeVertex(int width) {
	float* ver = new float[120];
	float a = width / 2;
	float points[8][3];
	for (int i = 0; i < 8; i++) {
		int x = a; int y = a; int z = a;
		if (i < 4) {
			z = -a;
		}
		if (i % 4 == 0 || i % 4 == 3) {
			x = -a;
		}
		if (i % 4 == 2 || i % 4 == 3) {
			y = -a;
		}
		points[i][0] = x;
		points[i][1] = y;
		points[i][2] = z;
	}
	
	//facepoints
	int ver_ind = 0;

	float facePoints[4][3];

	// Front and back faces
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			facePoints[i][j] = points[i][j];
		}
	}
	ver_ind = createCubePlane(facePoints, ver, ver_ind);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			facePoints[i][j] = points[i + 4][j];
		}
	}
	ver_ind = createCubePlane(facePoints, ver, ver_ind);
	
	// Sides
	for (int i = 0; i < 4; i++) {
		int ind = i;
		for (int j = 0; j < 3; j++) {
			facePoints[0][j] = points[ind][j];
			facePoints[1][j] = points[(ind + 1) % 4 + 4][j];
			facePoints[2][j] = points[(ind + 1) % 4][j];
			facePoints[3][j] = points[ind + 4][j];
		}
		ver_ind = createCubePlane(facePoints, ver, ver_ind);
	}

	//last face: left face
	facePoints[0][3] = points[0][3];
	facePoints[1][3] = points[4][3];
	facePoints[2][3] = points[7][3];
	facePoints[3][3] = points[3][3];
	ver_ind = createCubePlane(facePoints, ver, ver_ind);

	return ver;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	int window_width = 1024;
	int window_height = 576;

	//shader from : https://www.shadertoy.com/view/4l23DK

	window = glfwCreateWindow(window_width, window_height, "International NASA Earth Flag", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	else
	{
		fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

		glViewport(0, 0, window_width, window_height);
		glEnable(GL_DEPTH_TEST);

		ShaderUtil shaderUtil;
		shaderUtil.Load("vs.shader", "fs.shader");

		//load texture for channel0
		GLuint texture = LoadTexture("img/gray_noise.png");

		// Set the transformation matrices
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		//glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
		glm::mat4 view = glm::lookAt(
			glm::vec3(3.0f, 3.0f, 3.0f), // Camera position
			glm::vec3(0.0f, 0.0f, 0.0f), // Look at point
			glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
		);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);

		GLint modelLoc = glGetUniformLocation(shaderUtil.getProgramId(), "model");
		GLint viewLoc = glGetUniformLocation(shaderUtil.getProgramId(), "view");
		GLint projectionLoc = glGetUniformLocation(shaderUtil.getProgramId(), "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		shaderUtil.SetMat4("model", model);
		shaderUtil.SetMat4("view", view);
		shaderUtil.SetMat4("projection", projection);



		//float resolution[3] = { window_width, window_height, 1.0f };
		float resolution[3] = { 400, 400, 1.0f };

		// Points for triangle
		float points[12] = {
			-1.0f, -1.0f, // Bottom-left
			1.0f, -1.0f,  // Bottom-right
			-1.0f, 1.0f,  // Top-left

			1.0f, -1.0f,  // Bottom-right
			1.0f, 1.0f,   // Top-right
			-1.0f, 1.0f   // Top-left
		};

		float vertices[] = {
			// Positions          // Texture Coords
			// Front Face
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

			// Back Face
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

			// Left Face
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			// Right Face
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			 // Bottom Face
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

			// Top Face
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};

		float pyramidVertices[] = {

			// Base (Bottom Face)
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
			 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // Bottom-right
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // Top-right

			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // Top-right
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  // Top-left
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left

			// Side 1 (Front Face)
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
			 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // Bottom-right
			 0.0f,  0.0f,  0.5f,  0.5f, 1.0f,  // Apex

			 // Side 1 (Front Face - Second Triangle)
			 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
			  0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // Bottom-right
			  0.0f,  0.0f,  0.5f,  0.5f, 1.0f,  // Apex

			  // Side 2 (Right Face)
			   0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
			   0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  // Top-left
			   0.0f,  0.0f,  0.5f,  0.5f, 1.0f,  // Apex

			   // Side 2 (Right Face - Second Triangle)
				0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
				0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  // Top-left
				0.0f,  0.0f,  0.5f,  0.5f, 1.0f,  // Apex

				// Side 3 (Back Face)
				 0.5f,  0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
				-0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  // Top-left
				 0.0f,  0.0f,  0.5f,  0.5f, 1.0f,  // Apex

				 // Side 3 (Back Face - Second Triangle)
				  0.5f,  0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
				 -0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  // Top-left
				  0.0f,  0.0f,  0.5f,  0.5f, 1.0f,  // Apex

				  // Side 4 (Left Face)
				  -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
				  -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // Top-left
				  0.0f,  0.0f,  0.5f,  0.5f, 1.0f,   // Apex

				  // Side 4 (Left Face - Second Triangle)
				  -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,  // Bottom-left
				  -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // Top-left
				  0.0f,  0.0f,  0.5f,  0.5f, 1.0f    // Apex
		};

		// Create VAO and VBO
		unsigned int VBO, VAO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Texture coord attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		shaderUtil.Use();
		//-----------------------------

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Rotate 50 degrees per second
			float angle = glfwGetTime() * glm::radians(50.0f); 
			glm::mat4 model = glm::mat4(1.0f);                // Reset to identity matrix
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
			shaderUtil.SetMat4("model", model);


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			shaderUtil.SetUniforms(resolution, static_cast<float>(glfwGetTime()), 0);


			// Draw triangle
			//glDrawArrays(GL_TRIANGLES, 0, 6);

			// draw cube
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

		shaderUtil.Delete();

	}

	glfwTerminate();
	return 0;
}
