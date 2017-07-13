
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
	IF_RGBA32F = GL_RGBA32F,	//!<	four channel, 32 bit per channel.  probably can't render to this in iOS.
	IF_RGBA16F = GL_RGBA16F_EXT,	//!<	four channel, 16 bit per channel.
	IF_Depth24 = GL_DEPTH_COMPONENT24,
	IF_Depth16 = GL_DEPTH_COMPONENT16,
};
enum PixelFormat	{
	PF_None = 0,
	PF_Depth = GL_DEPTH_COMPONENT,
	PF_RGBA = GL_RGBA,
};
enum PixelType	{
	PT_Float = GL_FLOAT,
	PT_UByte = GL_UNSIGNED_BYTE,
	PT_HalfFloat = GL_HALF_FLOAT,
	PT_UShort88 = GL_UNSIGNED_SHORT_8_8_APPLE
};
