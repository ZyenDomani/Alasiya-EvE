
1. Fundamentals
A celestial body usually orbits the sun in an elliptical orbit. Perturbations from other planets causes small deviations from this elliptical orbit, but an unperturbed elliptical orbit can be used as a first approximation, and sometimes as the final approximation. If the distance from the Sun to the planet always is the same, then the planet follows a circular orbit. No planet does this, but the orbits of Venus and Neptune are very close to circles. Among the planets, Mercury and Pluto have orbits that deviate the most from a circle, i.e. are the most eccentric. Many asteroids have even more eccentric orbits, but the most eccentric orbits are to be found among the comets. Halley's comet, for instance, is closer to the Sun than Venus at perihelion, but farther away from the Sun than Neptune at aphelion. Some comets have even more eccentric orbits that are best approximated as a parabola. These orbits are not closed - a comet following a parabolic orbit passes the Sun only once, never to return. In reality these orbits are extremely elongated ellipses though, and these comets will eventually return, sometimes after many millennia.

The perihelion and aphelion are the points in the orbit when the planet is closest to and most distant from the Sun. A parabolic orbit only has a perihelion of course.

The perigee and apogee are points in the Moon's orbit (or the orbit of an artificial Earth satellite) which are closest to and most distant from the Earth.

The celestial sphere is an imaginary sphere around the observer, at an arbitrary distance.

The celestial equator is the Earth's equatorial plane projected on the celestial sphere.

The ecliptic is the plane of the Earth's orbit. This is also the plane of the Sun's yearly apparent motion. The ecliptic is inclined by approximately 23.4_deg to the celestial equator. The ecliptic intersects the celestial equator at two points: The Vernal Point (or "the first point of Aries"), and the Autumnal Point. The Vernal Point is the point of origin for two different commonly used celestial coordinates: equatorial coordinates and ecliptic coordinates.

Right Ascension and Declination are equatorial coordinates using the celestial equator as a fundamental plane. At the Vernal Point both the Right Ascension and the Declination are zero. The Right Ascension is usually measured in hours and minutes, where one revolution is 24 hours (which means 1 hour equals 15 degrees). It's counted countersunwise along the celestial equator. The Declination goes from +90 to -90 degrees, and it's positive north of, and negative south of, the celestial equator.

Longitude and Latitude are ecliptic coordinates, which use the ecliptic as a fundamental plane. Both are measured in degrees, and these coorinates too are both zero at the Vernal Point. The Longitude is counted countersunwise along the ecliptic. The Latitude is positive north of the ecliptic. Of course longitude and latitude are also used as terrestial coordinates, to measure a position of a point on the surface of the Earth.

Heliocentric, Geocentric, Topocentric. A position relative to the Sun is heliocentric. If the position is relative to the center of the Earth, then it's geocentric. A topocentric position is relative to an observer on the surface of the Earth. Within the aim of our accuracy of 1-2 arc minutes, the difference between geocentric and topocentric position is negligible for all celestial bodies except the Moon (and some occasional asteroid which happens to pass very close to the Earth).

The orbital elements consist of 6 quantities which completely define a circular, elliptic, parabolic or hyperbolic orbit. Three of these quantities describe the shape and size of the orbit, and the position of the planet in the orbit:

a  Mean distance, or semi-major axis
e  Eccentricity
T  Time at perihelion

A cirular orbit has zero eccentricity. An elliptical orbit has an eccentricity between zero and one. A parabolic orbit has an eccentricity of exactly one. Finally, a hyperbolic orbit has an eccentricity larger than one. A parabolic orbit has an infinite semi-major axis, a, therefore one instead gives the perihelion distance, q, for a parabolic orbit:

q  Perihelion distance  = a * (1 - e)

It is customary to give q instead of a also for hyperbolic orbit, and for elliptical orbits with eccentricity close to one.

The three remaining orbital elements define the orientation of the orbit in space:

i  Inclination, i.e. the "tilt" of the orbit relative to the
ecliptic.  The inclination varies from 0 to 180 degrees. If
the inclination is larger than 90 degrees, the planet is in
a retrogade orbit, i.e. it moves "backwards".  The most
well-known celestial body with retrogade motion is Comet Halley.

N  (usually written as "Capital Omega") Longitude of Ascending
Node. This is the angle, along the ecliptic, from the Vernal
Point to the Ascending Node, which is the intersection between
the orbit and the ecliptic, where the planet moves from south
of to north of the ecliptic, i.e. from negative to positive
latitudes.

w  (usually written as "small Omega") The angle from the Ascending
node to the Perihelion, along the orbit.

These are the primary orbital elements. From these many secondary orbital elements can be computed:

q  Perihelion distance  = a * (1 - e)

Q  Aphelion distance    = a * (1 + e)

P  Orbital period       = 365.256898326 * a**1.5/sqrt(1+m) days,
where m = the mass of the planet in solar masses (0 for
comets and asteroids). sqrt() is the square root function.

n  Daily motion         = 360_deg / P    degrees/day

t  Some epoch as a day count, e.g. Julian Day Number. The Time
at Perihelion, T, should then be expressed as the same day count.

t - T   Time since Perihelion, usually in days

M  Mean Anomaly         = n * (t - T)  =  (t - T) * 360_deg / P
Mean Anomaly is 0 at perihelion and 180 degrees at aphelion

L  Mean Longitude       = M + w + N

E  Eccentric anomaly, defined by Kepler's equation:   M = E - e * sin(E)
An auxiliary angle to compute the position in an elliptic orbit

v  True anomaly: the angle from perihelion to the planet, as seen
from the Sun

r  Heliocentric distance: the planet's distance from the Sun.

x,y,z  Rectangular coordinates. Used e.g. when a heliocentric
position (seen from the Sun) should be converted to a
corresponding geocentric position (seen from the Earth).

This relation is valid for an elliptic orbit:

r * cos(v) = a * (cos(E) - e)
r * sin(v) = a * sqrt(1 - e*e) * sin(E)


Two other popular programming language are C and C++. The standard library of these languages are better equipped with trigonometric functions. You'll find sin/cos/tan and their inverses, and even an atan2() function among the standard functions. All you need to do is to define some macros to get the trig functions in degrees (include all parentheses in these macro definitions), and to define a rev() function which reduces an angle to between 0 and 360 degrees, and a cbrt() function which computes the cube root.

#define PI          3.14159265358979323846
#define RADEG       (180.0/PI)
#define DEGRAD      (PI/180.0)
#define sind(x)     sin((x)*DEGRAD)
#define cosd(x)     cos((x)*DEGRAD)
#define tand(x)     tan((x)*DEGRAD)
#define asind(x)    (RADEG*asin(x))
#define acosd(x)    (RADEG*acos(x))
#define atand(x)    (RADEG*atan(x))
#define atan2d(y,x) (RADEG*atan2((y),(x)))


double rev( double x )
{
    return  x - floor(x/360.0)*360.0;
}


double cbrt( double x )
{
    if ( x > 0.0 )
        return exp( log(x) / 3.0 );
    else if ( x < 0.0 )
        return -cbrt(-x);
    else /* x == 0.0 */
        return 0.0;
}

In C++ the macros could preferably be defined as inline functions instead - this enables better type checking and also makes overloading of these function names possible.





2. Rectangular and spherical coordinates
The position of a planet can be given in one of several ways. Two different ways that we'll use are rectangular and spherical coordinates.

Suppose a planet is situated at some RA, Decl and r, where RA is the Right Ascension, Decl the declination, and r the distance in some length unit. If r is unknown or irrelevant, set r = 1. Let's convert this to rectangular coordinates, x,y,z:

x = r * cos(RA) * cos(Decl)
y = r * sin(RA) * cos(Decl)
z = r * sin(Decl)

(before we compute the sine/cosine of RA, we must first convert RA from hours/minutes/seconds to hours + decimals. Then the hours are converted to degrees by multiplying by 15)

If we know the rectangular coordinates, we can convert to spherical coordinates by the formulae below:

r    = sqrt( x*x + y*y + z*z )
RA   = atan2( y, x )
Decl = asin( z / r ) = atan2( z, sqrt( x*x + y*y ) )

At the north and south celestial poles, both x and y are zero. Since atan2(0,0) is undefined, the RA is undefined too at the celestial poles. The simplest way to handle this is to assign RA some arbitrary value, e.g. zero. Close to the celestial poles the formula asin(z/r) to compute the declination becomes sensitive to round-off errors - here the formula atan2(z,sqrt(x*x+y*y)) is preferable.

Not only equatorial coordinates can be converted between spherical and rectangular. These conversions can also be applied to ecliptic and horizontal coordinates. Just exchange RA,Decl with long,lat (ecliptic coordinates) or azimuth,altitude (horizontal coordinates).

A coordinate system can be rotated. If a rectangular coordinate system is rotated around, say, the X axis, one can easily compute the new x,y,z coordinates. As an example, let's consider rotating an ecliptic x,y,z system to an equatorial x,y,z system. This rotation is done around the X axis (which points to the Vernal Point, the common point of origin in ecliptic and equatorial coordinates), through an angle of oblecl (the obliquity of the ecliptic, which is approximately 23.4 degrees):

xequat = xeclip
yequat = yeclip * cos(oblecl) - zeclip * sin(oblecl)
zequat = yeclip * sin(oblecl) + zeclip * cos(oblecl)

Now the x,y,z system is equatorial. It's easily rotated back to ecliptic coordinates by simply switching sign on oblecl:

xeclip = xequat
yeclip = yequat * cos(-oblecl) - zequat * sin(-oblecl)
zeclip = yequat * sin(-oblecl) + zequat * cos(-oblecl)

When computing sin and cos of -oblecl, one can use the identities:

cos(-x) = cos(x), sin(-x) = -sin(x)

Now let's put this together to convert directly from spherical ecliptic coordinates (long, lat) to spherical equatorial coordinates (RA, Decl). Since the distance r is irrelevant in this case, let's set r=1 for simplicity.

Example: At the Summer Solstice the Sun's ecliptic longitude is 90 degrees. The Sun's ecliptic latitude is always very nearly zero. Suppose the obliquity of the ecliptic is 23.4 degrees:

xeclip = cos(90_deg) * cos(0_deg) = 0.0000
yeclip = sin(90_deg) * cos(0_deg) = 1.0000
zeclip = sin(0_deg)               = 0.0000

Rotate through oblecl = 23.4_deg:

xequat = 0.0000
yequat = 1.0000 * cos(23.4_deg) - 0.0000 * sin(23.4_deg)
zequat = 1.0000 * sin(23.4_deg) + 0.0000 * cos(23.4_deg)

Our equatorial rectangular coordinates become:

x = 0
y = cos(23.4_deg) = 0.9178
z = sin(23.4_deg) = 0.3971

The "distance", r, becomes: sqrt( 0.8423 + 0.1577 ) = 1.0000 i.e. unchanged

RA   = atan2( 0.9178, 0 ) = 90_deg
Decl = asin( 0.3971 / 1.0000 ) = 23.40_deg

Alternatively:

Decl = atan2( 0.3971, sqrt( 0.8423 + 0.0000 ) ) = 23.40_deg

Here we immediately see how simple it is to compute RA, thanks to the atan2() function: no need to consider in which quadrant it falls, the atan2() function handles this.


 3. The time scale
 The time scale in these formulae are counted in days. Hours, minutes, seconds are expressed as fractions of a day. Day 0.0 occurs at 2000 Jan 0.0 UT (or 1999 Dec 31, 0:00 UT). This "day number" d is computed as follows (y=year, m=month, D=date, UT=UT in hours+decimals):

 d = 367*y - 7 * ( y + (m+9)/12 ) / 4 + 275*m/9 + D - 730530

 Note that the formula above is only valid from March 1900 to February 2100.
 Below is another formula, which is valid over the entire Gregorian Calendar:

 d = 367*y - 7 * ( y + (m+9)/12 ) / 4 - 3 * ( ( y + (m-9)/7 ) / 100 + 1 ) / 4 + 275*m/9 + D - 730515

 Note that ALL divisions here should be INTEGER divisions. In Pascal, use "div" instead of "/", in MS-Basic, use "\" instead of "/". In Fortran, C and C++ "/" can be used if both y and m are integers. Finally, include the time of the day, by adding:

 d = d + UT/24.0        (this is a floating-point division)



 4. The orbital elements
 The primary orbital elements are here denoted as:

 N = longitude of the ascending node
 i = inclination to the ecliptic (plane of the Earth's orbit)
 w = argument of perihelion
 a = semi-major axis, or mean distance from Sun
 e = eccentricity (0=circle, 0-1=ellipse, 1=parabola)
 M = mean anomaly (0 at perihelion; increases uniformly with time)

 Related orbital elements are:

 w1 = N + w   = longitude of perihelion
 L  = M + w1  = mean longitude
 q  = a*(1-e) = perihelion distance
 Q  = a*(1+e) = aphelion distance
 P  = a ^ 1.5 = orbital period (years if a is in AU, astronomical units)
 T  = Epoch_of_M - (M(deg)/360_deg) / P  = time of perihelion
 v  = true anomaly (angle between position and perihelion)
 E  = eccentric anomaly


 To describe the position in the orbit, we use three angles: Mean Anomaly, True Anomaly, and Eccentric Anomaly. They are all zero when the planet is in perihelion:
 Mean Anomaly (M): This angle increases uniformly over time, by 360 degrees per orbital period. It's zero at perihelion. It's easily computed from the orbital period and the time since last perihelion.
 True Anomaly (v): This is the actual angle between the planet and the perihelion, as seen from the central body (in this case the Sun). It increases non-uniformly with time, changing most rapidly at perihelion.
 Eccentric Anomaly (E): This is an auxiliary angle used in Kepler's Equation, when computing the True Anomaly from the Mean Anomaly and the orbital eccentricity.
 Note that for a circular orbit (eccentricity=0), these three angles are all equal to each other.

 Another quantity we will need is ecl, the obliquity of the ecliptic, i.e. the "tilt" of the Earth's axis of rotation (currently 23.4 degrees and slowly decreasing). First, compute the "d" of the moment of interest (section 3). Then, compute the obliquity of the ecliptic:

 ecl = 23.4393 - 3.563E-7 * d


 When computing M (and, for the Moon, when computing N and w as well), one will quite often get a result that is larger than 360 degrees,
or negative (all angles are here computed in degrees).
If negative, add 360 degrees until positive.
If larger than 360 degrees, subtract 360 degrees until the value is less than 360 degrees.
Note that, in most programming languages, one must then multiply these angles with pi/180 to convert them to radians, before taking the sine or cosine of them.


 5. The position of the Sun
 First, compute the eccentric anomaly E from the mean anomaly M and from the eccentricity e (E and M in degrees):

 E = M + e*(180/pi) * sin(M) * ( 1.0 + e * cos(M) )

 or (if E and M are expressed in radians):

 E = M + e * sin(M) * ( 1.0 + e * cos(M) )

 Note that the formulae for computing E are not exact; however they're accurate enough here.

 Then compute the Sun's distance r and its true anomaly v from:

 xv = r * cos(v) = cos(E) - e
 yv = r * sin(v) = sqrt(1.0 - e*e) * sin(E)

 v = atan2( yv, xv )
 r = sqrt( xv*xv + yv*yv )

 (note that the r computed here is later used as rs)

 atan2() is a function that converts an x,y coordinate pair to the correct angle in all four quadrants. It is available as a library function in Fortran, C and C++. In other languages, one has to write one's own atan2() function. It's not that difficult:

 atan2( y, x ) = atan(y/x)                 if x positive
 atan2( y, x ) = atan(y/x) +- 180 degrees  if x negative
 atan2( y, x ) = sign(y) * 90 degrees      if x zero

 Now, compute the Sun's true longitude:

 lonsun = v + w

 Convert lonsun,r to ecliptic rectangular geocentric coordinates xs,ys:

 xs = r * cos(lonsun)
 ys = r * sin(lonsun)

 (since the Sun always is in the ecliptic plane, zs is of course zero). xs,ys is the Sun's position in a coordinate system in the plane of the ecliptic. To convert this to equatorial, rectangular, geocentric coordinates, compute:

 xe = xs
 ye = ys * cos(ecl)
 ze = ys * sin(ecl)

 Finally, compute the Sun's Right Ascension (RA) and Declination (Dec):

 RA  = atan2( ye, xe )
 Dec = atan2( ze, sqrt(xe*xe+ye*ye) )


 5b. The Sidereal Time
 Quite often we need a quantity called Sidereal Time. The Local Sideral Time (LST) is simply the RA of your local meridian. The Greenwich Mean Sideral Time (GMST) is the LST at Greenwich. And, finally, the Greenwich Mean Sidereal Time at 0h UT (GMST0) is, as the name says, the GMST at Greenwich Midnight. However, we will here extend the concept of GMST0 a bit, by letting "our" GMST0 be the same as the conventional GMST0 at UT midnight but also let GMST0 be defined at any other time such that GMST0 will increase by 3m51s every 24 hours. Then this formula will be valid at any time:

 GMST = GMST0 + UT

 We also need the Sun's mean longitude, Ls, which can be computed from the Sun's v and w as follows:

 Ls = v + w

 The GMST0 is easily computed from Ls (divide by 15 if you want GMST0 in hours rather than degrees), GMST is then computed by adding the UT, and finally the LST is computed by adding your local longitude (east longitude is positive, west negative).

 Note that "time" is given in hours while "angle" is given in degrees. The two are related to one another due to the Earth's rotation: one hour is here the same as 15 degrees. Before adding or subtracting a "time" and an "angle", be sure to convert them to the same unit, e.g. degrees by multiplying the hours by 15 before adding/subtracting:

 GMST0 = Ls + 180_degrees
 GMST = GMST0 + UT
 LST  = GMST + local_longitude

 The formulae above are written as if times are expressed in degrees. If we instead assume times are given in hours and angles in degrees, and if we explicitly write out the conversion factor of 15, we get:

 GMST0 = (Ls + 180_degrees)/15 = Ls/15 + 12_hours
 GMST = GMST0 + UT
 LST  = GMST + local_longitude/15

 6. The position of the Moon and of the planets
 First, compute the eccentric anomaly, E, from M, the mean anomaly, and e, the eccentricity. As a first approximation, do (E and M in degrees):

 E = M + e*(180/pi) * sin(M) * ( 1.0 + e * cos(M) )

 or, if E and M are in radians:

 E = M + e * sin(M) * ( 1.0 + e * cos(M) )

 If e, the eccentricity, is less than about 0.05-0.06, this approximation is sufficiently accurate. If the eccentricity is larger, set E0=E and then use this iteration formula (E and M in degrees):

 E1 = E0 - ( E0 - e*(180/pi) * sin(E0) - M ) / ( 1 - e * cos(E0) )

 or (E and M in radians):

 E1 = E0 - ( E0 - e * sin(E0) - M ) / ( 1 - e * cos(E0) )

 For each new iteration, replace E0 with E1. Iterate until E0 and E1 are sufficiently close together (about 0.001 degrees). For comet orbits with eccentricites close to one, a difference of less than 1E-4 or 1E-5 degrees should be required.

 If this iteration formula won't converge, the eccentricity is probably too close to one. Then you should instead use the formulae for near-parabolic or parabolic orbits.

 Now compute the planet's distance and true anomaly:

 xv = r * cos(v) = a * ( cos(E) - e )
 yv = r * sin(v) = a * ( sqrt(1.0 - e*e) * sin(E) )

 v = atan2( yv, xv )
 r = sqrt( xv*xv + yv*yv )

 7. The position in space
 Compute the planet's position in 3-dimensional space:

 xh = r * ( cos(N) * cos(v+w) - sin(N) * sin(v+w) * cos(i) )
 yh = r * ( sin(N) * cos(v+w) + cos(N) * sin(v+w) * cos(i) )
 zh = r * ( sin(v+w) * sin(i) )

 For the Moon, this is the geocentric (Earth-centered) position in the ecliptic coordinate system. For the planets, this is the heliocentric (Sun-centered) position, also in the ecliptic coordinate system. If one wishes, one can compute the ecliptic longitude and latitude (this must be done if one wishes to correct for perturbations, or if one wants to precess the position to a standard epoch):

 lonecl = atan2( yh, xh )
 latecl = atan2( zh, sqrt(xh*xh+yh*yh) )

 As a check one can compute sqrt(xh*xh+yh*yh+zh*zh), which of course should equal r (except for small round-off errors).


 8. Precession
 If one wishes to compute the planet's position for some standard epoch, such as 1950.0 or 2000.0 (e.g. to be able to plot the position on a star atlas), one must add the correction below to lonecl. If a planet's and not the Moon's position is computed, one must also add the same correction to lonsun, the Sun's longitude. The desired Epoch is expressed as the year, possibly with a fraction.

 lon_corr = 3.82394E-5 * ( 365.2422 * ( Epoch - 2000.0 ) - d )

 If one wishes the position for today's epoch (useful when computing rising/setting times and the like), no corrections need to be done.


 9. Perturbations of the Moon
 If the position of the Moon is computed, and one wishes a better accuracy than about 2 degrees, the most important perturbations has to be taken into account. If one wishes 2 arc minute accuracy, all the following terms should be accounted for. If less accuracy is needed, some of the smaller terms can be omitted.

 First compute:

 Ms, Mm             Mean Anomaly of the Sun and the Moon
 Nm                 Longitude of the Moon's node
 ws, wm             Argument of perihelion for the Sun and the Moon
 Ls = Ms + ws       Mean Longitude of the Sun  (Ns=0)
 Lm = Mm + wm + Nm  Mean longitude of the Moon
 D = Lm - Ls        Mean elongation of the Moon
 F = Lm - Nm        Argument of latitude for the Moon

 Add these terms to the Moon's longitude (degrees):

 -1.274 * sin(Mm - 2*D)          (the Evection)
 +0.658 * sin(2*D)               (the Variation)
 -0.186 * sin(Ms)                (the Yearly Equation)
 -0.059 * sin(2*Mm - 2*D)
 -0.057 * sin(Mm - 2*D + Ms)
 +0.053 * sin(Mm + 2*D)
 +0.046 * sin(2*D - Ms)
 +0.041 * sin(Mm - Ms)
 -0.035 * sin(D)                 (the Parallactic Equation)
 -0.031 * sin(Mm + Ms)
 -0.015 * sin(2*F - 2*D)
 +0.011 * sin(Mm - 4*D)

 Add these terms to the Moon's latitude (degrees):

 -0.173 * sin(F - 2*D)
 -0.055 * sin(Mm - F - 2*D)
 -0.046 * sin(Mm + F - 2*D)
 +0.033 * sin(F + 2*D)
 +0.017 * sin(2*Mm + F)

 Add these terms to the Moon's distance (Earth radii):

 -0.58 * cos(Mm - 2*D)
 -0.46 * cos(2*D)

 All perturbation terms that are smaller than 0.01 degrees in longitude or latitude and smaller than 0.1 Earth radii in distance have been omitted here. A few of the largest perturbation terms even have their own names! The Evection (the largest perturbation) was discovered already by Ptolemy a few thousand years ago (the Evection was one of Ptolemy's epicycles). The Variation and the Yearly Equation were both discovered by Tycho Brahe in the 16'th century.

 The computations can be simplified by omitting the smaller perturbation terms. The error introduced by this seldom exceeds the sum of the amplitudes of the 4-5 largest omitted terms. If one only computes the three largest perturbation terms in longitude and the largest term in latitude, the error in longitude will rarley exceed 0.25 degrees, and in latitude 0.15 degrees.


 11. Geocentric (Earth-centered) coordinates
 Now we have computed the heliocentric (Sun-centered) coordinate of the planet, and we have included the most important perturbations. We want to compute the geocentric (Earth-centerd) position. We should convert the perturbed lonecl, latecl, r to (perturbed) xh, yh, zh:

 xh = r * cos(lonecl) * cos(latecl)
 yh = r * sin(lonecl) * cos(latecl)
 zh = r               * sin(latecl)

 If we are computing the Moon's position, this is already the geocentric position, and thus we simply set xg=xh, yg=yh, zg=zh. Otherwise we must also compute the Sun's position: convert lonsun, rs (where rs is the r computed here) to xs, ys:

 xs = rs * cos(lonsun)
 ys = rs * sin(lonsun)

 (Of course, any correction for precession should be added to lonecl and lonsun before converting to xh,yh,zh and xs,ys).

 Now convert from heliocentric to geocentric position:

 xg = xh + xs
 yg = yh + ys
 zg = zh

 We now have the planet's geocentric (Earth centered) position in rectangular, ecliptic coordinates.


 12. Equatorial coordinates
 Let's convert our rectangular, ecliptic coordinates to rectangular, equatorial coordinates: simply rotate the y-z-plane by ecl, the angle of the obliquity of the ecliptic:

 xe = xg
 ye = yg * cos(ecl) - zg * sin(ecl)
 ze = yg * sin(ecl) + zg * cos(ecl)

 Finally, compute the planet's Right Ascension (RA) and Declination (Dec):

 RA  = atan2( ye, xe )
 Dec = atan2( ze, sqrt(xe*xe+ye*ye) )

 Compute the geocentric distance:

 rg = sqrt(xg*xg+yg*yg+zg*zg) = sqrt(xe*xe+ye*ye+ze*ze)

 Thie completes our computation of the equatorial coordinates.


 12b. Azimuthal coordinates
 To find the azimuthal coordinates (azimuth and altitude) we proceed by computing the HA (Hour Angle) of the object. But first we must compute the LST (Local Sidereal Time), which we do as described in 5b above. When we know LST, we can easily compute HA from:

 HA = LST - RA

 HA is usually given in the interval -12 to +12 hours, or -180 to +180 degrees. If HA is zero, the object can be seen directly to the south. If HA is negative, the object is to the east of south, and if HA is positive, the object is to the west of south. IF your computed HA should fall outside this interval, add or subtract 24 hours (or 360 degrees) until HA falls within this interval.

 Now it's time to convert our objects HA and Decl to local azimuth and altitude. To do that, we also must know lat, our local latitude. Then we proceed as follows:

 x = cos(HA) * cos(Decl)
 y = sin(HA) * cos(Decl)
 z = sin(Decl)

 xhor = x * sin(lat) - z * cos(lat)
 yhor = y
 zhor = x * cos(lat) + z * sin(lat)

 az  = atan2( yhor, xhor ) + 180_degrees
 alt = asin( zhor ) = atan2( zhor, sqrt(xhor*xhor+yhor*yhor) )

 This completes our calculation of the local azimuth and altitude. Note that azimuth is 0 at North, 90 deg at East, 180 deg at South and 270 deg at West. Altitude is of course 0 at the (mathematical) horizon, 90 deg at zenith, and negative below the horizon.

