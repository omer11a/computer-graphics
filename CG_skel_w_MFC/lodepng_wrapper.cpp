#include "StdAfx.h"
#include "lodepng.h"
#include "lodepng_wrapper.h"
#include <iostream>
#include <string>
#include <vector>

bool readPng(
	const std::string fileName,
	unsigned int * width,
	unsigned int * height,
	unsigned char ** pixel_array,
	bool containsNormals)
{
	std::vector<unsigned char> pixels;
	LodePNGColorType colorType = containsNormals ? LCT_RGB : LCT_RGBA;
	int element_size = containsNormals ? 3 : 4;
	unsigned error = lodepng::decode(pixels, *width, *height, fileName, colorType);

	// If there's an error, display it.
	if (error != 0) {
		std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		return false;
	}
	*pixel_array = new unsigned char[pixels.size()];

	int wInc = (*width) * element_size;//width in char
	for (int i = 0; i < (*height) / 2; i++) {
		int top = i*wInc;
		int bot = (*height - i - 1) * wInc;
		for (int j = 0; j < wInc; j++) {
			// Swap the chars around.
			(*pixel_array)[top + j] = pixels[bot + j];
			(*pixel_array)[bot + j] = pixels[top + j];
		}
	}

	return true;
}
