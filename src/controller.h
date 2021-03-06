#pragma once
#include <iostream>

class Engine;

struct SUserInputState
{
	bool firePressed = false;
	bool accelaratePressed = false;
	bool deccelaratePressed = false;
	int  accelarateTick = 0;
	bool leftPressed = false;
	bool rightPressed = false;
	bool upPressed = false;
	bool downPressed = false;
	bool menuPressed = false;

	float left_right_value = 0.0f;
	float up_down_value = 0.0f;


	void reset()
	{
		accelarateTick = 0;
		left_right_value = 0.0f;
		up_down_value = 0.0f;
	}

	void printDebug()
	{
	#define DEBUGUSERSTATE(key) std::cout << #key " status: " << ((key) ? "pressed" : "unpressed" ) << std::endl;

		std::cout << std::endl;

        DEBUGUSERSTATE(firePressed)
        DEBUGUSERSTATE(accelaratePressed)
        DEBUGUSERSTATE(deccelaratePressed)
        DEBUGUSERSTATE(leftPressed)
        DEBUGUSERSTATE(rightPressed)
        DEBUGUSERSTATE(upPressed)
        DEBUGUSERSTATE(downPressed)
        DEBUGUSERSTATE(menuPressed)

		std::cout << std::endl;
	}
};

class Controller
{
	public:
		Controller() {}
		virtual ~Controller() {}
		virtual void update(Engine& engine) = 0;
};
