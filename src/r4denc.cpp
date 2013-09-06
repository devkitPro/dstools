/*
	Copyright(C) 2007 yasu
	             2009 WinterMute

	http://hp.vector.co.jp/authors/VA013928/
	http://www.usay.jp/
	http://www.yasu.nu/
	http://www.devkitpro.org
	
2007/04/22 21:00 - First version

2009/01/08	- combined decode/encode in single app, switch on extension (.nds/.dat)

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <string>
#include <string.h>

#define BIT_AT(n, i) ((n >> i) & 1)


void r4denc(FILE *in, FILE *out, bool decode) {

	int r, n = 0;

	unsigned char buf[512];	

	while ((r = fread(buf, 1, 512, in)) > 0) {

		unsigned short key = n ^ 0x484A;

		for (int i = 0; i < 512; i ++) {

			unsigned char xorkey = 0;
			if (key & 0x4000) xorkey |= 0x80;
			if (key & 0x1000) xorkey |= 0x40;
			if (key & 0x0800) xorkey |= 0x20;
			if (key & 0x0200) xorkey |= 0x10;
			if (key & 0x0080) xorkey |= 0x08;
			if (key & 0x0040) xorkey |= 0x04;
			if (key & 0x0002) xorkey |= 0x02;
			if (key & 0x0001) xorkey |= 0x01;
			
			if (!decode) buf[i] ^= xorkey;

			unsigned int k = ((buf[i] << 8) ^ key) << 16;
			unsigned int x = k;

			for (int j = 1; j < 32; j ++)
				x ^= k >> j;

			key = 0x0000;

			if (BIT_AT(x, 23)) key |= 0x8000;
			if (BIT_AT(k, 22)) key |= 0x4000;
			if (BIT_AT(k, 21)) key |= 0x2000;
			if (BIT_AT(k, 20)) key |= 0x1000;
			if (BIT_AT(k, 19)) key |= 0x0800;
			if (BIT_AT(k, 18)) key |= 0x0400;
			if (BIT_AT(k, 17) != BIT_AT(x, 31)) key |= 0x0200;
			if (BIT_AT(k, 16) != BIT_AT(x, 30)) key |= 0x0100;
			if (BIT_AT(k, 30) != BIT_AT(k, 29)) key |= 0x0080;
			if (BIT_AT(k, 29) != BIT_AT(k, 28)) key |= 0x0040;
			if (BIT_AT(k, 28) != BIT_AT(k, 27)) key |= 0x0020;
			if (BIT_AT(k, 27) != BIT_AT(k, 26)) key |= 0x0010;
			if (BIT_AT(k, 26) != BIT_AT(k, 25)) key |= 0x0008;
			if (BIT_AT(k, 25) != BIT_AT(k, 24)) key |= 0x0004;
			if (BIT_AT(k, 25) != BIT_AT(x, 26)) key |= 0x0002;
			if (BIT_AT(k, 24) != BIT_AT(x, 25)) key |= 0x0001;

			if (decode) buf[i] ^= xorkey;
		}
		fwrite(buf, 1, r, out);
		n ++;
	}
	
}

int main(int argc, char *argv[]) {

	puts("Yasu software - r4denc");

	if (argc < 2) {
		puts("Usage: r4dec in-file [out-file]");
		return 1;
	}

	bool decodeFlag = false;

	std::string infile = argv[1];
	std::string outfile;
	size_t lastdot;
	std::string ext, outext;
	
	if ( (lastdot = infile.rfind("."))!= std::string::npos ) {
		ext = infile.substr(lastdot);
	}

	if (argc < 3) {
		outfile = infile.substr(0,lastdot);
	} else {
		outfile = argv[2];
	}

	if (strcasecmp(ext.c_str(),".dat")==0) {
		decodeFlag=true;
		outext = ".nds";
	} else if ( strcasecmp(ext.c_str(),".nds")==0) {
		decodeFlag=false;
		outext = ".DAT";
	} else {
		printf(".nds or .dat required\n");
		exit(1);		
	}

	if ( (lastdot = outfile.rfind("."))!= std::string::npos ) {
		outfile = outfile.substr(0,lastdot);
	}
	
	outfile += outext;

	FILE *in = fopen(infile.c_str(), "rb");

	if (in == NULL) {
		printf("Error: cannot open %s for reading\n", infile.c_str());
		exit(1);
	}

	FILE *out = fopen(outfile.c_str(), "wb");

	if (out == NULL) {
		fclose(in);
		printf("Error: cannot open %s for writing\n", outfile.c_str());
		exit(1);
	}

	r4denc(in, out, decodeFlag);

	printf("%scoded %s to %s\n",decodeFlag?"de":"en",infile.c_str(),outfile.c_str());

	fclose(out);
	fclose(in);

	return 0;
}
