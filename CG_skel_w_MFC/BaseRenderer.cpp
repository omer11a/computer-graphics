#pragma once

#include <cmath>
#include "BaseRenderer.h"
using namespace std;

BaseRenderer::BaseRenderer() : m_width(512), m_height(512) {
	min_size = min(m_width, m_height) * 0.5;
}

BaseRenderer::BaseRenderer() : m_width(512), m_height(512) {
	min_size = min(m_width, m_height) * 0.5;
}