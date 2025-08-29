/**
 * Vector.cpp
 *      A 3 dimensional vector using long long signed integers.  (double precision not required and int math faster)
 *
 * @Author:     Allan
 * @Version:    0.1
 * @Date:       9Feb25
 */

#include <math.h>

#include "Vector.h"


double Vector::magnitude() const
{
    double m;

    //calculate the length of the Vector
    m = (x * x) + (y * y) + (z * z);
    m = sqrt(m);

    return m;
}

void Vector::normalize()
{
    double m;

    //calculate the length of the Vector
    m = (x * x) + (y * y) + (z * z);
    m = sqrt(m);

    // if the length is zero then the Vector is zero so return
    if (m == 0)
    {
        return;
    }

    //Scale the Vector to a unit length
    x /= m;
    y /= m;
    z /= m;
}

double Vector::dotProduct(const Vector &v) const
{
    return ((x * v.x) + (y * v.y) + (z * v.z));
}

Vector Vector::crossProduct(const Vector &v) const
{
    double nx = y * v.z - z * v.y;
    double ny = z * v.x - x * v.z;
    double nz = x * v.y - y * v.x;
    return Vector(nx, ny, nz);
}

Vector Vector::reflection(Vector norm) const
{
    Vector refl;
    refl = norm * (-2 * dotProduct(norm));
    refl = (*this) - refl;
    return refl;
}

Vector Vector::refraction(Vector norm, double fact) const
{
    Vector refr;
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

void Vector::set(double nx, double ny, double nz)
{
    x = nx;
    y = ny;
    z = nz;
}

void Vector::set(const Vector &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
}

Vector& Vector::operator=(const Vector &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vector Vector::operator+(const Vector &v) const
{
    return Vector(*this) += v;
}

Vector& Vector::operator+=(const Vector &v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vector Vector::operator-(const Vector &v) const
{
    return Vector(*this) -= v;
}

Vector& Vector::operator-=(const Vector &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vector Vector::operator*(const Vector &v) const
{
    return Vector(*this) *= v;
}

Vector& Vector::operator*=(const Vector &v)
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

Vector Vector::operator/(const Vector &v) const
{
    return Vector(*this) /= v;
}

Vector& Vector::operator/=(const Vector &v)
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

Vector Vector::operator*(const double &k) const
{
    return Vector(*this) *= k;
}

Vector& Vector::operator*=(const double &k)
{
    x *= k;
    y *= k;
    z *= k;
    return *this;
}

Vector Vector::operator/(const double &k) const
{
    return Vector(*this) /= k;
}

Vector& Vector::operator/=(const double &k)
{
    x /= k;
    y /= k;
    z /= k;
    return *this;
}
