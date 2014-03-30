#define PI 3.14159265359
#define PI2 6.28318530718
#define PI_INV 0.318309886184
#define SQRT2 1.41421356237

vec3 GetLight(){
	return vec3(15.0, 12.0, 30.0);
}

vec3 HueToRGB(float hue)
{
	vec3 s = vec3(1.0, -1.0, -1.0);
	return sqrt(clamp(vec3(-1.0,2.0, 2.0)+s*abs(vec3(3.0, 2.0, 4.0 )+vec3(-hue*6.0)), 0.0, 1.0));
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

#ifdef VERTEX_SHADER
layout(location = 0) in vec2 aTexCoord;
layout(location = 1) in vec2 aPos;
layout(location = 2) in vec2 aVel;
layout(location = 3) in vec2 aAccel;
layout(location = 4) in float aStartTime;
layout(location = 5) in vec3 aColor;

uniform mat4 uProjectionMatrix;
uniform float uCurrentTime;

out vec2 vTexCoord;
out vec3 vColor;

void main()
{
	vColor = aColor;

	vTexCoord = aTexCoord;
	//world_vertex += 0.001*noise_fac;
	gl_Position = uProjectionMatrix*(vec4(aPos, -0.5, 1.0));
}
#endif


#ifdef FRAGMENT_SHADER
in vec2 vTexCoord;
in vec3 vColor;
out vec4 oFragColor;
uniform float uCurrentTime;
uniform sampler2D uTexture0;
uniform sampler2D uTexture1;

void main()
{	
	vec4 diffuse_map = DebugColor();//texture2D(uTexture0, vTexCoord);
	vec4 profile_map = texture2D(uTexture0, vTexCoord);
	oFragColor.xyz = profile_map.xyz;//Overlay(vColor, profile_map.y);
	//oFragColor.xyz = Overlay(vColor, oFragColor.xyz );
	oFragColor.a = 1.0;
}
#endif
