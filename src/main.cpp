#include <vector>
#include <cmath>
#include "plane.h"
#include "Util/perlin.h"
#include "effect.h"
#include "audiointerface.h"
#include "SMDmodel.h"
#include <memory>
#include <iostream>
#include "engine.h"
#include <cstring>

void interpretCommandLineOptions(int argc, char** argv, SCommandLineOptions& options)
{
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "--debug-gpu") == 0)
		{
			options.bDebugContext = true;
		}
		if (strcmp(argv[i], "--with-vulkan") == 0)
		{
			options.bWithVulkan = true;
		}
	}
}

int main(int argc, char** argv)
{
	SCommandLineOptions options;
	interpretCommandLineOptions(argc, argv, options);

	auto& engine = Engine::get();
	engine.startup(options);

	float plane1color[] = {0.8f, 0.8f, 0.8f};
	float plane2color[] = {0.0f, 0.4f, 0.4f};

	auto p1 = std::make_unique <Plane>(Vec3(32, 3, 1.0));
	auto p2 = std::make_unique <Plane>(Vec3(32, 61, 11.0));

	auto controller1 = std::make_unique <PlanePlayerController>(p1.get());
	auto controller2 = std::make_unique <PlaneAIController>(p2.get());

	p1->setColor(plane1color);
	p2->setColor(plane2color);

	engine.setPlayerEntity(p1.get());
	engine.addWorldEntity(std::move(p1));
	engine.addWorldEntity(std::move(p2));
	engine.addController(std::move(controller1));
	engine.addController(std::move(controller2));

	/* program main loop */
	engine.enterGameLoop();
}
