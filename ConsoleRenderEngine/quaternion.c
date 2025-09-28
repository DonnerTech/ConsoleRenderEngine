#include "quaternion.h"

//Based on https://github.com/MartinWeigel/Quaternion/blob/master/Quaternion.c
//Some information from https://en.wikipedia.org/wiki/Quaternion

Quaternion quat_identity(void)
{
	return (Quaternion){1.0, 0.0, 0.0, 0.0};
}

Quaternion quat_from_axis_angle(Vector3 axis, double angle)
{
	Quaternion quat = quat_identity();

	quat.w = cos(angle / 2.0);
	double c = sin(angle / 2.0);
	quat.x = c * axis.x;
	quat.y = c * axis.y;
	quat.z = c * axis.z;
	
	return quat;
}

Quaternion quat_from_euler(double roll, double pitch, double yaw)
{
	double cy = cos(yaw * 0.5f);
	double sy = sin(yaw * 0.5f);
	double cr = cos(roll * 0.5f);

	double sr = sin(roll * 0.5f);
	double cp = cos(pitch * 0.5f);
	double sp = sin(pitch * 0.5f);

	Quaternion quat = quat_identity();
	quat.w = cy * cr * cp + sy * sr * sp;
	quat.x = cy * sr * cp - sy * cr * sp;
	quat.y = cy * cr * sp + sy * sr * cp;
	quat.z = sy * cr * cp - cy * sr * sp;

	return quat;
}

Quaternion quat_conjugate(Quaternion q)
{
	Quaternion quat = quat_identity();

	quat.w = q.w;
	quat.x = -q.x;
	quat.y = -q.y;
	quat.z = -q.z;

	return quat;
}


double quat_norm(Quaternion q)
{
	return sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
}

Quaternion quat_normalize(Quaternion q)
{
	double length = quat_norm(q);

	return (Quaternion){q.w / length, q.x / length, q.y / length, q.z / length};
}

Quaternion quat_inverse(Quaternion q)
{
	double norm_sq = quat_norm(q);
	
	if (norm_sq == 0.0)
	{
		return quat_identity();
	}

	return quat_normalize(quat_conjugate(q));
}

Quaternion quat_multiply(Quaternion q1, Quaternion q2)
{
	Quaternion quat = quat_identity();

	// the matrix mult
	quat.w = q1.w * q2.w - q1.x * q2.x - q1.y* q2.y- q1.z * q2.z;
	quat.x = q1.x * q2.w + q1.w * q2.x + q1.y * q2.z - q1.z * q2.y;
	quat.y = q1.w * q2.y- q1.x * q2.z + q1.y* q2.w + q1.z * q2.x;
	quat.z = q1.w * q2.z + q1.x * q2.y- q1.y* q2.x + q1.z * q2.w;

	return quat;
}

Vector3 quat_rotate_vector(const Quaternion q, Vector3 vector)
{
	Vector3 result;

	double ww = q.w * q.w;
	double xx = q.x * q.x;
	double yy = q.y * q.y;
	double zz = q.z * q.z;
	double wx = q.w * q.x;
	double wy = q.w * q.y;
	double wz = q.w * q.z;
	double xy = q.x * q.y;
	double xz = q.x * q.z;
	double yz = q.y * q.z;

	result.x = ww * vector.x + 2 * wy * vector.z - 2 * wz * vector.y +
		xx * vector.x + 2 * xy * vector.y + 2 * xz * vector.z -
		zz * vector.x - yy * vector.x;
	result.y = 2 * xy * vector.x + yy * vector.y + 2 * yz * vector.z +
		2 * wz * vector.x - zz * vector.y + ww * vector.y -
		2 * wx * vector.z - xx * vector.y;
	result.z = 2 * xz * vector.x + 2 * yz * vector.y + zz * vector.z -
		2 * wy * vector.x - yy * vector.z + 2 * wx * vector.y -
		xx * vector.z + ww * vector.z;

	return result;
}

Quaternion quat_from_angular_velocity(Vector3 omega, double dt)
{
	double angle = sqrt(omega.x * omega.x + omega.y * omega.y + omega.z * omega.z) * dt;
	if (angle < 1e-8) return quat_identity();

	Vector3 axis = { omega.x, omega.y, omega.z };
	double inv_len = 1.0 / sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
	axis.x *= inv_len; axis.y *= inv_len; axis.z *= inv_len;

	return quat_from_axis_angle(axis, angle);
}

Quaternion quat_integrate(Quaternion q, Vector3 omega, double dt)
{
	Quaternion dq = quat_from_angular_velocity(omega, dt);
	Quaternion result = quat_multiply(dq, q);
	return quat_normalize(result);
}

void quat_to_matrix(Quaternion q, double m[3][3])
{
	q = quat_normalize(q);

	double xx = q.x * q.x;
	double yy = q.y * q.y;
	double zz = q.z * q.z;
	double xy = q.x * q.y;
	double xz = q.x * q.z;
	double yz = q.y * q.z;
	double wx = q.w * q.x;
	double wy = q.w * q.y;
	double wz = q.w * q.z;

	m[0][0] = 1.0 - 2.0 * (yy + zz);
	m[0][1] = 2.0 * (xy - wz);
	m[0][2] = 2.0 * (xz + wy);

	m[1][0] = 2.0 * (xy + wz);
	m[1][1] = 1.0 - 2.0 * (xx + zz);
	m[1][2] = 2.0 * (yz - wx);

	m[2][0] = 2.0 * (xz - wy);
	m[2][1] = 2.0 * (yz + wx);
	m[2][2] = 1.0 - 2.0 * (xx + yy);
}


