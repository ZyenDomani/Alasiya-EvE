
/**
 * @name DestinyOrbit.cpp
 *  Orbit classe for kepler orbit equations
 *
 * @author: allan
 * @date 20 December 2018
 */

#include "DestinyOrbit.h"


double DestinyOrbit::eccentric_anomaly_from_mean(double e, double M, double tolerance)
{
    /*Convert mean anomaly to eccentric anomaly.
     *    Implemented from [A Practical Method for Solving the Kepler Equation][1]
     *    by Marc A. Murison from the U.S. Naval Observatory
     *    [1]: http://murison.alpheratz.net/dynamics/twobody/KeplerIterations_summary.pdf
     */
    double E = 0;
    double Mnorm = fmod(M, 2 * EvE::Trig::Pi);
    double E0 = M + (-1 / 2 * pow(e, 3) + e + (pow(e, 2) + 3 / 2 * cos(M) * pow(e, 3)) * cos(M)) * sin(M);
    double dE = tolerance + 1;
    int count = 100;
    while (dE > tolerance or count > 0) {
        double t1 = cos(E0);
        double t2 = -1 + e * t1;
        double t3 = sin(E0);
        double t4 = e * t3;
        double t5 = -E0 + t4 + Mnorm;
        double t6 = t5 / (1 / 2 * t5 * t4 / t2 + t2);
        E = E0 - t5 / ((1 / 2 * t3 - 1 / 6 * t1 * t6) * e * t6 + t2);
        dE = abs(E - E0);
        E0 = E;
        --count;
    }
    return E;
}

double DestinyOrbit::eccentric_anomaly_from_true(double e, double f)
{
    /*Convert true anomaly to eccentric anomaly. */
    double E = atan2(sqrt(1 - pow(e, 2)) * sin(f), e + cos(f));
    E = mod(E, 2 * EvE::Trig::Pi);
    return E;
}

void DestinyOrbit::uvw_from_elements(double i, double N, double w, double f, GVector& U, GVector& V, GVector& W)
{
    /*Return U, V, W unit vectors.
     *    :param float i: Inclination (:math:`i`) [rad]
     *    :param float N:  Right ascension of ascending node (:math:`\Omega`) [rad]
     *    :param float w: Argument of periapsis (:math:`\omega`) [rad]
     *    :param float f: True anomaly (:math:`f`) [rad]
     *    :return: Radial direction unit vector (:math:`U`)
     *    :return: Transversal (in-flight) direction unit vector (:math:`V`)
     *    :return: Out-of-plane direction unit vector (:math:`W`)
     *    :rtype: :py:class:`numpy.ndarray`
     */
    double u = w + f;

    double sin_u = sin(u);
    double cos_u = cos(u);
    double sin_N = sin(N);
    double cos_N = cos(N);
    double sin_i = sin(i);
    double cos_i = cos(i);

    U = GVector(
        cos_u * cos_N - sin_u * sin_N * cos_i,
        cos_u * sin_N + sin_u * cos_N * cos_i,
        sin_u * sin_i
    );

    V = GVector(
        -sin_u * cos_N - cos_u * sin_N * cos_i,
        -sin_u * sin_N + cos_u * cos_N * cos_i,
        cos_u * sin_i
    );

    W = GVector(
        sin_N * sin_i,
        -cos_N * sin_i,
        cos_i
    );
}

OrbitData DestinyOrbit::elements_from_state_vector(GPoint r, GVector v, int mass)
{
    GVector h = angular_momentum(r, v);
    GVector n = node_vector(h);

    double mu = Gc * mass;

    GVector ev = eccentricity_vector(r, v, mu);

    double E = specific_orbital_energy(r, v, mu);

    double a = -mu / (2 * E);
    double e = ev.length();
    double N = 0;
    double w = 0;
    double f = 0;

    float SMALL_NUMBER = 1e-15f;

    // Inclination is the angle between the angular
    // momentum vector and its z component.
    double i = acos(h.z / h.length());

    if (abs(i - 0) < SMALL_NUMBER) {
        // For non-inclined orbits, N is undefined;
        if (abs(e - 0) < SMALL_NUMBER) {
            // For circular orbits, place periapsis
            // at ascending node by convention
        } else {
            // Argument of periapsis is the angle between
            // eccentricity vector and its x component.
            double w = acos(ev.x / ev.length());
        }
    } else {
        // Right ascension of ascending node is the angle
        // between the node vector and its x component.
        N = acos(n.x / n.length());
        if (n.y < 0)
            N = 2 * EvE::Trig::Pi - N;

        // Argument of periapsis is angle between
        // node and eccentricity vectors.
        w = acos(n.dotProduct(ev) / (n.length() * ev.length()));
    }
    if (abs(e - 0) < SMALL_NUMBER) {
        if (abs(i - 0) < SMALL_NUMBER) {
            // True anomaly is angle between position
            // vector and its x component.
            f = acos(r.x / r.length());
            if (v.x > 0)
                f = 2 * EvE::Trig::Pi - f;
        } else {
            // True anomaly is angle between node
            // vector and position vector.
            f = acos(n.dotProduct(r) / (n.length() * r.length()));
            if (n.dotProduct(v) > 0)
                f = 2 * EvE::Trig::Pi - f;
        }
    } else {
        if (ev.z < 0)
            w = 2 * EvE::Trig::Pi - w;

        // True anomaly is angle between eccentricity
        // vector and position vector.
        f = acos(ev.dotProduct(r) / (ev.length() * r.length()));

        if (r.dotProduct(v) < 0)
            f = 2 * EvE::Trig::Pi - f;
    }
    return OrbitData(a, e, i, N, w, f);
}



#if (0) {
// @ classmethod
void with_apside_altitudes(alt1, alt2, i=0, N=0, w=0, M0=0, body=0, ref_epoch=2000) {
    /*Initialise orbit with given apside altitudes. */

    altitudes = [alt1, alt2];
    altitudes.sort();

    pericenter_altitude = altitudes[0];
    apocenter_altitude = altitudes[1];

    apocenter_radius = radius_from_altitude(apocenter_altitude, body);
    pericenter_radius = radius_from_altitude(pericenter_altitude, body);

    a, e = elements_for_apsides(apocenter_radius, pericenter_radius);

    return OrbitData(a, e, i, N, w, M0, body, ref_epoch);
}
// @ classmethod
void with_apside_radii(radius1, radius2, i=0, N=0, w=0, M0=0, body=None, ref_epoch=J2000) {
    /*Initialise orbit with given apside radii. */

    radii = [radius1, radius2];
    radii.sort();

    pericenter_radius = radii[0];
    apocenter_radius = radii[1];

    a, e = elements_for_apsides(apocenter_radius, pericenter_radius);

    return OrbitData(a, e, i, N, w, M0, body, ref_epoch);
}
// @ classmethod
void from_state_vector(r, v, body, ref_epoch=J2000) {
    /*Create orbit from given state vector. */
    elements = elements_from_state_vector(r, v, body.mu);

    self = OrbitData(
        a=elements.a,
        elements.e,
        i=elements.i,
        N=elements.N,
        w=elements.w,
        M0=mean_anomaly_from_true(elements.e, elements.f),
                     body=body,
                     ref_epoch);

    // Fix mean anomaly at epoch for new orbit and position.
    oldM0 = self.M0;
    self.M0 = ou.mod(self.M - self.n * self.t, 2 * EvE::Trig::Pi);
    assert self.M0 == oldM0;

    // Now check that the computed properties for position and velocity are
    // reasonably close to the inputs.
    // 1e-4 is a large uncertainty, but we don't want to throw an error
    // within small differences (e.g. 1e-4 m is 0.1 mm)
    if (abs(self.v - v) > 1e-4).any() or (abs(self.r - r) > 1e-4).any()
        raise RuntimeError(
            'Failed to set orbital elements for velocity. Please file a bug'
        ' report at https://github.com/RazerM/orbital/issues');

    return self;
}
// @ classmethod
void from_tle(line1, line2, body) {
    /*Create object by parsing TLE using SGP4. */

    // Get state vector at TLE epoch
    sat = sgp4.io.twoline2rv(line1, line2, wgs72);
    r, v = sgp4.propagation.sgp4(sat, 0);
    ref_epoch = time.Time(sat.epoch, scale='utc');

    // Convert km to m
    r, v = np.array(r) * kilo, np.array(v) * kilo;

    return cls.from_state_vector(r, v, body=body, ref_epoch);
}
// @ property
void epoch(self) {
    /*Current epoch calculated from time since ref_epoch. */
    return self.ref_epoch + time.TimeDelta(self.t, format='sec');
}
// @ epoch.setter
void epoch(value) {
    /*Set epoch, adjusting current mean anomaly (from which
     *        other anomalies are calculated).
     */
    t = (value - self.ref_epoch).sec;
    self._M = self.M0 + self.n * t;
    self._M = ou.mod(self._M, 2 * EvE::Trig::Pi);
    self._t = t;
}
// @ property
void t(self) {
    /*Time since ref_epoch. */
    return self._t;
}
// @ t.setter
void t(value) {
    /*Set time since ref_epoch, adjusting current mean anomaly (from which
     *        other anomalies are calculated).
     */
    self._M = self.M0 + self.n * value;
    self._M = ou.mod(self._M, 2 * EvE::Trig::Pi);
    self._t = value;
}
// @ property
void M(self) {
    /*Mean anomaly [rad]. */
    return self._M;
}
// @ M.setter
void M(value) {
    warnings.warn('Setting anomaly does not set time, use KeplerianElements'
    '.propagate_anomaly_to() instead.', OrbitalWarning);
    self._M = ou.mod(value, 2 * EvE::Trig::Pi);
}
// @ property
void E(self) {
    /*Eccentric anomaly [rad]. */
    return eccentric_anomaly_from_mean(self.e, self._M);
}
// @ E.setter
void E(value) {
    warnings.warn('Setting anomaly does not set time, use KeplerianElements'
    '.propagate_anomaly_to() instead.', OrbitalWarning);
    self._M = mean_anomaly_from_eccentric(self.e, value);
}
// @ property
void f(self) {
    /*True anomaly [rad]. */
    return true_anomaly_from_mean(self.e, self._M);
}
// @ f.setter
void f(value) {
    warnings.warn('Setting anomaly does not set time, use KeplerianElements'
    '.propagate_anomaly_to() instead.', OrbitalWarning);
    self._M = mean_anomaly_from_true(self.e, value)
}
// @ property
void a(self) {
    return self._a;
}
// @ a.setter
void a(value) {
    /*Set semimajor axis and fix M0.
     *        To fix self.M0, self.n is called. self.n is a function of self.a
     *        This is safe, because the new value for self._a is set first, then
     *        self.M0 is fixed.
     */
    self._a = value;
    self.M0 = ou.mod(self.M - self.n * self.t, 2 * EvE::Trig::Pi);
}
// @ property
void r(self) {
    /*Position vector (:py:class:`orbital.utilities.Position`) [m]. */
    pos = orbit_radius(self.a, self.e, self.f) * self.U;
    return Position(x=pos[0], y=pos[1], z=pos[2]);
}
// @ property
void v(self) {
    /*Velocity vector (:py:class:`orbital.utilities.Velocity`) [m/s]. */
    r_dot = sqrt(self.body.mu / self.a) * (self.e * sin(self.f)) / sqrt(1 - pow(self.e, 2));
    rf_dot = sqrt(self.body.mu / self.a) * (1 + self.e * cos(self.f)) / sqrt(1 - pow(self.e, 2));
    vel = r_dot * self.U + rf_dot * self.V;
    return Velocity(x=vel[0], y=vel[1], z=vel[2]);
}
// @ v.setter
void v(value) {
    /*Set velocity by altering orbital elements.
     *        This method uses 3 position variables, and 3 velocity
     *        variables to set the 6 orbital elements.
     */
    r, v = self.r, value;
    elements = elements_from_state_vector(r, v, self.body.mu);
    self._a = elements.a;
    self.e = elements.e;
    self.i = elements.i;
    self.N = elements.N;
    self.w = elements.w;
    with warnings.catch_warnings();
    warnings.simplefilter('ignore', category=OrbitalWarning);
    self.f = elements.f;

    // Fix mean anomaly at epoch for new orbit and position.
    self.M0 = ou.mod(self.M - self.n * self.t, 2 * EvE::Trig::Pi);

    // Now check that the computed properties for position and velocity are
    // reasonably close to the inputs.
    // 1e-4 is a large uncertainty, but we don't want to throw an error
    // within small differences (e.g. 1e-4 m is 0.1 mm)
    if (abs(self.v - v) > 1e-4).any() or (abs(self.r - r) > 1e-4).any()
        raise RuntimeError(
            'Failed to set orbital elements for velocity. Please file a bug'
        ' report at https://github.com/RazerM/orbital/issues');
}
// @ property
void n(self) {
    /*Mean motion [rad/s]. */
    return sqrt(self.body.mu / pow(self.a, 3));
}
// @ n.setter
void n(value) {
    /*Set mean motion by adjusting semimajor axis. */
    self.a = pow((self.body.mu / pow(value, 2)), (1 / 3));
}
// @ property
void T(self) {
    /*Period [s]. */
    return 2 * EvE::Trig::Pi / self.n;
}
// @ T.setter
void T(value) {
    /*Set period by adjusting semimajor axis. */
    self.a = pow((self.body.mu * pow(value, 2) / (4 * pow(EvE::Trig::Pi, 2))), (1 / 3));
}
// @ property
void fpa(self) {
    return arctan(self.e * sin(self.f) / (1 + self.e * cos(self.f)));
}
void propagate_anomaly_to(**kwargs) {
    /*Propagate to time in future where anomaly is equal to value passed in.
     *        :param M: Mean anomaly [rad]
     *        :param E: Eccentricity anomaly [rad]
     *        :param f: True anomaly [rad]
     *        This will propagate to a maximum of 1 orbit ahead.
     *        .. note::
     *           Only one parameter should be passed in.
     */
    operation = PropagateAnomalyTo(**kwargs);
    self.apply_maneuver(operation);
}
void propagate_anomaly_by(**kwargs) {
    /*Propagate to time in future by an amount equal to the anomaly passed in.
     *        :param M: Mean anomaly [rad]
     *        :param E: Eccentricity anomaly [rad]
     *        :param f: True anomaly [rad]
     *        .. note::
     *           Only one parameter should be passed in.
     */
    operation = PropagateAnomalyBy(**kwargs);
    self.apply_maneuver(operation);
}
void __getattr__(attr) {
    /*Dynamically respond to correct apsis names for given body. */
    if not attr.startswith('__')
        for apoapsis_name in self.body.apoapsis_names:
            if attr == '{}_radius'.format(apoapsis_name)
                return self.apocenter_radius
                for periapsis_name in self.body.periapsis_names:
                    if attr == '{}_radius'.format(periapsis_name)
                        return self.pericenter_radius
                        raise AttributeError(
                            "'{name}' object has no attribute '{attr}'"
                            .format(name=type(self).__name__, attr=attr))
}
void apply_maneuver(maneuver, iter=False, copy=False) {
    /* Apply maneuver to orbit.
     *        :param maneuver: Maneuver
     *        :type maneuver: :py:class:`maneuver.Maneuver`
     *        :param bool iter: Return an iterator.
     *        :param bool copy: Each orbit yielded by the generator will be a copy.
     *        If :code:`iter=True`, the returned iterator is of each intermediate orbit
     *        and the next operation, as shown in this table:
     *        +-------------------------------------+------------------+
     *        |                Orbit                |    Operation     |
     *        +=====================================+==================+
     *        | Original orbit                      | First operation  |
     *        +-------------------------------------+------------------+
     *        | Orbit after first operation applied | Second operation |
     *        +-------------------------------------+------------------+
     *        The final orbit is not returned, as it is accessible after the method has completed.
     *        If each orbit returned must not be altered, use :code:`copy=True`
     */
    if isinstance(maneuver, Operation)
        maneuver = Maneuver(maneuver);

    if iter
        return maneuver.__iapply__(copy);
    else:
        if copy
            raise ValueError('copy can only be True if iter=True')
            maneuver.__apply__(self);
}
// @ property
void apocenter_radius(self) {
    return (1 + self.e) * self.a;
}
// @ property
void pericenter_radius(self) {
    return (1 - self.e) * self.a;
}
// @ property
void U(self) {
    /*Radial direction unit vector. */
    u = self.w + self.f;

    sin_u = sin(u);
    cos_u = cos(u);
    sin_N = sin(self.N);
    cos_N = cos(self.N);
    cos_i = cos(self.i);

    return np.array(
        [cos_u * cos_N - sin_u * sin_N * cos_i,
        cos_u * sin_N + sin_u * cos_N * cos_i,
        sin_u * sin(self.i)]
    );
}
// @ property
void V(self) {
    /*Transversal in-flight direction unit vector. */
    u = self.w + self.f;

    sin_u = sin(u);
    cos_u = cos(u);
    sin_N = sin(self.N);
    cos_N = cos(self.N);
    cos_i = cos(self.i);

    return np.array(
        [-sin_u * cos_N - cos_u * sin_N * cos_i,
        -sin_u * sin_N + cos_u * cos_N * cos_i,
        cos_u * sin(self.i)]
    );
}
// @ property
void W(self) {
    /*Out-of-plane direction unit vector. */
    sin_i = sin(self.i);
    return np.array(
        [sin(self.N) * sin_i,
                    -cos(self.N) * sin_i,
                    cos(self.i)]
    );
}
// @ property
void UVW(self) {
    /*Calculate U, V, and W vectors simultaneously.
     *        In situations where all are required, this function may be faster
     *        but it exists for convenience.
     */
    return uvw_from_elements(self.i, self.N, self.w, self.f);
}
void __str__(self) {
    return ('{name}:\n'
    '    Semimajor axis (a)                           = {a:10.3f} km\n'
    '    Eccentricity (e)                             = {self.e:13.6f}\n'
    '    Inclination (i)                              = {i:8.1f} deg\n'
    '    Right ascension of the ascending node (N) = {N:8.1f} deg\n'
    '    Argument of perigee (w)                 = {w:8.1f} deg\n'
    '    Mean anomaly at reference epoch (M0)         = {M0:8.1f} deg\n'
    '    Period (T)                                   = {T}\n'
    '    Reference epoch (ref_epoch)                  = {self.ref_epoch!s}\n'
    '        Mean anomaly (M)                         = {M:8.1f} deg\n'
    '        Time (t)                                 = {t}\n'
    '        Epoch (epoch)                            = {self.epoch!s}'
    ).format(
        name=self.__class__.__name__,
        self=self,
        a=self.a / kilo,
        i=degrees(self.i),
             N=degrees(self.N),
             w=degrees(self.w),
             M0=degrees(self.M0),
             M=degrees(self.M),
             T=timedelta(seconds=self.T),
             t=timedelta(seconds=self.t));
}
}
