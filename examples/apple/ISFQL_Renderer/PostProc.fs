/*{
	"DESCRIPTION": "",
	"CREDIT": "",
	"ISFVSN": "2",
	"CATEGORIES": [
		"XXX"
	],
	"INPUTS": [
		{
			"NAME": "inputImage",
			"TYPE": "image"
		},
		{
			"NAME": "opaque",
			"TYPE": "bool",
			"DEFAULT": 1
		}
	],
	"PASSES": [
	]
	
}*/

void main()	{
	vec4		inputPixelColor = IMG_NORM_PIXEL(inputImage, isf_FragNormCoord.xy);
	
	//gl_FragColor.rgb = vec3(1.0) - inputPixelColor.rgb;
	gl_FragColor.rgb = inputPixelColor.rgb;
	
	if (opaque)	{
		gl_FragColor.a = 1.0;
	}
	else	{
		gl_FragColor.a = inputPixelColor.a;
	}
}
