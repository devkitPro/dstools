#include <string>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <climits>
#include <cstdio>
#include "image.h"

FIBITMAP *loadImage(const char *filename)
{
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
	FIBITMAP *temp = FreeImage_Load(fif, filename);
	if (temp == NULL) return (FIBITMAP*)NULL;

	FIBITMAP *temp2 = FreeImage_ConvertTo16Bits555(temp); // quantisize to 555
	FreeImage_Unload(temp);
	FIBITMAP *dib = FreeImage_ConvertTo24Bits(temp2); // the FI paletter-thingie likes 24 bit
	FreeImage_Unload(temp2);
	if (dib == NULL) return (FIBITMAP*)NULL;

	FreeImage_FlipVertical(dib);
	return dib;
}

bool saveImage(FIBITMAP * dib, const char *filename)
{
	assert(dib != NULL);
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
	FreeImage_FlipVertical(dib);
	return (TRUE == FreeImage_Save(fif, dib, filename));
}

// this routine takes advantage of that the palettizer seems to use the low color-indices only
int countUniqueColors(FIBITMAP *dib)
{
	assert(dib != NULL);
	assert(FIC_PALETTE == FreeImage_GetColorType(dib));

	unsigned char *bits = FreeImage_GetBits(dib);
	int pitch = FreeImage_GetPitch(dib);
	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	int max = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (bits[x] > max) max = bits[x];
		}
		bits += pitch;
	}
	return max + 1;
}

bool colorEqual(const RGBQUAD &c1, const RGBQUAD &c2, int err)
{
	if (abs(c1.rgbRed   - c2.rgbRed  ) > err) return false;
	if (abs(c1.rgbGreen - c2.rgbGreen) > err) return false;
	if (abs(c1.rgbBlue  - c2.rgbBlue ) > err)  return false;
	return true;
}

int colorDiff(const RGBQUAD &c1, const RGBQUAD &c2)
{
	int maxDiff = 0;
	if (abs(c1.rgbRed   - c2.rgbRed  ) > maxDiff) maxDiff = abs(c1.rgbRed   - c2.rgbRed  );
	if (abs(c1.rgbGreen - c2.rgbGreen) > maxDiff) maxDiff = abs(c1.rgbGreen - c2.rgbGreen );
	if (abs(c1.rgbBlue  - c2.rgbBlue ) > maxDiff) maxDiff = abs(c1.rgbBlue  - c2.rgbBlue);
	return maxDiff;
}

RGBQUAD colorLerp(const RGBQUAD &c1, const RGBQUAD &c2, float t)
{
	RGBQUAD result;
	result.rgbRed   =  int(c1.rgbRed * t + c2.rgbRed * (1-t));
	result.rgbGreen =  int(c1.rgbGreen * t + c2.rgbGreen * (1-t));
	result.rgbBlue  =  int(c1.rgbBlue * t + c2.rgbBlue * (1-t));
	return result;
}

bool compressTexture(const char *filename)
{
	FIBITMAP *dib = loadImage(filename);
	if (NULL == dib)
	{
		fprintf(stderr, "failed to load image");
		return false;
	}

	const int tileWidth = 4;
	const int tileHeight = 4;
	const int colorErr = 8;

	Image img(dib);
	if (img.getWidth() % tileWidth != 0 || img.getHeight() % tileHeight != 0 )
	{
		fprintf(stderr, "width and height must be dividable by 4!");
		return false;
	}

	Image decompResult;
	decompResult.create(img.getWidth(), img.getHeight(), 24);

	std::vector<RGBQUAD> palette;
	std::vector<unsigned char> blocks;
	std::vector<unsigned short> headers;

	RGBQUAD padColor;
	padColor.rgbRed   = 0;
	padColor.rgbGreen = 0;
	padColor.rgbBlue  = 0;

	int lerpable = 0;
	int nonlerpable = 0;

	for (int y = 0; y < img.getHeight(); y += tileHeight)
	{
		for (int x = 0; x < img.getHeight(); x += tileWidth)
		{
			RGBQUAD lerpColors[2];
			int colorCount = 4;
			Image tile = FreeImage_Copy(img.getFIBitmap(), x, y, x + tileWidth, y + tileHeight);
			Image tileQuant = FreeImage_ColorQuantizeEx(tile.getFIBitmap(), FIQ_WUQUANT, colorCount, 0, NULL);
			assert(NULL != tileQuant.getFIBitmap());

			BITMAPINFO *info = FreeImage_GetInfo(tileQuant.getFIBitmap());
			colorCount = countUniqueColors(tileQuant.getFIBitmap());
			int colorPadCount = colorCount;

			// add palette to global palette
			RGBQUAD *pal = FreeImage_GetPalette(tileQuant.getFIBitmap());

			int blockMode = 2; // if all else fails
			if (colorCount == 4)
			{
				if (colorEqual(pal[1], colorLerp(pal[0], pal[3], 0.375), 2*colorErr) &&
					colorEqual(pal[2], colorLerp(pal[0], pal[3], 0.625), 2*colorErr))
				{
					lerpable++;
					lerpColors[0] = pal[0];
					lerpColors[1] = pal[3];

					BYTE a = 1;
					BYTE b = 3;
					FreeImage_SwapColors(tileQuant.getFIBitmap(), &pal[a], &pal[b], true);
					FreeImage_SwapPaletteIndices(tileQuant.getFIBitmap(), &a, &b);

					pal = lerpColors;
					colorCount = 2;
					blockMode = 3;

				} else nonlerpable++;
			}
			else if (colorCount == 3)
			{
				if (colorEqual(pal[1], colorLerp(pal[0], pal[2], 0.5), 2*colorErr))
				{
					lerpable++;
					lerpColors[0] = pal[0];
					lerpColors[1] = pal[2];

					BYTE a = 1;
					BYTE b = 2;
					FreeImage_SwapColors(tileQuant.getFIBitmap(), &pal[a], &pal[b], true);
					FreeImage_SwapPaletteIndices(tileQuant.getFIBitmap(), &a, &b);

					pal = lerpColors;
					colorCount = colorPadCount = 2;
					blockMode = 1;
				}
				else
				{
					colorPadCount = 4; // round up, colors are allocated two by two 
					nonlerpable++;
				}
			}
			else if (colorCount == 1)
			{
				colorPadCount = 2; // round up, colors are allocated two by two 
			}
			assert((colorPadCount & 1) == 0);

			int base;
			int minDiff = INT_MAX;
			int minDiffBase = 0;

			for (base = 0; base < int(palette.size()) - (colorCount - 1); base += 2)
			{
				int currMaxDiff = 0;
				for (int i = 0; i < colorCount; ++i)
				{
					int diff = colorDiff(palette[base + i], pal[i]);
					currMaxDiff = std::max(currMaxDiff, diff);
				}

				if (currMaxDiff < minDiff)
				{
					minDiff = currMaxDiff;
					minDiffBase = base;
				}
			}

			if (minDiff < colorErr) base = minDiffBase;
			else
			{
				base = int(palette.size());
				for (int i = 0; i < colorCount; ++i) palette.push_back(pal[i]);
				for (int i = colorCount; i < colorPadCount; ++i) palette.push_back(padColor);
			}

			unsigned short header = 0;
			assert((base & 1) == 0);
			header |= (base >> 1) & ((1 << 14) - 1);
			header |= blockMode << 14;
			headers.push_back(header);

			for (int iy = tileHeight - 1; iy >= 0; --iy)
			{
				unsigned char row = 0;
				for (int ix = 0; ix < tileWidth; ++ix)
				{
					int val = tileQuant.getPixelIndex(ix, iy);
					assert((val & 3) == val);
					row |= val << (ix * 2);
				}
				blocks.push_back(row);
			}

			FreeImage_Paste(decompResult.getFIBitmap(), tileQuant.getFIBitmap(), x, y, 255);
		}
	}

	printf("lerpable pals: %d (%d non)\n", lerpable, nonlerpable);
	FIBITMAP *dib2 = FreeImage_ConvertTo24Bits(decompResult.getFIBitmap());
	std::string resultName(filename);
	resultName += std::string(".result.png");
	saveImage(dib2, resultName.c_str());
	printf("total colors: %d\n", palette.size());
	FreeImage_Unload(dib2);

	{
		std::string outName(filename);
		outName += std::string(".tiles.raw");
		FILE *fp = fopen(outName.c_str(), "wb");
		if (NULL == fp)
		{
			perror("failed to dump tiles");
			return false;
		}

		fwrite(&blocks[0], sizeof(blocks[0]), blocks.size(), fp);
		fclose(fp);
	}
	{
		std::string outName(filename);
		outName += std::string(".headers.raw");
		FILE *fp = fopen(outName.c_str(), "wb");
		if (NULL == fp)
		{
			perror("failed to dump headers");
			return false;
		}

		fwrite(&headers[0], sizeof(headers[0]), headers.size(), fp);
		fclose(fp);
	}
	{
		std::string outName(filename);
		outName += std::string(".pal");
		FILE *fp = fopen(outName.c_str(), "wb");
		if (NULL == fp)
		{
			perror("failed to dump headers");
			return false;
		}
		for (size_t i = 0; i < palette.size(); ++i)
		{
			RGBQUAD col = palette[i];
			unsigned short col555 =
				(col.rgbRed >> 3) |
				((col.rgbGreen >> 3) << 5) |
				((col.rgbBlue >> 3) << 10);
			fwrite(&col555, 2, 1, fp);
		}
		fclose(fp);
	}
	return true;
}
