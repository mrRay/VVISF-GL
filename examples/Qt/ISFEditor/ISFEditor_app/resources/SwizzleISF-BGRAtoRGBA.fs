/*{
	"DESCRIPTION": "Converts BGRA to RGBA",
	"CREDIT": "by zoidberg",
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

void main()
{
	gl_FragColor = (blackFlag) ? vec4(0.0, 0.0, 0.0, 1.0) : IMG_NORM_PIXEL(inputImage, isf_FragNormCoord).bgra;
	//gl_FragColor = IMG_NORM_PIXEL(inputImage, isf_FragNormCoord).bgra;
}
