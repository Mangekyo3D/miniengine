#pragma once

#include "batch.h"

class IRenderer;

class WorldTile {
	public:
		WorldTile(uint16_t resolution);
		virtual ~WorldTile();
		void setup_draw_operations(IRenderer* renderer);
		void generateProcedural();
		float getHeightAt(float x, float y);
		float getHeightAtGrid(uint16_t x, uint16_t y);
		uint16_t getResolution() { return m_resolution; }

	private:
		uint16_t m_resolution;
		Mesh <VertexFormatVNT> m_mesh;
		CIndexedInstancedBatch* m_batch = nullptr;
};
