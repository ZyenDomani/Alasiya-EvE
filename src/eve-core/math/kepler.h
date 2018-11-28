#ifndef _KEPLERCLASS_H_
#define _KEPLERCLASS_H_

#include "stdafx.h"
#include "object.h"

class KeplerClass
{
public:
	KeplerClass(std::vector<objectClass>& object_);
	~KeplerClass();

	vec3 positionAtE(int cuerpo, int segundos_);

private:

	double eccentricAnomaly(int cuerpo, double M);

	struct SolveKeplerFunc1 : public std::unary_function<double, double>
	{
		double ecc;
		double M;

		SolveKeplerFunc1(double _ecc, double _M) : ecc(_ecc), M(_M) {};

		double operator()(double x) const
		{
			return M + ecc * sin(x);
		}
	};

	struct SolveKeplerFunc2 : public std::unary_function<double, double>
	{
		double ecc;
		double M;

		SolveKeplerFunc2(double _ecc, double _M) : ecc(_ecc), M(_M) {};

		double operator()(double x) const
		{
			return x + (M + ecc * sin(x) - x) / (1 - ecc * cos(x));
		}
	};

	struct SolveKeplerLaguerreConway : public std::unary_function<double, double>
	{
		double ecc;
		double M;

		SolveKeplerLaguerreConway(double _ecc, double _M) : ecc(_ecc), M(_M) {};

		double operator()(double x) const
		{
			double s = ecc * sin(x);
			double c = ecc * cos(x);
			double f = x - s - M;
			double f1 = 1 - c;
			double f2 = s;
			x += -5 * f / (f1 + sign(f1) * sqrt(abs(16 * f1 * f1 - 20 * f * f2)));

			return x;
		}
	};

	struct SolveKeplerLaguerreConwayHyp : public std::unary_function<double, double>
	{
		double ecc;
		double M;

		SolveKeplerLaguerreConwayHyp(double _ecc, double _M) : ecc(_ecc), M(_M) {};

		double operator()(double x) const
		{
			double s = ecc * sinh(x);
			double c = ecc * cosh(x);
			double f = s - x - M;
			double f1 = c - 1;
			double f2 = s;
			x += -5 * f / (f1 + sign(f1) * sqrt(abs(16 * f1 * f1 - 20 * f * f2)));

			return x;
		}
	};

	std::vector<objectClass>& object;
};
#endif