
enum Target	{
	Target_None,
	Target_RB,
	Target_2D = GL_TEXTURE_2D,
	Target_Cube = GL_TEXTURE_CUBE_MAP
};
enum InternalFormat	{
	IF_None = 0,
	IF_R = GL_RED,
	IF_RGB = GL_RGB,
	IF_RGBA = GL_RGBA,
	IF_RGBA32F = GL_RGBA32F_ARB,
	IF_Depth24 = GL_DEPTH_COMPONENT24,
	IF_RGB_DXT1 = GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
	IF_RGBA_DXT5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
	IF_YCoCg_DXT5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
	IF_A_RGTC = GL_COMPRESSED_RED_RGTC1
};
enum PixelFormat	{
	PF_None = 0,
	PF_Depth = GL_DEPTH_COMPONENT,
	PF_R = GL_RED,
	PF_BGRA = GL_BGRA,
	PF_YCbCr_422 = GL_YCBCR_422_APPLE,
	PF_RGBA = GL_RGBA,
};
enum PixelType	{
	PT_Float = GL_FLOAT,
	PT_UByte = GL_UNSIGNED_BYTE,
	PT_UInt_8888_Rev = GL_UNSIGNED_INT_8_8_8_8_REV,
	PT_HalfFloat = GL_HALF_FLOAT,
	PT_UShort88 = GL_UNSIGNED_SHORT_8_8_APPLE
};
