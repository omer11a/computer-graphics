#pragma once
#include "StdAfx.h"
#include <string>

bool readPng(
	const std::string fileName,
	unsigned int * width,
	unsigned int * height,
	unsigned char ** pixel_array,
	bool shouldInvertY = true
);