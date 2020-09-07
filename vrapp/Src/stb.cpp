#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"


unsigned char * image_load( const char * filename, int * w, int * h, int * n )
{
	return stbi_load( filename, w, h, n, 0 );
}

unsigned char * image_load_from_memory( const char * buf, int bytes, int * w, int * h, int * n ) {
	return stbi_load_from_memory( reinterpret_cast<const unsigned char *>(buf), bytes, w, h, n, 0);
}

void image_free( unsigned char * img )
{
	stbi_image_free( img );
}

void image_store_png( const char * filename, int w, int h, int n, unsigned char * img )
{
	stbi_write_png( filename, w, h, n, img, 0 );
}

