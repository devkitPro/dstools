#include <stdio.h>
#include <FreeImage.h>
#include "compress_texture.h"

int main(int argc, char* argv[])
{
	int exitcode = 0;

	FreeImage_DeInitialise();

	for (int i = 1; i < argc; ++i)
	{
		printf("compressing %s...\n", argv[i]);
		if (!compressTexture(argv[i]))
		{
			exitcode = -1;
			break;
		}
	}

	FreeImage_DeInitialise();

	return exitcode;
}
