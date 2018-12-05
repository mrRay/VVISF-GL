/*
{
  "CATEGORIES" : [
    "Generator"
  ],
  "DESCRIPTION" : "Test Card Generator",
  "CREDIT" : "Ported by Imimot. Original: https:\/\/github.com\/keijiro\/ShaderSketches\/blob\/master\/Fragment\/TestCard.glsl",
  "INPUTS": []
}
*/

float saturate(float x) { return clamp(x, 0.0, 1.0); }
vec3 saturate(vec3 x) { return clamp(x, 0.0, 1.0); }

vec3 hue2rgb(float h)
{
    h = fract(saturate(h)) * 6.0 - 2.0;
    return saturate(vec3(abs(h - 1.0) - 1.0, 2.0 - abs(h), 2.0 - abs(h - 2.0)));
}

void main(void)
{
    float scale = 27.0 / RENDERSIZE.y;                             // grid scale
    vec2 area = vec2(floor(13.0 / RENDERSIZE.y * RENDERSIZE.x), 13.0); // size of inner area

    vec2 p0 = floor(isf_FragNormCoord*RENDERSIZE) - RENDERSIZE / 2.0; // position (pixel)
    vec2 p1 = p0 * scale;                       // position (grid)

    // gray background with crosshair
    float c1 = 1.0 - step(2.0, min(abs(p0.x), abs(p0.y))) * 0.5;

    // grid lines
    vec2 grid = step(scale, abs(0.5 - fract(p1 * 0.5)));
    c1 = saturate(c1 + 2.0 - grid.x - grid.y);

    // outer area checker
    vec2 checker = step(0.49999, fract(floor(p1 * 0.5 + 0.5) * 0.5));
    if (any(greaterThan(abs(p1), area))) c1 = abs(checker.x - checker.y);

    float corner = sqrt(8.0) - length(abs(p1) - area + 4.0); // corner circles
    float circle = 12.0 - length(p1);                      // big center circle
    float mask = saturate(circle / scale);               // center circls mask

    // grayscale bars
    float bar1 = saturate(p1.y < 5.0 ? floor(p1.x / 4.0 + 3.0) / 5.0 : p1.x / 16.0 + 0.5);
    c1 = mix(c1, bar1, mask * saturate(ceil(4.0 - abs(5.0 - p1.y))));

    // basic color bars
    vec3 bar2 = hue2rgb((p1.y > -5.0 ? floor(p1.x / 4.0) / 6.0 : p1.x / 16.0) + 0.5);
    vec3 c2 = mix(vec3(c1), bar2, mask * saturate(ceil(4.0 - abs(-5.0 - p1.y))));

    // big circle line
    c2 = mix(c2, vec3(1.0), saturate(2.0 - abs(max(circle, corner)) / scale));

    gl_FragColor = vec4(c2, 1.0);
}
