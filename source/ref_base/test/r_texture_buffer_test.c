#include "r_texture_format.h"
#include "unity.h"
#include "r_texture_buf.h"

#define STB_DS_IMPLEMENTATION 1
#include "stb_ds.h"

void Sys_Error( const char *format, ... ){
}

void setUp(void)
{
}

void tearDown()
{
}



void test_CreateAliasTextureBuffer(void)
{
  uint8_t* c = malloc(768);
  struct texture_buf_s buffer = {0};
  struct texture_buf_desc_s texturedesc = { 
    .width = 16, 
    .height = 16, 
    .def = R_BaseFormatDef( R_FORMAT_RGB8_UNORM ), 
    .alignment = 1 
  };
  const int res = T_AliasTextureBuf( &buffer, &texturedesc, c, 768);
  TEST_ASSERT_EQUAL_INT( TEXTURE_BUF_SUCCESS, res );
  TEST_ASSERT_EQUAL_INT( T_Size(&buffer), 768 );
  TEST_ASSERT_EQUAL_INT( T_PixelW(&buffer), 16);
  TEST_ASSERT_EQUAL_INT( T_PixelH(&buffer), 16);
  TEST_ASSERT_EQUAL_INT( T_LogicalW(&buffer), 16);
  TEST_ASSERT_EQUAL_INT( T_LogicalH(&buffer), 16);
  TEST_ASSERT_TRUE( buffer.flags & TEX_BUF_IS_ALIASED);

  const int res2 = T_AliasTextureBuf( &buffer, &texturedesc, c, 128);
  TEST_ASSERT_EQUAL_INT( TEXTURE_BUF_SUCCESS, res2 );
  
  T_FreeTextureBuf(&buffer);
  free(c);
}

void test_PromoteTextureBuffer(void) {
  uint8_t* c = malloc(768);
  struct texture_buf_s buffer = {0};
  struct texture_buf_desc_s texturedesc = { 
    .width = 16, 
    .height = 16, 
    .def = R_BaseFormatDef( R_FORMAT_RGB8_UNORM ), 
    .alignment = 1 
  };
  const int res = T_AliasTextureBuf( &buffer, &texturedesc, c, 768);
  TEST_ASSERT_EQUAL_INT( TEXTURE_BUF_SUCCESS, res );

  T_PromteTextureBuf(&buffer);
  TEST_ASSERT_NOT_EQUAL(buffer.buffer, c );
  TEST_ASSERT_FALSE(buffer.flags & TEX_BUF_IS_ALIASED);


  T_FreeTextureBuf(&buffer);
  free(c);

}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST( test_CreateAliasTextureBuffer );
	RUN_TEST( test_PromoteTextureBuffer );
	return UNITY_END();
}
