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
	bool shouldInvertY
) {
	std::vector<unsigned char> pixels;
	unsigned error = lodepng::decode(pixels, *width, *height, fileName, LCT_RGBA);

	// If there's an error, display it.
	if (error != 0) {
		std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		return false;
	}
	*pixel_array = new unsigned char[pixels.size()];

	int wInc = (*width) * 4;//width in char
	for (int i = 0; i < (*height) / 2; i++) {
		int top = i*wInc;
		int bot = (*height - i - 1) * wInc;
		for (int j = 0; j < wInc; j++) {
			// Swap the chars around.
			if (shouldInvertY) {
				(*pixel_array)[top + j] = pixels[bot + j];
				(*pixel_array)[bot + j] = pixels[top + j];
			} else {
				(*pixel_array)[top + j] = pixels[top + j];
				(*pixel_array)[bot + j] = pixels[bot + j];
			}
		}
	}

	return true;
}