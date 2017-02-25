//Redwanul Mutee rm4243 02/09/17
//Game Programming HW 1


#ifdef _WINDOWS
#include <GL/glew.h>
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"

#define STB_IMAGE_IMPLEMENTATION  
#include "stb_image.h"
#include "ShaderProgram.h"



GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}


SDL_Window* displayWindow;

int main(int argc, char *argv[])
{

	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Homework 1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif


	//Setup
	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint marioTexture = LoadTexture(RESOURCE_FOLDER"mario.png");
	GLuint starTexture = LoadTexture(RESOURCE_FOLDER"star.png");
	GLuint bowserTexture = LoadTexture(RESOURCE_FOLDER"bowser.png");

	Matrix projectionMatrix;
	Matrix viewMatrix;
	Matrix modelMatrix1;
	Matrix modelMatrix2;
	Matrix modelMatrix3;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	float lastFrameTicks = 0.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClear(GL_COLOR_BUFFER_BIT);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		float vertices[] = { -1.1, -1.35, 1.1, -1.35, 1.1, 1.35, -1.1, -1.35, 1.1, 1.35, -1.1, 1.35 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, marioTexture);
		program.setModelMatrix(modelMatrix1);
		modelMatrix1.identity();
		modelMatrix1.Translate(-2.3, 0.0, 0.0);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		glBindTexture(GL_TEXTURE_2D, starTexture);
		program.setModelMatrix(modelMatrix2);
		//modelMatrix2.identity();
		modelMatrix2.Scale(0.999, 0.999, 1.0);
		modelMatrix2.Rotate(2.5 * elapsed * 45.0 * (3.1415926 / 180.0));
		glDrawArrays(GL_TRIANGLES, 0, 6);


		glBindTexture(GL_TEXTURE_2D, bowserTexture);
		program.setModelMatrix(modelMatrix3);
		modelMatrix3.identity();
		modelMatrix3.Translate(2.3, 0.0, 0.0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
