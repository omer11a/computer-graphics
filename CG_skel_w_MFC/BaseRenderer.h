#pragma once

#include "vec.h"
#include "mat.h"

class BaseRenderer {
protected:
	int m_width, m_height, min_size;

public:
	explicit BaseRenderer();
	explicit BaseRenderer(int width, int height);
	
	
};
