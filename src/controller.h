#pragma once

class Controller
{
	public:
		Controller() {}
		virtual ~Controller(){}
		virtual void update() = 0;
};
