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
		},
		{
			"NAME": "colorType",
			"TYPE": "long",
			"DEFAULT": 2,
			"VALUES": [
				0,
				1,
				2,
				3,
				4,
				5
			],
			"LABELS": [
				"BT.601",
				"Full",
				"BT.709",
				"YPbPrSD",
				"YPbPrHD",
				"T.871 (JFIF)"
			]
		}
	]
}*/

//const mat3 YCbCr_RGB_601_mat = mat3(1.164, 1.164, 1.164, 0.0, -0.392, 2.017, 1.596, -0.813, 0.0);
//const vec3 YCbCr_RGB_601_offsets = vec3(16., 128., 128.);
const mat3 YCbCr_RGB_601_mat = mat3(1.164, 1.164, 1.164, 0.0, -0.392, 2.017, 1.596, -0.813, 0.0);
const vec3 YCbCr_RGB_601_offsets = vec3(16., 128., 128.);

//const mat3 YCbCr_RGB_Full_mat = mat3(1.0, 1.0, 1.0, 0.0, -0.343, 1.765, 1.4, -0.711, 0.0);
//const vec3 YCbCr_RGB_Full_offsets = vec3(0.0, 128., 128.);
const mat3 YCbCr_RGB_Full_mat = mat3(1.0, 1.0, 1.0, 0.0, -0.344, 1.773, 1.403, -0.714, 0.0);
const vec3 YCbCr_RGB_Full_offsets = vec3(0.0, 128., 128.);

//const mat3 YCbCr_RGB_709_mat = mat3(1.164, 1.164, 1.164, 0.0, -0.213, 2.112, 1.793, -0.533, 0.0);
//const vec3 YCbCr_RGB_709_offsets = vec3(0.06274509803922, 0.50196078431373, 0.50196078431373);
const mat3 YCbCr_RGB_709_mat = mat3(1.164, 1.164, 1.164, 0.0, -0.213, 2.112, 1.793, -0.533, 0.0);
const vec3 YCbCr_RGB_709_offsets = vec3(16., 128., 128.);

const mat3 YPbPr_RGB_SD_mat = mat3(1.0, 1.0, 1.0, 0.0, -0.344, 1.772, 1.402, -0.714, 0.);
const vec3 YPbPr_RGB_SD_offsets = vec3(0., 128., 128.);

const mat3 YPbPr_RGB_HD_mat = mat3(1.0, 1.0, 1.0, 0.0, -0.187, 1.856, 1.575, -0.468, 0.0);
const vec3 YPbPr_RGB_HD_offsets = vec3(0., 128., 128.);

void main()
{
	//ivec2		iSrcCoords = ivec2(gl_FragCoord.x/2., RENDERSIZE.y-gl_FragCoord.y);
	//vec2		srcCoords = vec2(0.5, 0.5) + vec2(iSrcCoords.x, iSrcCoords.y);
	
	if (blackFlag)	{
		gl_FragColor = vec4(0., 0., 0., 1.);
		return;
	}
	
	vec2		srcCoords = vec2(floor(gl_FragCoord.x / 2.), gl_FragCoord.y);
	srcCoords += vec2(0.5, 0.0);
	
	vec4		CrY1CbY0 = IMG_PIXEL(inputImage, srcCoords);
	//	convert RGB from 0.-1. to 0.-255.  this is just easier- most eq's and whitepapers are 8-bit
	CrY1CbY0 *= vec4(255.);
	
	//	figure out which luma component we should be using depending on which fragment we're processing
	vec3			ycbcr;
	if (mod(floor(gl_FragCoord.x), 2.) != 0.0)	{
		ycbcr = vec3(CrY1CbY0.a, CrY1CbY0.r, CrY1CbY0.b);
	}
	else	{
		ycbcr = vec3(CrY1CbY0.g, CrY1CbY0.r, CrY1CbY0.b);
	}
	
	//	do the color correction using the specified mode
	if (colorType==0)	{
		vec3		rgb0 = (YCbCr_RGB_601_mat * (ycbcr - YCbCr_RGB_601_offsets));
		gl_FragColor.rgb = rgb0.bgr/vec3(255.);
	}
	else if (colorType==1)	{
		vec3		rgb0 = (YCbCr_RGB_Full_mat * (ycbcr - YCbCr_RGB_Full_offsets));
		gl_FragColor.rgb = rgb0.bgr/vec3(255.);
	}
	else if (colorType==2)	{
		vec3		rgb0 = (YCbCr_RGB_709_mat * (ycbcr - YCbCr_RGB_709_offsets));
		gl_FragColor.rgb = rgb0.bgr/vec3(255.);
	}
	else if (colorType==3)	{
		vec3		rgb0 = (YPbPr_RGB_SD_mat * (ycbcr - YPbPr_RGB_SD_offsets));
		gl_FragColor.rgb = rgb0.bgr/vec3(255.);
	}
	else if (colorType==4)	{
		vec3		rgb0 = (YPbPr_RGB_HD_mat * (ycbcr - YPbPr_RGB_HD_offsets));
		gl_FragColor.rgb = rgb0.bgr/vec3(255.);
	}
	else if (colorType==5)	{
		//	these eq's are from ITU-T T.871, the "JFIF full-range YCbCr"
		vec4		rgb0;
		rgb0.r = clamp( ycbcr[0] + (1.402 * (ycbcr[2] - 128.)), 0., 255.);
		rgb0.g = clamp( ycbcr[0] - ((0.114 * 1.772 * (ycbcr[1] - 128.)) + (0.299 * 1.402 * (ycbcr[2] - 128.)))/0.587, 0., 255.);
		rgb0.b = ycbcr[0] + (1.772 * (ycbcr[1] - 128.));
		gl_FragColor.rgb = rgb0.bgr/vec3(255.);
	}
	
	
	gl_FragColor.a = 1.0;
}
