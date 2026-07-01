//Single translation unit that compiles the stb library implementations
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

//stb_vorbis.c contains its own implementation (no macro needed)
#include <stb_vorbis.c>
