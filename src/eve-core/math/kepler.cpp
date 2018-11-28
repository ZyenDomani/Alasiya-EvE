#include "stdafx.h"
#include "kepler.h"

KeplerClass::KeplerClass(std::vector<objectClass>& object_) : object(object_)
{
}

KeplerClass::~KeplerClass()
{
}

vec3 KeplerClass::positionAtE(int cuerpo, int segundos_)
{
	double x, y;
	double kepler_eccentric_anomaly = eccentricAnomaly(cuerpo, object[cuerpo].getMeanAnomaly() + segundos_ * object[cuerpo].getMeanMotion());

	if (object[cuerpo].getEccentricity()< 1.0)
	{
		double a = object[cuerpo].getPerigee() / (1.0 - object[cuerpo].getEccentricity());
		x = a * (cos(kepler_eccentric_anomaly) - object[cuerpo].getEccentricity());
		y = a * sqrt(1 - (object[cuerpo].getEccentricity()*object[cuerpo].getEccentricity())) * sin(kepler_eccentric_anomaly);
	}
	else if (object[cuerpo].getEccentricity() > 1.0)
	{
		double a = object[cuerpo].getPerigee() / (1.0 - object[cuerpo].getEccentricity());
		x = -a * (object[cuerpo].getEccentricity() - cosh(kepler_eccentric_anomaly));
		y = -a * sqrt((object[cuerpo].getEccentricity()*object[cuerpo].getEccentricity()) - 1) * sinh(kepler_eccentric_anomaly);
	}
	else
	{
		// TODO: Handle parabolic orbits
		x = 0.0;
		y = 0.0;
	}

	Mat3d orbitPlaneRotation = (zrotation(object[cuerpo].getLongitudeOfAscendingNode()) *
		xrotation(object[cuerpo].getInclination()) *
		zrotation(object[cuerpo].getArgumentOfPerigee()));

	vec3 p = orbitPlaneRotation * vec3{ x, y, 0 };

	return{ p.x, p.y, p.z };
}

double KeplerClass::eccentricAnomaly(int cuerpo, double M)
{
	if (object[cuerpo].getEccentricity() == 0.0)
	{
		// Circular orbit
		return M;
	}
	else if (object[cuerpo].getEccentricity() < 0.2)
	{
		// Low eccentricity, so use the standard iteration technique
		Solution sol = solve_iteration_fixed(SolveKeplerFunc1(object[cuerpo].getEccentricity(), M), M, 5);
		return sol.first;
	}
	else if (object[cuerpo].getEccentricity() < 0.9)
	{
		// Higher eccentricity elliptical orbit; use a more complex but
		// much faster converging iteration.
		Solution sol = solve_iteration_fixed(SolveKeplerFunc2(object[cuerpo].getEccentricity(), M), M, 6);
		// Debugging
		// printf("ecc: %f, error: %f mas\n",
		//        eccentricity, radToDeg(sol.second) * 3600000);
		return sol.first;
	}
	else if (object[cuerpo].getEccentricity() < 1.0)
	{
		// Extremely stable Laguerre-Conway method for solving Kepler's
		// equation.  Only use this for high-eccentricity orbits, as it
		// requires more calcuation.
		double E = M + 0.85 * object[cuerpo].getEccentricity() * sign(sin(M));
		Solution sol = solve_iteration_fixed(SolveKeplerLaguerreConway(object[cuerpo].getEccentricity(), M), E, 8);
		return sol.first;
	}
	else if (object[cuerpo].getEccentricity() == 1.0)
	{
		// Nearly parabolic orbit; very common for comets
		// TODO: handle this
		return M;
	}
	else
	{
		// Laguerre-Conway method for hyperbolic (ecc > 1) orbits.
		double E = log(2 * M / object[cuerpo].getEccentricity() + 1.85);
		Solution sol = solve_iteration_fixed(SolveKeplerLaguerreConwayHyp(object[cuerpo].getEccentricity(), M), E, 30);
		return sol.first;
	}
}