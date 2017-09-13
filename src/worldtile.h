#pragma once

#include "batch.h"
#include <memory>

class Renderer;

class WorldTile {
	public:
		WorldTile(uint16_t resolution);
		virtual ~WorldTile();
		void setup_draw_operations(Renderer *);
		void generateProcedural();
		float getHeightAt(float x, float y);
		float getHeightAtGrid(uint16_t x, uint16_t y);
		uint16_t getResolution() { return m_resolution; }

	private:
		uint16_t m_resolution;
		Mesh m_mesh;
		std::unique_ptr <Material> m_material;
};
