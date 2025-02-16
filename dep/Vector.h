/**
 * Vector.cpp
 *      A 3 dimensional vector using long long signed integers.  (double precision not required and int math faster)
 *
 * @Author:     Allan
 * @Version:    0.1
 * @Date:       9Feb25
 */


#ifndef _EVE_SERVER_CORE_MATH_VECTOR_H
#define	_EVE_SERVER_CORE_MATH_VECTOR_H


#include "../utils/misc.h"


class Vector
{
public:
    int64 x, y, z;

    Vector()
    {
        x = 0;
        y = 0;
        z = 0;
    }

    Vector(int64 nx, int64 ny, int64 nz)
    {
        x = nx;
        y = ny;
        z = nz;
    }

    Vector(const Vector &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    Vector(const Vector &from, const Vector &to)
    {
        x = (to.x - from.x);
        y = (to.y - from.y);
        z = (to.z - from.z);
    }

    Vector copy() const
    {
        return Vector(*this);
    }

    double magnitude() const;

    double length() const
    {
        return magnitude();
    }

    double lengthSquared() const
    {
        return (x * x) + (y * y) + (z * z);
    }

    /**
     * Scale the Vector so it has a length of 1.
     */
    void normalize();

    /**
     * Calculate the dot product of this vector and the provided vector.
     * @param v The provided vector.
     * @return The dot product.
     * @note If both Vectors are unit Vectors then the result is the cosine of the angle between them.
     * otherwise it's scaled by the product of their lengths.
     */
    double dotProduct(const Vector &v) const;

    /**
     * Calculate the cross product of this vector and the provided vector.
     * @param v The provided vector.
     * @return The cross product of the 2 vectors.
     * @note The resultant vector is perpendicular to a plane formed by the two vectors.
     * @note The magnitude of the result is the area formed by the parallelogram of the two vectors.
     */
    Vector crossProduct(const Vector &v) const;

    /**
     * Calculate the reflection vector for a surface with the specified normal.
     * @param norm The surface normal of the surface to reflect from.
     * @return The reflection vector.
     */
    Vector reflection(Vector norm) const;

    /**
     * Calculate the refraction vector for a surface with the specified normal.
     * @param norm The surface normal of the surface to refract through.
     * @param fact The refraction factor.
     * @return The refraction vector.
     */
    Vector refraction(Vector norm, double fact) const;

    /**
     * Set the values of the vector.
     * @param nx The new value for X.
     * @param ny The new value for Y.
     * @param nz The new value for Z.
     */
    void set(int64 nx, int64 ny, int64 nz);
    /**
     * Set the values of the vector.
     * @param v The new value for the vector.
     */
    void set(const Vector &v);

    bool operator==(const Vector &v)
    {
        return v.x == x && v.y == y && v.z == z;
    }
    bool operator!=(const Vector &v)
    {
        return v.x != x || v.y != y || v.z != z;
    }

    Vector& operator=(const Vector &v);

    Vector operator+(const Vector &v) const;
    Vector& operator+=(const Vector &v);
    Vector operator-(const Vector &v) const;
    Vector& operator-=(const Vector &v);
    Vector operator*(const Vector &v) const;
    Vector& operator*=(const Vector &v);
    Vector operator/(const Vector &v) const;
    Vector& operator/=(const Vector &v);

    //scale the Vector
    Vector operator*(const double &k) const;
    Vector& operator*=(const double &k);
    Vector operator/(const double &k) const;
    Vector& operator/=(const double &k);

    bool isNotZero()
    {
        return x != 0 || y != 0 || z != 0;
    }

    // Public functions for manipulating 3D coordinates in space:
    // Take existing (x,y,z) point and use that as the center of a sphere of 'radius' and
    // modify it to be a new (x,y,z) point randomly placed on that sphere about the original
    // center coordinate: (x,y,z)

    void MakeRandomPointOnSphere(int64 radius)
    {
        double theta = MakeRandomFloat(0.0, (2 * M_PI));
        double phi = MakeRandomFloat(0.0, (2 * M_PI));
        x += radius * sin(theta) * cos(phi);
        y += radius * sin(theta) * sin(phi);
        z += radius * cos(theta);
    }

    // Take existing (x,y,z) point and use that as the center of two spheres of 'radiusInner', the
    // smaller radius sphere, and 'radiusOuter', the larger radius sphere, and modify
    // the original coordinate to be a new (x,y,z) point randomly placed somewhere inside the volume
    // enclosed between the smaller sphere and the large sphere

    void MakeRandomPointOnSphereLayer(int64 radiusInner, int64 radiusOuter)
    {
        double theta = MakeRandomFloat(0.0, (2 * EvE::Trig::Pi));
        double phi = MakeRandomFloat(0.0, (2 * EvE::Trig::Pi));
        double intermediateRadius = MakeRandomFloat(radiusInner, radiusOuter);
        x += intermediateRadius * sin(theta) * cos(phi);
        y += intermediateRadius * sin(theta) * sin(phi);
        z += intermediateRadius * cos(theta);
    }

    class GPoint : public Vector {
    public:
        GPoint():Vector(){}
        GPoint(Ga::GaFloat v):Vector(v){}
        GPoint(const Ga::GaFloat *v):Vector(v){}
        GPoint(Ga::GaFloat X,Ga::GaFloat Y,Ga::GaFloat Z):Vector(X, Y, Z){}
        GPoint(const GPoint& oth):Vector(oth){}
        GPoint(const Vector& oth):Vector(oth){}

        // Public functions for manipulating 3D coordinates in space:
        // Take existing (x,y,z) point and use that as the center of a sphere of 'radius' and
        // modify it to be a new (x,y,z) point randomly placed on that sphere about the original
        // center coordinate: (x,y,z)
        void MakeRandomPointOnSphere(double radius)
        {
            double theta = MakeRandomFloat( 0.0, (2*M_PI) );
            double phi = MakeRandomFloat( 0.0, (2*M_PI) );
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
            double theta = MakeRandomFloat( 0.0, (2*M_PI) );
            double phi = MakeRandomFloat( 0.0, (2*M_PI) );
            double intermediateRadius = MakeRandomFloat( radiusInner, radiusOuter );
            x += intermediateRadius * sin(theta) * cos(phi);
            z += intermediateRadius * sin(theta) * sin(phi);
            y += intermediateRadius * cos(theta);
        }
    };

    class GVector : public Vector {
    public:
        GVector():Vector(){}
        GVector(Ga::GaFloat v):Vector(v){}
        GVector(const Ga::GaFloat *v):Vector(v){}
        GVector(Ga::GaFloat X,Ga::GaFloat Y,Ga::GaFloat Z):Vector(X, Y, Z){}
        GVector(const GPoint& oth):Vector(oth){}
        GVector(const Vector& oth):Vector(oth){}
        GVector(const GPoint& from, const GPoint& to)
        : Vector( (to.x-from.x), (to.y-from.y), (to.z-from.z) ) {}
    };



    //TODO: inline most of this crap.

    class GPoint {
public:
    GPoint();
    GPoint(double x, double y, double z);

    inline void operator()(double nx, double ny, double nz) { pt[0] = nx; pt[1] = ny; pt[2] = nz; }

    inline const double x() const { return(pt[0]); }
    inline const double y() const { return(pt[1]); }
    inline const double z() const { return(pt[2]); }

    GPoint cross(const GPoint &them) const;
    double dot3(const GPoint &them) const;

    const GPoint &operator+=(const GPoint &v2);
    const GPoint &operator-=(const GPoint &v2);
    const GPoint &operator*=(const double c);
    const GPoint &operator/=(const double c);

    double pt[3];

};

GPoint operator+(const GPoint &v1, const GPoint &v2);
GPoint operator-(const GPoint &v1, const GPoint &v2);
GPoint operator*(const GPoint &v1, const double c);
GPoint operator/(const GPoint &v1, const double c);
GPoint operator*(const double c, const GPoint &v1);
GPoint operator/(const double c, const GPoint &v1);

class GVector : public GPoint {
public:
    GVector();
    GVector(const GPoint &them);
    GVector(const GPoint &from, const GPoint &to);
    GVector(double x, double y, double z);

    void normalize();
    double normalize_getlen();    //normalize and return the calculated length while your at it
    double length() const;
    double length2() const;    //length squared
};

};

#endif  // _EVE_SERVER_CORE_MATH_VECTOR_H
