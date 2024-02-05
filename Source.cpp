#include <SDL.h>
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <list>

using namespace std;

const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 600;

float cameraCenterX = 0;
float cameraCenterY = 0;

const float CAMERA_WIDTH = 12;

float pixelsPerUnit = SCREEN_WIDTH / CAMERA_WIDTH;

const float GRAVITY_ACCEL = -4;
const float horizontalSpeed = 3;

bool inputs[3];

void PrintFloat(float printing) {
	cout << printing;
	cout << '\n';
}

float GetXScreenCoord(float xCoord) {
	return pixelsPerUnit * (xCoord - cameraCenterX) + (SCREEN_WIDTH /2); 
}

float GetYScreenCoord(float yCoord) {
	return (SCREEN_HEIGHT / 2) - (yCoord - cameraCenterY) * pixelsPerUnit;
}

class Player {
public:
	float xPos;
	float yPos;

	float size;

	float xVelocity = 0;
	float yVelocity = 0;

	bool grounded = false;

	Player(int _xPos, int _yPos, float _size) {
		this->size = _size;

		this->xPos = _xPos;
		this->yPos = _yPos;
	}

	void VerticalPhysics(float deltaTime) {
		CheckGrounded();

		if (!grounded) {
			yVelocity += GRAVITY_ACCEL * deltaTime;
		}
		else if (yVelocity < 0) {
			yVelocity = -2;

			if (inputs[0]) {
				yVelocity = 3;
			}
		}
	}

	void HorizontalPhysics(float deltaTime) {
		xVelocity = 0 + -(int)inputs[1] + (int)inputs[2];
		xVelocity *= horizontalSpeed;
	}

	void AdvancePhysics(float deltaTime) {
		VerticalPhysics(deltaTime);
		HorizontalPhysics(deltaTime);
	}

	void MovePlayer(float deltaTime);

	void RenderToScreen(SDL_Surface* screen) {
		SDL_Rect playerRect;

		playerRect.x = GetXScreenCoord(xPos - (size / 2));
		playerRect.y = GetYScreenCoord(yPos + (size / 2));
		playerRect.w = size * pixelsPerUnit;
		playerRect.h = size * pixelsPerUnit;

		SDL_FillRect(screen, &playerRect, SDL_MapRGB(screen->format, 0, 0, 200));
	}

private:
	void CheckGrounded();

	void MoveX(float deltaTime);

	void MoveY(float deltaTime);
};

class Box {
	public:
		float xPos;
		float yPos;

		float width;
		float height;

		Box(float _xPos, float _yPos, float _width, float _height) {
			xPos = _xPos;
			yPos = _yPos;

			width = _width;
			height = _height;
		}

		bool IsOverlapping(Box* checking) {
			return (width + checking->width > GetMaxHorizontalDistance(checking->xPos, checking->width)) &&
				(height + checking->height > GetMaxVerticalDistance(checking->yPos, checking->height) );
		}

		bool IsOverlapping(Player* checking) {
			return (width + checking->size > GetMaxHorizontalDistance(checking->xPos, checking->size) ) &&
				(height + checking->size > GetMaxVerticalDistance(checking->yPos, checking->size) );
		}

		void RenderToScreen(SDL_Surface* screen) {
			SDL_Rect boxRect;

			boxRect.x = GetXScreenCoord(xPos - width / 2);
			boxRect.y = GetYScreenCoord(yPos + height / 2);
			boxRect.w = width * pixelsPerUnit;
			boxRect.h = height * pixelsPerUnit;

			SDL_FillRect(screen, &boxRect, SDL_MapRGB(screen->format, 200, 0, 0));
		}

	private:
		float GetMaxHorizontalDistance(float checkingXPos, float checkingWidth) {
			return abs( min(checkingXPos - checkingWidth / 2, xPos - width / 2) - max(xPos + width / 2, checkingXPos + checkingWidth / 2) );
		}

		float GetMaxVerticalDistance(float checkingYPos, float checkingHeight) {
			return abs( min(checkingYPos - checkingHeight / 2, yPos - height / 2) - max(yPos + height / 2, checkingYPos + checkingHeight / 2));
		}
};

list<Box> allBoxes;

 void Player::CheckGrounded() {
	 Box* groundCheck = new Box(xPos, yPos - size, size, size / 2);

	 grounded = false;

	 for (Box box : allBoxes) {
		 if (box.IsOverlapping(groundCheck)) {
			 grounded = true;
			 
			 break;
		 }
	 }

	 delete groundCheck;
}

 void Player::MoveX(float deltaTime) {
	 float nextXPos = xPos + xVelocity * deltaTime;

	 Box nextPosition = Box(nextXPos, yPos, size, size);

	 for (Box box : allBoxes) {
		 if (box.IsOverlapping(&nextPosition)) {
			 return;
		 }
	 }

	 xPos = nextXPos;
 }

 void Player::MoveY(float deltaTime) {
	 float nextYPos = yPos + yVelocity * deltaTime;

	 Box nextPosition = Box(xPos, nextYPos, size, size);

	 for (Box box : allBoxes) {
		 if (box.IsOverlapping(&nextPosition)) {
			 return;
		 }
	 }

	 yPos = nextYPos;
 }

void Player::MovePlayer(float deltaTime) {
	MoveX(deltaTime);
	MoveY(deltaTime);
}

void OnKeyDownEvent(SDL_Event* e) {
	switch (e->key.keysym.sym) {
		case SDLK_SPACE:
			inputs[0] = true;
			break;
		case SDLK_LEFT:
			inputs[1] = true;
			break;
		case SDLK_RIGHT:
			inputs[2] = true;
	}
}

void OnKeyUpEvent(SDL_Event* e) {
	switch (e->key.keysym.sym) {
		case SDLK_SPACE:
			inputs[0] = false;
			break;
		case SDLK_LEFT:
			inputs[1] = false;
			break;
		case SDLK_RIGHT:
			inputs[2] = false;
			break;
	}
}

int main(int argc, char* args[]) {
	SDL_Window* window = NULL;
	SDL_Surface* screen = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize Video System");
		return 0;
	}

	window = SDL_CreateWindow("Squart", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	
	if (window == NULL) {
		printf("Could not create window");
		return 0;
	}

	screen = SDL_GetWindowSurface(window);

	SDL_Event e; 
	bool quit = false; 
	
	Player test = Player(0, 0, 0.25f);
	Box testingBox = Box(0, -4, 12, 0.75f);

	allBoxes.push_front(testingBox);

	while (quit == false) { 
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			if (e.type == SDL_KEYDOWN) {
				OnKeyDownEvent(&e);
			}
			if (e.type == SDL_KEYUP) {
				OnKeyUpEvent(&e);
			}
		}

		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
		
		test.AdvancePhysics(0.016666f);
		test.MovePlayer(0.016666f);

		test.RenderToScreen(screen);
		
		testingBox.RenderToScreen(screen);

		SDL_UpdateWindowSurface(window);

		Sleep(16.666f);
	}

	return 1;
}