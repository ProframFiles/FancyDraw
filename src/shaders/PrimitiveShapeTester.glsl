#define PI 3.14159265359
#define PI_HALF 1.57079632679
#define PI2 6.28318530718
#define PI_INV 0.318309886184
#define SQRT2 1.41421356237


vec4 UnpackColor(uint color)
{
	uvec4 masks = uvec4(0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	uvec4 shifts = uvec4(0, 8, 16, 24);
	uvec4 cvec = (uvec4 (color, color, color, color) & masks) >> shifts;
	return vec4(cvec)*vec4(0.00392157);
}

vec4 AlphaBlend(vec4 top, vec4 bottom)
{
	float max_a = max(top.a, bottom.a);
	return vec4(mix(top.rgb, bottom.rgb, (max_a- top.a)/max_a ), max_a);
}

float LinOsc(const float angle)
{
	return 2.0*abs(0.5-fract(angle));
}

float NSin(const float angle)
{
	return 0.5*sin(angle)+0.5;
}

//0 == red, 1.0 == red
vec3 HueToRGB(float hue)
{
	vec3 s = vec3(1.0, -1.0, -1.0);
	return sqrt(clamp(vec3(-1.0,2.0, 2.0)+s*abs(vec3(3.0, 2.0, 4.0 )+vec3(-hue*6.0)), 0.0, 1.0));
}

// the first coord returned is the distance to the stroke line
// the second ditnce returned is the distance from the fill area
vec2 EdgeDistance(const vec2 pos,const vec2 edge, const float radius)
{
	// pretend we have a point outside the box
	// edge: (20, 20) , pos = (23, 20)
	// to_edge = -3, 0
	// abs_edge_vec = (3, 0)
	// abs_edge_d = 0
	// outside_edge_distance = 3
	// abs_edge_d = 3

	// pretend we have a point inside the box
	// edge: (20, 20) , pos = (5, 5)
	// to_edge = 15, 15
	// abs_edge_vec = (15, 15)
	// abs_edge_d = 15
	// outside_edge_distance = 0
	// abs_edge_d = 15

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
layout(location = 8) in vec2 aExtra;

uniform mat4 uProjectionMatrix;
#endif


#ifdef FRAGMENT_SHADER
#define INOUT centroid in
#else 
#define INOUT centroid out
#endif

noperspective INOUT vec2 vTexCoord;
noperspective INOUT vec2 vShapeExtent;
noperspective INOUT vec2 vPos;
flat INOUT float vCornerRadius;
flat INOUT float vStrokeWidth;
flat INOUT vec4 vStrokeColor;
flat INOUT vec4 vFillColor;


uniform float uCurrentTime;
uniform vec2 uMousePos;

#ifdef VERTEX_SHADER
void main()
{
	vTexCoord = aTexCoord*aShapeExtent + aShiftDirection*vec2(aStrokeWidth*0.5);
	
	vShapeExtent = abs(aShapeExtent);
	vCornerRadius = aCornerRadius;
	vStrokeWidth = aStrokeWidth;
	vStrokeColor = UnpackColor(aStrokeColor);
	vFillColor = UnpackColor(aFillColor);
	
	vPos = aPos+vTexCoord;
	//vPos.x += LinOsc(uCurrentTime);
	gl_Position = uProjectionMatrix*(vec4(vPos, -0.001*gl_InstanceID, 1.0));
}
#endif


//////////////////////////////////////////////////////////////////
// Fragment shader only
//////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER
//layout(origin_upper_left)
 in vec4 gl_FragCoord;
out vec4 oFragColor;

void main()
{	
	float x_dist = (vShapeExtent.x - abs(vTexCoord.x));
	float is_fractional = step(0, 1.0-x_dist);
	oFragColor.g = gl_FragCoord.x;
	oFragColor.b = vPos.x;
	oFragColor.r = vTexCoord.x;
	oFragColor.a = 1.0;
}
#endif
