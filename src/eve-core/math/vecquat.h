//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																																						//
// Aqui se definen las matematicas de vectores, quaternios y matrices. Usamos estructuras en vez de clases para aumentar la velocidad de los calculos	//
//																																						//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _VECQUAT_H_
#define _VECQUAT_H_

#include "stdafx.h"
#include "solve.h"

typedef std::pair<double, double> Solution;

const double phi_d = 3.1415926535897932; // numero pi con formato de precision doble
const float phi_f = 3.1415926535f; // numero pi con formato de precision doble
// const scientific_ phi_q = 3.1415926535897932384626433832795028841971693993751; // numero pi con formato de precision cientifica

const double GRAVITATIONAL_CONSTANT = 6.67384e-11;
#define DELTA 1e-6f				// Small number for comparing floating point numbers

// Conversiones de precision doble
#define ToRadians_d(degree) ((degree) * (phi_d / 180.0))
#define ToDegrees_d(radian) ((radian) * (180.0 / phi_d))

#define ToRadians_f(degree) ((degree) * (phi_f / 180.0f))
#define ToDegrees_f(radian) ((radian) * (180.0f / phi_f))

#define ToRadians_q(degree) ((degree) * (phi_q / 180.0))
#define ToDegrees_q(radian) ((radian) * (180.0 / phi_q))

#define feet2meters(feet) (feet * 0.3048)
#define meters_to_feet(meters)(meters*3.28084f)

#define kgm3_to_slugft3(kgm3) (kgm3*0.0019403203f)

#define AU2meters(AU) (AU * 149597870700.0)
#define meters2AU(meters) (meters / 149597870700.0)

#define ToAtmospheres(pascals) (pascals * 9.86923e-6)
#define Pascals_to_Psf(pascals) (pascals *0.0209) // Pascals to Pounds Per Square Foot

#define Kg_to_lbs(kilos)(kilos*2.20462)


#define Limit360(a)while( a > 360.0f )a -= 360.0f
#define Limit2phi(a)while( a > 2*phi )a -= 2*phi

struct HandleMovingPart
{
	HandleMovingPart()
	{
		start_seconds = 0.f;
		end_seconds = 0.f;
		object_name = "";
		rotation_x = 0.0;
		rotation_y = 0.0;
		rotation_z = 0.0;
		traslation_x = 0.0;
		traslation_y = 0.0;
		traslation_z = 0.0;
	}

	float start_seconds;
	float end_seconds;
	std::string object_name;
	double rotation_x, rotation_y, rotation_z;
	double traslation_x, traslation_y, traslation_z;
};

struct vec2 { double x, y; };
struct vec4 { double x, y, z, w; };

inline int sign(double x)
{
	if (x < 0)
		return -1;
	else if (x > 0)
		return 1;
	else
		return 0;
}

inline int sign_b(double x)
{
	if (x < 0)
		return -1;
	else return 1;
}

struct engineStruct
{
	Vector3 position;
	float depth, size;
	Vector3 orientation;
};

struct vec3 {
	double x, y, z;
	vec3 operator+ (const vec3 & rhs) const { return{ x + rhs.x, y + rhs.y, z + rhs.z }; }
	vec3 operator+ (const double & rhs) const { return{ x + rhs, y + rhs, z + rhs }; }
	vec3 operator- (const vec3 & rhs) const { return{ x - rhs.x, y - rhs.y, z - rhs.z }; }
	vec3 operator- (const double & rhs) const { return{ x - rhs, y - rhs, z - rhs }; }
	double operator* (const vec3 & rhs) const { return{ x * rhs.x + y * rhs.y + z * rhs.z }; }
	void operator+=(const vec3 &rhs) { x += rhs.x; y += rhs.y; z += rhs.z; }
	void operator-=(const vec3 &rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; }
	vec3 operator^ (const vec3 & rhs) const { return{ y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x }; }
	vec3 operator* (const double & rhs)const { return{ x * rhs, y * rhs, z * rhs }; }
	vec3 operator/ (const double & rhs) const { return{ x / rhs, y / rhs, z / rhs }; }
	void operator/=(const double &rhs) { x /= rhs; y /= rhs; z /= rhs; }
	void operator*=(const double &rhs) { x *= rhs; y *= rhs; z *= rhs; }
	void normalize() { const double longitud = std::sqrt(x * x + y * y + z * z); x /= longitud; y /= longitud; z /= longitud; };
	vec3 operator-() const { return{ -x, -y, -z }; }
	bool operator== (const vec3 & rhs) const { return{ x == rhs.x && y == rhs.y && z == rhs.z }; }
	bool operator!= (const vec3 & rhs) const { return{ x != rhs.x || y != rhs.y || z != rhs.z }; }

	vec3 mul(const vec3 & rhs) const { return{ x * rhs.x, y * rhs.y, z * rhs.z }; }
	vec3 div(const vec3 & rhs) const { return{ x / rhs.x, y / rhs.y, z / rhs.z }; }
	vec3 sqrt() const
	{
		vec3 signo = vec3{ sign_b(x), sign_b(y), sign_b(z) };
		return vec3{ std::sqrt(abs(x)), std::sqrt(abs(y)), std::sqrt(abs(z)) }.mul(signo);
	}
};

struct quat {

	double x, y, z, w;

	quat operator* (const quat & rhs) const {
		return{ w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
			w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
			w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x,
			w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z, };
	}
	void normalize() {
		double s = sqrt(w * w + x * x + y * y + z * z);
		double invs = 1 / s;
		x *= invs;
		y *= invs;
		z *= invs;
		w *= invs;
	}
	quat operator+ (const quat & rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	quat operator- (const quat & rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	quat operator- (const quat & rhs) const { quat a = { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w }; return a; }
	quat operator*= (double s) { x *= s; y *= s; z *= s; w *= s; return *this; }
	double operator^(const quat & rhs) const { return{ x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w }; } // quat dot product
	quat operator-() const { return{ -x, -y, -z, -w }; }
	//*
	quat operator* (const double & rhs) const { return{ x * rhs, y * rhs, z * rhs, w * rhs }; }
	double norm(const quat q) const { return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w; }
	double abs(const quat q) const { return sqrt(norm(q)); }
	quat operator~ () const { return{ -x, -y, -z, w }; } // conjugate
	quat operator/ (const double & rhs) const { return *this *(1 / rhs); }
	quat operator/ (const quat & rhs) const { return *this *(~rhs / abs(rhs)); } // unrotate
	//*/

	vec3 operator* (const vec3 & vec) const
	{
		vec3 vn(vec);
		vn.normalize();

		quat vecQuat, resQuat;
		vecQuat.x = vn.x;
		vecQuat.y = vn.y;
		vecQuat.z = vn.z;
		vecQuat.w = 0.0f;

		resQuat = vecQuat * ~(*this);
		resQuat = *this * resQuat;

		return vec3{ resQuat.x, resQuat.y, resQuat.z };
	}
};

inline quat identity_quat()
{
	return{ 0.0, 0.0, 0.0, 1.0 };
}

struct FACET
{
	Vector3 p1, p2, p3;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//															Vector Math																				//

inline double distances(vec3 a, vec3 b)
{
	return sqrt(((b.x - a.x)*(b.x - a.x)) + ((b.y - a.y)*(b.y - a.y)) + ((b.z - a.z)*(b.z - a.z)));
}

inline float distances_f(Vector3 a, Vector3 b)
{
	return sqrtf(((b.x - a.x)*(b.x - a.x)) + ((b.y - a.y)*(b.y - a.y)) + ((b.z - a.z)*(b.z - a.z)));
}

inline double length(vec3 vect)
{
	return sqrt(vect.x * vect.x + vect.y * vect.y + vect.z * vect.z);
}

inline float length_f(Vector3 vect)
{
	return sqrtf(vect.x * vect.x + vect.y * vect.y + vect.z * vect.z);
}

inline vec3 normalized(vec3 a) { return a / length(a); }

inline vec3 add_proportional(vec3 vect, double value_)
{
	vec3 signo = { sign_b(vect.x), sign_b(vect.y), sign_b(vect.z) };
	return vect - ((vect / length(vect))*value_)*signo;
}

inline Vector3 normalized_f(Vector3 a) { return a / a.Length(); }


inline vec3 exp(vec3 vect)
{
	return{ exp(vect.x), exp(vect.y), exp(vect.z) };
}


inline vec3 matrix32_vec2_multiplication(double matrix[3][2], vec2 vect) // Multiplicacion de una matriz 3x2 por un vector2
{
	double b[2] = { vect.x, vect.y };
	double c[3];

	for (int i = 0; i<3; i++){
		c[i] = 0;
	}

	for (int i = 0; i<3; i++){
		for (int j = 0; j<2; j++){
			c[i] += (matrix[i][j] * b[j]);
		}
	}

	return{ c[0], c[1], c[2] };
}

inline Vector3 matrix32_Vector2_multiplication(float matrix[3][2], Vector2 vect) // Multiplicacion de una matriz 3x2 por un Vector2
{
	float b[2] = { vect.x, vect.y };
	float c[3];

	for (int i = 0; i<3; i++){
		c[i] = 0;
	}

	for (int i = 0; i<3; i++){
		for (int j = 0; j<2; j++){
			c[i] += (matrix[i][j] * b[j]);
		}
	}

	return{ c[0], c[1], c[2] }; // asi era antes pero derepente se rompio y las orbitas se comenzaron a dibujar mal, entonces tuve que cambiar al de abajo
	//return{ -c[0], c[2], -c[1] };
}

inline double angle_from_two_vectors(vec3 u, vec3 v)
{
	double valor = acos((u* v) / (length(u)*length(v)));
	if (isnan(valor)) return 0.0;
	return valor;
}

inline float angle_from_two_vectors_f(Vector3 u, Vector3 v)
{
	float valor = acosf(u.Dot(v) / (u.Length()*v.Length()));
	if (isnan(valor)) return 0.0f;
	return valor;
}
//																																					//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//															Quaternion math																			//
inline vec3 vector_rotation_by_quaternion(vec3 v, quat q)
{
	// Extract the vector part of the quaternion
	vec3 u = { q.x, q.y, q.z };

	// Extract the scalar part of the quaternion
	double s = q.w;

	// Do the math
	return u* (2.0 * (u* v)) + v* (s*s - (u* u)) + (u^ v)* (2.0 * s);
}

//																																					//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline vec3 vector_vector_rotation_by_angle(vec3 v1, vec3 v2, double angle)
{
	vec3 axis_ = normalized(v1^v2);
	angle = angle / 2.0;

	quat q_temp = { 0.0, 0.0, 0.0, 1.0 };
	q_temp.x = axis_.x * sin(angle);
	q_temp.y = axis_.y * sin(angle);
	q_temp.z = axis_.z * sin(angle);
	q_temp.w = cos(angle);

	return vector_rotation_by_quaternion(v1, q_temp);
}

// Matriz 4x4 de doble precision
struct Mat4d
{
	Mat4d::Mat4d() // Inicializamos la matriz como matriz de identidad
	{
		r[0][0] = 1.0; r[0][1] = 0.0; r[0][2] = 0.0; r[0][3] = 0.0;
		r[1][0] = 0.0; r[1][1] = 1.0; r[1][2] = 0.0; r[1][3] = 0.0;
		r[2][0] = 0.0; r[2][1] = 0.0; r[2][2] = 1.0; r[2][3] = 0.0;
		r[3][0] = 0.0; r[3][1] = 0.0; r[3][2] = 0.0; r[3][3] = 1.0;
	}

	double r[4][4];

	void scaling(const vec3 & rhs) // usamos un vector3 para crear una matriz 4x4 de escala
	{
		r[0][0] = rhs.x; r[0][1] = 0.0; r[0][2] = 0.0; r[0][3] = 0.0;
		r[1][0] = 0.0; r[1][1] = rhs.y; r[1][2] = 0.0; r[1][3] = 0.0;
		r[2][0] = 0.0; r[2][1] = 0.0; r[2][2] = rhs.z; r[2][3] = 0.0;
		r[3][0] = 0.0; r[3][1] = 0.0; r[3][2] = 0.0; r[3][3] = 1.0;
	}

	void translation(const vec3 & rhs) // usamos un vector3 para crear una matriz 4x4 de traslado
	{
		r[0][0] = 1; r[0][1] = 0.0; r[0][2] = 0.0; r[0][3] = 0.0;
		r[1][0] = 0.0; r[1][1] = 1; r[1][2] = 0.0; r[1][3] = 0.0;
		r[2][0] = 0.0; r[2][1] = 0.0; r[2][2] = 1; r[2][3] = 0.0;
		r[3][0] = rhs.x; r[3][1] = rhs.y; r[3][2] = rhs.z; r[3][3] = 1.0;
	}

	Mat4d operator*(const Mat4d & rhs)const // Multiplicacion de matrices 4x4
	{
#define MATMUL(R, C) (r[R][0] * rhs.r[0][C] + r[R][1] * rhs.r[1][C] + r[R][2] * rhs.r[2][C] + r[R][3] * rhs.r[3][C])

		Mat4d valor;

		valor.r[0][0] = MATMUL(0, 0); valor.r[0][1] = MATMUL(0, 1); valor.r[0][2] = MATMUL(0, 2); valor.r[0][3] = MATMUL(0, 3);
		valor.r[1][0] = MATMUL(1, 0); valor.r[1][1] = MATMUL(1, 1); valor.r[1][2] = MATMUL(1, 2); valor.r[1][3] = MATMUL(1, 3);
		valor.r[2][0] = MATMUL(2, 0); valor.r[2][1] = MATMUL(2, 1); valor.r[2][2] = MATMUL(2, 2); valor.r[2][3] = MATMUL(2, 3);
		valor.r[3][0] = MATMUL(3, 0); valor.r[3][1] = MATMUL(3, 1); valor.r[3][2] = MATMUL(3, 2); valor.r[3][3] = MATMUL(3, 3);

#undef MATMUL

		return valor;
	}

};

inline Mat4d toMatrix4(quat quaternion) // creacion de una matriz 4x4 usando un quaternio
{
	double wx = quaternion.w * quaternion.x * 2.0;
	double wy = quaternion.w * quaternion.y * 2.0;
	double wz = quaternion.w * quaternion.z * 2.0;
	double xx = quaternion.x * quaternion.x * 2.0;
	double xy = quaternion.x * quaternion.y * 2.0;
	double xz = quaternion.x * quaternion.z * 2.0;
	double yy = quaternion.y * quaternion.y * 2.0;
	double yz = quaternion.y * quaternion.z * 2.0;
	double zz = quaternion.z * quaternion.z * 2.0;

	Mat4d valor;

	valor.r[0][0] = 1.0 - yy - zz; valor.r[0][1] = xy - wz; valor.r[0][2] = xz + wy; valor.r[0][3] = 0.0;
	valor.r[1][0] = xy + wz; valor.r[1][1] = 1.0 - xx - zz; valor.r[1][2] = yz - wx; valor.r[1][3] = 0.0;
	valor.r[2][0] = xz - wy; valor.r[2][1] = yz + wx; valor.r[2][2] = 1.0 - xx - yy; valor.r[2][3] = 0.0;
	valor.r[3][0] = 0.0; valor.r[3][1] = 0.0; valor.r[3][2] = 0.0; valor.r[3][3] = 1.0;

	return valor;
}

struct Mat3d
{
	Mat3d::Mat3d() // Inicializamos la matriz como matriz de identidad
	{
		r[0][0] = 1.0; r[0][1] = 0.0; r[0][2] = 0.0;
		r[1][0] = 0.0; r[1][1] = 1.0; r[1][2] = 0.0;
		r[2][0] = 0.0; r[2][1] = 0.0; r[2][2] = 1.0;
	}

	double r[3][3];


	Mat3d operator*(const Mat3d & rhs)const // Multiplicacion de matrices 4x4
	{

#define MATMUL(R, C) (r[R][0] * rhs.r[0][C] + r[R][1] * rhs.r[1][C] + r[R][2] * rhs.r[2][C])

		Mat3d valor;

		valor.r[0][0] = MATMUL(0, 0); valor.r[0][1] = MATMUL(0, 1); valor.r[0][2] = MATMUL(0, 2);
		valor.r[1][0] = MATMUL(1, 0); valor.r[1][1] = MATMUL(1, 1); valor.r[1][2] = MATMUL(1, 2);
		valor.r[2][0] = MATMUL(2, 0); valor.r[2][1] = MATMUL(2, 1); valor.r[2][2] = MATMUL(2, 2);

#undef MATMUL

		return valor;
	}

	vec3 operator*(const vec3 & v) const
	{
		return vec3{ r[0][0] * v.x + r[0][1] * v.y + r[0][2] * v.z,
			r[1][0] * v.x + r[1][1] * v.y + r[1][2] * v.z,
			r[2][0] * v.x + r[2][1] * v.y + r[2][2] * v.z };
	}
};

inline Mat3d xrotation(double angle)
{
	double c = cos(angle);
	double s = sin(angle);

	Mat3d mat_;
	double r[3][3] = { 1, 0.0, 0.0, 0.0, c, -s, 0.0, s, c };
	mat_.r[0][0] = r[0][0]; mat_.r[1][0] = r[1][0]; mat_.r[2][0] = r[2][0];
	mat_.r[0][1] = r[0][1]; mat_.r[1][1] = r[1][1]; mat_.r[2][1] = r[2][1];
	mat_.r[0][2] = r[0][2]; mat_.r[1][2] = r[1][2]; mat_.r[2][2] = r[2][2];

	return mat_;
}


inline Mat4d Mat4Xrotation(double angle)
{
	double c = cos(angle);
	double s = sin(angle);

	Mat4d mat_;
	double r[4][4] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, c, -s, 0.0,
		0.0, s, c, 0.0,
		0.0, 0.0, 0.0, 1.0 };

	mat_.r[0][0] = r[0][0]; mat_.r[1][0] = r[1][0]; mat_.r[2][0] = r[2][0]; mat_.r[3][0] = r[3][0];
	mat_.r[0][1] = r[0][1]; mat_.r[1][1] = r[1][1]; mat_.r[2][1] = r[2][1]; mat_.r[3][1] = r[3][1];
	mat_.r[0][2] = r[0][2]; mat_.r[1][2] = r[1][2]; mat_.r[2][2] = r[2][2]; mat_.r[3][2] = r[3][2];
	mat_.r[0][3] = r[0][3]; mat_.r[1][3] = r[1][3]; mat_.r[2][3] = r[2][3]; mat_.r[3][3] = r[3][3];

	return mat_;
}

inline Mat4d Mat4Yrotation(double angle)
{
	double c = cos(angle);
	double s = sin(angle);

	Mat4d mat_;
	double r[4][4] = {
		c, 0.0, s, 0.0,
		0.0, 1.0, 0.0, 0.0,
		-s, 0.0, c, 0.0,
		0.0, 0.0, 0.0, 1.0 };

	mat_.r[0][0] = r[0][0]; mat_.r[1][0] = r[1][0]; mat_.r[2][0] = r[2][0]; mat_.r[3][0] = r[3][0];
	mat_.r[0][1] = r[0][1]; mat_.r[1][1] = r[1][1]; mat_.r[2][1] = r[2][1]; mat_.r[3][1] = r[3][1];
	mat_.r[0][2] = r[0][2]; mat_.r[1][2] = r[1][2]; mat_.r[2][2] = r[2][2]; mat_.r[3][2] = r[3][2];
	mat_.r[0][3] = r[0][3]; mat_.r[1][3] = r[1][3]; mat_.r[2][3] = r[2][3]; mat_.r[3][3] = r[3][3];

	return mat_;
}

inline Mat4d Mat4Zrotation(double angle)
{
	double c = cos(angle);
	double s = sin(angle);

	Mat4d mat_;
	double r[4][4] = {
		c, -s, 0.0, 0.0,
		s, c, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0 };

	mat_.r[0][0] = r[0][0]; mat_.r[1][0] = r[1][0]; mat_.r[2][0] = r[2][0]; mat_.r[3][0] = r[3][0];
	mat_.r[0][1] = r[0][1]; mat_.r[1][1] = r[1][1]; mat_.r[2][1] = r[2][1]; mat_.r[3][1] = r[3][1];
	mat_.r[0][2] = r[0][2]; mat_.r[1][2] = r[1][2]; mat_.r[2][2] = r[2][2]; mat_.r[3][2] = r[3][2];
	mat_.r[0][3] = r[0][3]; mat_.r[1][3] = r[1][3]; mat_.r[2][3] = r[2][3]; mat_.r[3][3] = r[3][3];

	return mat_;
}

inline Matrix Matrix_Look_at_Position(Vector3& pos_, Vector3& look_at_, Vector3& up_)
{
	Vector3 w = pos_ - look_at_;
	w = w / length_f(w);

	Vector3 u = up_.Cross(w);
	u = u / length_f(u);

	Vector3 v = w.Cross(u);

	return Matrix(
		u.x, u.y, u.z, 0.f,
		v.x, v.y, v.z, 0.f,
		w.x, w.y, w.z, 0.f,
		pos_.x, pos_.y, pos_.z, 1.f
		);
}



inline Mat3d zrotation(double angle)
{
	double c = cos(angle);
	double s = sin(angle);

	Mat3d mat_;
	double r[3][3] = { c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0 };
	mat_.r[0][0] = r[0][0]; mat_.r[1][0] = r[1][0]; mat_.r[2][0] = r[2][0];
	mat_.r[0][1] = r[0][1]; mat_.r[1][1] = r[1][1]; mat_.r[2][1] = r[2][1];
	mat_.r[0][2] = r[0][2]; mat_.r[1][2] = r[1][2]; mat_.r[2][2] = r[2][2];

	return mat_;
}

inline quat Mat3dToQuaternion(const Mat3d& m)
{
	quat q;
	double trace = m.r[0][0] + m.r[1][1] + m.r[2][2];
	double root;
	double epsilon = std::numeric_limits<double>::epsilon() * 1e3;

	if (trace >= epsilon - 1)
	{
		root = sqrt(trace + 1);
		q.w = 0.5 * root;
		root = 0.5 / root;
		q.x = (m.r[2][1] - m.r[1][2]) * root;
		q.y = (m.r[0][2] - m.r[2][0]) * root;
		q.z = (m.r[1][0] - m.r[0][1]) * root;
	}
	else
	{
		// Set i to the largest element of the diagonal
		int i = 0;
		if (m.r[1][1] > m.r[i][i])
			i = 1;
		if (m.r[2][2] > m.r[i][i])
			i = 2;
		int j = (i == 2) ? 0 : i + 1;
		int k = (j == 2) ? 0 : j + 1;

		root = sqrt(m.r[i][i] - m.r[j][j] - m.r[k][k] + 1);
		double* xyz[3] = { &q.x, &q.y, &q.z };
		*xyz[i] = 0.5 * root;
		root = 0.5 / root;
		q.w = (m.r[k][j] - m.r[j][k]) * root;
		*xyz[j] = (m.r[j][i] + m.r[i][j]) * root;
		*xyz[k] = (m.r[k][i] + m.r[i][k]) * root;
	}

	return q;
}

inline quat vecToVecRotation(vec3& u, vec3& v)
{
	vec3 w = u^ v;
	double l = length(w)*length(w);
	double real_part = u* v;
	if (l < 1.6e-12f && real_part < 0)
	{
		w = abs(u.x) > abs(u.z) ? vec3{ -u.y, u.x, 0.f } / sqrt(u.y*u.y + u.x*u.x)
			: vec3{ 0.f, -u.z, u.y } / sqrt(u.y*u.y + u.z*u.z);
		return quat{ w.x, w.y, w.z, 0 };
	}
	real_part += sqrt(real_part*real_part + l);
	return quat{ w.x, w.y, w.z, real_part } *(1 / sqrt(real_part*real_part + l));
}


inline quat slerp(const quat q0, const quat q1, double t)
{
	const double Nlerp_Threshold = 0.99999;

	double cosAngle = q0^ q1;

	// Assuming the quaternions representat rotations, ensure that we interpolate
	// through the shortest path by inverting one of the quaternions if the
	// angle between them is negative.
	quat qstart;
	if (cosAngle < 0)
	{
		qstart = -q0;
		cosAngle = -cosAngle;
	}
	else
	{
		qstart = q0;
	}

	// Avoid precision troubles when we're near the limit of acos range and
	// perform a linear interpolation followed by a normalize when interpolating
	// very small angles.
	if (cosAngle > Nlerp_Threshold)
	{
		quat q = qstart*(1 - t) + q1*t;
		q.normalize();
		return q;
	}

	double angle = acos(cosAngle);
	double interpolatedAngle = t * angle;

	// qstart and q2 will form an orthonormal basis in the plane of interpolation.
	quat q2 = q1 - qstart * cosAngle;
	q2.normalize();

	return qstart * cos(interpolatedAngle) + q2 * sin(interpolatedAngle);
}

inline quat quatlookAt(const vec3& from, const vec3& to, const vec3& up)
{
	vec3 n = to - from;
	n.normalize();
	vec3 v = n ^ up;
	v.normalize();
	vec3 u = v ^ n;

	Mat3d ma;
	ma.r[0][0] = v.x; ma.r[0][1] = v.y; ma.r[0][2] = v.z;
	ma.r[1][0] = u.x; ma.r[1][1] = u.y; ma.r[1][2] = u.z;
	ma.r[2][0] = -n.x; ma.r[2][1] = -n.y; ma.r[2][2] = -n.z;

	return Mat3dToQuaternion(ma);
}

inline Matrix Mat4d_to_D3DXMATRIX(const Mat4d Mat4matriz)
{
	Matrix matriz;

	matriz._11 = float(Mat4matriz.r[0][0]);
	matriz._12 = float(Mat4matriz.r[0][1]);
	matriz._13 = float(Mat4matriz.r[0][2]);
	matriz._14 = float(Mat4matriz.r[0][3]);

	matriz._21 = float(Mat4matriz.r[1][0]);
	matriz._22 = float(Mat4matriz.r[1][1]);
	matriz._23 = float(Mat4matriz.r[1][2]);
	matriz._24 = float(Mat4matriz.r[1][3]);

	matriz._31 = float(Mat4matriz.r[2][0]);
	matriz._32 = float(Mat4matriz.r[2][1]);
	matriz._33 = float(Mat4matriz.r[2][2]);
	matriz._34 = float(Mat4matriz.r[2][3]);

	matriz._41 = float(Mat4matriz.r[3][0]);
	matriz._42 = float(Mat4matriz.r[3][1]);
	matriz._43 = float(Mat4matriz.r[3][2]);
	matriz._44 = float(Mat4matriz.r[3][3]);

	return matriz;
}

inline Mat4d D3DXMATRIX_to_Mat4d(const Matrix Mat4matriz)
{
	Mat4d matriz;

	matriz.r[0][0] = float(Mat4matriz._11);
	matriz.r[0][1] = float(Mat4matriz._12);
	matriz.r[0][2] = float(Mat4matriz._13);
	matriz.r[0][3] = float(Mat4matriz._14);

	matriz.r[1][0] = float(Mat4matriz._21);
	matriz.r[1][1] = float(Mat4matriz._22);
	matriz.r[1][2] = float(Mat4matriz._23);
	matriz.r[1][3] = float(Mat4matriz._24);

	matriz.r[2][0] = float(Mat4matriz._31);
	matriz.r[2][1] = float(Mat4matriz._32);
	matriz.r[2][2] = float(Mat4matriz._33);
	matriz.r[2][3] = float(Mat4matriz._34);

	matriz.r[3][0] = float(Mat4matriz._41);
	matriz.r[3][1] = float(Mat4matriz._42);
	matriz.r[3][2] = float(Mat4matriz._43);
	matriz.r[3][3] = float(Mat4matriz._44);

	return matriz;
}

inline Vector3 RowVector3byMatrix(Vector3 vect_, Matrix mat_)
{
	vect_ *= -1;
	return{ mat_._11 * vect_.x + mat_._12 * vect_.y + mat_._13 * vect_.z,
		mat_._21 * vect_.x + mat_._22 * vect_.y + mat_._23 * vect_.z,
		mat_._31 * vect_.x + mat_._32 * vect_.y + mat_._33 * vect_.z };
}

inline vec3 RowVec3byMat4d(vec3 vect_, Mat4d mat_)
{
	return{ mat_.r[0][0] * vect_.x + mat_.r[1][0] * vect_.y + mat_.r[2][0] * vect_.z,
		mat_.r[0][1] * vect_.x + mat_.r[1][1] * vect_.y + mat_.r[2][1] * vect_.z,
		mat_.r[0][2] * vect_.x + mat_.r[1][2] * vect_.y + mat_.r[2][2] * vect_.z };
}

inline vec3 ColumnVec3byMat4d(vec3 vect_, Mat4d mat_)
{
	return{ mat_.r[0][0] * vect_.x + mat_.r[0][1] * vect_.y + mat_.r[0][2] * vect_.z,
		mat_.r[1][0] * vect_.x + mat_.r[1][1] * vect_.y + mat_.r[1][2] * vect_.z,
		mat_.r[2][0] * vect_.x + mat_.r[2][1] * vect_.y + mat_.r[2][2] * vect_.z };
}

inline Vector4 Vec4Mat4d(const Vector4 &lhs, const Mat4d &rhs)
{
	return Vector4(
		(lhs.x * float(rhs.r[0][0])) + (lhs.y * float(rhs.r[1][0])) + (lhs.z * float(rhs.r[2][0])) + (lhs.w * float(rhs.r[3][0])),
		(lhs.x * float(rhs.r[0][1])) + (lhs.y * float(rhs.r[1][1])) + (lhs.z * float(rhs.r[2][1])) + (lhs.w * float(rhs.r[3][1])),
		(lhs.x * float(rhs.r[0][2])) + (lhs.y * float(rhs.r[1][2])) + (lhs.z * float(rhs.r[2][2])) + (lhs.w * float(rhs.r[3][2])),
		(lhs.x * float(rhs.r[0][3])) + (lhs.y * float(rhs.r[1][3])) + (lhs.z * float(rhs.r[2][3])) + (lhs.w * float(rhs.r[3][3])));
}

inline vec3 d3d_to_vec(Vector3 vect){ return{ vect.x, vect.y, vect.z }; };

inline Vector3 vec_to_d3d(vec3 vect){ return{ float(vect.x), float(vect.y), float(vect.z) }; };

inline Quaternion quat_to_d3dquat(quat quaternion){ return{ float(quaternion.x), float(quaternion.y), float(quaternion.z), float(quaternion.w) }; };

inline quat d3dquat_toquat(Quaternion quaternion){ return{ double(quaternion.x), double(quaternion.y), double(quaternion.z), double(quaternion.w) }; };

inline vec3 scientific_to_d3d_coords(vec3 vect_){ return{ -vect_.x, vect_.z, -vect_.y }; };

inline vec3 d3d_to_scientific_coords(vec3 vect_){ return{ -vect_.x, -vect_.z, vect_.y }; };

inline Vector3 d3d_to_scientific_coords_f(Vector3 vect_){ return{ -vect_.x, -vect_.z, vect_.y }; };

inline quat quatChangeCoordSystem(quat quat_)
{
	Matrix mat_ = DirectX::XMMatrixRotationQuaternion(quat_to_d3dquat(quat_));

	mat_._13 = -mat_._13;
	mat_._23 = -mat_._23;
	mat_._31 = -mat_._31;
	mat_._32 = -mat_._32;

	return d3dquat_toquat(DirectX::XMQuaternionRotationMatrix(mat_));
}

inline quat Slerp(quat q1, quat q2, double lambda)
{
	quat qr = { 0.0, 0.0, 0.0, 1.0, };
	double dotproduct = q1^q2;

	double theta, st, sut, sout, coeff1, coeff2;

	// algorithm adapted from Shoemake's paper
	lambda = lambda / 2.0;

	theta = acos(dotproduct);
	if (theta<0.0) theta = -theta;

	st = sin(theta);
	sut = sin(lambda*theta);
	sout = sin((1 - lambda)*theta);
	coeff1 = sout / st;
	coeff2 = sut / st;

	qr.x = coeff1*q1.x + coeff2*q2.x;
	qr.y = coeff1*q1.y + coeff2*q2.y;
	qr.z = coeff1*q1.z + coeff2*q2.z;
	qr.w = coeff1*q1.w + coeff2*q2.w;

	qr.normalize();

	return qr;
}

inline vec3 GetArbitraryAxis(const vec3& normalized_axis, const quat& rotation)
{
	return vector_rotation_by_quaternion(normalized_axis, rotation);
}

inline Vector3 GetArbitraryAxis_f(const Vector3& normalized_axis, const quat& rotation)
{
	return vec_to_d3d(vector_rotation_by_quaternion(d3d_to_vec(normalized_axis), rotation));
}

inline vec3 GetAxisX(const quat& rotation)
{
	Matrix M = Matrix::CreateFromQuaternion(quat_to_d3dquat(rotation));
	return{ M(0, 0), M(1, 0), M(2, 0) };
}

inline vec3 GetAxisY(const quat& rotation)
{
	Matrix M = Matrix::CreateFromQuaternion(quat_to_d3dquat(rotation));
	return{ M(0, 1), M(1, 1), M(2, 1) };
}

inline vec3 GetAxisZ(const quat& rotation)
{
	Matrix M = Matrix::CreateFromQuaternion(quat_to_d3dquat(rotation));
	return{ M(0, 2), M(1, 2), M(2, 2) };
}

// convertimos el formato del tiempo a algo facilmente legible, como un reloj digital
inline std::string format_time_output(double time_)
{
	int anhos = int(time_ / 31536000);
	int dias = int(((time_)) / 86400) % 365;
	int horas = int((time_) / 3600) % 24;
	int minutos = int((time_) / 60) % 60;
	int segundos = int(time_) % 60;

	std::stringstream horillasd, minutillosd, segundillosd;
	horillasd << std::setfill('0') << std::setw(2) << horas;
	minutillosd << std::setfill('0') << std::setw(2) << minutos;
	segundillosd << std::setfill('0') << std::setw(2) << segundos;

	if (anhos >= 1)
	{
		if (anhos == 1)
		{
			if (dias == 1) return std::to_string(anhos) + " year, " + std::to_string(dias) + " day, " + horillasd.str() + ":" + minutillosd.str();
			else return std::to_string(anhos) + " year, " + std::to_string(dias) + " days, " + horillasd.str() + ":" + minutillosd.str();
		}

		else
		{
			if (dias == 1) return std::to_string(anhos) + " years, " + std::to_string(dias) + " day, " + horillasd.str() + ":" + minutillosd.str();
			else return std::to_string(anhos) + " years, " + std::to_string(dias) + " days, " + horillasd.str() + ":" + minutillosd.str();
		}
	}

	else if (dias >= 1)
	{
		if (dias == 1) return std::to_string(dias) + " day, " + horillasd.str() + ":" + minutillosd.str();
		else return std::to_string(dias) + " days, " + horillasd.str() + ":" + minutillosd.str();
	}

	else return horillasd.str() + ":" + minutillosd.str() + ":" + segundillosd.str();
}

// convertimos el formato de las distancias a unidades astronomicas, kilometros y metros
inline std::string format_distance_output(double distancia_f)
{
	double distancia_absoluta = abs(distancia_f);
	std::string retornar;
	double valor_retorno = 0;;
	std::stringstream texto_formateado;
	texto_formateado.imbue(std::locale(""));

	double num = distancia_absoluta;
	int intpart = (int)num;
	double decpart = num - intpart;

	if (distancia_absoluta < 1.0e3)
	{
		valor_retorno = distancia_absoluta;
		if (decpart != 0) texto_formateado << std::setiosflags(std::ios::fixed) << std::setprecision(2) << valor_retorno;
		else texto_formateado << setiosflags(std::ios::fixed) << std::setprecision(0) << valor_retorno;
		retornar = texto_formateado.str() + " mts.";
	}

	else if (distancia_absoluta < 1.0e11)
	{
		valor_retorno = distancia_absoluta / 1.0e3;
		if (decpart != 0.0)	texto_formateado << std::setiosflags(std::ios::fixed) << std::setprecision(2) << valor_retorno;
		else texto_formateado << setiosflags(std::ios::fixed) << std::setprecision(0) << valor_retorno;
		retornar = texto_formateado.str() + " kms.";
	}

	else
	{
		valor_retorno = distancia_absoluta / 1495978707.0e2;
		if (decpart != 0)	texto_formateado << setiosflags(std::ios::fixed) << std::setprecision(4) << valor_retorno;
		else texto_formateado << setiosflags(std::ios::fixed) << std::setprecision(0) << valor_retorno;
		retornar = texto_formateado.str() + " AU.";
	}

	if (distancia_f < 0.0) retornar = "-" + retornar;

	return retornar;
}

inline std::string format_decimal_number(double number_, int decimal_places)
{
	std::stringstream texto_formateado;
	texto_formateado.imbue(std::locale(""));
	texto_formateado << std::setiosflags(std::ios::fixed) << std::setprecision(decimal_places) << number_;

	return texto_formateado.str();
}

// convertimos el formato de las velocidades a kms/s, mts/s y kms/h
inline std::string format_speed_output(double speed_)
{
	double velocidad_absoluta = abs(speed_);
	std::string retornar;
	double valor_retorno = 0;;
	std::stringstream texto_formateado;
	texto_formateado.imbue(std::locale(""));

	double num = velocidad_absoluta;
	int intpart = (int)num;
	double decpart = num - intpart;

	if (velocidad_absoluta < 1.0e3)
	{
		valor_retorno = velocidad_absoluta*3.6;
		if (decpart != 0) texto_formateado << std::setiosflags(std::ios::fixed) << std::setprecision(2) << valor_retorno;
		else texto_formateado << setiosflags(std::ios::fixed) << std::setprecision(0) << valor_retorno;
		retornar = texto_formateado.str() + " kms/h.";
	}

	else if (velocidad_absoluta < 1.0e11)
	{
		valor_retorno = velocidad_absoluta / 1.0e3;
		if (decpart != 0.0)	texto_formateado << std::setiosflags(std::ios::fixed) << std::setprecision(2) << valor_retorno;
		else texto_formateado << setiosflags(std::ios::fixed) << std::setprecision(0) << valor_retorno;
		retornar = texto_formateado.str() + " kms/s.";
	}

	else
	{
		valor_retorno = velocidad_absoluta / 1495978707.0e2;
		if (decpart != 0)	texto_formateado << setiosflags(std::ios::fixed) << std::setprecision(4) << valor_retorno;
		else texto_formateado << setiosflags(std::ios::fixed) << std::setprecision(0) << valor_retorno;
		retornar = texto_formateado.str() + " AU/s.";
	}

	return retornar;
}

#endif