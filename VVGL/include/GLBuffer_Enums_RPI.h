
enum Target	{
	Target_None,
	Target_RB,
	Target_2D = GL_TEXTURE_2D,
};
enum InternalFormat	{
	IF_None = 0,
	IF_RGB = GL_RGB,
	IF_RGBA = GL_RGBA,
	IF_RGBA16 = GL_RGBA16,
	IF_Depth16 = GL_DEPTH_COMPONENT16,
};
enum PixelFormat	{
	PF_None = 0,
	PF_Depth = GL_DEPTH_COMPONENT,
	PF_BGRA = GL_BGRA_EXT,
	PF_YCbCr_422 = GL_APPLE_rgb_422,
	PF_RGBA = GL_RGBA,
};
enum PixelType	{
	PT_Float = GL_FLOAT,
	PT_UByte = GL_UNSIGNED_BYTE,
	PT_UShort = GL_UNSIGNED_SHORT,
	PT_UShort88 = GL_UNSIGNED_SHORT_8_8_APPLE
};
