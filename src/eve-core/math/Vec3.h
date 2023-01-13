/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     eve-moo
 */

/**
 * A 3 dimensional double vector.
 */

#ifndef VECTOR3D_H
#define	VECTOR3D_H


#include "../utils/misc.h"


class Vec3
{
public:
    double x, y, z;

    Vec3()
    {
        x = 0;
        y = 0;
        z = 0;
    }

    Vec3(double nx, double ny, double nz)
    {
        x = nx;
        y = ny;
        z = nz;
    }

    Vec3(const Vec3 &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    Vec3(const Vec3 &from, const Vec3 &to)
    {
        x = (to.x - from.x);
        y = (to.y - from.y);
        z = (to.z - from.z);
    }

    /**
     * Return a copy of the vector.
     * @return The copy of the vector.
     */
    Vec3 copy() const
    {
        return Vec3(*this);
    }

    /**
     * Retrieves the length of the Vector.
     * @return The length of the vector.
     */
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
     * otherwise it's scaled by the product of there lengths.
     */
    double dotProduct(const Vec3 &v) const;

    /**
     * Calculate the cross product of this vector and the provided vector.
     * @param v The provided vector.
     * @return The cross product of the 2 vectors.
     * @note The resultant vector is perpendicular to a plane formed by the two vectors.
     * @note The magnitude of the result is the area formed by the parallelogram of the two vectors.
     */
    Vec3 crossProduct(const Vec3 &v) const;

    /**
     * Calculate the reflection vector for a surface with the specified normal.
     * @param norm The surface normal of the surface to reflect from.
     * @return The reflection vector.
     */
    Vec3 reflection(Vec3 norm) const;

    /**
     * Calculate the refraction vector for a surface with the specified normal.
     * @param norm The surface normal of the surface to refract through.
     * @param fact The refraction factor.
     * @return The refraction vector.
     */
    Vec3 refraction(Vec3 norm, double fact) const;

    /**
     * Set the values of the vector.
     * @param nx The new value for X.
     * @param ny The new value for Y.
     * @param nz The new value for Z.
     */
    void set(double nx, double ny, double nz);
    /**
     * Set the values of the vector.
     * @param v The new value for the vector.
     */
    void set(const Vec3 &v);

    bool operator==(const Vec3 &v)
    {
        return v.x == x && v.y == y && v.z == z;
    }
    bool operator!=(const Vec3 &v)
    {
        return v.x != x || v.y != y || v.z != z;
    }

    Vec3& operator=(const Vec3 &v);

    Vec3 operator+(const Vec3 &v) const;
    Vec3& operator+=(const Vec3 &v);
    Vec3 operator-(const Vec3 &v) const;
    Vec3& operator-=(const Vec3 &v);
    Vec3 operator*(const Vec3 &v) const;
    Vec3& operator*=(const Vec3 &v);
    Vec3 operator/(const Vec3 &v) const;
    Vec3& operator/=(const Vec3 &v);

    //scale the Vec3
    Vec3 operator*(const double &k) const;
    Vec3& operator*=(const double &k);
    Vec3 operator/(const double &k) const;
    Vec3& operator/=(const double &k);

    bool isNotZero()
    {
        return x != 0 || y != 0 || z != 0;
    }

    // Public functions for manipulating 3D coordinates in space:
    // Take existing (x,y,z) point and use that as the center of a sphere of 'radius' and
    // modify it to be a new (x,y,z) point randomly placed on that sphere about the original
    // center coordinate: (x,y,z)

    void MakeRandomPointOnSphere(double radius)
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

    void MakeRandomPointOnSphereLayer(double radiusInner, double radiusOuter)
    {
        double theta = MakeRandomFloat(0.0, (2 * M_PI));
        double phi = MakeRandomFloat(0.0, (2 * M_PI));
        double intermediateRadius = MakeRandomFloat(radiusInner, radiusOuter);
        x += intermediateRadius * sin(theta) * cos(phi);
        y += intermediateRadius * sin(theta) * sin(phi);
        z += intermediateRadius * cos(theta);
    }
};

#endif

