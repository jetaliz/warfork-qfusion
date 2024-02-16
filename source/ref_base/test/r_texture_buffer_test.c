#include "r_texture_format.h"
#include "r_texture_buf.h"

#define STB_DS_IMPLEMENTATION 1
#include "stb_ds.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

void Sys_Error( const char *format, ... ){
   assert_true(false);
}

void test_alias_texture_buffer(void **state)
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
  assert_int_equal( TEXTURE_BUF_SUCCESS, res );
  assert_uint_equal( T_Size( &buffer ), 768 );
  assert_uint_equal( T_PixelW( &buffer ), 16 );
  assert_uint_equal( T_PixelH( &buffer ), 16 );
  assert_uint_equal( T_LogicalW( &buffer ), 16 );
  assert_uint_equal( T_LogicalH( &buffer ), 16 );
  assert_true( buffer.flags & TEX_BUF_IS_ALIASED);

  const int res2 = T_AliasTextureBuf( &buffer, &texturedesc, c, 128);
  assert_int_equal( TEXTURE_BUF_SUCCESS, res2 );
  
  T_FreeTextureBuf(&buffer);
  free(c);
}


void test_swap_rg_texture_buffer(void **state) {

  uint8_t src[8 * 8 * 3] = {
    228, 0, 0, 238, 198, 0, 65, 52, 0, 206, 253, 0, 45, 212, 0, 138, 147, 0, 61, 151, 0, 100, 167, 0,
    23, 15, 0, 140, 212, 0, 183, 114, 0, 189, 28, 0, 249, 121, 0, 101, 225, 0, 138, 148, 0, 233, 39, 0,
    223, 97, 0, 119, 113, 0, 140, 85, 0, 20, 64, 0, 216, 205, 0, 228, 37, 0, 222, 171, 0, 66, 160, 0,
    200, 58, 0, 5, 130, 0, 239, 32, 0, 228, 65, 0, 196, 4, 0, 17, 66, 0, 130, 134, 0, 238, 26, 0,
    69, 225, 0, 247, 35, 0, 104, 91, 0, 21, 215, 0, 65, 32, 0, 196, 58, 0, 172, 223, 0, 230, 85, 0,
    32, 168, 0, 205, 49, 0, 32, 38, 0, 223, 24, 0, 149, 233, 0, 73, 39, 0, 21, 29, 0, 127, 246, 0,
    11, 196, 0, 181, 48, 0, 2, 207, 0, 105, 42, 0, 200, 238, 0, 188, 207, 0, 97, 21, 0, 54, 174, 0,
    170, 127, 0, 91, 24, 0, 243, 121, 0, 182, 91, 0, 122, 55, 0, 23, 103, 0, 94, 2, 0, 169, 217, 0
  };
  uint8_t expected[8 * 8 * 3] = {
    0, 228, 0, 198, 238, 0, 52, 65, 0, 253, 206, 0, 212, 45, 0, 147, 138, 0, 151, 61, 0, 167, 100, 0,
    15, 23, 0, 212, 140, 0, 114, 183, 0, 28, 189, 0, 121, 249, 0, 225, 101, 0, 148, 138, 0, 39, 233, 0,
    97, 223, 0, 113, 119, 0, 85, 140, 0, 64, 20, 0, 205, 216, 0, 37, 228, 0, 171, 222, 0, 160, 66, 0,
    58, 200, 0, 130, 5, 0, 32, 239, 0, 65, 228, 0, 4, 196, 0, 66, 17, 0, 134, 130, 0, 26, 238, 0,
    225, 69, 0, 35, 247, 0, 91, 104, 0, 215, 21, 0, 32, 65, 0, 58, 196, 0, 223, 172, 0, 85, 230, 0,
    168, 32, 0, 49, 205, 0, 38, 32, 0, 24, 223, 0, 233, 149, 0, 39, 73, 0, 29, 21, 0, 246, 127, 0,
    196, 11, 0, 48, 181, 0, 207, 2, 0, 42, 105, 0, 238, 200, 0, 207, 188, 0, 21, 97, 0, 174, 54, 0,
    127, 170, 0, 24, 91, 0, 121, 243, 0, 91, 182, 0, 55, 122, 0, 103, 23, 0, 2, 94, 0, 217, 169, 0
  };
  struct texture_buf_s buffer = {0};
  struct texture_buf_desc_s texturedesc = { 
    .width = 8, 
    .height = 8, 
    .def = R_BaseFormatDef( R_FORMAT_RGB8_UNORM ), 
    .alignment = 1 
  };
  const int res = T_AliasTextureBuf( &buffer, &texturedesc, src, sizeof(src));
  assert_int_equal( TEXTURE_BUF_SUCCESS, res);
  enum texture_logical_channel_e swizzle[] = {
    R_LOGICAL_C_GREEN,
    R_LOGICAL_C_RED,
    R_LOGICAL_C_BLUE,
  };
  T_SwizzleInplace(&buffer, swizzle);
  assert_memory_equal(expected, src, sizeof(expected));
}

void test_promote_textuire_buffer(void **state) {
  uint8_t* c = malloc(768);
  struct texture_buf_s buffer = {0};
  struct texture_buf_desc_s texturedesc = { 
    .width = 16, 
    .height = 16, 
    .def = R_BaseFormatDef( R_FORMAT_RGB8_UNORM ), 
    .alignment = 1 
  };
  const int res = T_AliasTextureBuf( &buffer, &texturedesc, c, 768);
  assert_int_equal( TEXTURE_BUF_SUCCESS, res );

  T_PromteTextureBuf(&buffer);
  assert_ptr_not_equal(buffer.buffer, c );
  assert_false(buffer.flags & TEX_BUF_IS_ALIASED);

  T_FreeTextureBuf(&buffer);
  free(c);
}

int main( void )
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_alias_texture_buffer ),
		cmocka_unit_test( test_swap_rg_texture_buffer ),
		cmocka_unit_test( test_promote_textuire_buffer ),
	};

	return cmocka_run_group_tests( tests, NULL, NULL );
}
