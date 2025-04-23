// raster.frag
uniform sampler2D rasterTexture;
uniform float minValue;
uniform float maxValue;
varying vec2 qt_TexCoord0;

void main() {
    float value = texture2D(rasterTexture, qt_TexCoord0).r;
    float norm = clamp((value - minValue) / (maxValue - minValue), 0.0, 1.0);
    vec3 color = mix(vec3(0.1), vec3(norm, 1.0 - norm, 0.2 + 0.6 * norm), step(0.0, norm));
    gl_FragColor = vec4(color, 1.0);
}
