


#include "vector.h"
#include "const.h"

#ifdef __cplusplus
//#ifdef CLIENT_DLL

// cl_dlls/cl_util.cpp
// ...all of this file really.
vec3_t vec3_origin(0, 0, 0);




float Length(const float* v)
{
	int	i;
	float length;

	length = 0;
	for (i = 0; i < 3; i++)
		length += v[i] * v[i];
	length = sqrt(length);		// FIXME

	return length;
}



float Distance(const vec3_t v1, const vec3_t v2)
{
	vec3_t d;
	VectorSubtract_f(v2, v1, d);
	return Length(d);
}

//MODDD - NEW. Only involve the X and Y coords
float Distance2D(const vec3_t v1, const vec3_t v2)
{
	float theDist;
	float d[2];
	//(d)[0] = (v2)[0] - (v1)[0]; (d)[1] = (v2)[1] - (v1)[1]; (d)[2] = (v2)[2] - (v1)[2];
	d[0] = v2[0] - v1[0];
	d[1] = v2[1] - v1[1];
	// And return the length with from the two coord differences.
	theDist = 0.0f;
	theDist += d[0] * d[0];
	theDist += d[1] * d[1];
	theDist = sqrt(theDist);		// FIXME
	return theDist;
}


// NEW!  Given a delta vector (point2 minus point 1 already done), get the distance from it.
float DistanceFromDelta(const vec3_t vDelta)
{
	return Length(vDelta);
}
float Distance2DFromDelta(const vec3_t vDelta)
{
	float theDist;
	theDist = 0.0f;
	theDist += vDelta[0] * vDelta[0];
	theDist += vDelta[1] * vDelta[1];
	theDist = sqrt(theDist);		// FIXME
	return theDist;
}


void VectorAngles(const float* forward, float* angles)
{
	float tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180.0f / M_PI);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(forward[2], tmp) * 180.0f / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

float VectorNormalize(float* v)
{
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt(length);		// FIXME

	if (length)
	{
		ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}


void VectorMA(const float* veca, float scale, const float* vecb, float* vecc)
{
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] + scale * vecb[2];
}
void VectorScale(const float* in, float scale, float* out)
{
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}
void VectorInverse(float* v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}





//#endif
#endif

