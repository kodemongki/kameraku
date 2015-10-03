/** $id: iwanj@users.sourceforge.net
*/

const char* KFragShader_natural = " \
precision mediump float; \
uniform sampler2D vTex; \
varying vec2 vCoord; \
void main(void) \
{ \
	gl_FragColor.rgba = texture2D(vTex, vCoord).bgra; \
} \
";

const char* KFragShader_thermal = " \
precision mediump float; \
uniform sampler2D vTex; \
varying vec2 vCoord; \
const vec3 cold = vec3(0.0, 0.0, 1.0); \
const vec3 warm = vec3(1.0, 1.0, 0.0); \
const vec3 hot = vec3(1.0, 0.0, 0.0); \
void main(void) \
{ \
	vec3 col = texture2D(vTex, vCoord).bgr; \
	float lum = (col.r + col.g + col.b) / 3.0; \
	gl_FragColor.a = 1.0; \
	if (lum < 0.5) \
	{ \
		gl_FragColor.rgb = mix(cold, warm, 2.0 * lum); \
	} \
	else \
	{ \
		gl_FragColor.rgb = mix(warm, hot, 2.0 * (lum - 0.5)); \
	} \
} \
";

const char* KFragShader_toon = " \
precision mediump float; \
uniform sampler2D vTex; \
varying vec2 vCoord; \
const float step_w = 0.0015625; \
const float step_h = 0.0027778; \
void main(void) \
{ \
    vec3 t1 = texture2D(vTex, vec2(vCoord.x - step_w, vCoord.y - step_h)).bgr; \
    vec3 t2 = texture2D(vTex, vec2(vCoord.x, vCoord.y - step_h)).bgr; \
    vec3 t3 = texture2D(vTex, vec2(vCoord.x + step_w, vCoord.y - step_h)).bgr; \
    vec3 t4 = texture2D(vTex, vec2(vCoord.x - step_w, vCoord.y)).bgr; \
    vec3 t5 = texture2D(vTex, vCoord).bgr; \
    vec3 t6 = texture2D(vTex, vec2(vCoord.x + step_w, vCoord.y)).bgr; \
    vec3 t7 = texture2D(vTex, vec2(vCoord.x - step_w, vCoord.y + step_h)).bgr; \
    vec3 t8 = texture2D(vTex, vec2(vCoord.x, vCoord.y + step_h)).bgr; \
    vec3 t9 = texture2D(vTex, vec2(vCoord.x + step_w, vCoord.y + step_h)).bgr; \
    \
    vec3 xx= t1 + 2.0 * t2 + t3 - t7 - 2.0 * t8 - t9; \
    vec3 yy = t1 - t3 + 2.0 * t4 - 2.0 * t6 + t7 - t9; \
     \
    vec3 rr = sqrt(xx * xx + yy * yy); \
    float r = t5.r < 0.25 ? 0.0 : (t5.r < 0.5 ? 0.25 : (t5.r < 0.75 ? 0.5 : 1.0)); \
    float g = t5.g < 0.25 ? 0.0 : (t5.g < 0.5 ? 0.25 : (t5.g < 0.75 ? 0.5 : 1.0)); \
    float b = t5.b < 0.25 ? 0.0 : (t5.b < 0.5 ? 0.25 : (t5.b < 0.75 ? 0.5 : 1.0)); \
    gl_FragColor.a = 1.0; \
    gl_FragColor.rgb = (1.0 - rr) * vec3(r, g, b); \
} \
";

const char* KFragShader_mono = " \
precision mediump float; \
uniform sampler2D vTex; \
varying vec2 vCoord; \
void main(void) \
{ \
	vec3 color = texture2D(vTex, vCoord).bgr; \
	float lum = color.r * 0.2125 + color.g * 0.7154 + color.b * 0.0721; \
	gl_FragColor.a = 1.0; \
	gl_FragColor.rgb = vec3(lum, lum, lum); \
} \
";

const char* KFragShader_night = " \
precision mediump float; \
uniform sampler2D vTex; \
uniform sampler2D vNoise; \
uniform float vTime; \
varying vec2 vCoord; \
void main(void) \
{ \
	vec2 uv; \
	uv.x = 0.4 * sin(vTime * 50.0); \
	uv.y = 0.4 * cos(vTime * 50.0); \
	vec3 color = texture2D(vTex, vCoord).bgr; \
	float lum = dot(vec3(0.30, 0.59, 0.11), color); \
	if (lum < 0.2) \
		color = 3.0 * color;  \
	vec3 noise = texture2D(vNoise, vec2(vCoord.x + uv.x, vCoord.y + uv.y)).rgb * 0.2; \
	vec3 night = color + noise;  \
	gl_FragColor.a = 1.0; \
	gl_FragColor.rgb = night * vec3(0.1, 0.95, 0.2); \
} \
";

const char* KFragShader_edge = " \
precision mediump float; \
uniform sampler2D vTex; \
varying vec2 vCoord; \
const float step_w = 0.0015625; \
const float step_h = 0.0027778; \
void main(void) \
{ \
	vec3 t1 = texture2D(vTex, vec2(vCoord.x - step_w, vCoord.y - step_h)).bgr; \
	vec3 t2 = texture2D(vTex, vec2(vCoord.x, vCoord.y - step_h)).bgr; \
	vec3 t3 = texture2D(vTex, vec2(vCoord.x + step_w, vCoord.y - step_h)).bgr; \
	vec3 t4 = texture2D(vTex, vec2(vCoord.x - step_w, vCoord.y)).bgr; \
	vec3 t5 = texture2D(vTex, vCoord).bgr; \
	vec3 t6 = texture2D(vTex, vec2(vCoord.x + step_w, vCoord.y)).bgr; \
	vec3 t7 = texture2D(vTex, vec2(vCoord.x - step_w, vCoord.y + step_h)).bgr; \
	vec3 t8 = texture2D(vTex, vec2(vCoord.x, vCoord.y + step_h)).bgr; \
	vec3 t9 = texture2D(vTex, vec2(vCoord.x + step_w, vCoord.y + step_h)).bgr; \
	gl_FragColor.a = 1.0; \
	gl_FragColor.rgb = (8.0 * t5) - (t1 + t2 + t3 + t4 + t6 + t7 + t8 + t9); \
} \
";

const char* KFragShader_negative = " \
precision mediump float; \
uniform sampler2D vTex; \
varying vec2 vCoord; \
void main(void) \
{ \
	vec3 color = texture2D(vTex, vCoord).bgr; \
	gl_FragColor.a = 1.0; \
	gl_FragColor.rgb = vec3(1.0 - color.r, 1.0 - color.g, 1.0 - color.b); \
} \
";
