
/**
 * @name Trig.h
 *    math defines and methods for trig used in EvEmu
 *  using EvE::Trig namespace
 * @Author:         Allan
 * @date:   30 Aug 2015
 */

#pragma once
#ifndef EvE__Trig_H
#define EvE__Trig_H

#include <cmath>
#include "../eve-compat.h"

// EvE uses the 3d right hand cartesian coordinate system, centered on star.
// +x is right, +y is up elevation, +z is "into", or up in 2d

namespace EvE {

    namespace Trig {

        // Global constants optimized for single-precision math hardware
        const double E                   =  2.71828182845904523536028747135;
        const double halfPi              =  1.57079632679489661923132169164;
        const double Pi                  =  3.14159265358979323846264338328;
        const double Pi2                 =  6.28318530717958647692528676656;
        const double FivePiSq            = 49.3480220054467930941724549994; // 5 * PI^2
        const double RadiansInDegrees    =  0.0174532925199432957692369076849;   //  pi/180
        const double DegreesInRadians    = 57.2957795130823208767981548141;   //  180/pi

        inline double Deg2Rad(double deg) { return (deg * RadiansInDegrees); }
        inline double Rad2Deg(double rad) { return (rad * DegreesInRadians); }


        // High-speed, branchless Bhāskara I Sine approximation
        inline double FastSin(float x) {
            // 1. Map angle smoothly to the core [-PI, PI] domain without while loops
            // This floating-point modulus keeps processing times flat even at -O0
            x = x - (std::floor((x + Pi) * (1.0f / Pi2)) * Pi2);

            // 2. Extract sign bit safely to make the rest of the math completely absolute
            float sign = (x < 0.0f) ? -1.0f : 1.0f;
            float absX = (x < 0.0f) ? -x : x;

            // 3. Bhāskara I formula: 16x(PI - x) / (5PI^2 - 4x(PI - x))
            double piMinusX = Pi - absX;
            double numerator = 16.0f * absX * piMinusX;
            double denominator = FivePiSq - (4.0f * absX * piMinusX);

            return sign * (numerator / denominator);
        }

        // High-speed Bhāskara I Cosine approximation
        inline double FastCos(float x) {
            // cos(x) is mathematically identical to sin(x + PI/2)
            return FastSin(x + halfPi);
        }

        // High-performance, highly vectorizable exponential approximation
        inline double fast_exp(double x) {
            // Basic polynomial approximation for e^x where x is negative
            // Accurate enough for positioning look-aheads and physics blending
            x = 1.0 + x / 256.0;
            x *= x; x *= x; x *= x; x *= x;
            x *= x; x *= x; x *= x; x *= x;
            return x;
        }
    }
}


/*
 * azimuth is the counterclockwise angle in the x-y plane measured in radians from the positive x-axis.
 * elevation is the elevation angle in radians from the x-y plane.
 */

/*
The mapping from spherical coordinates to three-dimensional Cartesian coordinates is

x = r .* cos(elevation) .* cos(azimuth)
y = r .* cos(elevation) .* sin(azimuth)
z = r .* sin(elevation)
*/
        //Vector3d Sph2Cart(float az, float ele, float radius) {        }

/*The mapping from three-dimensional Cartesian coordinates to spherical coordinates is

azimuth = atan2(y,x)
elevation = atan2(z,sqrt(x.^2 + y.^2))
r = sqrt(x.^2 + y.^2 + z.^2)

The notation for spherical coordinates is not standard.
For the cart2sph function, elevation is measured from the x-y plane.
Notice that if elevation = 0, the point is in the x-y plane.
If elevation = pi/2, then the point is on the positive z-axis.
*/

        //void Cart2Sph(Vector3d pos, float& az, float& ele, float& radius) {        }


/*
 * def RayToPlaneIntersection(P, d, Q, n):
 *    """
 *        The intersection point(S) on a plane where d shot from P would intersect
 *        the plane defined by Q and n.
 *
 *        If the P lies on the plane defined by n and Q, there are infinite number of
 *        intersection points so the function returns P.
 *
 *        d' = - Q.Dot(n)
 *        t = -(n.Dot(P) + d' )/n.Dot(d)
 *        S = P + t*d
 *    """
 *    denom = geo2.Vec3Dot(n, d)
 *    if abs(denom) < 1e-05:
 *        return P
 *    else:
 *        distance = -geo2.Vec3Dot(Q, n)
 *        t = -(geo2.Vec3Dot(n, P) + distance) / denom
 *        scaledRay = geo2.Scale(d, t)
 *        ret = geo2.Add(scaledRay, P)
 *        return geo2.Vector(*ret)
 */

#endif  //EvE__Trig_H