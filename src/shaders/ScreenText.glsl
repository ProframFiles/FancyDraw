

#define PI 3.14159265359
#define PI2 6.28318530718
#define PI_INV 0.318309886184
#define SQRT2 1.41421356237

vec3 GetLight(){
	return vec3(15.0, 12.0, 30.0);
}

vec4 Red(){ return vec4(1.0, 0.0, 0.0, 1.0 );}
vec4 Green(){ return vec4(0.0, 1.0, 0.0, 1.0 );}
vec4 Blue(){ return vec4(0.0, 0.0, 1.0, 1.0 );}
vec4 Yellow(){ return vec4(1.0, 1.0, 0.0, 1.0 );}

float LinStep(const float bottom, const float top, const float val)
{
	return clamp((val-bottom)/(top-bottom), 0.0, 1.0 );
}

vec4 AlphaBlend(const vec4 top, const vec4 bottom)
{

	float max_a = max(top.a, bottom.a);
	return vec4(mix(top.rgb, bottom.rgb, (max_a- top.a)/max_a ), max_a);
}

vec3 HueToRGB(float hue)
{
	vec3 s = vec3(1.0, -1.0, -1.0);
	return sqrt(clamp(vec3(-1.0,2.0, 2.0)+s*abs(vec3(3.0, 2.0, 4.0 )+vec3(-hue*6.0)), 0.0, 1.0));
}

vec4 UnpackColor(uint color)
{
	uvec4 masks = uvec4(0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	uvec4 shifts = uvec4(0, 8, 16, 24);
	uvec4 cvec = (uvec4 (color, color, color, color) & masks) >> shifts;
	return vec4(cvec)*vec4(0.00392157);
}

vec4 DebugColor()
{
	return vec4(1.0, 0.0, 1.0, 1.0);
}
vec3 rotate_by_quat(vec3 v, vec4 q)
{
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w*v );
}
vec3 reverse_rotate_by_quat(vec3 v, vec4 q)
{
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) - q.w*v );
}
vec3 Screen(vec3 color, vec3 light )
{
	return 1.0 - (1.0 - color)*(1.0 - light);
}

vec3 Overlay(vec3 color, vec3 light)
{
	vec3 ba = 2.0*light*color;
	vec3 s = sign(ba-color);
	return color+s*min(abs(ba-color), abs(2.0*Screen(color, light)-color));
}
vec3 Overlay(vec3 color, float light)
{
	return Overlay(color, vec3(light));
}

// special markup for shader parsing:

#ifdef FRAGMENT_SHADER
#define DEF_VARYING(qual, type, name) qual in type gs ## name;

#elif defined(GEOMETRY_SHADER)
#define DEF_VARYING(q, t, n) q in t vs ## n ## []; q out t gs ## n;

#elif defined(VERTEX_SHADER)
#define DEF_VARYING(qual, type, name) qual out type vs ## name;

#endif

#define geometry_main main
#define vertex_main main
#define fragment_main main 

#ifdef VERTEX_SHADER


// per vertex attributes
layout(location = 0) in vec2 aPos;

#endif

//////////////////////////////////////////////////////////////////
// Shared Uniforms
//////////////////////////////////////////////////////////////////


uniform float uCurrentTime;

#ifdef VERTEX_SHADER


void vertex_main()
{
	gl_Position = (vec4(aPos, -0.1, 1.0));
}
#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D uTextTexture;
uniform sampler2D uPrimitiveTexture;
uniform sampler2D uPrimitiveDepth;

uniform vec4 uZoomOffset;


in vec4 gl_FragCoord;
out vec4 oFragColor;

void fragment_main()
{	
	ivec2 sample_pos = ivec2(floor((gl_FragCoord.xy*uZoomOffset.xy)));
	vec2 text_samp = texelFetch(uTextTexture, sample_pos, 0).rg;
	float text_depth = 1.0-text_samp.g;
	vec4 prim_samp = texelFetch(uPrimitiveTexture, sample_pos, 0);
	float prim_depth = texelFetch(uPrimitiveDepth, sample_pos, 0).x;

	float text_rgb = mix(1.0, 0.0, text_samp.r > 0.001);
	vec4 text_color = vec4(text_rgb, text_rgb, text_rgb, max(text_samp.r, 0.0));
	

	//oFragColor = vec4(text_depth, text_depth, prim_depth, 1.0);
	vec4 blended_color = mix(AlphaBlend(text_color, prim_samp),
									 AlphaBlend(prim_samp, text_color), 
									 prim_depth > text_depth);
	vec4 background_color = vec4(1.0);
	oFragColor = mix(background_color, blended_color, min(prim_depth, text_depth) < 0.99 );
	//	oFragColor = mix(Green(), Blue(), prim_depth > text_depth);
}
#endif
