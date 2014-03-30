#define PI 3.14159265359
#define PI2 6.28318530718
#define PI_INV 0.318309886184
#define SQRT2 1.41421356237

vec3 HueToRGB(float hue)
{
	vec3 s = vec3(1.0, -1.0, -1.0);
	return sqrt(clamp(vec3(-1.0,2.0, 2.0)+s*abs(vec3(3.0, 2.0, 4.0 )+vec3(-hue*6.0)), 0.0, 1.0));
}

vec4 DebugColor()
{
	return vec4(1.0, 0.0, 1.0, 1.0);
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
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uProjectionMatrix;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	//world_vertex += 0.001*noise_fac;
	gl_Position = uProjectionMatrix*(vec4(aPos, -0.5, 1.0));
}
#endif


#ifdef FRAGMENT_SHADER
in vec2 vTexCoord;
out vec4 oFragColor;
uniform sampler2D uTexture0;

void main()
{
	oFragColor = texelFetch(uTexture0,ivec2(vTexCoord), 0);
	//oFragColor.xyz = Overlay(vColor, oFragColor.xyz );
	oFragColor.a = 1.0;
}
#endif
