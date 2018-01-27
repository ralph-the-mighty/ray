#pragma once
#include <windows.h>


struct offscreen_buffer {
	BITMAPINFO Info;
	int Width;
	int Height;
	int Pitch;
	unsigned char* Buffer;
};