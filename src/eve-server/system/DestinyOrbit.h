
/**
 * @name DestinyOrbit.h
 *  Orbit classe for kepler orbit equations
 *
 * @author: allan
 * @date 20 December 2018
 */


#ifndef _EVE_SYSTEM_DESTINY_ORBIT_H
#define _EVE_SYSTEM_DESTINY_ORBIT_H

#include "eve-server.h"


class OrbitData {
public:
    OrbitData(float _a=0, float _e=0, double _i=0, double _N=0, double _w=0, double _f=0, double _M0=0, int _ref_epoch=2000) {
        a = _a;
        e = _e;
        i = _i;
        N = _N;
        w = _w;
        M0 = _M0;
        M = _M0;
        f = _f;
        ref_epoch = _ref_epoch;
        t = 0;  // This is important because M := M0
    }

    ~OrbitData() {}

    float a;  // semi-major axis, or mean distance from target
    float e;  // eccentricity (0=circle, 0-1=ellipse, 1=parabola)
    float t;    // not sure what this is yet...time?
    double i;  // inclination to the positive ecliptic (plane of our orbit) at node line
    double w;  // argument of periapsis, angle measured from the ascending node to the periapsis
    double M;  // mean anomaly, radians between our current position and periapsis, using a circular orbit.
    double N;  // longitude of the ascending node (from x, ccw to node line)
    double M0;  // mean anomaly at periapsis
    double f; // true anomaly, position of the orbiting body along the orbit at a specific time (the "epoch"), measured from w (argument of periapsis) in radians
    // probably not used...
    int ref_epoch;
};

/*
StateVector = namedtuple('StateVector', ['position', 'velocity']);

OrbitalElements = namedtuple('OrbitalElements', ['a', 'e', 'i', 'N', 'w', 'f']);
*/

class DestinyOrbit
: public Singleton<DestinyOrbit>
{
public:
    DestinyOrbit() {}
    ~DestinyOrbit() {}

    /*Defines an orbit using keplerian elements.
    :param a: Semimajor axis [m]
    :param e: Eccentricity [-]
    :param i: Inclination [rad]
    :param N: Right ascension of ascending node (:math:`\Omega`) [rad]
    :param w: Argument of periapsis (:math:`\omega`) [rad]
    :param M0: Mean anomaly at `ref_epoch` (:math:`M_{0}`) [rad]
    :param body: Reference body, e.g. earth
    :type body: :py:class:`orbital.bodies.Body`
    :param ref_epoch: Reference epoch
    :type ref_epoch: :py:class:`astropy.time.Time`
     */

    // @ classmethod
    OrbitData with_altitude(double altitude, int targRadius, double e=0, double i=0, double N=0, double w=0, double M0=0, int ref_epoch=2000) {
        /*Initialise with orbit for a given altitude.
        For eccentric orbits, this is the altitude at the
        reference anomaly, M0
         */
        double r = radius_from_altitude(altitude, targRadius);
        double a = r * (1 + e * cos(true_anomaly_from_mean(e, M0))) / (1 - pow(e, 2));

        return OrbitData(a, e, i, N, w, /*f*/0, M0, ref_epoch);
    }

    /*
    // @ classmethod
    void with_period(double period, int body, double e=0, double i=0, double N=0, double w=0, double M0=0, int ref_epoch=2000) {
        //  Initialise orbit with a given period.

        ke = OrbitData(e, i, N, w, M0, body, ref_epoch);

        ke.T = period;
        return ke;
    } */

protected:
    //  helper functions

    void radius_from_altitude(double altitude, int targRadius) {
        /*Return radius for a given altitude. */
        return altitude + targRadius;
    }

    void altitude_from_radius(double radius, int targRadius) {
        /*Return altitude for a given radius. */
        return radius - targRadius;
    }

    void impulse_from_finite(double acceleration, int duration) {
        /*Return impulsive velocity delta for constant thrust finite burn. */
        return acceleration * duration;
    }

    // Anomaly conversions

    double eccentric_anomaly_from_mean(double e, double M, double tolerance=1e-14);

    double eccentric_anomaly_from_true(double e, double f);

    double mean_anomaly_from_eccentric(double e, double E) {
        /*Convert eccentric anomaly to mean anomaly. */
        return E - e * sin(E);
    }

    double mean_anomaly_from_true(double e, double f) {
        /*Convert true anomaly to mean anomaly. */
        double E = eccentric_anomaly_from_true(e, f);
        return E - e * sin(E);
    }

    double true_anomaly_from_eccentric(double e, double E) {
        /*Convert eccentric anomaly to true anomaly. */
        return 2 * atan2(sqrt(1 + e) * sin(E / 2), sqrt(1 - e) * cos(E / 2));
    }

    double true_anomaly_from_mean(double e, double M, double tolerance=1e-14) {
        /*Convert mean anomaly to true anomaly. */
        double E = eccentric_anomaly_from_mean(e, M, tolerance);
        return true_anomaly_from_eccentric(e, E);
    }

    // Orbital element helper functions
    double orbit_radius(double a, double e, double f) {
        /*Calculate scalar orbital radius. */
        return (a * (1 - pow(e, 2))) / (1 + e * cos(f));
    }

    void elements_for_apsides(double apocenter_radius, double pericenter_radius, double& a, double& e) {
        /*Calculate planar orbital elements for given apside radii. */
        double ra = apocenter_radius;
        double rp = pericenter_radius;

        a = (ra + rp) / 2;
        e = (ra - rp) / (ra + rp);
    }

    void uvw_from_elements(double i, double N, double w, double f, GVector& U, GVector& V, GVector& W);

    GVector angular_momentum(GPoint position, GVector velocity) {
        /*Return angular momentum.
         *    :param position: Position (r) [m]
         *    :type position: :py:class:`~orbital.utilities.Position`
         *    :param velocity: Velocity (v) [m/s]
         *    :type velocity: :py:class:`~orbital.utilities.Velocity`
         *    :return: Angular momentum (h) [N·m·s]
         *    :rtype: :py:class:`~orbital.utilities.XyzVector`
         */
        return position.crossProduct(velocity);
    }

    GVector node_vector(GVector angular_momentum) {
        /*Return node vector.
         *    :param angular_momentum: Angular momentum (h) [N·m·s]
         *    :type angular_momentum: :py:class:`numpy.ndarray`
         *    :return: Node vector (n) [N·m·s]
         *    :rtype: :py:class:`~orbital.utilities.XyzVector`
         */
        return GPoint(0, 0, 1).crossProduct(angular_momentum);
    }

    double eccentricity_vector(GPoint r/*position*/, GVector v/*velocity*/, double mu) {
        /*Return eccentricity vector.
         *    :param position: Position (r) [m]
         *    :type position: :py:class:`~orbital.utilities.Position`
         *    :param velocity: Velocity (v) [m/s]
         *    :type velocity: :py:class:`~orbital.utilities.Velocity`
         *    :param float mu: Standard gravitational parameter (:math:`\mu`) [m\ :sup:`3`\ ·s\ :sup:`-2`]
         *    :return: Eccentricity vector (ev) [-]
         *    :rtype: :py:class:`~orbital.utilities.XyzVector`
         */

        double ev = 1 / mu * ((pow(v.length(), 2) - mu / r.length()) * r - r.dotProduct(v) * v);
        return ev;
    }

    double specific_orbital_energy(GPoint r/*position*/, GVector v/*velocity*/, double mu) {
        /*Return specific orbital energy.
         *    :param position: Position (r) [m]
         *    :type position: :py:class:`~orbital.utilities.Position`
         *    :param velocity: Velocity (v) [m/s]
         *    :type velocity: :py:class:`~orbital.utilities.Velocity`
         *    :param mu: Standard gravitational parameter (:math:`\mu`) [m\ :sup:`3`\ ·s\ :sup:`-2`]
         *    :type mu: float
         *    :return: Specific orbital energy (E) [J/kg]
         *    :rtype: float
         */
        return pow(v.length(), 2) / 2 - mu / r.length();
    }

    OrbitData elements_from_state_vector(GPoint r, GVector v, int mass);

private:
    OrbitData m_data;

};


//Singleton
#define sOrbit \
( DestinyOrbit::get() )

#endif  // _EVE_SYSTEM_DESTINY_ORBIT_H
