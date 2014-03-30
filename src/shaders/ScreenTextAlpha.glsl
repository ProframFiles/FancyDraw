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
	return clamp((val-bottom)/(top-bottom),0.0, 1.0);
}

vec3 HueToRGB(float hue)
{
	vec3 s = vec3(1.0, -1.0, -1.0);
	return sqrt(clamp(vec3(-1.0,2.0, 2.0)+s*abs(vec3(3.0, 2.0, 4.0 )+vec3(-hue*6.0)), 0.0, 1.0));
}

// slope is rise over run
// we return run over rise
float AbsSlope(const vec2 dxdy)
{
	vec2 abs_dxdy = abs(dxdy);
	return  min(abs_dxdy.x, abs_dxdy.y)/max(abs_dxdy.x, abs_dxdy.y);
}

float Coverage(const float border_dis, const vec2 dxdy)
{
	// interpret AbsSlope as (Rise, Run). Rise is guaranteed to be bigger,
	// so we can avoid issues with division by zero
	float slope = AbsSlope(dxdy);
	float angleness = dot(dxdy, vec2(1.0, 1.0));
	float to_border = angleness*border_dis;
	//angleness is positive when we are sampling inside the boundary (push the border forward)

	// forward the end points along x by 0.5*(Run/Rise)
	// forward the end points along x by (Run/Rise)*forward
	vec2 ends = vec2(to_border+0.5*slope, to_border-0.5*slope);
	float max_end = min(max(ends.x, ends.y), 0.5);
	float min_end = max(min(ends.x, ends.y), -0.5);
	// end ppoints are at a maximum of 0.5
	// left area 1->max_end
	// middle area max_end -> min_end
	// right area min_end -> -0.5
	float slice_height = min((max_end-min_end)/(slope+0.00001), 1.0);
	// height of the sliced portion = max(abs(min-max)*(Rise/Run), 1.0)
	float coverage = 1.0 - (0.5-max_end) - 0.5*(max_end-min_end)*slice_height;
	//if angleness is negative then the rectangular portion of the sliced column is not covered
	coverage -= (max_end-min_end)*(1.0-slice_height)*step(0.0, -angleness*(max_end-min_end));
	return coverage;

}

vec4 UnpackColor(uint color)
{
	uvec4 masks = uvec4(0x000000FF);
	uvec4 shifts = uvec4(0, 8, 16, 24);
	uvec4 cvec = (uvec4 (color) >> shifts) & masks;
	vec4 ret = vec4(cvec)*vec4(1.0/255.5);
	return ret;
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
//{akj:use geometry}




#define geometry_main main
#define vertex_main main
#define fragment_main main 

uniform mat4 uProjectionMatrix;
uniform float uCurrentTime;


#ifdef VERTEX_SHADER
struct VertexInstance
{
	vec2 StringPosition;
	vec2 Scale;
	float StartTime;
	float ZDepth;
	float OuterRange;
	float TotalTexSize;
	uint InnerColor;
	uint SolidColor;
	uint OuterColor;
	uint FontIndex;
};
// 48 bytes best case, 160 bytes worst case

// apparently this block must be less than 16k basic machine units
// (from GL_MAX_UNIFORM_BLOCK_SIZE) so that's capacity = 102 - 341

layout(std140) uniform StringInstances
{
	VertexInstance uStringData[80];
};

// per vertex attributes
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aVelocity;
layout(location = 2) in vec2 aAccel;
layout(location = 3) in vec2 aTexCenter;
layout(location = 4) in vec2 aTexSize;
layout(location = 5) in float aClockwiseRadians;
layout(location = 6) in uint aIndex;


#endif

//////////////////////////////////////////////////////////////////
// Shared Uniforms
//////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////
// Shared varyings
//////////////////////////////////////////////////////////////////




#ifdef VERTEX_SHADER
flat out vec2 vsRanges;
flat out float vsStartTime;
flat out float vsZDepth;
flat out vec4 vsColor;

// vertex specific outs
smooth out vec2 vsCenterPos;
smooth out vec2 vsQuadSize;
smooth out vec2 vsTexCenter;
smooth out vec2 vsTexSize;
flat out vec2 vsTexPadding;
flat out vec2 vsPosPadding;



void vertex_main()
{
	float t = uCurrentTime- uStringData[aIndex].StartTime;
	vec2 pos = aPosition + aVelocity*t + 0.5*(aAccel)*t*t;
	vec2 scale = uStringData[aIndex].Scale;// *vec2((1.0+0.1*(sin(3.5*t) *sin(0.35*t))), 1.0);

	vsCenterPos = uStringData[aIndex].StringPosition + pos*scale ;
	// use for AA sample offset if desired
	vec2 sample_offset = vec2(0.0);//(uSampleOffset/uStringData[aIndex].TotalTexSize)/ uStringData[aIndex].Scale;
	vsTexCenter = aTexCenter + sample_offset;
	vsTexSize = aTexSize;
	vsQuadSize = aTexSize*scale;
	vsPosPadding = uStringData[aIndex].OuterRange*scale;
	vsTexPadding = vec2(uStringData[aIndex].OuterRange);
	vsZDepth = uStringData[aIndex].ZDepth;
	vsStartTime = uStringData[aIndex].StartTime;
	float fuzz_range = 1.0/(scale.x*uStringData[aIndex].OuterRange);
	vsRanges.x = fuzz_range*uStringData[aIndex].TotalTexSize;
	vsRanges.y = 1.0/uStringData[aIndex].OuterRange;

	vsColor = UnpackColor(uStringData[aIndex].SolidColor);

}
#endif

#ifdef GEOMETRY_SHADER
flat in vec2 vsRanges[];
flat in float vsStartTime[];
flat in float vsZDepth[];
flat in vec4 vsColor[];

// vertex specific outs
smooth in vec2 vsCenterPos[];
smooth in vec2 vsQuadSize[];
smooth in vec2 vsTexCenter[];
smooth in vec2 vsTexSize[];
flat in vec2 vsTexPadding[];
flat in vec2 vsPosPadding[];

smooth out vec2 gsTexCoord;
flat out vec2 gsRanges;
flat out float gsStartTime;
flat out float gsZDepth;
flat out vec4 gsColor;

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

void geometry_main()
{
	// emit from left to right:  UL -> UR -> LL -> LR

	gsRanges = vsRanges[0];
	gsStartTime = vsStartTime[0];
	vec2 pos;
	vec2 dir;
	vec2 pos_snap = vec2(0.0, 0.0);
	vec2 pos_to_tex = vec2(1.0, -1.0)*(vsTexPadding[0]/vsPosPadding[0]);

	dir = vec2(-1.0, -1.0);
	gsTexCoord = vsTexCenter[0] + vec2(0.0, -1.0)*vsTexSize[0] + dir*vec2(1.0, 1.0)*vsTexPadding[0];
	pos =  vsCenterPos[0]+vec2(0.0, -1.0)*vsQuadSize[0] + dir*vsPosPadding[0];

	gl_Position = uProjectionMatrix*(vec4(pos, -0.1, 1.0));
	gsZDepth = vsZDepth[0];
	gsColor = vsColor[0];
	EmitVertex();
	
	gsRanges = vsRanges[0];
	gsStartTime = vsStartTime[0];

	dir = vec2(1.0, -1.0);
	gsTexCoord = vsTexCenter[0] + vec2(1.0, -1.0)*vsTexSize[0]+ dir*vec2(1.0, 1.0)*vsTexPadding[0];
	pos = vsCenterPos[0]+vec2(1.0, -1.0)*vsQuadSize[0] + dir*vsPosPadding[0];

	gl_Position = uProjectionMatrix*(vec4(pos, -0.1, 1.0));
	gsZDepth = vsZDepth[0];
	gsColor = vsColor[0];
	EmitVertex();

	gsRanges = vsRanges[0];
	gsStartTime = vsStartTime[0];

	dir = vec2(-1.0, 1.0);
	gsTexCoord = vsTexCenter[0] + vec2(0.0, 0.0)*vsTexSize[0]+ dir*vec2(1.0, 1.0)*vsTexPadding[0];
	pos = vsCenterPos[0]+vec2(0.0, 0.0)*vsQuadSize[0] + dir*vsPosPadding[0];
	gl_Position = uProjectionMatrix*(vec4(pos, -0.1, 1.0));
	gsZDepth = vsZDepth[0];
	gsColor = vsColor[0];
	EmitVertex();

	gsRanges = vsRanges[0];
	gsStartTime = vsStartTime[0];

	dir = vec2(1.0, 1.0);
	gsTexCoord = vsTexCenter[0] + vec2(1.0, 0.0)*vsTexSize[0] + dir*vec2(1.0, 1.0)*vsTexPadding[0];
	pos = vsCenterPos[0]+vec2(1.0, 0.0)*vsQuadSize[0] + dir*vsPosPadding[0];
	gl_Position = uProjectionMatrix*(vec4(pos, -0.1, 1.0));
	gsZDepth = vsZDepth[0];
	gsColor = vsColor[0];
	EmitVertex();
	EndPrimitive();
}
#endif

#ifdef FRAGMENT_SHADER
flat in vec2 gsRanges;
flat in float gsStartTime;
flat in float gsZDepth;
flat in vec4 gsColor;

uniform float uDirectProportion;
uniform float uAlpha;


layout(origin_upper_left) in vec4 gl_FragCoord;

smooth in vec2 gsTexCoord;

uniform sampler2D uFontTexture;

uniform vec2 uFudgeFactors;

out vec4 oFragColor;


vec4 SampleBilinear(const vec2 tex_coord)
{
	vec2 f_index = tex_coord-vec2(0.5); 
	ivec2 icoord = ivec2(floor(tex_coord));
	ivec2 incr = ivec2(ceil(f_index)) - icoord;
	ivec2 decr = ivec2(ivec2(floor(f_index)) - icoord);
	vec2 incr_weights = 1.0-(icoord+0.5+incr-tex_coord);
	vec2 decr_weights = 1.0-incr_weights;
	vec4 ret = (incr_weights.x)*incr_weights.y
							*texelFetch(uFontTexture,icoord+ivec2(1,0)*incr+ivec2(0,1)*incr, 0);
	ret += (incr_weights.x)*decr_weights.y
							*texelFetch(uFontTexture,icoord+ivec2(1,0)*incr+ivec2(0,1)*decr, 0);
	ret += (decr_weights.x)*incr_weights.y
							*texelFetch(uFontTexture,icoord+ivec2(1,0)*decr+ivec2(0,1)*incr, 0);
	ret += (decr_weights.x)*decr_weights.y
							*texelFetch(uFontTexture,icoord+ivec2(1,0)*decr+ivec2(0,1)*decr, 0);
	return ret;
}

vec4 SampleNearest(const vec2 tex_coord)
{
	return 	vec4(texelFetch(uFontTexture, ivec2(floor(tex_coord)), 0));
}

float RenderTexDirect(const vec2 tex_coord)
{
	return 1.0-texelFetch(uFontTexture, ivec2(floor(tex_coord)), 0).r;
}

float RenderDistanceField(const vec2 tex_coord)
{
	vec4 samp = SampleBilinear(tex_coord);//texture2D(uFontTexture, gsTexCoord);
	float edge_shift = uFudgeFactors.x;
	//float h2 = uFudgeFactors.y*((1.0*(abs(samp.b-0.5)+abs( samp.g-0.5))/gsRanges.x) +edge_shift);
	float h = uFudgeFactors.y*(((0.5-samp.r)/gsRanges.x) +edge_shift);
	vec2 dsamp = abs(normalize(vec2(samp.g-0.5, samp.b-0.5)));
	//float edge = step(middle, h);
	//return LinStep(0.0, 1.0, Coverage( abs(h), sign(h)*dsamp));
	return LinStep(-0.5, 0.5, h);
}


void fragment_main()
{
	float edge = mix(RenderDistanceField(gsTexCoord), RenderTexDirect(gsTexCoord), uDirectProportion);
	//float edge = LinStep(-0.5, 0.5, h);
	//float inner_portion = step(1.0-gsRanges.x*0.45-gsRanges.y,  (1.0 - samp));
	//float mult = step(-0.5, inner_portion);
	//float mult = samp.a*step(0.00001, edge);
	oFragColor = gsColor;
	//oFragColor.r = 1.0-2.0*length(vec2(samp.b-0.5, samp.g-0.5));
	oFragColor.a *= pow(edge, 1.4);//(clamp(max(Coverage(h+0.0, dsamp), h), 0, 1))*mult;
	// we blend using blend_max, so this has to be inverted
	// if we have non-0 coverage, then the depth is gsZdepth
	// oFragColor.g = mix(0.0, 1.0-gsZDepth, mult);
}
#endif
