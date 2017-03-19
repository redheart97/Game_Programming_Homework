//Redwanul Mutee rm4243 03/03/17
//Game Programming HW 3

/*
Notes:
	I should've created an Entity class and used Inheritance to make the 
player, enemy, and bullet classes. Would've been less code and neater code.
Neater in the sense that all the objects could've been placed in a vector
of entities and the update/render functions could easily be called on the
elements of the vector using loops. I could've also made the code neater
by using seperate compilation but it looked like a hassle. I also didn't
add option to reset game.
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
#include <stdlib.h> 

#define STB_IMAGE_IMPLEMENTATION  
#include "stb_image.h"
#include "ShaderProgram.h"

SDL_Window* displayWindow;
enum GameState { MAIN_MENU, GAME_LEVEL, GAME_OVER };
int state;

//Setup function
ShaderProgram* Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Simplified Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 640, 360);
	return new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
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
void DrawText(ShaderProgram* program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
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

//Spritesheet class
class SheetSprite {
public:
	SheetSprite() {}
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) :
		textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}

	float size;
	float u;
	float v;
	float width;
	float height; 
	int textureID; 
	void Draw(ShaderProgram* program);
};

//SpriteSheet class draw function
void SheetSprite::Draw(ShaderProgram* program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size ,
		0.5f * size * aspect, -0.5f * size };
	
	glUseProgram(program->programID); //Not sure if needed
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID); //Not sure if needed
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

class Bullet {
public:
	Bullet() : speed(1.5f), direction(0.0f), alive(0), xPosition(0.0f), yPosition(0.0f) {}
	Bullet(ShaderProgram* program, SheetSprite spritesheet) : 
		program(program), sprite(spritesheet), speed(1.5f), direction(0.0f), alive(0), xPosition(0.0f), yPosition(0.0f) {}

	bool alive;
	float xPosition;
	float yPosition;
	float speed;
	float direction;
	Matrix modelMatrix;
	SheetSprite sprite;
	ShaderProgram* program;
	void Update(float elapsed);	
};

class Player {
public:
	Player() : xPosition(0.0f), yPosition(0.0f), speed(0.0f), alive(true), lastFrameTicks(0.0f) {}
	Player(ShaderProgram* program, SheetSprite sprite, SheetSprite shotSprite, float x, float y, float speed) :
		program(program), sprite(sprite), shotSprite(shotSprite), alive(true), xPosition(x), yPosition(y), 
		speed(speed), lastFrameTicks(0.0f), keys(SDL_GetKeyboardState(NULL)), bullet(Bullet(program,shotSprite)) {}

	bool alive;
	float xPosition;
	float yPosition;
	float speed;
	float lastFrameTicks;
	const Uint8* keys;
	Bullet bullet;
	Matrix modelMatrix;
	SheetSprite sprite;
	SheetSprite shotSprite;
	ShaderProgram* program;
	void Update();
};

class Enemy {
public:
	Enemy() : xPosition(0.0f), yPosition(0.0f), speed(0.03f), alive(true), direction(1.0f), lastFrameTicks(0.0f) {}
	Enemy(ShaderProgram* program, SheetSprite sprite, SheetSprite shotSprite, float x, float y, float speed) :
		program(program), sprite(sprite), shotSprite(shotSprite), alive(true), xPosition(x), yPosition(y),
		speed(speed), direction(1.0f), lastFrameTicks(0.0f), keys(SDL_GetKeyboardState(NULL)), bullet(Bullet(program, shotSprite)) {}

	bool alive;
	float xPosition;
	float yPosition;
	float speed;
	float direction;
	float lastFrameTicks;
	const Uint8* keys;
	Bullet bullet;
	Matrix modelMatrix;
	SheetSprite sprite;
	SheetSprite shotSprite;
	ShaderProgram* program;
	void Update(Player& player, int& enemiesAlive);
};

void Bullet::Update(float elapsed) {
	//Set modelmatrix and translation for bullet
	program->setModelMatrix(modelMatrix);
	modelMatrix.identity();
	modelMatrix.Translate(xPosition, yPosition, 0.0f);

	//If off screen then bullet is no longer alive. If bullet is alive then progress it using velocity
	if ((yPosition < -1.6 && direction == -1.0f) || (yPosition > 1.6 && direction == 1.0f)) { alive = false; }
	if (alive) { yPosition += elapsed*speed*direction; sprite.Draw(program); }
}

void Player::Update() {
	if (alive) {
		//Fps
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		//Set modelmatrix and translation for Player
		program->setModelMatrix(modelMatrix);
		modelMatrix.identity();
		modelMatrix.Translate(xPosition, yPosition, 0.0f);

		//Player shoots bullet
		if (keys[SDL_SCANCODE_SPACE]) {
			if (!bullet.alive) {
				bullet.alive = true;
				bullet.direction = 1.0f;
				bullet.xPosition = this->xPosition;
				bullet.yPosition = this->yPosition + (this->sprite.height * 2.0f);
			}
		}

		//Moving Right and Left
		if (keys[SDL_SCANCODE_RIGHT] && ((xPosition + (sprite.width / 2)) < 1.05f)) {
			xPosition += elapsed * speed;
		}
		else if (keys[SDL_SCANCODE_LEFT] && ((xPosition - (sprite.width / 2)) > -1.05f)) {
			xPosition -= elapsed * speed;
		}
		
		//Draw player sprite and Update player bullet
		sprite.Draw(program);
		bullet.Update(elapsed);
	}
} 

void Enemy::Update(Player& player, int& enemiesAlive) {
	if (alive) {
		//Fps
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		//Enemy Movement: Slowly progress enemy to the right, drops down a bit and reset xPosition once xPosition > 1
		xPosition += direction * speed * elapsed;
		
		//Set modelmatrix and translation for Enemy
		program->setModelMatrix(modelMatrix);
		modelMatrix.identity();
		modelMatrix.Translate(xPosition, yPosition, 0.0f);

		//Enemy Shoots Bullet: 1/35000 chance that enemy fires its bullet
		int probEnemyFires = rand() % 35000 + 1;
		if (player.alive && !bullet.alive && probEnemyFires == 7) {
			bullet.alive = true;
			bullet.direction = -1.0f;
			bullet.xPosition = this->xPosition;
			bullet.yPosition = this->yPosition - (this->sprite.height * 2.0f);
		}

		//Collision Detection between enemy and player bullet: enemy dies, player bullet "dies", enemy numbers decrease by 1
		if (xPosition + sprite.width > player.bullet.xPosition + player.bullet.sprite.width &&
			xPosition - sprite.width < player.bullet.xPosition - player.bullet.sprite.width &&
			yPosition + sprite.height > player.bullet.yPosition - player.bullet.sprite.height &&
			yPosition - sprite.height < player.bullet.yPosition + player.bullet.sprite.height)
		{
			player.bullet.alive = false;
			alive = false;
			enemiesAlive--;
		}

		//Collision Detection between player and enemy bullet: player dies, enemy bullet "dies"
		if (player.xPosition + player.sprite.width > bullet.xPosition + bullet.sprite.width &&
			player.xPosition - player.sprite.width < bullet.xPosition - bullet.sprite.width &&
			player.yPosition + player.sprite.height > bullet.yPosition - bullet.sprite.height &&
			player.yPosition - player.sprite.height < bullet.yPosition + bullet.sprite.height)
		{
			bullet.alive = false;
			player.alive = false;
		}

		//Collision Detection between player and enemy: player dies, enemy dies
		if (player.xPosition + player.sprite.width < xPosition + sprite.width &&
			player.xPosition - player.sprite.width > xPosition - sprite.width &&
			player.yPosition + player.sprite.height > yPosition - sprite.height &&
			player.yPosition - player.sprite.height < yPosition + sprite.height)
		{
			player.alive = false;
			player.bullet.alive = false;
			alive = false;
			bullet.alive = false;
		}

		//Draw enemy sprite and update enemy bullet
		sprite.Draw(program);
		bullet.Update(elapsed);
	}
}


int main(int argc, char* argv[])
{
	//ShaderProgram setup and various matricies
	ShaderProgram* program = Setup();
	Matrix mainMenuModelMatrix;
	Matrix gameOverModelMatrix;
	Matrix viewMatrix;
	Matrix projectionMatrix;
	projectionMatrix.setOrthoProjection(-1.1f, 1.1f, -1.6f, 1.6f, -1.0f, 1.0f);

	//Loading sprite sheet and font sheet
	GLuint spriteSheet = LoadTexture(RESOURCE_FOLDER"sheet.png");
	GLuint fontSheet = LoadTexture(RESOURCE_FOLDER"font.png");

	//Creating sprites for player, enemy and their bullets.
	SheetSprite playerSprite(spriteSheet, 224.0f / 1024.0f, 832.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f, 0.15f);
	SheetSprite enemySprite1(spriteSheet, 120.0f / 1024.0f, 604.0f / 1024.0f, 104.0f / 1024.0f, 84.0f / 1024.0f, 0.15f);
	SheetSprite enemySprite2(spriteSheet, 143.0f / 1024.0f, 293.0f / 1024.0f, 104.0f / 1024.0f, 84.0f / 1024.0f, 0.15f);
	SheetSprite enemySprite3(spriteSheet, 120.0f / 1024.0f, 520.0f / 1024.0f, 104.0f / 1024.0f, 84.0f / 1024.0f, 0.15f);
	SheetSprite enemySprite4(spriteSheet, 133.0f / 1024.0f, 412.0f / 1024.0f, 104.0f / 1024.0f, 84.0f / 1024.0f, 0.15f);
	SheetSprite playerBulletSprite(spriteSheet, 858.0f / 1024.0f, 230.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.1f);
	SheetSprite enemyBulletSprite(spriteSheet, 849.0f / 1024.0f, 310.0f / 1024.0f, 9.0f / 1024.0f, 37.0f / 1024.0f, 0.1f);

	//Creating instance of player and vector of enemies. Also creating the shape of the enemy herd: 4 by 7 array
	Player player(program,playerSprite,playerBulletSprite,0.0f,-0.95f,1.0f);
	std::vector<Enemy> enemies;
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 7; column++) {
			SheetSprite tempEnemySprite;
			if (row == 0) { tempEnemySprite = enemySprite1; }
			else if (row == 1) { tempEnemySprite = enemySprite2; }
			else if (row == 2) { tempEnemySprite = enemySprite3; }
			else if (row = 3) { tempEnemySprite = enemySprite4; }
			Enemy enemy(program, tempEnemySprite, enemyBulletSprite, -0.85f + (column * 0.30f), 1.2f - (row * 0.35f), 0.04f);
			enemies.push_back(enemy);
		}
	}
	int enemiesAlive = enemies.size();

	//GameLoop and EventLoop
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		//Setting projection and view matrix
		glClear(GL_COLOR_BUFFER_BIT);
		program->setProjectionMatrix(projectionMatrix);
		program->setViewMatrix(viewMatrix);
		glUseProgram(program->programID);

		//Using switch-case statements for different game states
		switch (state) {
			case MAIN_MENU:
				//Setting matrix to dispay text for Main_Menu State
				program->setModelMatrix(mainMenuModelMatrix);
				mainMenuModelMatrix.identity();
				mainMenuModelMatrix.Translate(-1.05f, 0.0f, 0.0f);
				DrawText(program, fontSheet, "Press Enter or Space to Start Game!", 0.070f, 0.00001f);
				if (keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_RETURN2] || keys[SDL_SCANCODE_SPACE]) { state = GAME_LEVEL; }
				break;

			case GAME_LEVEL:
				if (!player.alive || (enemiesAlive <= 0)) { state = GAME_OVER; }
				else {
					player.Update();
					for (size_t i = 0; i < enemies.size(); i++) {
						enemies[i].Update(player, enemiesAlive);
						//If enemies hit the x-boundaries, change their x-direciton and progress them down just a bit
						if (((enemies[i].xPosition + (enemies[i].sprite.width / 2)) > 1.05f) || 
							((enemies[i].xPosition - (enemies[i].sprite.width / 2)) < -1.05f)) {
							for (Enemy& e : enemies) {
								e.direction *= -1.0f;
								e.yPosition -= 0.1f;
							}
						}
					}
				}
				break;

			case GAME_OVER:
				//Setting matrix to dispay text for Game_Over State
				program->setModelMatrix(gameOverModelMatrix);
				gameOverModelMatrix.identity();
				gameOverModelMatrix.Translate(-1.05f, 0.0f, 0.0f);
				gameOverModelMatrix.Scale(1.1f,1.1f,0.0f);
				//Win and loss conditions displayed
				if (!player.alive) { DrawText(program, fontSheet, "You have lost! Press Space to Quit.", 0.070f, 0.00001f); }
				else { DrawText(program, fontSheet, "You have won! Press Space to Quit.", 0.070f, 0.00001f); }
				//Option to exit game. No option to reset game yet.
				if (keys[SDL_SCANCODE_SPACE]) { done = true; }
				break;
			}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
