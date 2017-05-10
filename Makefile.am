# Makefile.am -- Process this file with automake to produce Makefile.in

bin_PROGRAMS = r4denc dlditool dslink nds_texcompress

r4denc_SOURCES		=	src/r4denc/r4denc.cpp

dlditool_SOURCES	=	src/dlditool/dlditool.c

dslink_SOURCES		=	src/dslink/little.h src/dslink/ndsheader.h src/dslink/main.cpp

nds_texcompress_SOURCES	=	src/nds_texcompress/nds_texcompress.cpp src/nds_texcompress/image.h src/nds_texcompress/compress_texture.cpp

nds_texcompress_LDADD	=	$(FREEIMAGE_LIBS)

EXTRA_DIST = autogen.sh
