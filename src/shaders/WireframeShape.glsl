

// keep the two blank lines at the top so that line numbers in the debug messages
// match up
#define PI 3.14159265359

#define PI_HALF 1.57079632679
#define PI2 6.28318530718
#define PI_INV 0.318309886

#define SQRT2 1.41421356237
#define HALFSQRT2 0.70710678118

vec2 Perpendicular(const vec2 dir)
{
	return vec2(-dir.y, dir.x);
}

vec2 ShortestVecToLine(const vec2 point, const vec2 on_line, const vec2 dir)
{
	return abs(dot((on_line-point), Perpendicular(dir)))*(on_line-point);
}

float DistanceToLine(const vec2 point, const vec2 on_line, const vec2 dir)
{
	return  abs(dot((on_line-point), Perpendicular(dir)));
}

vec4 UnpackColor(uint color)
{
	uvec4 masks = uvec4(0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	uvec4 shifts = uvec4(0, 8, 16, 24);
	uvec4 cvec = (uvec4 (color, color, color, color) & masks) >> shifts;
	return vec4(cvec)*vec4(0.00392157);
}


vec3 HueToRGB(float hue)
{
	vec3 s = vec3(1.0, -1.0, -1.0);
	return sqrt(clamp(vec3(-1.0,2.0, 2.0)+s
		*abs(vec3(3.0, 2.0, 4.0 )+vec3(-hue*6.0)), 0.0, 1.0));
}

vec4 Green(){ return vec4(0.0, 1.0, 0.0, 1.0);}

// special markup for shader parsing:
//{akj:use geometry}


#define geometry_main main
#define vertex_main main
#define fragment_main main 

//////////////////////////////////////////////////////////////////
// Shared Uniforms
//////////////////////////////////////////////////////////////////


uniform mat4 uProjectionMatrix;
uniform float uCurrentTime;
uniform vec4 uScreenSize;


//////////////////////////////////////////////////////////////////
// Shared varyings
//////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////
// Vertex shader only
//////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER
layout(location = 0) in vec2 aTexCoord;
layout(location = 1) in vec2 aShiftDirection;
layout(location = 2) in vec2 aPos;
layout(location = 3) in vec2 aShapeExtent;
layout(location = 4) in vec4 aCornerRadius;
layout(location = 5) in float aStrokeWidth;
layout(location = 6) in uint aStrokeColor;
layout(location = 7) in uint aFillColor;
layout(location = 8) in float aDepth;

noperspective out vec2 vsPos;
flat out vec3 vsToEdge;
flat out vec2 vsCenter;


void vertex_main()
{
	vec2 texcoord = aTexCoord*aShapeExtent + aShiftDirection*vec2(aStrokeWidth+1);
	vsToEdge = vec3(0.5);
	vsPos = aPos+texcoord;
	vsCenter = vsPos;
	//world_vertex += 0.001*noise_fac;

	gl_Position = uProjectionMatrix*(vec4(aPos+texcoord, 0.0, 1.0));
}

#endif
//////////////////////////////////////////////////////////////////
// Geometry shader only
//////////////////////////////////////////////////////////////////
#ifdef GEOMETRY_SHADER
noperspective in vec2 vsPos[];
flat in vec3 vsToEdge[];
flat in vec2 vsCenter[];


noperspective out vec2 gsPos;
flat out vec3  gsToEdge;
flat out vec2  gsCenter;

layout (triangles) in;
layout (triangle_strip, max_vertices=9) out;

void geometry_main()
{
	
	vec3 lengths = vec3(length(vsPos[1]-vsPos[2])
											, length(vsPos[2]-vsPos[0])
											, length(vsPos[1]-vsPos[0]) );

	float inv_sum = 1.0/(lengths.x+lengths.z+ lengths.y);
	vec2 center = (lengths.x*vsPos[0] + lengths.y*vsPos[1] + lengths.z*vsPos[2])*inv_sum;
	vec4 glCenterPos = (lengths.x*gl_in[0].gl_Position 
											+ lengths.y*gl_in[1].gl_Position 
											+ lengths.z*gl_in[2].gl_Position)
											* inv_sum;
											
	vec2 p_edge;
	float d_line;

	vec2 edge = normalize(vsPos[1] - vsPos[0]);
	p_edge = Perpendicular(edge);
	d_line = DistanceToLine(center, vsPos[1], edge);
	gsToEdge = vec3(p_edge, d_line);
	gsCenter = center;
	gsPos = vsPos[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	gsToEdge = vec3(p_edge, d_line);
	gsPos = vsPos[1];
	gsCenter = center;
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	gsToEdge = vec3(p_edge, d_line);
	gsCenter = center;
	gsPos = center;
	gl_Position = glCenterPos;
	EmitVertex();
	EndPrimitive();

	edge = normalize(vsPos[2] - vsPos[1]);
	p_edge = Perpendicular(edge);
	d_line = DistanceToLine(center, vsPos[2], edge);
	gsToEdge = vec3(p_edge, d_line);
	gsCenter = center;
	gsPos = vsPos[1];
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	gsToEdge = vec3(p_edge, d_line);
	gsPos = vsPos[2];
	gsCenter = center;
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	gsToEdge = vec3(p_edge, d_line);
	gsCenter = center;
	gsPos = center;
	gl_Position = glCenterPos;
	EmitVertex();
	EndPrimitive();

	edge = normalize(vsPos[0] - vsPos[2]);
	p_edge = Perpendicular(edge);
	d_line = DistanceToLine(center, vsPos[0], edge);
	gsToEdge = vec3(p_edge, d_line);
	gsCenter = center;
	gsPos = vsPos[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	gsToEdge = vec3(p_edge, d_line);
	gsPos = vsPos[0];
	gsCenter = center;
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	gsToEdge = vec3(p_edge, d_line);
	gsCenter = center;
	gsPos = center;
	gl_Position = glCenterPos;
	EmitVertex();
	EndPrimitive();
}


//////////////////////////////////////////////////////////////////
// Fragment shader only
//////////////////////////////////////////////////////////////////
#endif

#ifdef FRAGMENT_SHADER
noperspective in vec2 gsPos;
flat in vec3 gsToEdge;
flat in vec2 gsCenter;


out vec4 oFragColor;
void fragment_main()
{	
	// stroke edge_distance is a
	float to_center = max(1.2-abs( abs(dot(gsToEdge.xy, gsPos - gsCenter)) - gsToEdge.z ), 0.0);
	oFragColor = Green();
	oFragColor.a = to_center;
	//AlphaBlend(sc, fc);
	//oFragColor.xyz = vec3(1.0);//HueToRGB(fract(gl_FragCoord.x));
	//oFragColor.a = 1.0;
}
#endif
