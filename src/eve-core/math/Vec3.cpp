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

#include <math.h>

#include "Vec3.h"

double Vec3::magnitude() const
{
    double m;

    //calculate the length of the Vec3
    m = (x * x) + (y * y) + (z * z);
    m = sqrt(m);

    return m;
}

void Vec3::normalize()
{
    double m;

    //calculate the length of the Vec3
    m = (x * x) + (y * y) + (z * z);
    m = sqrt(m);

    // if the length is zero then the Vec3 is zero so return
    if (m == 0)
    {
        return;
    }

    //Scale the Vec3 to a unit length
    x /= m;
    y /= m;
    z /= m;
}

double Vec3::dotProduct(const Vec3 &v) const
{
    return ((x * v.x) + (y * v.y) + (z * v.z));
}

Vec3 Vec3::crossProduct(const Vec3 &v) const
{
    double nx = y * v.z - z * v.y;
    double ny = z * v.x - x * v.z;
    double nz = x * v.y - y * v.x;
    return Vec3(nx, ny, nz);
}

Vec3 Vec3::reflection(Vec3 norm) const
{
    Vec3 refl;
    refl = norm * (-2 * dotProduct(norm));
    refl = (*this) - refl;
    return refl;
}

Vec3 Vec3::refraction(Vec3 norm, double fact) const
{
    Vec3 refr;
    double n_r = this->dotProduct(norm);
    double k = 1 - (n_r * n_r);
    k = 1 - (fact * fact) * k;
    if (k < 0)
    {
        return refr;
    }
    refr = (*this) * fact;
    refr -= norm * (fact * n_r + sqrt(k));
    return refr;
}

void Vec3::set(double nx, double ny, double nz)
{
    x = nx;
    y = ny;
    z = nz;
}

void Vec3::set(const Vec3 &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
}

Vec3& Vec3::operator=(const Vec3 &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vec3 Vec3::operator+(const Vec3 &v) const
{
    return Vec3(*this) += v;
}

Vec3& Vec3::operator+=(const Vec3 &v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vec3 Vec3::operator-(const Vec3 &v) const
{
    return Vec3(*this) -= v;
}

Vec3& Vec3::operator-=(const Vec3 &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vec3 Vec3::operator*(const Vec3 &v) const
{
    return Vec3(*this) *= v;
}

Vec3& Vec3::operator*=(const Vec3 &v)
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

Vec3 Vec3::operator/(const Vec3 &v) const
{
    return Vec3(*this) /= v;
}

Vec3& Vec3::operator/=(const Vec3 &v)
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

Vec3 Vec3::operator*(const double &k) const
{
    return Vec3(*this) *= k;
}

Vec3& Vec3::operator*=(const double &k)
{
    x *= k;
    y *= k;
    z *= k;
    return *this;
}

Vec3 Vec3::operator/(const double &k) const
{
    return Vec3(*this) /= k;
}

Vec3& Vec3::operator/=(const double &k)
{
    x /= k;
    y /= k;
    z /= k;
    return *this;
}
