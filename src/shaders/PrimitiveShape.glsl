

#define PI 3.14159265359
#define PI_HALF 1.57079632679
#define PI2 6.28318530718
#define PI_INV 0.318309886184
#define SQRT2 1.41421356237
#define HALFSQRT2 0.707106781186548


vec4 UnpackColor(uint color)
{
	uvec4 masks = uvec4(0x000000FF);
	uvec4 shifts = uvec4(0, 8, 16, 24);
	uvec4 cvec = (uvec4 (color) >> shifts) & masks;
	vec4 ret = vec4(cvec)*vec4(1.0/255.5);
	return ret;
}

vec4 AlphaBlend(vec4 top, vec4 bottom)
{
	float max_a = max(top.a, bottom.a);
	return vec4(mix(top.rgb, bottom.rgb, (max_a- top.a)/max_a ), max_a);
}


vec3 HueToRGB(float hue)
{
	vec3 s = vec3(1.0, -1.0, -1.0);
	return sqrt(clamp(vec3(-1.0,2.0, 2.0)+s*abs(vec3(3.0, 2.0, 4.0 )+vec3(-hue*6.0)), 0.0, 1.0));
}

vec4 DistanceToEdge(const vec2 pos,const vec2 point,const float radius)
{
	vec2 radial_corner = point - vec2(radius);
	vec2 radial_vec = pos-radial_corner;
	bool has_radial_bound = all(greaterThan(radial_vec, vec2(0.0)));
	float radial_length = length(radial_vec);
	
	vec2 man_dist = (point-pos);
	float manhattan_distance = min(man_dist.x, man_dist.y);
	float radial_distance = mix(manhattan_distance,(radius - radial_length), has_radial_bound) ;
	float fuzz_dist = mix(1.0, sqrt(abs(dot(radial_vec/radial_length, vec2(1.0, 1.0)))), has_radial_bound);
	return vec4( radial_distance, fuzz_dist, fuzz_dist-1.0, 1.0/fuzz_dist);
}		

// the first coord returned is the distance to the stroke line
// the second ditnce returned is the distance from the fill area
vec2 EdgeDistance(const vec2 pos,const vec2 edge, const float radius)
{
	

	vec2 to_edge = edge - pos ;
	vec2 abs_edge_vec = abs(to_edge);
	float abs_edge_d = min(abs_edge_vec.x, abs_edge_vec.y);
	float outside_edge_distance = min(to_edge.x, to_edge.y);
	
	//TODO: fix this
	to_edge -= radius;
	// to edge has the distance to the corner (negative means outside)
	// if both directions are outside, then we want to limit things
	float r = radius-distance(pos, edge-vec2(radius));
	float r_mix = step(0.0, -max(to_edge.x, to_edge.y));
	// if r is 0, then we should ignore it, otherwise it replaces abs_edge
	outside_edge_distance = -min(mix(outside_edge_distance, r, r_mix), 0);
	abs_edge_d = max(mix( abs_edge_d,abs(r), r_mix), outside_edge_distance);
	//abs_edge_d = max(abs_edge_d, outside_edge_distance);

	// we have to be limited by the distance beyond the edge
	return vec2(abs_edge_d, outside_edge_distance);

}

#ifdef FRAGMENT_SHADER
#define INOUT in
#else 
#define INOUT out
#endif

centroid INOUT vec2 vTexCoord;
centroid INOUT vec2 vShapeExtent;
flat INOUT float vCornerRadius;
flat INOUT float vStrokeWidth;
flat INOUT float vDepth;
flat INOUT float vInverseStrokeAlpha;
flat INOUT vec4 vStrokeColor;
flat INOUT vec4 vFillColor;
flat INOUT vec2 vFillTexOffset;
flat INOUT float vFillTexMix;

//////////////////////////////////////////////////////////////////
// Vertex shader only
//////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER
layout(location = 0) in vec2 aTexCoord;
layout(location = 1) in vec2 aShiftDirection;
layout(location = 2) in vec2 aPos;
layout(location = 3) in vec2 aShapeExtent;
layout(location = 4) in float aCornerRadius;
layout(location = 5) in float aStrokeWidth;
layout(location = 6) in uint aStrokeColor;
layout(location = 7) in uint aFillColor;
layout(location = 9) in vec2 aFillCoord;
layout(location = 8) in float aDepth;
layout(location = 10) in float aExtraFillAlpha;

uniform mat4 uProjectionMatrix;
uniform float uCurrentTime;

void main()
{
	vTexCoord = aTexCoord*aShapeExtent + aShiftDirection*vec2(aStrokeWidth+2);
	vShapeExtent = aShapeExtent;
	vCornerRadius = aCornerRadius;
	vStrokeWidth = aStrokeWidth;
	vStrokeColor = UnpackColor(aStrokeColor);
	vFillColor = UnpackColor(aFillColor);
	vInverseStrokeAlpha = 1.0/vStrokeColor.a;
	vDepth = aDepth;
	//world_vertex += 0.001*noise_fac;
	gl_Position = uProjectionMatrix*(vec4(aPos+vTexCoord, 0.0, 1.0));
}
#endif




//////////////////////////////////////////////////////////////////
// Fragment shader only
//////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER
layout(origin_upper_left) in vec4 gl_FragCoord;
uniform float uCurrentTime;

out vec4 oFragColor;
out float gl_FragDepth;
void main()
{	
	// stroke edge_distance is a 
	vec4 ed = DistanceToEdge(abs(vTexCoord), vShapeExtent, vCornerRadius);
	vec4 sc = vStrokeColor;
	sc.a *= mix(0.0, 1.0, clamp(vStrokeWidth+1.0-abs(ed.x)+ed.z, 0, ed.y)*ed.w);
	vec4 fc = vFillColor;
	fc.a *= mix(0.0, 1.0, clamp(1 + ed.x+ed.z, 0, ed.y)*ed.w);
	oFragColor = AlphaBlend(sc, fc);
	gl_FragDepth = mix(1.0, vDepth, oFragColor.a > 0.001);
	//AlphaBlend(sc, fc);
	//oFragColor.xyz = vec3(1.0);//HueToRGB(fract(gl_FragCoord.x));
	//oFragColor.a = 1.0;
}
#endif
