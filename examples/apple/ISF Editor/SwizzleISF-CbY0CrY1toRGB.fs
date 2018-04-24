/*{
	"DESCRIPTION": "makes several assumptions: first, inputImage is YCbCr image data that has been loaded into OpenGL as an RGBA texture.  second, that RENDERSIZE is twice the width of inputImage, so if inputImage is 50x100, the RENDERSIZE should be 100x100.  decodes the YCbCr image data into an RGB image.",
	"CREDIT": "shader code taken from v002 and various web posts",
	"CATEGORIES": [
		"TEST-GLSL FX"
	],
	"INPUTS": [
		{
			"NAME": "inputImage",
			"TYPE": "image"
		},
		{
			"NAME": "blackFlag",
			"TYPE": "bool",
			"DEFAULT": 0
		}
	]
}*/

//	YCbCr -> RGB
const mat3 YCbCr_RGB_601_mat = mat3(1.164, 1.164, 1.164, 0.0, -0.392, 2.017, 1.596, -0.813, 0.0);
const vec3 YCbCr_RGB_601_offsets = vec3(0.06274509803922, 0.50196078431373, 0.50196078431373);

const mat3 YCbCr_RGB_Full_mat = mat3(1.0, 1.0, 1.0, 0.0, -0.343, 1.765, 1.4, -0.711, 0.0);
const vec3 YCbCr_RGB_Full_offsets = vec3(0.0, 0.50196078431373, 0.50196078431373);

const mat3 YCbCr_RGB_709_mat = mat3(1.164, 1.164, 1.164, 0.0, -0.213, 2.112, 1.793, -0.533, 0.0);
const vec3 YCbCr_RGB_709_offsets = vec3(0.06274509803922, 0.50196078431373, 0.50196078431373);

void main()
{
	//ivec2		iSrcCoords = ivec2(gl_FragCoord.x/2., RENDERSIZE.y-gl_FragCoord.y);
	//vec2		srcCoords = vec2(0.5, 0.5) + vec2(iSrcCoords.x, iSrcCoords.y);
	
	if (blackFlag)	{
		gl_FragColor = vec4(0., 0., 0., 1.);
		return;
	}
	
	vec2		srcCoords = vec2(floor(gl_FragCoord.x / 2.), gl_FragCoord.y);
	srcCoords += vec2(0.5, 0.5);
	
	vec4		CrY1CbY0 = IMG_PIXEL(inputImage, srcCoords);
	
	if (mod(floor(gl_FragCoord.x), 2.) != 0.0)	{
		vec3		Y0CbCr = vec3(CrY1CbY0.a, CrY1CbY0.r, CrY1CbY0.b);
		vec3		rgb0 = (YCbCr_RGB_601_mat * (Y0CbCr - YCbCr_RGB_601_offsets));
		gl_FragColor.rgb = rgb0.bgr;
	}
	else	{
		vec3		Y1CbCr = vec3(CrY1CbY0.g, CrY1CbY0.r, CrY1CbY0.b);
		vec3		rgb1 = (YCbCr_RGB_601_mat * (Y1CbCr - YCbCr_RGB_601_offsets));
		gl_FragColor.rgb = rgb1.bgr;
	}
	
	gl_FragColor.a = 1.0;
	
	//gl_FragColor = vec4(isf_FragNormCoord.x, isf_FragNormCoord.y, 0., 1.);
	//gl_FragColor = IMG_THIS_NORM_PIXEL(inputImage);
}
