#pragma once

#define AKJ_PI 3.1415926535897932384626433832795028842
#define AKJ_PIf 3.1415926535897932384626433832795028842f
#define AKJ_INV_PI  0.318309886183791
#define AKJ_INV_PIf  0.318309886183791f
#define INV_360 0.00277777777777777777777777777777
#define INV_360f 0.00277777777777777777777777777777f


#include <math.h>
#include <limits>
#include <cassert>
#include <vector>
#include "akjLog.hpp"
#include "akjCoord2.hpp"
#include "akjExceptional.hpp"
#ifdef max
#define AKJ_WHY_WOULD_YOU_DEFINE_MAX
#undef max
#endif
#ifdef min
#define AKJ_WHY_WOULD_YOU_DEFINE_MIN
#undef min
#endif


namespace akj{
namespace unsafe
{
	union UintAndFloat
	{
		UintAndFloat(uint u_in) : u(u_in){}
		float f;
		uint32_t u;
	};
	inline float UintBitsToFloat(uint32_t u)
	{
		const UintAndFloat uf(u);
		return *reinterpret_cast<const float*>(&uf);
	}
	inline float ToFloat(uint32_t val)
	{
		AKJ_ASSERT(val&(~0x7FFFFF));
		return UintBitsToFloat( val | 0x4b000001) - 8388608.0f;
	}
}

inline float MaxFloat(){
	return std::numeric_limits<float>::max();
}

inline float NormalizeRadians(float radians){
	return fmodf(radians+AKJ_PIf, 2.0*AKJ_PIf) - AKJ_PIf ;
}

inline double ToRadians(double degrees)
{
	const double frac_part = fmod(degrees*INV_360+0.5, 1.0) - 0.5 ;
	return frac_part * (2.0 * AKJ_PI);
}
inline float ToRadians(float degrees)
{
	const float frac_part = fmodf(degrees*INV_360f+0.5f, 1.0f) - 0.5f ;
	return frac_part * (2.0f * AKJ_PIf);
}
inline float ToDegrees(float radians)
{
	const float frac_part = (fmodf(radians + AKJ_PIf, 2.0f*AKJ_PIf) - AKJ_PIf) * AKJ_INV_PIf  ;
	return frac_part * 180.0f;
}
inline double ToDegrees(double radians)
{
	const double frac_part = (fmod(radians + AKJ_PI, 2.0*AKJ_PI) - AKJ_PI) * AKJ_INV_PI  ;
	return frac_part * 180.0;
}

template <typename T>
T Lerp(const T& lower, const float mix, const T& upper)
{
	return (Clamp(lower, mix, upper)-lower)/(upper-lower);
}


void InverseTranspose4f(float* ptr);


//convenience class for working with 3 dimensional coordinates.
struct cCoord3
{
	cCoord3():x(0.0f),y(0.0f),z(0.0f){};
	explicit cCoord3(float n):x(n), y(n), z(n){};
	cCoord3(float in_x, float in_y, float in_z):x(in_x),y(in_y),z(in_z){};
	cCoord3(const cCoord3& rhs):x(rhs.x),y(rhs.y),z(rhs.z){};
	cCoord3(cCoord3&& rhs):x(rhs.x),y(rhs.y),z(rhs.z){};
	const cCoord3& operator=(const cCoord3& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}
	inline float lengthSquared() const
	{
		return x*x + y*y + z*z;
	}
	inline float length() const
	{
		const float l =sqrt(lengthSquared());
		assert(l==l);
		return l;
	}
	cCoord3 normalized() const
	{
		const float l = length();
		return l== 0.0f ? cCoord3::Zeros() : cCoord3(x/l, y/l, z/l);
	}

	cCoord3 cross(const cCoord3& rhs) const
	{
		return cCoord3(y*rhs.z - rhs.y*z, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x);
	}
	float dot(const cCoord3& rhs) const
	{
		return rhs.x*x+rhs.y*y+rhs.z*z;
	}
	bool AnyLess(const cCoord3& other) const {
		return (other.x > x || other.y > y || other.z > z);
	}
	bool AnyNonZero() const {
		return (x != 0.0f || y != 0.0f || z != 0.0f);
	}
	bool AnyGreater(const cCoord3& other)const {
		return (other.x < x || other.y < y || other.z < z);
	}
	void TruncateMagnitude(float mag){
		if( x > mag){
			x = mag;
		}
		else if (x < -mag){
			x = -mag;
		}
		if( y > mag){
			y = mag;
		}
		else if (y < -mag){
			y = -mag;
		}
		if(z > mag){
			z = mag;
		}
		else if (z < -mag){
			z = -mag;
		}
	}
	bool AllLess(const cCoord3& other) const {
		return (other.x > x && other.y > y && other.z > z);
	}
	bool AllGreater(const cCoord3& other)const {
		return (other.x < x && other.y < y && other.z < z);
	}
	inline static cCoord3 Zeros()
	{
		return cCoord3(0.0f, 0.0f, 0.0f);
	}
	inline static cCoord3 Ones()
	{
		return cCoord3(1.0f, 1.0f, 1.0f);
	}
	inline static cCoord3 XAxis()
	{
		return cCoord3(1.0f, 0.0f, 0.0f);
	}
	inline static cCoord3 YAxis()
	{
		return cCoord3(0.0f, 1.0f, 0.0f);
	}
	inline static cCoord3 ZAxis()
	{
		return cCoord3(0.0f, 0.0f, 1.0f);
	}
	void CopyToArray(float* float_array) const
	{
		*float_array = x;
		float_array[1] = y;
		float_array[2] = z;
	}
	cCoord3 least_axis() const
	{
		float ax = fabs(x);
		float ay = fabs(y);
		float az = fabs(z);

		if(ax < ay)
		{
			if(az < ax)
			{
				return cCoord3::ZAxis();
			}
			else
			{
				return cCoord3::XAxis();
			}
		}
		else if(ay < az)
		{
			return cCoord3::YAxis();
		}
		return cCoord3::ZAxis();
	}
	float x;
	float y;
	float z;
private:
	// we want these to be 16 byte aligned (thats what sse2 and the video card want)
	// float mPlaceholder;
};

inline cCoord3 operator+(const cCoord3& lhs, const cCoord3& rhs)
{
	return cCoord3(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
}
inline cCoord3 operator-(const cCoord3& lhs, const cCoord3& rhs)
{
	return cCoord3(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
}
inline cCoord3 operator-(cCoord3 lhs)
{
	return cCoord3(-lhs.x, -lhs.y, -lhs.z);
}
inline cCoord3 operator+=(cCoord3& lhs, const cCoord3& rhs)
{
	lhs = lhs + rhs;
	return lhs;
}
inline cCoord3 operator-=(cCoord3& lhs, const cCoord3& rhs)
{
	lhs = lhs - rhs;
	return lhs;
}
inline cCoord3 operator*(float factor, const cCoord3& rhs)
{
	return cCoord3(factor*rhs.x, factor*rhs.y, factor*rhs.z);
}
inline cCoord3 operator*(const cCoord3& rhs, float factor)
{
	return cCoord3(factor*rhs.x, factor*rhs.y, factor*rhs.z);
}
inline cCoord3 operator*(const cCoord3& lhs, const cCoord3& rhs)
{
	return cCoord3(lhs.x*rhs.x, lhs.y*rhs.y, lhs.z*rhs.z);
}
inline cCoord3 operator*=(cCoord3& lhs, const cCoord3& rhs)
{
	lhs = lhs * rhs;
	return lhs;
}
inline cCoord3 operator*=(cCoord3& lhs, float factor)
{
	lhs = lhs * factor;
	return lhs;
}

inline cCoord3 operator/(const cCoord3& lhs, const cCoord3& rhs)
{
	return cCoord3(lhs.x/rhs.x, lhs.y/rhs.y, lhs.z/rhs.z);
}
inline cCoord3 operator/(const cCoord3& rhs, float factor)
{
	return cCoord3(rhs.x/factor, rhs.y/factor, rhs.z/factor);
}
inline cCoord3 operator/=(cCoord3& lhs, const cCoord3& rhs)
{
	lhs = lhs / rhs;
	return lhs;
}
inline cCoord3 operator/=(cCoord3& lhs, float factor)
{
	lhs = lhs / factor;
	return lhs;
}
struct cCoord4
{
	cCoord4():x(0.0f),y(0.0f),z(0.0f),w(0.0f){};
	explicit cCoord4(float n):x(n), y(n), z(n), w(n){};
	cCoord4(float in_x, float in_y, float in_z, float in_w):x(in_x),y(in_y),z(in_z),w(in_w){};
	cCoord4(const cCoord3& rhs, float w):x(rhs.x),y(rhs.y),z(rhs.z),w(w){};
	cCoord4(const cCoord4& rhs):x(rhs.x),y(rhs.y),z(rhs.z),w(rhs.w){};
	const cCoord4& operator=(const cCoord4& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		w = rhs.w;
		return *this;
	}
	inline float lengthSquared() const
	{
		return x*x + y*y + z*z + w*w;
	}
	inline float length() const
	{
		const float l =sqrt(lengthSquared());
		assert(l==l);
		return l;
	}
	cCoord4 normalized() const
	{
		const float l = length();
		return l== 0.0f ? cCoord4::Zeros() : cCoord4(x/l, y/l, z/l, w/l);
	}
	void CopyToArray(float* float_array) const
	{
		*float_array = x;
		float_array[1] = y;
		float_array[2] = z;
		float_array[3] = w;
	}
	cCoord3 XYZ() const {
		return cCoord3(x, y, z);
	}
	inline static cCoord4 Zeros()
	{
		return cCoord4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	inline static cCoord4 Ones()
	{
		return cCoord4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	float x;
	float y;
	float z;
	float w;
};
class cAngleAxis
{
public:
	cAngleAxis():angle(0.0f),x(0.0f),y(0.0f),z(1.0f){
		normalizeAxis();
	};
	cAngleAxis(float in_radians, const cCoord3& axis):angle(in_radians), x(axis.x), y(axis.y), z(axis.z){
		normalizeAxis();
	}
	cAngleAxis(float in_radians, float in_x, float in_y, float in_z):angle(in_radians),x(in_x),y(in_y),z(in_z){
		normalizeAxis();
	};

	void normalizeAxis()
	{
		const float inv_length = 1.0f/sqrt(x*x + y*y + z*z);
		x *= inv_length;
		y *= inv_length;
		z *= inv_length;
	}

	// formula from http://en.wikipedia.org/wiki/Axis-angle_representation
	cCoord3 rotate(const cCoord3& vec) const
	{
		const float c = cosf(angle);
		const float s = sinf(angle);
		const cCoord3 w(x, y, z);
		return c*vec + s*(w.cross(vec))+(1.0f-c)*w.dot(vec)*w ;
	}
	cCoord3 rev_rotate(const cCoord3& vec) const
	{
		const float c = cosf(-angle);
		const float s = sinf(-angle);
		const cCoord3 w(x, y, z);
		return c*vec + s*(w.cross(vec))+(1.0f-c)*w.dot(vec)*w ;
	}
	float angle;
	float x;
	float y;
	float z;
};


// quaternion class
// most of the math here was either guessed at/derived by myself,
// and most of the ideas come from trawling wikipedia and misc. forum postings
class cUnitQuat
{
public:
	cUnitQuat():s(0.0f, 0.0f, 0.0f),w(1.0f){}
	cUnitQuat(float wadda,const cCoord3& vector):s(vector),w(wadda){
		assert(wadda==wadda);
		const float ls = s.lengthSquared();
		if(ls == 0.0f || w == 0.0f)
		{
			w = 1.0f;
			s = cCoord3(0.0f, 0.0f, 0.0f);
		}
		else if(ls + w*w - 1.0f > 0.0005f)
		{
			s = sqrt( (1.0f-w*w)/s.lengthSquared()) * s;
		}
	}
	explicit cUnitQuat(const cAngleAxis& aa)
	{
		const float a_by_2 = aa.angle*0.5f;
		s = sin(a_by_2)*cCoord3(aa.x, aa.y, aa.z);
		w = cos(a_by_2);
	}
	cUnitQuat(const cCoord3& from, const cCoord3& to)
	{
		const cCoord3 from_normed = from.normalized();
		const cCoord3 to_normed = to.normalized();
		const cCoord3 m = (from_normed + to_normed).normalized();
		s = m.cross(to_normed);
		w = m.dot(to_normed);
	}
	cCoord3 rotate(const cCoord3& v) const
	{
		//return (w*w+s.lengthSquared())*v + (2.0f*v.dot(s))*s + (2.0f*w)*(s.cross(v));
		return v + 2.0f * s.cross( s.cross(v) + w*v );
	}
	cCoord3 rev_rotate(const cCoord3& v) const
	{
		//return (w*w-s.lengthSquared())*v + (2.0f*v.dot(s))*s - (2.0f*w)*(s.cross(1.0f*v));
		return v + 2.0f * s.cross( s.cross(v) - w*v );
	}
	cAngleAxis to_angle_axis() const
	{
		if(w >= 1.0)
		{
			return cAngleAxis(0.0f, 0.0f, 0.0f, 1.0f);
		}
		const float angle = 2.0f*atan2(s.length(), w);
		return cAngleAxis(angle, s);
	}
	cUnitQuat scale(float factor) const
	{
		if(w >= 1.0f)
		{
			return cUnitQuat();
		}
		const float half_theta = acosf(w)*factor;
		const float sign_theta = half_theta > 0 ? 1.0f: -1.0f;
		float new_w = cos(half_theta);
		return cUnitQuat(sign_theta*new_w, sqrt( (1.0f-new_w*new_w)/s.lengthSquared() ) * s );
	}
	inline static cUnitQuat YZ()
	{
		return cUnitQuat(0.707106769f, cCoord3(0.707106769f, 0.0f, 0.0f));
	}
	inline static cUnitQuat XZ()
	{
		return cUnitQuat(0.707106769f, cCoord3(0.0f, 0.707106769f, 0.0f));
	}
	inline static cUnitQuat YX()
	{
		return cUnitQuat(0.707106769f, cCoord3(0.0f, 0.0f, 0.707106769f));
	}
	inline static cUnitQuat ZY()
	{
		return cUnitQuat(0.707106769f, cCoord3(-0.707106769f, 0.0f, 0.0f));
	}
	inline static cUnitQuat ZX()
	{
		return cUnitQuat(0.707106769f, cCoord3(0.0f, -0.707106769f, 0.0f));
	}
	inline static cUnitQuat XY()
	{
		return cUnitQuat(0.707106769f, cCoord3(0.0f, 0.0f, -0.707106769f));
	}
	cCoord3 s;
	float w;
};
inline cUnitQuat operator*(const cUnitQuat& lhs, const cUnitQuat& rhs)
{
	return cUnitQuat(lhs.w*rhs.w - lhs.s.dot(rhs.s), lhs.w*rhs.s + rhs.w*lhs.s + lhs.s.cross(rhs.s));
}
inline cUnitQuat& operator*=(cUnitQuat& lhs, const cUnitQuat& rhs)
{
	lhs = lhs * rhs;
	return lhs;
}
inline cUnitQuat Slerp(const cUnitQuat& lhs, const cUnitQuat& rhs, float t)
{
	return lhs.scale(1.0f-t)*rhs.scale(t);
}

struct cEllipse3D
{
	cEllipse3D()
	{
		mA = 0.0f;
		mB = 0.0f;
		mE = 0.0f;
	}
	cEllipse3D(const cCoord3& x_axis, const cCoord3& z_axis, float a, float b)
	{
		SetXAxis(x_axis);
		SetZAxis(z_axis);
		SetAB(a, b);
	}
	~cEllipse3D(){};
	void SetXAxis(const cCoord3& x_axis)
	{
		mXAxis = x_axis;
	}
	void SetZAxis(const cCoord3& z_axis)
	{
		mZAxis = z_axis;
	}
	cCoord3 GetZAxis() const
	{
		return mZAxis;
	}
	cCoord3 GetXAxis() const
	{
		return mXAxis;
	}
	void SetAB(const float a, const float b)
	{
		mA = a;
		mB = b;
		// and eccentricity
		const float e = a> 0.0 ? sqrt((a*a-b*b)/(a*a)) : 0.0f;
		mE = e;
	}
	float GetE() const
	{
		return mE;
	}
	float GetA() const
	{
		return mA;
	}
	float GetB() const
	{
		return mB;
	}
	cCoord3 GetEllipseCoord(float t) const
	{
		cCoord3 ret;
		GetEllipseCoords(&t, &ret, 1);
		return ret;
	}
	void GetEllipseCoords(const float* t, cCoord3* coords, int num_coords) const
	{
		const float a = GetA();
		const float b = GetB();
		const cCoord3 x_axis = GetXAxis();
		const cCoord3 z_axis = GetZAxis();
		const cCoord3 origin = GetE()*x_axis;
		for (int i = 0; i < num_coords; i++)
		{
			const float this_t = AKJ_PIf*2.0f*t[i];
			coords[i] = cosf(this_t)*a*x_axis + sinf(this_t)*b*z_axis - origin;
		}	
	}
private:
	cCoord3 mXAxis;
	cCoord3 mZAxis;
	float mA;
	float mB;
	float mE;
};


struct cRay{
	cRay()
		:mDirection(0.0f)
		,mLength(0.0f)
	{}
	cRay(cCoord3 dir, float length)
		:mDirection(dir)
		,mLength(length)
		{}
	float mLength;
	cCoord3 mDirection;
};
class cTriangle {
public:
	cTriangle(){}
	cTriangle(cCoord3 v1, cCoord3 v2, cCoord3 v3, cCoord3 n, bool normalize = true)
		:mNormal(n)
	{
		mVertices[0] = v1;
		mVertices[1] = v2;
		mVertices[2] = v3;


		if(normalize){
			const float len = n.length();
			AKJ_ASSERT(len > 0.0);
			mNormal = n.normalized();
		}
		cCoord3 normal = ((mVertices[1]-mVertices[0]).cross(mVertices[0]-mVertices[2])).normalized();
		AKJ_ASSERT(std::abs(normal.dot(mNormal)) -1.0f <0.001f );
	}
	void DebugPrint() const{
		//Log::Debug("tri = [%f,%f,%f;%f,%f,%f;%f,%f,%f];",
			//			mVertices[0].x,mVertices[0].y, mVertices[0].z,
			//			mVertices[1].x,mVertices[1].y, mVertices[1].z,
		//				mVertices[2].x,mVertices[2].y, mVertices[2].z);
	}
	float PerimeterSq() const{
		return (mVertices[0]-mVertices[1]).lengthSquared()
			+(mVertices[0]-mVertices[2]).lengthSquared()
			+(mVertices[2]-mVertices[1]).lengthSquared();
	}
	float Area() const {
		return (mVertices[0]-mVertices[2]).cross((mVertices[0]-mVertices[1])).length()/2.0f;
	}
	//tried this on my own, but gave up after debugging it for an entire day
	//this one is adapted from http://www.gamedev.net/topic/552906-closest-point-on-triangle/
	// I'll try again sometime I'm not under deadline
	//note this is not used in the cs314 project
	cCoord3 ClosestPoint(const cCoord3& point ) const;
	cCoord3 GetVertex(int i) const{
		return mVertices[i];
	}
	cCoord3 GetNormal() const{
		return mNormal;
	}
	cCoord3 MeanLocation() const{
		return (1.0f/3.0f)*(mVertices[0]+mVertices[1]+mVertices[2]);
	}
	void SetVertex(int i, cCoord3 v){
		mVertices[i] = v;
	}
	void SetNormal(cCoord3 n){
		mNormal = n;
	}
private:
	cCoord3 mVertices[3];
	cCoord3 mNormal; 
};
struct cTrianglePerimeterComparator {
    bool operator() (const cTriangle& lhs, const cTriangle& rhs) const{
        return lhs.PerimeterSq() < rhs.PerimeterSq();
    }
};

class cPlane3 {
public:
	explicit cPlane3(cCoord3 normal)
		:mNormal(normal)
		,mLocation(0.0f)
	{}
	cPlane3(cCoord3 normal, float location)
		:mNormal(normal)
		,mLocation(location)
	{}
	explicit cPlane3(const cTriangle& tri)
		:mNormal(tri.GetNormal())
		,mLocation(tri.MeanLocation().dot(tri.GetNormal()))
	{}
	~cPlane3(){};
	const cCoord3& NormalVec() const{
		return mNormal;
	}
	cCoord3 ReflectVector(cCoord3 old_vector) const{
		const float normal_component = old_vector.dot(mNormal);
		return old_vector + mNormal*(std::abs(normal_component) - normal_component);
	}
	bool DistanceAlongRay(float& result, const cCoord3 origin, const cCoord3 direction) const{
		const float dir_dot = direction.dot(mNormal);
		if(dir_dot == 0.0f){
			return false;
		}
		else if(dir_dot > 0.0f){
			const cCoord3 v_to_plane = mLocation*mNormal- origin;
			const float closest_d = mNormal.dot(v_to_plane);
			result = closest_d/dir_dot;
			return true;
		}
		
		const cCoord3 v_to_plane = (-mLocation)* (-mNormal)- origin;
		const float closest_d = mNormal.dot(v_to_plane);
		result = closest_d/dir_dot;
		return true;
		
	}
	float DistanceToPoint(const cCoord3& point) const{
		return point.dot(mNormal) - mLocation;
	}
private:
	cCoord3 mNormal;
	float mLocation;
};


class cAABB{
public:
	cAABB(cCoord3 center)
		:mLowerCorner(center)
		,mUpperCorner(center)
	{
		AKJ_ASSERT(!mLowerCorner.AnyGreater(mUpperCorner));
	}
	cAABB(cCoord3 center, float radius)
		:mLowerCorner(center - cCoord3(radius))
		,mUpperCorner(center + cCoord3(radius))
	{
		AKJ_ASSERT(!mLowerCorner.AnyGreater(mUpperCorner));
	}
	cAABB(cCoord3 lower, cCoord3 upper )
		:mLowerCorner(lower)
		,mUpperCorner(upper)
	{
		AKJ_ASSERT(!mLowerCorner.AnyGreater(mUpperCorner));
	}
	~cAABB(){}
	void Reset(cCoord3 center){
		mUpperCorner = center;
		mLowerCorner = center;
	}
	void ExpandToFit(const cCoord3& c){
		if(c.x > mUpperCorner.x) mUpperCorner.x = c.x;
		if(c.y > mUpperCorner.y) mUpperCorner.y = c.y;
		if(c.z > mUpperCorner.z) mUpperCorner.z = c.z;
		
		if(c.x < mLowerCorner.x) mLowerCorner.x = c.x;
		if(c.y < mLowerCorner.y) mLowerCorner.y = c.y;
		if(c.z < mLowerCorner.z) mLowerCorner.z = c.z;
	}
	void ExpandToFitX(float val)
	{
		if(val > mUpperCorner.x) mUpperCorner.x = val;
		else if(val < mLowerCorner.x) mLowerCorner.x = val;
	}
	void ExpandToFitY(float val)
	{
		if(val > mUpperCorner.y) mUpperCorner.y = val;
		else if(val < mLowerCorner.y) mLowerCorner.y = val;
	}
	void ExpandToFitZ(float val)
	{
		if(val > mUpperCorner.z) mUpperCorner.z = val;
		else if(val < mLowerCorner.z) mLowerCorner.z = val;
	}
	void MoveBox(const cCoord3& vec){
		mLowerCorner += vec;
		mUpperCorner += vec;
	}
	bool CheckWithinX(const cCoord3& p){
		return !(p.x > mUpperCorner.x || p.x < mLowerCorner.x) ;
	}
	bool CheckWithinY(const cCoord3& p){
		return !(p.y > mUpperCorner.y || p.y < mLowerCorner.y) ;
	}
	bool CheckWithinZ(const cCoord3& p){
		return !(p.z > mUpperCorner.z || p.z < mLowerCorner.z) ;
	}
	bool CheckWithin(const cCoord3& p){
		if(p.x > mUpperCorner.x || p.y > mUpperCorner.y || p.z > mUpperCorner.z){
			return false;
		}
		if(p.x < mLowerCorner.x || p.y < mLowerCorner.y || p.z < mLowerCorner.z){
			return false;
		}
		return true;
	}
	bool CheckIntersect(const cAABB& other) const{
		return !(mLowerCorner.AnyGreater(other.mUpperCorner) || mUpperCorner.AnyLess(other.mLowerCorner));
	}
	bool RayIntersection(float& result, const cCoord3 origin, const cCoord3 direction){
		cRay ray;
		if(RayIntersection( ray, origin, direction)){
			result = ray.mLength;
			return true;
		}
		return false;

	}
	bool RayIntersection(cRay& result, const cCoord3 origin, const cCoord3 direction);
	cCoord3 UpperCorner() const { 
		return mUpperCorner; 
	}
	cCoord3 LowerCorner() const { 
		return mLowerCorner; 
	}
	cCoord3 Range() const {
		return mUpperCorner - mLowerCorner;
	}
	float XRange() const {
		return mUpperCorner.x-mLowerCorner.x;
	}
	float YRange() const {
		return mUpperCorner.y-mLowerCorner.y;
	}
	float ZRange() const {
		return mUpperCorner.z-mLowerCorner.z;
	}
private:
	cCoord3 mUpperCorner;
	cCoord3 mLowerCorner;


};

cCoord4 GenerateBiTangent(const cCoord3& edge1,const cCoord3& edge2, const cCoord3& normal, const cCoord3& st1, const cCoord3& st2);
// approximation for the circumference of an ellipse. Lifted from wikipedia
inline float EllipseCircumference(float semi_major, float semi_minor)
{
	const float diff_ab = (semi_major - semi_minor);
	const float sum_ab = (semi_major + semi_minor);
	const float q  = (diff_ab*diff_ab)/(sum_ab*sum_ab);
	return AKJ_PIf*sum_ab*(1.0f+(3.0f*q)/(10.0f + sqrtf(4.0f - 3.0f*q)) );
}

inline int GreatestCommonDivisor(int a, int b){
	while(a != b){
		if(a > b){
			a -= b;
		}
		else if(b > a){
			b -= a;
		}
	}
	return a;
}

inline int LeastCommonMultiple(int a, int b){
	return (a*b)/GreatestCommonDivisor(a,b);
}

void GenerateNGon(int faces, std::vector<cCoord3>& vertices);
}
