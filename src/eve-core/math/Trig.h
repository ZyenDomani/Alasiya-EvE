
/**
 * @name EVE_Trig.h
 *    math defines and methods for trig used in EvEmu
 *  using EVE namespace
 * @Author:         Allan
 * @date:   30 Aug 2015
 */

#include "math/gpoint.h"

namespace EvE {
    namespace Trig {

        const double E = 2.71828182845904523536;
        const double Pi = 3.1415926535897932384626433832795;
        const double RadiansInDegrees = 0.01745329251;    //  pi/180
        const double DegreesInRadians = 57.2957795131;   //  180/pi

        inline double Deg2Rad(double deg) { return (deg * RadiansInDegrees); }
        inline double Rad2Deg(double rad) { return (rad * DegreesInRadians); }


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
        //GPoint Sph2Cart(float az, float ele, float radius) {        }

/*The mapping from three-dimensional Cartesian coordinates to spherical coordinates is

azimuth = atan2(y,x)
elevation = atan2(z,sqrt(x.^2 + y.^2))
r = sqrt(x.^2 + y.^2 + z.^2)

The notation for spherical coordinates is not standard.
For the cart2sph function, elevation is measured from the x-y plane.
Notice that if elevation = 0, the point is in the x-y plane.
If elevation = pi/2, then the point is on the positive z-axis.
*/

        //void Cart2Sph(GPoint pos, float& az, float& ele, float& radius) {        }


    }
}