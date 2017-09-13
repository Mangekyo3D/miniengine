#pragma once
#include <string>

class Shader 
{
public:
	enum class EType {
		eVertex,
		eFragment,
		eGeometry,
		eTesselationEval,
		eTesselationControl,
		eCompute
	};

	Shader(std::string filename, EType type);
	~Shader();

	bool compile();
	void unload();
	EType getType() { return m_type; }
	uint32_t getID() { return m_ID; }

private:
	int shaderTypeToGLType(EType type);

	uint32_t m_ID;
	std::string m_filename;
	EType m_type;
};
