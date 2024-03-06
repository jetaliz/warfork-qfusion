#include "r_texture_buffer_load.h"

#include "r_texture_buf.h"
#include "r_texture_format.h"
#include "stb_image.h"

#include "../qcommon/qfiles.h"

static uint32_t pallet[256];

bool T_LoadImagePCX(char *filename, struct texture_buf_s* buffer, uint8_t** pallet) {
	struct {
		char manufacturer;
		char version;
		char encoding;
		char bits_per_pixel;
		unsigned short xmin, ymin, xmax, ymax;
		unsigned short hres, vres;
		unsigned char palette[48];
		char reserved;
		char color_planes;
		unsigned short bytes_per_line;
		unsigned short palette_type;
		char filler[58];
		unsigned char data; // unbounded
	} *pcx;

	uint8_t *raw;
	size_t len = R_LoadFile( filename, (void **)&raw );
	pcx = (void *)raw;

	pcx->xmin = LittleShort( pcx->xmin );
	pcx->ymin = LittleShort( pcx->ymin );
	pcx->xmax = LittleShort( pcx->xmax );
	pcx->ymax = LittleShort( pcx->ymax );
	pcx->hres = LittleShort( pcx->hres );
	pcx->vres = LittleShort( pcx->vres );
	pcx->bytes_per_line = LittleShort( pcx->bytes_per_line );
	pcx->palette_type = LittleShort( pcx->palette_type );

	raw = &pcx->data;

	if( pcx->manufacturer != 0x0a || pcx->version != 5 || pcx->encoding != 1 || pcx->bits_per_pixel != 8 ||
		len < 768 ) {
		ri.Com_DPrintf( S_COLOR_YELLOW "Bad pcx file %s\n", filename );
		R_FreeFile( pcx );
		return false;
	}

	struct texture_buf_desc_s desc = {
		.alignment = 1,
		.height = pcx->ymax + 1,
		.width = pcx->xmax + 1,
		.def = R_BaseFormatDef(R_FORMAT_RGB8_UNORM)
	};
	uint8_t* mem = Q_Malloc(desc.height * desc.width * 3 + 768);
	T_AliasTextureBuf_Free(buffer, &desc, mem, buffer->width * buffer->height * 3, mem, Q_Free);
	uint8_t* pal = mem + T_Size(buffer);
	
	// copying pallet to the end of the image buffer
	memcpy( pal, (uint8_t *)pcx + len - 768, 768 );
	if(pallet) {  
		(*pallet) = pal ;
	} 
  
	int dataByte, runLength;
	for(size_t y = 0; y < T_LogicalH(buffer); y++ ) {
		for(size_t x = 0; x < T_LogicalW(buffer); ) {
			dataByte = *raw++;

			runLength = 1;
			if( ( dataByte & 0xC0 ) == 0xC0 ) {
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			} 

			while( runLength-- > 0 ) {
				uint8_t* block = &buffer->buffer[(buffer->rowPitch * y) + (x * RT_BlockSize(buffer->def))];
				block[0] = pal[dataByte * 3 + 0];
				block[1] = pal[dataByte * 3 + 1];
				block[2] = pal[dataByte * 3 + 2];
				x++;
			}
		}
	}

	R_FreeFile( raw);

	if( raw - (uint8_t *)pcx > len ) {
		ri.Com_DPrintf( S_COLOR_YELLOW "PCX file %s was malformed", filename );
		return false;
	}

	return true;

}

static void __R_stbi_free_image(void* p) {
	stbi_uc* img = (stbi_uc*) p;
	stbi_image_free(img);
}

bool T_LoadImageSTBI(char *filename, struct texture_buf_s* buffer ) {
	uint8_t* data;
	size_t size = R_LoadFile( filename, ( void ** ) &data );
	if(data == NULL) {
		ri.Com_Printf(S_COLOR_YELLOW "can't resolve file: %s", filename);
		return false;
	}

	struct texture_buf_desc_s desc = {
		.alignment = 1,
	};

	int channelCount = 0;
	int w;
	int h;
	stbi_uc* stbiBuffer = stbi_load_from_memory( data, size, &w, &h, &channelCount, 0 );
	desc.width = w;
	desc.height = h;
	switch(channelCount) {
		case 1:
			desc.def = R_BaseFormatDef(R_FORMAT_A8_UNORM);
			break;
		case 2:
			desc.def = R_BaseFormatDef(R_FORMAT_L8_A8_UNORM);
			break;
		case 3:
			desc.def = R_BaseFormatDef(R_FORMAT_RGB8_UNORM);
			break;
		case 4:
			desc.def = R_BaseFormatDef(R_FORMAT_RGBA8_UNORM);
			break;
		default:
			stbi_image_free(stbiBuffer);
			ri.Com_Printf(S_COLOR_YELLOW "unhandled channel count: %d", channelCount);
			return false;
	}
	R_FreeFile( data );
	const int res = T_AliasTextureBuf_Free(buffer, &desc, stbiBuffer, 0, stbiBuffer, __R_stbi_free_image);
	assert(res == TEXTURE_BUF_SUCCESS);
	return true;
}

void T_SetPallet(uint32_t p[256]) {
	memcpy(pallet, p, sizeof(pallet));
}


//void T_LoadPalletQ1()
//{
//	static const uint8_t host_quakepal[768] =
//#include "../qcommon/quake1pal.h"
//		;
//
//	// get the palette
//	uint8_t *raw;
//	size_t len = R_LoadFile( "gfx/palette.lmp", (void **)&raw );
//	const uint8_t* pal = ( raw && len >= 768 ) ? raw : host_quakepal;
//
//	for(size_t i = 0; i < 256; i++ ) {
//
//		uint32_t v = COLOR_RGBA( pal[i * 3 + 0], pal[i * 3 + 1], pal[i * 3 + 2], 255 );
//		pallet[i] = LittleLong( v );
//	}
//	pallet[255] = 0; // 255 is transparent
//
//	R_FreeFile( raw );
//
//}
//
//void T_LoadPalletQ2()
//{
//	uint8_t* pal = NULL;
//	struct texture_buf_s buffer = {};
//	T_LoadImagePCX( "pics/colormap.pcx", &buffer, &pal);
//	for(size_t i = 0; i < 256; i++ ) {
//		uint32_t v = COLOR_RGBA( pal[i * 3 + 0], pal[i * 3 + 1], pal[i * 3 + 2], 255 );
//		pallet[i] = LittleLong( v );
//	}
//	T_FreeTextureBuf(&buffer);
//	pallet[255] &= LittleLong( 0xffffff ); // 255 is transparent
//}

uint32_t* T_Pallet() {
	return pallet;
}

bool T_LoadImageWAL(char *filename, struct texture_buf_s* tex) {
	// load the file
	uint8_t* buf = NULL;
	R_LoadFile( filename, (void **)&buf);
	if( !buf) {
		return false;
	}

	q2miptex_t* mt = (q2miptex_t *)buf;
	size_t rows = LittleLong( mt->width );
	size_t columns = LittleLong( mt->height );
	uint8_t* data = buf + LittleLong( mt->offsets[0] );
	uint32_t size = rows * columns;

	// determine the number of channels
	size_t i = 0;
	for( i = 0; i < size && data[i] != 255; i++ ) {}	
	const unsigned samples = ( i < size ) ? 4 : 3;

  struct texture_buf_desc_s desc = {
    .width = columns,
    .height = rows,
    .alignment = 1,
    .def = (samples == 4) ? R_BaseFormatDef(R_FORMAT_RGBA8_UNORM): R_BaseFormatDef(R_FORMAT_RGB8_UNORM) 
  };
  T_ReallocTextureBuf(tex, &desc);

  for( size_t y = 0; y < T_LogicalH( tex ); y++ ) {
	  for( size_t x = 0; x < T_LogicalW( tex ); ) {
		  const size_t index = ( rows * y ) + x;
		  uint32_t *block = (uint32_t *)&tex->buffer[( tex->rowPitch * y ) + ( x * RT_BlockSize( tex->def ) )];
		  uint8_t p = data[index];
		  if( samples == 4 ) {
			  ( *block ) = pallet[p];
			  if( p == 255 ) {
				  // transparent, so scan around for another color
				  // to avoid alpha fringes
				  // FIXME: do a full flood fill so mips work...
				  if( i > rows && data[i - rows] != 255 ) {
					  p = data[i - rows];
				  } else if( i < size - rows && data[i + rows] != 255 ) {
					  p = data[i + rows];
				  } else if( i > 0 && data[i - 1] != 255 ) {
					  p = data[i - 1];
				  } else if( i < size - 1 && data[i + 1] != 255 ) {
					  p = data[i + 1];
				  } else {
					  p = 0;
				  }

				  // copy rgb components
				  ( (uint8_t *)&block[i] )[0] = ( (uint8_t *)&pallet[p] )[0];
				  ( (uint8_t *)&block[i] )[1] = ( (uint8_t *)&pallet[p] )[1];
				  ( (uint8_t *)&block[i] )[2] = ( (uint8_t *)&pallet[p] )[2];
			  }
		  } else {
			  ( (uint8_t *)&block[i] )[0] = ( (uint8_t *)&pallet[p] )[0];
			  ( (uint8_t *)&block[i] )[1] = ( (uint8_t *)&pallet[p] )[1];
			  ( (uint8_t *)&block[i] )[2] = ( (uint8_t *)&pallet[p] )[2];
		  }
	  }
  }


	R_FreeFile( mt );
	return true;
}


