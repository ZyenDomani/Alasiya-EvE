#ifndef _AEROCLASS_H_
#define _AEROCLASS_H_

#include "stdafx.h"
#include "object.h"
#include "bullet.h"

class AeroDynamicsClass
{
public:
	AeroDynamicsClass(std::vector<objectClass>& object_, BulletClass& bullet_);
	~AeroDynamicsClass();

	void apply_aerodynamics(int objeto_, double dt_);
	vec3 getOrbitedRotationVec(int n);
	double getKIAS(int n);
	void manage_stability(int n, double dt_);
	float getRCS_maneuverability(int n);
	float getSCS_maneuverability(int n, double dt_);
	float getTrueAirSpeed(int objeto);
	float get_air_density(int objeto_);
	float getAirTemperature(int objeto);
	float get_stagnation_point_temperature(int objeto_);
	void rotate_axis_to_targetVector(int& n, char axis_, vec3& targetVector_, double power_);
	double test_rotationX(int& n, char& axis_, vec3& targetVector_, double speed_);
	double test_rotationY(int& n, char& axis_, vec3& targetVector_, double speed_);
	double test_rotationZ(int& n, char& axis_, vec3& targetVector_, double speed_);

	void ModifyTotalMovement(int n, vec3 movememnt, double dt_);

private:

	float get_atmospheric_pressure(int objeto_);
	float get_equivalent_air_speed(int objeto_, float dynamic_pressure_);
	float get_true_air_speed(int objeto_, float dynamic_pressure_, float air_density_);

	const float dry_air_molar_mass; // kg/mol
	const float univ_gas_constant; // J / (mol•K)
	const float specific_gas_constant; // J·kg-1·K-1
	const float kelvin; // 0 celsius to kelvin
	const float temperature_lapse_rate; // the rate at which atmospheric temperature decreases with an increase in altitude
	const float adiabatic_constant; // adiabatic constant for dry air
	const float recovery_factor; // empirical recovery factor for air temperature

	std::vector<objectClass>& object;
	BulletClass& bullet_physics;

};

#endif