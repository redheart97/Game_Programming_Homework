//Redwanul Mutee GAME PROGRAMMING HW 6. Adding music.
/*
Added sound for paddle-ball collision and sound when player wins.
Added music for gameplay. Could add gamestates in the future to make gameplay smoother.
*/

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
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION  
#include "stb_image.h"
#include "ShaderProgram.h"
#include <SDL_mixer.h> 


SDL_Window* displayWindow;
//Setup function
ShaderProgram setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Simplified Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 640, 360);
	return ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
}

//Function to load texture
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

//Function to draw text 
void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (size_t i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//Entity class: Paddle
class Paddle {
public:
	GLuint texture;
	float left;
	float right;
	float top;
	float bottom;
	float verticies[12];
	float textCoords[12];

	void setTexture(const char *filepath) { texture = LoadTexture(filepath); }
	void draw(ShaderProgram& program, Matrix& modelMatrix) {
		glBindTexture(GL_TEXTURE_2D, texture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticies);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		program.setModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}

	Paddle() : left(0.0), right(0.0), top(0.0), bottom(0.0),
		verticies{ left, bottom, right, bottom, right, top, right, top, left, top, left, bottom },
		textCoords{ 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 } {}
	Paddle(float a, float b, float c, float d) : left(a), right(b), top(c), bottom(d),
		verticies{ left, bottom, right, bottom, right, top, right, top, left, top, left, bottom },
		textCoords{ 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 } {}
};

//Entity class: Ball
class Ball {
public:
	GLuint texture;
	int angle;
	float xPosition;
	float yPosition;
	float xDirection;
	float yDirection;
	float verticies[12];
	float textCoords[12];

	void setTexture(const char *filepath) { texture = LoadTexture(filepath); }
	void draw(ShaderProgram& program, Matrix& modelMatrix) {
		glBindTexture(GL_TEXTURE_2D, texture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticies);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		program.setModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}

	Ball() : xPosition(0.0f), yPosition(0.0f), angle(45),
		verticies{ -0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 0.1f, 0.1f, -0.1f, 0.1f, -0.1f, -0.1f },
		textCoords{ 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f }
	{
		xDirection = cos(angle * 3.1415f / 180.0f);
		yDirection = sin(angle * 3.1415f / 180.0f);
	}
	Ball(float a, float b, int c) : xPosition(a), yPosition(b), angle(c),
		verticies{ -0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 0.1f, 0.1f, -0.1f, 0.1f, -0.1f, -0.1f },
		textCoords{ 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 }
	{
		xDirection = cos(angle * 3.1415f / 180.0f);
		yDirection = sin(angle * 3.1415f / 180.0f);
	}
};

int main(int argc, char *argv[])
{
	ShaderProgram program = setup();

	//Sound Variables
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Chunk* collisionSound = Mix_LoadWAV("outOfBounds.wav");
	Mix_Chunk* outOfBoundsSound = Mix_LoadWAV("collisionSound.wav");
	Mix_Music* gamePlayMusic = Mix_LoadMUS("gamePlayMusic.mp3");

	//Creating instances of entities
	Paddle leftPaddle(-3.3f, -3.2f, 0.5f, -0.5f);
	Paddle rightPaddle(3.2f, 3.3f, 0.5f, -0.5f);
	Ball ball(0.0f, 0.0f, 45);

	//Setting textures 
	GLint font = LoadTexture(RESOURCE_FOLDER"fontA.png");
	leftPaddle.setTexture(RESOURCE_FOLDER"paddleSprite.png");
	rightPaddle.setTexture(RESOURCE_FOLDER"paddleSprite.png");
	ball.setTexture(RESOURCE_FOLDER"ball.png");

	Matrix modelMatrixText;
	Matrix modelMatrixLeftPaddle;
	Matrix modelMatrixRightPaddle;
	Matrix modelMatrixBall;
	Matrix projectionMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	float lastFrameTicks = 0.0f;
	SDL_Event event;
	std::string winner = "none";
	bool isGameOn = false;
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

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		//If game is not on
		if (!isGameOn) {
			//Print text to screen
			modelMatrixText.identity();
			modelMatrixText.Translate(-3.0f, 1.8f, 0.0f);
			program.setModelMatrix(modelMatrixText);

			if (winner == "Left Player") {
				std::string winnerText = "Left Player has won!";
				DrawText(&program, font, winnerText, 0.20f, 0);
			}
			else if (winner == "Right Player") {
				std::string winnerText = "Right Player has won!";
				DrawText(&program, font, winnerText, 0.20f, 0);
			}
			else {
				std::string instructions = "Press Space to Start Game. ";
				DrawText(&program, font, instructions, 0.20f, 0.0f);
			}
			//Start game If Space is pressed
			if (keys[SDL_SCANCODE_SPACE]) { 
				isGameOn = true; 
				Mix_PlayMusic(gamePlayMusic, -1);
			}
		}

		//Left Paddle movement
		if (keys[SDL_SCANCODE_W]) {
			if (leftPaddle.top < 2.0f) {
				modelMatrixLeftPaddle.Translate(0.0f, 5 * elapsed, 0.0f);
				leftPaddle.bottom += 5 * elapsed;
				leftPaddle.top += 5 * elapsed;
			}
		}

		if (keys[SDL_SCANCODE_S]) {
			if (leftPaddle.bottom > -2.0f) {
				modelMatrixLeftPaddle.Translate(0.0f, -5 * elapsed, 0.0f);
				leftPaddle.bottom -= 5 * elapsed;
				leftPaddle.top -= 5 * elapsed;
			}
		}

		//Right Paddle movement
		if (keys[SDL_SCANCODE_UP]) {
			if (rightPaddle.top < 2.0f) {
				modelMatrixRightPaddle.Translate(0.0f, 5 * elapsed, 0.0f);
				rightPaddle.bottom += 5 * elapsed;
				rightPaddle.top += 5 * elapsed;
			}
		}

		if (keys[SDL_SCANCODE_DOWN]) {
			if (rightPaddle.bottom > -2.0f) {
				modelMatrixRightPaddle.Translate(0.0f, -5 * elapsed, 0.0f);
				rightPaddle.bottom -= 5 * elapsed;
				rightPaddle.top -= 5 * elapsed;
			}
		}

		//If game is on
		if (isGameOn) {
			//Drawing Ball, left Paddle and right Paddle
			leftPaddle.draw(program, modelMatrixLeftPaddle);
			rightPaddle.draw(program, modelMatrixRightPaddle);
			ball.draw(program, modelMatrixBall);

			//Keeping track of ball position
			ball.xPosition += ball.xDirection*elapsed*1.5f;
			ball.yPosition += ball.yDirection*elapsed*1.5f;

			//Ball movement: if ball hits top or bottom of screen
			if (ball.yPosition + 0.1f >= 2.0f || ball.yPosition - 0.1f <= -2.0f) { ball.yDirection *= -1.0f; }

			//Ball movement: if ball hits either paddle
			else if ((ball.xPosition - 0.1f <= leftPaddle.right && ball.yPosition - 0.1f <= leftPaddle.top && ball.yPosition + 0.1f >= leftPaddle.bottom) ||
				(ball.xPosition + 0.1f >= rightPaddle.left && ball.yPosition - 0.1f <= rightPaddle.top && ball.yPosition + 0.1f >= rightPaddle.bottom))
			{
				Mix_PlayChannel(-1, collisionSound, 0);
				ball.xDirection *= -1.0f;
			}

			//If left player wins
			else if (ball.xPosition + 0.1f >= rightPaddle.right) {
				Mix_PlayChannel(-1, outOfBoundsSound, 0);
				isGameOn = false;
				winner = "Left Player";
			}

			//If right player wins
			else if (ball.xPosition - 0.1f <= leftPaddle.left) {
				Mix_PlayChannel(-1, outOfBoundsSound, 0);
				isGameOn = false;
				winner = "Right Player";
			}

			//Movement of ball realized through translations
			modelMatrixBall.Translate(ball.xDirection*elapsed*1.5f, ball.yDirection*elapsed*1.5f, 0.0f);
		}
		SDL_GL_SwapWindow(displayWindow);
	}

	//Cleanup
	Mix_FreeChunk(collisionSound);
	Mix_FreeChunk(outOfBoundsSound);
	Mix_FreeMusic(gamePlayMusic);

	SDL_Quit();
	return 0;
}