// Copyright © 2014 CCP ehf.

#pragma once
#ifndef Vector3d_H
#define Vector3d_H

#include <math.h>

#include "Trig.h"
#include "../utils/misc.h"

struct Vector3d
{
public:
	Vector3d() : x(0.0),y(0.0),z(0.0) {};
	Vector3d( const double * c) { x = c[0]; y = c[1]; z = c[2];};
	Vector3d( const Vector3d& v ) { x = v.x; y = v.y; z = v.z; };
	//Vector3d( const Vector3& v ) { x = v.x; y = v.y; z = v.z; };
	Vector3d( double _x, double _y, double _z ) { x = _x; y = _y; z = _z;};
	Vector3d( const float* v ){x = v[0]; y = v[1]; z = v[2];};

	// Casting
	operator double* () { return (double *)&x; };
	operator const double* () const { return (const double *)&x; };
	//const Vector3 AsVector3() const { return Vector3( (float)x, (float)y, (float)z); };

	// Vector3d Assignment operators
	Vector3d& operator += ( const Vector3d& v ) { x+=v.x; y+=v.y; z+=v.z; return *this; };
	Vector3d& operator -= ( const Vector3d& v ) { x-=v.x; y-=v.y; z-=v.z; return *this; };
	Vector3d& operator *= ( double c ) { x*=c; y*=c; z*=c; return *this; };
	Vector3d& operator /= ( double c ) { c = 1.0/c; x*=c; y*=c; z*=c; return *this; };
	// Vector3 Assignment operators
	//Vector3d& operator += ( const Vector3& v ) { x+=v.x; y+=v.y; z+=v.z; return *this; };
	//Vector3d& operator -= ( const Vector3& v ) { x-=v.x; y-=v.y; z-=v.z; return *this; };

	// Unary operators
	Vector3d operator + () const { return *this; };
	Vector3d operator - () const { return Vector3d(-x,-y,-z); };

	// Binary operators
	const Vector3d operator + ( const Vector3d& v ) const { return Vector3d( x+v.x, y+v.y, z+v.z );};
	const Vector3d operator - ( const Vector3d& v ) const { return Vector3d( x-v.x, y-v.y, z-v.z );};

	// Vector3 binary operators
	//const Vector3d operator + ( const Vector3& v ) const { return Vector3d( x+v.x, y+v.y, z+v.z );};
	//const Vector3d operator - ( const Vector3& v ) const { return Vector3d( x-v.x, y-v.y, z-v.z );};

	const Vector3d operator * ( double c ) const { return Vector3d( x*c, y*c, z*c ); };
	const Vector3d operator / ( double c ) const { c = 1.0/c; return Vector3d( x*c, y*c, z*c ); };

	// Dot product as multiplication
	const double operator * ( const Vector3d& v ) const { return x*v.x + y*v.y + z*v.z; };

	// This allows the order of operations to be the other way around without these classes knowing about Vector3d
	friend const Vector3d operator * ( double c, const struct Vector3d& v) { return v*c; };
	//friend const Vector3d operator + ( const Vector3& v, const Vector3d& vd ) { return Vector3d( v.x + vd.x, v.y + vd.y, v.z + vd.z ); };
	//friend const Vector3d operator - ( const Vector3& v, const Vector3d& vd ) { return Vector3d( v.x - vd.x, v.y - vd.y, v.z - vd.z ); };

	bool operator == ( const Vector3d& v ) const { return x==v.x && y==v.y && z==v.z; };
	bool operator != ( const Vector3d& v ) const { return x!=v.x || y!=v.y || z!=v.z; };

	double Length() const { return std::sqrt(x*x+y*y+z*z);};
	double LengthSq() const { return x*x+y*y+z*z;};

        double dotProduct(const Vector3d& oth) {
            return (x*oth.x+y*oth.y+z*oth.z);
        }

	Vector3d& Normalize()
	{
		double norm = Length();
		if (norm==0.0)
                    return *this;
		norm=1.0/norm;
		x *= norm;
		y *= norm;
		z *= norm;
		return *this;
	};

	Vector3d& Cross(const Vector3d& v)
	{
		double xt,yt,zt;
		xt = y * v.z - z * v.y;
		yt = z * v.x - x * v.z;
		zt = x * v.y - y * v.x;

		x = xt;
		y = yt;
		z = zt;
		return *this;
	};

        // Public functions for manipulating 3D coordinates in space:
        // Take existing (x,y,z) point and use that as the center of a sphere of 'radius' and
        // modify it to be a new (x,y,z) point randomly placed on that sphere about the original
        // center coordinate: (x,y,z)
        void MakeRandomPointOnSphere(double radius)
        {
            double theta = MakeRandomFloat( 0.0, EvE::Trig::Pi2);
            double phi = MakeRandomFloat( 0.0, EvE::Trig::Pi2);
            x += radius * sin(theta) * cos(phi);
            z += radius * sin(theta) * sin(phi);
            y += radius * cos(theta);
        }

        // Take existing (x,y,z) point and use that as the center of two spheres of 'radiusInner', the
        // smaller radius sphere, and 'radiusOuter', the larger radius sphere, and modify
        // the original coordinate to be a new (x,y,z) point randomly placed somewhere inside the volume
        // enclosed between the smaller sphere and the large sphere
        void MakeRandomPointOnSphereLayer(double radiusInner, double radiusOuter)
        {
            double theta = MakeRandomFloat( 0.0, EvE::Trig::Pi2);
            double phi = MakeRandomFloat( 0.0, EvE::Trig::Pi2);
            double intermediateRadius = MakeRandomFloat( radiusInner, radiusOuter );
            x += intermediateRadius * sin(theta) * cos(phi);
            z += intermediateRadius * sin(theta) * sin(phi);
            y += intermediateRadius * cos(theta);
        }

        bool isZero() const {
            return ((x == 0.0) and (y == 0.0) and (z == 0.0));
        }
        bool notZero() const {
            return ((x != 0.0) and (y != 0.0) and (z != 0.0));
        }
        bool isNaN() const {
            return (std::isnan(x) or std::isnan(y) or std::isnan(z));
        }
        bool isInf() const {
            return (std::isinf(x) or std::isinf(y) or std::isinf(z));
        }

        double x,y,z;
};

#endif