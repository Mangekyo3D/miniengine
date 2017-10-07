#pragma once

#include "batch.h"

class Renderer;

class WorldTile {
	public:
		WorldTile(uint16_t resolution);
		virtual ~WorldTile();
		void setup_draw_operations();
		void generateProcedural();
		float getHeightAt(float x, float y);
		float getHeightAtGrid(uint16_t x, uint16_t y);
		uint16_t getResolution() { return m_resolution; }

	private:
		uint16_t m_resolution;
		Mesh <VertexFormatVNT> m_mesh;
		CBatch* m_batch = nullptr;
};
