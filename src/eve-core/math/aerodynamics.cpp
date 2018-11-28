#include "stdafx.h"
#include "aerodynamics.h"

AeroDynamicsClass::AeroDynamicsClass(std::vector<objectClass>& object_, BulletClass& bullet_) :
object(object_), bullet_physics(bullet_),
dry_air_molar_mass(0.0289644f), // kg/mol
univ_gas_constant(8.31447f), // J / (mol•K)
specific_gas_constant(287.058f), // J·kg-1·K-1
kelvin(273.15f), // 0 celsius to kelvin
temperature_lapse_rate(0.0065f), // Kelvin/m, the rate at which atmospheric temperature decreases with an increase in altitude
adiabatic_constant(1.4f), // adiabatic constant for dry air
recovery_factor(0.98f) // empirical temperature recovery factor for dry air
{
}

AeroDynamicsClass::~AeroDynamicsClass()
{
}

float AeroDynamicsClass::get_atmospheric_pressure(int objeto_)
{
	// ecuacion para obtener la presion atmosferica a determinada altitud en pascales
	float temp_kelvin = (kelvin + object[object[objeto_].getOrbiting()].getSeaLevelTemperature());

	return float(object[object[objeto_].getOrbiting()].getSeaLevelPressure()*exp(-((object[objeto_].getG_Acceleration()*dry_air_molar_mass*object[objeto_].get_altitude()) / (univ_gas_constant*temp_kelvin))));
}

float AeroDynamicsClass::get_air_density(int objeto_)
{
	// ecuacion para obtener la densidad del aire a determinada altitud en kg/m3
	float temp_kelvin = (kelvin + object[object[objeto_].getOrbiting()].getSeaLevelTemperature());
	float absolut_temp_kelvin = float(max(213.f, temp_kelvin - (temperature_lapse_rate*object[objeto_].get_altitude()))); // clampeamos la temperatura a -60 para cualquier altitud sobre la troposfera, tenemos que hacer una formula mas exacta para mas realismo

	float pascals = get_atmospheric_pressure(objeto_);
	return pascals / (specific_gas_constant*absolut_temp_kelvin);
}

float AeroDynamicsClass::get_equivalent_air_speed(int objeto_, float dynamic_pressure_)
{
	return sqrtf((2 * dynamic_pressure_) / object[object[objeto_].getOrbiting()].getSeaLevelDensity());
}

float AeroDynamicsClass::get_true_air_speed(int objeto_, float dynamic_pressure_, float air_density_)
{
	return get_equivalent_air_speed(objeto_, dynamic_pressure_)*sqrtf(object[object[objeto_].getOrbiting()].getSeaLevelDensity() / air_density_);
}

void AeroDynamicsClass::apply_aerodynamics(int objeto_, double dt_)
{
	int orbited_ = object[objeto_].getOrbiting();
	object[objeto_].updateOrbitalPositionAndVelocity(object[orbited_].getPosition(), object[orbited_].getVelocity()); // *** DEBUG *** este calculo se repite en otros lugares y gasta tiempo de calculos

	vec3& orbital_position = object[objeto_].getOrbitalPosition();
	vec3& orbital_velocity_ = object[objeto_].getOrbitalVelocity();

	// true_velocity es la velocidad orbital menos las velocidad de rotacion del cuerpo orbitado, o sea es el true air speed
	object[objeto_].set_true_velocity(orbital_velocity_ - getOrbitedRotationVec(objeto_)); // OrbitedRotationVec es el vector de velocidad relativa de rotacion del cuerpo
	// orbitado con respecto a la nave

	object[objeto_].set_air_density(get_air_density(objeto_));

	//vec3 signo = { sign_b(true_velocity.x), sign_b(true_velocity.y), sign_b(true_velocity.z) }; // guardamos los signos del airflow_dir;
	//dynamic_pressure = (true_velocity.mul(true_velocity).mul(signo) *(0.5f*air_density));
	object[objeto_].set_dynamic_pressure(object[objeto_].get_true_velocity()*length(object[objeto_].get_true_velocity() *(0.5f*object[objeto_].get_air_density())));

	//true_air_speed = get_true_air_speed(objeto_, float(length(dynamic_pressure)), air_density);



	//Config.display_info = std::to_string(total_air_temperature - 273.15f);
	//Config.display_info = format_speed_output(true_air_speed) + ", " + format_speed_output(length(true_velocity));

	// Usamos la ecuacion de friccion atmosferica descrita por la NASA aqui https://spaceflightsystems.grc.nasa.gov/education/rocket/drageq.html
	// La friccion es una fuerza opuesta a la velocidad, aqui se define por el cuadrado del TAS (true air speed) por 0.5, por la presion atmosferica,
	// por la friccion aerodinamica en el angulo de ataque, por el area transversal total del objeto que enfrenta el drag.

	// determinamos el angulo entre el frente de la nave y la corriente de aire
	float angulo_total = float(angle_from_two_vectors(object[objeto_].get_true_velocity(), -GetAxisZ(object[objeto_].getTotalRotation())));
	object[objeto_].set_medio_angulo(angulo_total);

	if (object[objeto_].get_medio_angulo() > DirectX::XM_PIDIV2) object[objeto_].set_medio_angulo(object[objeto_].get_medio_angulo() - (angulo_total - DirectX::XM_PIDIV2));
	if (angulo_total > DirectX::XM_PIDIV2) angulo_total = DirectX::XM_PIDIV2 - (angulo_total - DirectX::XM_PIDIV2);
	object[objeto_].set_angulo_total_percent((angulo_total * 100) / DirectX::XM_PIDIV2);
	object[objeto_].set_medio_angulo_percent((object[objeto_].get_medio_angulo() * 100) / DirectX::XM_PIDIV2);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//													calculamos el arrastre																		//
	object[objeto_].set_area_frontal((angulo_total * 100) / DirectX::XM_PIDIV2);
	object[objeto_].set_area_frontal(object[objeto_].getFrontArea() - (object[objeto_].getFrontArea() * object[objeto_].get_area_frontal() / 100));

	object[objeto_].set_area_lateral((angulo_total * 100) / DirectX::XM_PIDIV2);
	object[objeto_].set_area_lateral(object[objeto_].getSideArea() * object[objeto_].get_area_lateral() / 100);

	object[objeto_].set_area_total(object[objeto_].get_area_frontal() + object[objeto_].get_area_lateral());
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	object[objeto_].set_friccion(object[objeto_].getFrontFriction() + (0.82f * object[objeto_].get_medio_angulo_percent() / 100)); // 0.82 es el coeficiente de arrastre de un cylindro largo
	// 0.82 drag coeficient for a long cylinder

	object[objeto_].set_Drag(object[objeto_].get_dynamic_pressure() * object[objeto_].get_friccion() * object[objeto_].get_area_total());
	//																																				//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//std::stringstream valor;
	//valor << std::fixed << std::setprecision(2) << max(0.f, (getAerodynamicHeat(objeto_) - kelvin));

	//std::stringstream valorb;
	//valorb << std::fixed << std::setprecision(2) << max(0.f, (getAirTemperature(objeto_) - kelvin));

	//object[objeto_].setVelocity(object[objeto_].getVelocity() - true_velocity*dt_);
	//std::string text_ = valor.str();// +", " + valorb.str();

	//Config.display_info = text_;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//												calculamos el levante aerodinamico																//
	// Usamos la ecuacion de elevacion descrita por la NASA aqui https://spaceflightsystems.grc.nasa.gov/education/rocket/lifteq.html
	// La elevacion es una fuerza perpendicular a la longitud de la nave, aqui se define por el cuadrado del TAS (true air speed) por 0.5,
	// por la presion atmosferica, por la elevacion que da la forma del airfoil de las alas en el angulo de ataque, por el area de las alas

	int upwards_push = 1;
	if (angle_from_two_vectors(object[objeto_].get_true_velocity(), GetAxisY(object[objeto_].getTotalRotation())) < DirectX::XM_PIDIV2) upwards_push = -1;

	float lift = object[objeto_].getMinLift() + (object[objeto_].getMaxLift() * object[objeto_].get_angulo_total_percent() / 100);  // no es correcto, pero ya da por el momento.
	// no es correcto porque usamos el levante maximo cuando la nave esta totalmente perpendicular al flujo de aire, o sea, nada que ver luego.
	// deberiamos usar el angulo critico normal entre el 15% y el 20% y si es mayor entonces comenzar a disminuir el levante proporcionalmente.
	// pero ya me canse por hoy, otro dia mba'e.

	vec3 lift_vec = object[objeto_].get_dynamic_pressure() * lift * object[objeto_].getWingSpan(); // *** DEBUG *** calcular el area de las alas!!!
	// lift_vec = lift_vec.mul(signo); // ponemos los signos del vector de velocidad al vector de levante aerodinamico para su correcto calculo
	// 1.45 lift coeficient for a plain airfoil

	object[objeto_].set_true_lift_vec(GetAxisY(object[objeto_].getTotalRotation())*length(lift_vec)*upwards_push);

	object[objeto_].setVelocity(object[objeto_].getVelocity() - (object[objeto_].get_Drag() / object[objeto_].getMass())*dt_); // agregamos el arrastre aerodinamico
	object[objeto_].setVelocity(object[objeto_].getVelocity() + (object[objeto_].get_true_lift_vec() / object[objeto_].getMass())*dt_); // agregamos el levante aerodinamico

	object[objeto_].setVelocityModified(true);

	manage_stability(objeto_, dt_);
}

float AeroDynamicsClass::getRCS_maneuverability(int n)
{
	if (object[n].getInAtmosphere())
	{
		float ratio_;
		ratio_ = float(length(object[n].get_dynamic_pressure() / object[n].getMass()))*dt_b;
		//if (!object[n].getInBullet()) ratio_ = float(length(object[n].get_dynamic_pressure() / object[n].getMass()))*dt_a;
		//else ratio_ = float(length(object[n].get_dynamic_pressure()));

		return max(0.f, float(object[n].getRCSpower() - ratio_));
	}

	else
	{
		return float(object[n].getRCSpower() / object[n].getMass())*dt_b;
		//if (!object[n].getInBullet()) return float(object[n].getRCSpower() / object[n].getMass())*dt_a;
		//else return float(object[n].getRCSpower());
	}
}

float AeroDynamicsClass::getSCS_maneuverability(int n, double dt_)
{
	return (float(length(object[n].get_dynamic_pressure()))*(sqrtf(100.f - object[n].get_medio_angulo_percent()) / 100.f)) / float(object[n].getMass());
}

float AeroDynamicsClass::getAirTemperature(int objeto_)
{
	// devuelve el calentamiento por friccion aerodinamica en grados kelvin, solo valido dentro de la troposfera
	float speed_of_sound = sqrtf(adiabatic_constant*(get_atmospheric_pressure(objeto_) / object[objeto_].get_air_density()));
	float mach_ = float(length(object[objeto_].get_true_velocity())) / speed_of_sound;

	float temp_kelvin = (kelvin + object[object[objeto_].getOrbiting()].getSeaLevelTemperature());
	float absolut_temp_kelvin = float(max(213.f, temp_kelvin - (temperature_lapse_rate*object[objeto_].get_altitude()))); // clampeamos la temperatura a -60 para cualquier altitud sobre la troposfera, tenemos que hacer una formula mas exacta para mas realismo

	//float ram_rise = recovery_factor*(length(true_velocity.mul(true_velocity)))*((adiabatic_constant - 1) / (adiabatic_constant * 2 * specific_gas_constant));
	float ram_rise = absolut_temp_kelvin*((adiabatic_constant - 1) / 2)*recovery_factor*(mach_*mach_);

	return absolut_temp_kelvin + ram_rise;
}

float AeroDynamicsClass::get_stagnation_point_temperature(int objeto_)
{
	// devuelve el calentamiento por friccion aerodinamica en el punto de estancamiento en grados kelvin, valido para todas las capas de la atmosfera
	// Con esta formula se gasta mucho calculo en conversiones, deberiamos encontrar una formula que use medidas internacionales y no imperiales

	float ffd = kgm3_to_slugft3(object[objeto_].get_air_density()); // free-flow density
	float sld = kgm3_to_slugft3(object[object[objeto_].getOrbiting()].getSeaLevelDensity()); // sea-level density
	float ffv = float(meters_to_feet(length(object[objeto_].get_true_velocity()))); // free_flow velocity
	float nr = meters_to_feet(object[objeto_].get_area_total() / 4); // nose radius, *** DEBUG *** tengo que encontrar una formula que calcule el radio del area transversal y poner aqui

	float cod = sld * ((2.023f * 1e-8f / nr) * powf(ffv, 0.3f)); // cross-over density

	float degrees_ = 1000 * powf(173.4f * sqrtf((ffd / sld) * (ffd / cod) / (1.f + ffd / cod)) * powf(ffv / 1e4f, 3.15f) / sqrtf(nr), 1.f / 4.f);

	if (degrees_ > 0 || degrees_ < 0) return degrees_;

	else return 0.f;
}


void AeroDynamicsClass::manage_stability(int n, double dt_)
{
	// ***DEBUG*** esto es cualquier cosa, un desastre total
	
	Vector3 earth_vec = normalized_f(object[n].getPosition_b() - object[object[n].getOrbiting()].getPosition_b());

	//double power_ = ToRadians_d(length(object[n].get_dynamic_pressure() / (object[n].get_medio_angulo() * object[n].getMass())))*dt_;
	
	//double power_b = ToRadians_d(length(object[n].get_dynamic_pressure() * (object[n].get_medio_angulo() / sqrt(object[n].getMass()))))*dt_;

	double power_c = ToRadians_d(length(object[n].get_dynamic_pressure() * (object[n].get_medio_angulo() / sqrt(object[n].getMass()))))*dt_;

	rotate_axis_to_targetVector(n, 'z', -normalized(object[n].get_true_velocity()), power_c); // esto hace que el avion apunte a la direccion del viento
	//rotate_axis_to_targetVector(n, 'y', d3d_to_vec(earth_vec), power_b); // esto alinea al avion horizontalmente

	/*
	// gravity turn test
	if (object[n].getMECurrentPower() > 0.0)
	{
		vec3 z_vec = -GetAxisZ(object[n].getTotalRotation());
		double angle = angle_from_two_vectors(z_vec, d3d_to_vec(earth_vec));

		// si el angulo es mayor a 90 grados establecemos el vector a seguir como la direccion horizontal de la nave, esto es para evitar
		// atajos raros al girar hacia el vector objetivo final que es el terreno, directamente hacia abajo
		vec3 target_vec;
		if (angle > DirectX::XM_PIDIV2) target_vec = object[n].getHorizontalDir();
		else target_vec = d3d_to_vec(earth_vec);

		if (angle > DirectX::XM_PIDIV2) angle = DirectX::XM_PI - angle;	
		double total_force = angle / DirectX::XM_PIDIV2;
		total_force = (total_force*object[n].getG_Acceleration());

		// aqui tenemos que multiplicar el angulo por la distancia entre la base y el centro de la masa
		double dist = length(object[n].getCenterOfMass());
		total_force = ToRadians_d(total_force * dist) * dt_b;

		vec3 z_vec_new = vector_vector_rotation_by_angle(z_vec, target_vec, total_force);
		quat z_rot = vecToVecRotation(z_vec, z_vec_new);
		object[n].setTotalRotation(z_rot*object[n].getTotalRotation());
	}
	*/
}

void AeroDynamicsClass::rotate_axis_to_targetVector(int& n, char axis_, vec3& targetVector_, double power_)
{
	double angle = 0.0;
	if (axis_ == 'x') angle = angle_from_two_vectors(GetAxisX(object[n].getTotalRotation()), targetVector_);
	if (axis_ == 'y') angle = angle_from_two_vectors(GetAxisY(object[n].getTotalRotation()), targetVector_);
	if (axis_ == 'z') angle = angle_from_two_vectors(GetAxisZ(object[n].getTotalRotation()), targetVector_);
	double max_speed = min(power_, angle / 10); // dividimos el angulo para suavizar la rotacion en aproximaciones bajas
	double clamp_speed = power_ * 20;
	double alfa, beta;

	if (axis_ != 'x')
	{
		if (test_rotationX(n, axis_, targetVector_, bullet_physics.getAngulosX(n)) == 0.0)
		{
			if (bullet_physics.getAngulosX(n) > 0.0)
			{
				if (bullet_physics.getAngulosX(n) > power_) bullet_physics.applyTorqueX(n, -power_);
				else bullet_physics.setAngularVel_X(n, 0.0);
			}
			else if (bullet_physics.getAngulosX(n) < 0.0)
			{
				if (bullet_physics.getAngulosX(n) < -power_) bullet_physics.applyTorqueX(n, power_);
				else bullet_physics.setAngularVel_X(n, 0.0);
			}
		}

		alfa = test_rotationX(n, axis_, targetVector_, bullet_physics.getAngulosX(n) + max_speed);
		beta = test_rotationX(n, axis_, targetVector_, bullet_physics.getAngulosX(n) - max_speed);

		if (alfa > beta)
		{
			if (abs(bullet_physics.getAngulosX(n) + max_speed)<clamp_speed) bullet_physics.applyTorqueX(n, max_speed);
		}

		else if (alfa < beta)
		{
			if (abs(bullet_physics.getAngulosX(n) - max_speed)<clamp_speed) bullet_physics.applyTorqueX(n, -max_speed);
		}
	}

	if (axis_ != 'y')
	{
		if (test_rotationX(n, axis_, targetVector_, bullet_physics.getAngulosY(n)) == 0.0)
		{
			if (bullet_physics.getAngulosY(n) > 0.0)
			{
				if (bullet_physics.getAngulosY(n) > power_) bullet_physics.applyTorqueY(n, -power_);
				else bullet_physics.setAngularVel_Y(n, 0.0);
			}
			else if (bullet_physics.getAngulosY(n) < 0.0)
			{
				if (bullet_physics.getAngulosY(n) < -power_) bullet_physics.applyTorqueY(n, power_);
				else bullet_physics.setAngularVel_Y(n, 0.0);
			}
		}

		alfa = test_rotationX(n, axis_, targetVector_, bullet_physics.getAngulosY(n) + max_speed);
		beta = test_rotationX(n, axis_, targetVector_, bullet_physics.getAngulosY(n) - max_speed);

		if (alfa > beta)
		{
			if (abs(bullet_physics.getAngulosY(n) + max_speed)<clamp_speed) bullet_physics.applyTorqueY(n, max_speed);
		}

		else if (alfa < beta)
		{
			if (abs(bullet_physics.getAngulosY(n) - max_speed)<clamp_speed) bullet_physics.applyTorqueY(n, -max_speed);
		}
	}

	if (axis_ != 'z')
	{
		if (test_rotationX(n, axis_, targetVector_, bullet_physics.getAngulosZ(n)) == 0.0)
		{
			if (bullet_physics.getAngulosZ(n) > 0.0)
			{
				if (bullet_physics.getAngulosZ(n) > power_) bullet_physics.applyTorqueZ(n, -power_);
				else bullet_physics.setAngularVel_Z(n, 0.0);
			}
			else if (bullet_physics.getAngulosZ(n) < 0.0)
			{
				if (bullet_physics.getAngulosZ(n) < -power_) bullet_physics.applyTorqueZ(n, power_);
				else bullet_physics.setAngularVel_Z(n, 0.0);
			}
		}

		alfa = test_rotationX(n, axis_, targetVector_, bullet_physics.getAngulosZ(n) + max_speed);
		beta = test_rotationX(n, axis_, targetVector_, bullet_physics.getAngulosZ(n) - max_speed);

		if (alfa > beta)
		{
			if (abs(bullet_physics.getAngulosZ(n) + max_speed)<clamp_speed) bullet_physics.applyTorqueZ(n, max_speed);
		}

		else if (alfa < beta)
		{
			if (abs(bullet_physics.getAngulosZ(n) - max_speed)<clamp_speed) bullet_physics.applyTorqueZ(n, -max_speed);
		}
	}
}

double AeroDynamicsClass::test_rotationX(int& n, char& axis_, vec3& targetVector_, double speed_)
{
	quat total_rotation_ = object[n].getTotalRotation();
	quat q_temp = { 0.0, 0.0, 0.0, 1.0 };
	double angulo_a_checkear = 0.0;
	double angulo_a_checkear_b = 0.0;

	if (axis_ == 'x') angulo_a_checkear = angle_from_two_vectors(GetAxisX(total_rotation_), targetVector_);
	else if (axis_ == 'y') angulo_a_checkear = angle_from_two_vectors(GetAxisY(total_rotation_), targetVector_);
	else angulo_a_checkear = angle_from_two_vectors(GetAxisZ(total_rotation_), targetVector_);

	q_temp.x = 1.0 * sin(speed_ / 2.0);
	q_temp.w = cos(speed_ / 2.0);
	total_rotation_ = q_temp* total_rotation_;

	if (axis_ == 'x') angulo_a_checkear_b = angle_from_two_vectors(GetAxisX(total_rotation_), targetVector_);
	else if (axis_ == 'y') angulo_a_checkear_b = angle_from_two_vectors(GetAxisY(total_rotation_), targetVector_);
	else angulo_a_checkear_b = angle_from_two_vectors(GetAxisZ(total_rotation_), targetVector_);

	if (angulo_a_checkear_b < angulo_a_checkear) return angulo_a_checkear - angulo_a_checkear_b;

	return 0.0;
}

double AeroDynamicsClass::test_rotationY(int& n, char& axis_, vec3& targetVector_, double speed_)
{
	quat total_rotation_ = object[n].getTotalRotation();
	quat q_temp = { 0.0, 0.0, 0.0, 1.0 };
	double angulo_a_checkear = 0.0;
	double angulo_a_checkear_b = 0.0;

	if (axis_ == 'x') angulo_a_checkear = angle_from_two_vectors(GetAxisX(total_rotation_), targetVector_);
	else if (axis_ == 'y') angulo_a_checkear = angle_from_two_vectors(GetAxisY(total_rotation_), targetVector_);
	else angulo_a_checkear = angle_from_two_vectors(GetAxisZ(total_rotation_), targetVector_);

	q_temp.y = 1.0 * sin(speed_ / 2.0);
	q_temp.w = cos(speed_ / 2.0);
	total_rotation_ = q_temp* total_rotation_;

	if (axis_ == 'x') angulo_a_checkear_b = angle_from_two_vectors(GetAxisX(total_rotation_), targetVector_);
	else if (axis_ == 'y') angulo_a_checkear_b = angle_from_two_vectors(GetAxisY(total_rotation_), targetVector_);
	else angulo_a_checkear_b = angle_from_two_vectors(GetAxisZ(total_rotation_), targetVector_);

	if (angulo_a_checkear_b < angulo_a_checkear) return angulo_a_checkear - angulo_a_checkear_b;

	return 0.0;
}

double AeroDynamicsClass::test_rotationZ(int& n, char& axis_, vec3& targetVector_, double speed_)
{
	quat total_rotation_ = object[n].getTotalRotation();
	quat q_temp = { 0.0, 0.0, 0.0, 1.0 };
	double angulo_a_checkear = 0.0;
	double angulo_a_checkear_b = 0.0;

	if (axis_ == 'x') angulo_a_checkear = angle_from_two_vectors(GetAxisX(total_rotation_), targetVector_);
	else if (axis_ == 'y') angulo_a_checkear = angle_from_two_vectors(GetAxisY(total_rotation_), targetVector_);
	else angulo_a_checkear = angle_from_two_vectors(GetAxisZ(total_rotation_), targetVector_);

	q_temp.z = 1.0 * sin(speed_ / 2.0);
	q_temp.w = cos(speed_ / 2.0);
	total_rotation_ = q_temp* total_rotation_;

	if (axis_ == 'x') angulo_a_checkear_b = angle_from_two_vectors(GetAxisX(total_rotation_), targetVector_);
	else if (axis_ == 'y') angulo_a_checkear_b = angle_from_two_vectors(GetAxisY(total_rotation_), targetVector_);
	else angulo_a_checkear_b = angle_from_two_vectors(GetAxisZ(total_rotation_), targetVector_);

	if (angulo_a_checkear_b < angulo_a_checkear) return angulo_a_checkear - angulo_a_checkear_b;

	return 0.0;
}

vec3 AeroDynamicsClass::getOrbitedRotationVec(int objeto_)
{
	int orbited_ = object[objeto_].getOrbiting();
	vec3& orbital_position = object[objeto_].getOrbitalPosition();
	vec3& orbital_velocity_ = object[objeto_].getOrbitalVelocity();

	quat total_rotation = object[orbited_].getTotalRotation();

	// *** DEBUG *** este mismo calculo de rotacion se repite en varios lugares, en el GUI por ejemplo para la brujula,
	// tambien en el showmaster para el calculo de impacto con el terreno
	quat unrotated = total_rotation / quat{ 0.0, 0.0, 0.0, 1.0 };
	vec3 rotated_position = vector_rotation_by_quaternion(orbital_position, unrotated);

	quat q_temp = { 0.0, 0.0, 0.0, 1.0 };
	double angle = bullet_physics.getAngulosX(orbited_);
	q_temp.x = 1.0 * sin(angle / 2.0);
	q_temp.w = cos(angle / 2.0);
	total_rotation = q_temp* total_rotation;

	q_temp = { 0.0, 0.0, 0.0, 1.0 };
	angle = bullet_physics.getAngulosY(orbited_);
	q_temp.y = 1.0 * sin(angle / 2.0);
	q_temp.w = cos(angle / 2.0);
	total_rotation = q_temp* total_rotation;

	q_temp = { 0.0, 0.0, 0.0, 1.0 };
	angle = bullet_physics.getAngulosZ(orbited_);
	q_temp.z = 1.0 * sin(angle / 2.0);
	q_temp.w = cos(angle / 2.0);
	total_rotation = q_temp* total_rotation;

	return vector_rotation_by_quaternion(rotated_position, ~total_rotation) - orbital_position;
}

void AeroDynamicsClass::ModifyTotalMovement(int n, vec3 movement, double dt_)
{
	int orbited_ = object[n].getOrbiting();

	// true_velocity es la velocidad orbital menos las velocidad de rotacion del cuerpo orbitado, o sea es el true air speed
	vec3 true_velocity_ = movement;

	float air_density_ = get_air_density(n);

	//vec3 signo = { sign_b(true_velocity_.x), sign_b(true_velocity_.y), sign_b(true_velocity_.z) }; // guardamos los signos del airflow_dir;
	vec3 dynamic_pressure_ = (true_velocity_*length(true_velocity_) *(0.5f*air_density_));

	vec3 drag_ = dynamic_pressure_ * object[n].get_friccion() * object[n].get_area_total();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int upwards_push = 1;
	if (angle_from_two_vectors(true_velocity_, GetAxisY(object[n].getTotalRotation())) < DirectX::XM_PIDIV2) upwards_push = -1;

	float lift = object[n].getMinLift() + (object[n].getMaxLift() * object[n].get_angulo_total_percent() / 100);  // no es correcto, pero ya da por el momento.
	// no es correcto porque usamos el levante maximo cuando la nave esta totalmente perpendicular al flujo de aire, o sea, nada que ver luego.
	// deberiamos usar el angulo critico normal entre el 15% y el 20% y si es mayor entonces comenzar a disminuir el levante proporcionalmente.
	// pero ya me canse por hoy, otro dia mba'e.

	vec3 lift_vec = dynamic_pressure_ * lift * object[n].getWingSpan(); // *** DEBUG *** calcular el area de las alas!!!
	// lift_vec = lift_vec.mul(signo); // ponemos los signos del vector de velocidad al vector de levante aerodinamico para su correcto calculo
	// 1.45 lift coeficient for a plain airfoil

	vec3 true_lift_vec_ = GetAxisY(object[n].getTotalRotation())*length(lift_vec)*upwards_push;

	movement -= (drag_ / object[n].getMass())*dt_; // agregamos el arrastre aerodinamico
	movement += (true_lift_vec_ / object[n].getMass())*dt_; // agregamos el levante aerodinamico

	object[n].setTotalMovement(movement);
	object[n].setVelocityModified(true);
}

double AeroDynamicsClass::getKIAS(int n) // returns the Indicated air speed in Knots
{
	return 17.18*sqrt(Pascals_to_Psf(length(object[n].get_dynamic_pressure())));
}

float AeroDynamicsClass::getTrueAirSpeed(int objeto)
{
	return object[objeto].get_true_air_speed();
}