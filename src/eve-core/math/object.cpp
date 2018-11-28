#include "stdafx.h"
#include "object.h"

objectClass::objectClass()
{
	type = 2;
	sub_type = "N/A";
	collision_shape_type = -1;
	model = 0;
	affects = 1; // 0 no afecta ni se deja afectar, 1 afecta y se deja afectar, 2 no afecta pero se deja afectar
	docked_to = -1;
	attached_to = "";
	orbiting = 0;
	texture1 = 0;
	texture2 = 0;
	texture3 = 0;
	ring_object = 0;
	clouds_object = 0;
	target_ = 0;
	constraint_type = 0;
	constraint_axis = ' ';
	min_constraint_move = 0.0;
	max_constraint_move = 0.0;
	current_constraint_move = 0.0;
	copy_number = 1; // numero del objeto en caso de registrarse copias del mismo
	seconds_check = 0.f;
	seconds_check_b = 0.f;
	local_fixture = 0;
	target_fixture = 0;

	vs = 0.0;
	former_altitud = 0.0;

	kill_rotation = false;
	auto_grapple = false;
	loaded_tm = false;
	casts_shadow = false;
	landed = false;
	ephemeris_OK = false;
	visible = true;
	has_atmosphere = false;
	has_clouds = false;
	has_rings = false;
	has_light_map = false;
	has_night_map = false;
	has_specular_map = false;
	has_rings = false;
	dock_ready = false;
	landing_gear = false;
	velocity_modified = false;
	aerodynamic_controls = false;
	quick_Me_Burst = false;
	quick_Hover_Burst = false;
	in_orbit = true;
	rocket_engine = true;
	sunlit = false;
	in_atmosphere = false;
	destroyed = false;
	is_parent = false;
	has_animations = false;
	rcs_traslation_enabled = false;
	in_bullet = false;
	in_virtual_bullet = false;
	bullet_constrained = false;
	highlighted = false;
	docking_data_updated = false;
	robotic_arm_dport = false;
	visual_helpers = false;

	bullet_constrained_to = 0;

	bullet_id = -1;

	gimbal = false;
	gimbal_angle = 0.0;
	center_of_mass = { 0.0, 0.0, 0.0 };
	horizontal_dir = { 0.0, 0.0, 0.0 };
	attached_position = { 0.0, 0.0, 0.0 };

	Forward_on = false;
	Backward_on = false;
	Up_on = false;
	Down_on = false;
	Right_on = false;
	Left_on = false;

	total_animations = 0;

	orbited_name = "N/A";
	target_name = "N/A";
	docked_to_name = "N/A";

	collision_shape_file = "N/A";

	x_positive_rot_on = false;
	y_positive_rot_on = false;
	z_positive_rot_on = false;

	x_negative_rot_on = false;
	y_negative_rot_on = false;
	z_negative_rot_on = false;
	
	h_level = false;
	radial_in = false;
	radial_out = false;
	pro_grade = false;
	retro_grade = false;
	normal = false;
	anti_normal = false;
	lock_target = false;
	match_speed = false;
	approach = false;
	AlignToTargetsUp = false;
	retro_diff = false;

	inner_ring = 1;
	outer_ring = 2;
	rings_texture = "";

	docked_rotation_x = 0.f;
	docked_rotation_y = 0.f;
	docked_rotation_z = 0.f;

	stages_file = "";
	stage = 1;
	top_absolute_parent = -1;
	top_artificial_parent = -1;
	fuel_consumption = 0.0;

	camera_prefered_axis = Vector3{ 0.f, 1.f, 0.f };

	orbital_position = { 0.0, 0.0, 0.0 };
	ground_speed_vec = { 0.0, 0.0, 0.0 };
	docked_position = { 0.0, 0.0, 0.0 };
	original_docked_position = { 0.0, 0.0, 0.0 };
	docked_position_b = { 0.0, 0.0, 0.0 };
	orbital_velocity = { 0.0, 0.0, 0.0 };
	specific_angular_momentum = { 0.0, 0.0, 0.0 };
	node_line = { 0.0, 0.0, 0.0 };
	unit_vector_to_ascending_node = { 0.0, 0.0, 0.0 };
	eccentricity_vector = { 0.0, 0.0, 0.0 };
	movimiento_total = { 0.0, 0.0, 0.0 };
	total_rotation = { 0.0, 0.0, 0.0, 1.0 };
	original_docked_rotation = { 0.0, 0.0, 0.0, 1.0 };
	inherited_rotation = { 0.0, 0.0, 0.0, 1.0 };

	position = vec3{ 0.0f, 0.0f, 0.0f };
	docked_former_position = vec3{ 0.0f, 0.0f, 0.0f };
	velocity = vec3{ 0.0f, 0.0f, 0.0f };
	position_b = Vector3{ 0.0f, 0.0f, 0.0f };
	position_bullet = Vector3{ 0.0f, 0.0f, 0.0f };
	position_bullet_b = Vector3{ 0.0f, 0.0f, 0.0f };
	position_c = Vector3{ 0.0f, 0.0f, 0.0f };
	velocity_b = Vector3{ 0.0f, 0.0f, 0.0f };
	velocity_c = Vector3{ 0.0f, 0.0f, 0.0f };

	sunset_color = Vector3{ 0.0f, 0.0f, 0.0f };
	daylight_color = Vector3{ 0.0f, 0.0f, 0.0f };
	atmosphere_thickness = 0.f;

	total_ForwardNozzles = 0;
	total_BackwardNozzles = 0;
	total_UpNozzles = 0;
	total_DownNozzles = 0;
	total_RightNozzles = 0;
	total_LeftNozzles = 0;

	total_PositiveXNozzles = 0;
	total_PositiveYNozzles = 0;
	total_PositiveZNozzles = 0;

	total_NegativeXNozzles = 0;
	total_NegativeYNozzles = 0;
	total_NegativeZNozzles = 0;

	Forward_power = 0.f;
	Backward_power = 0.f;
	Up_power = 0.f;
	Down_power = 0.f;
	Right_power = 0.f;
	Left_power = 0.f;

	x_positive_power = 0.f;
	y_positive_power = 0.f;
	z_positive_power = 0.f;

	x_negative_power = 0.f;
	y_negative_power = 0.f;
	z_negative_power = 0.f;

	angulo_total_percent = 0.f;
	medio_angulo_percent = 0.f;
	area_total = 0.f;
	area_frontal = 0.f;
	area_lateral = 0.f;
	medio_angulo = 0.f;
	friccion = 0.f;
	vv = 0.0;
	atmospheric_pressure = 0.f;
	air_density = 0.f;
	altitude = 0.0;
	true_air_speed = 0.f;
	dynamic_pressure = { 0.0, 0.0, 0.0 };
	Drag = { 0.0, 0.0, 0.0 };
	true_velocity = { 0.0, 0.0, 0.0 };
	true_lift_vec = { 0.0, 0.0, 0.0 };
	engine_color_start = Color{ 0.f, 0.f, 0.f, 1.f };
	engine_color_end = Color{ 0.f, 0.f, 0.f, 1.f };

	heading = 0.0;

	name = "N/A";
	original_name = "N/A";
	rings_texture = "";

	scale = 0.0;
	mass = 1.0;
	empty_mass = 1.0;
	semi_mayor_axis = 0.0;
	semi_minor_axis = 0.0;
	eccentricity = 0.0;
	inclination = 0.0;
	argument_of_perigee = 0.0;
	ground_speed = 0.0;
	longitude_of_ascending_node = 0.0;
	true_anomaly = 0.0;
	obliquity_to_orbit = 0.0;
	sidereal_rotation_period = 0.0;

	angulos_x = 0.0;
	angulos_y = 0.0;
	angulos_z = 0.0;

	equatorial_radius = 0.0;
	polar_radius = 0.0;
	flattening = 0.0;
	initial_rotation = 0.0;
	approach_dist = 0.f;
	g_acceleration = 0.0;
	ME_current_power = 0.0;
	Hover_current_power = 0.0;
	length_ = 0.f;
	width_ = 0.f;
	height_ = 0.f;
	wing_span = 0.f;
	FrontFriction = 0.f;
	top_area = 0.f;
	front_area = 0.f;
	side_area = 0.f;
	min_lift = 0.f;
	max_lift = 0.f;

	main_engine_power = 0.0;
	rcs_power = 0.0;
	hover_power = 0.0;
	fuel_capacity = 0.f;
	fuel = 0.f;
	rcs_capacity = 0.f;
	rcs = 0.f;
	o2_capacity = 0.f;
	o2 = 0.f;
	battery_capacity = 0.f;
	battery = 0.f;
	battery_charge_rate = 0.f;
	ground_sensor_distance = 0.f;
	total_docking_ports = 0;
	total_grapple_fixtures = 0;
	total_engines = 0;
	total_hover_engines = 0;
	total_rcs_nozzles = 0;

	docking_port_position = nullptr;
	docking_port_orientation = nullptr;
	dockingPortFree = nullptr;

	grapple_fixture_position = nullptr;
	grapple_fixture_orientation = nullptr;

	Engine = nullptr;
	Hover_Engine = nullptr;
	RCS_Nozzle = nullptr;

	animation = nullptr;

	ForwardNozzle = nullptr;
	BackwardNozzle = nullptr;
	UpNozzle = nullptr;
	DownNozzle = nullptr;
	RightNozzle = nullptr;
	LeftNozzle = nullptr;

	PositiveXnozzle = nullptr;
	PositiveYnozzle = nullptr;
	PositiveZnozzle = nullptr;

	NegativeXnozzle = nullptr;
	NegativeYnozzle = nullptr;
	NegativeZnozzle = nullptr;

	cockpit_camera = Vector3{ 0.f, 0.f, 0.f };
	sea_level_pressure = 0.f;
	sea_level_temperature = 0.f;
	sea_level_density = 0.f;

	World_ = Matrix::Identity;

	semi_latus_rectum = 0.0;
	orbital_period = 0.0;
	apogee = 0.0;
	perigee = 0.0;
	orbit_radius = 0.0;
	perigee_velocity = 0.0;
	apogee_velocity = 0.0;
	eccentric_anomaly = 0.0;
	mean_anomaly = 0.0;
	mean_motion = 0.0;
	time_since_perigee = 0.0;
	time_to_perigee = 0.0;
	time_to_apogee = 0.0;
	distance_to_apogee = 0.0;
	distance_to_perigee = 0.0;
	longitude = 0.0;
	latitude = 0.0;

	scale_b = 0.0f;
	distance_to_camera = 0.0f;
	atmosphere_ratio = 0.0f;
	clouds_ratio = 0.0f;
	inner_ring = 1.0f;
	outer_ring = 1.0f;

	ilumination = -1;
}

objectClass::objectClass(const objectClass& other)
{
	texture1 = other.texture1;
	texture2 = other.texture2;
	texture3 = other.texture3;
	type = other.type;
	collision_shape_type = other.collision_shape_type;
	affects = other.affects;
	copy_number = other.copy_number;
	orbiting = other.orbiting;
	model = other.model;
	docked_to = other.docked_to;
	attached_to = other.attached_to;
	ring_object = other.ring_object;
	clouds_object = other.clouds_object;
	target_ = other.target_;
	constraint_type = other.constraint_type;
	constraint_axis = other.constraint_axis;
	min_constraint_move = other.min_constraint_move;
	max_constraint_move = other.max_constraint_move;
	current_constraint_move = other.current_constraint_move;
	engine_color_start = other.engine_color_start;
	engine_color_end = other.engine_color_end;

	highlighted = other.highlighted;
	robotic_arm_dport = other.robotic_arm_dport;
	visual_helpers = other.visual_helpers;
	docking_data_updated = other.docking_data_updated;

	camera_prefered_axis = other.camera_prefered_axis;

	local_fixture = other.local_fixture;
	target_fixture = other.target_fixture;

	stages_file = other.stages_file;
	stage = other.stage;
	top_absolute_parent = other.top_absolute_parent;
	top_artificial_parent = other.top_artificial_parent;
	fuel_consumption = other.fuel_consumption;

	in_bullet = other.in_bullet;
	in_virtual_bullet = other.in_virtual_bullet;
	bullet_id = other.bullet_id;
	bullet_constrained = other.bullet_constrained;
	bullet_constrained_to = other.bullet_constrained_to;

	casts_shadow = other.casts_shadow;
	landed = other.landed;
	ephemeris_OK = other.ephemeris_OK;
	visible = other.visible;
	has_atmosphere = other.has_atmosphere;
	has_clouds = other.has_clouds;
	has_light_map = other.has_light_map;
	has_night_map = other.has_night_map;
	has_specular_map = other.has_specular_map;
	has_rings = other.has_rings;
	loaded_tm = other.loaded_tm;
	kill_rotation = other.kill_rotation;
	auto_grapple = other.auto_grapple;
	h_level = other.h_level;
	radial_in = other.radial_in;
	radial_out = other.radial_out;
	pro_grade = other.pro_grade;
	retro_grade = other.retro_grade;
	normal = other.normal;
	anti_normal = other.anti_normal;
	lock_target = other.lock_target;
	match_speed = other.match_speed;
	approach = other.approach;
	retro_diff = other.retro_diff;
	AlignToTargetsUp = other.AlignToTargetsUp;
	kill_rotationX = other.kill_rotationX;
	kill_rotationY = other.kill_rotationY;
	kill_rotationZ = other.kill_rotationZ;
	dock_ready = other.dock_ready;
	landing_gear = other.landing_gear;
	velocity_modified = other.velocity_modified;
	quick_Hover_Burst = other.quick_Hover_Burst;
	quick_Me_Burst = other.quick_Me_Burst;
	in_orbit = other.in_orbit;
	rocket_engine = other.rocket_engine;
	sub_type = other.sub_type;

	is_parent = other.is_parent;
	horizontal_dir = other.horizontal_dir;

	orbited_name = other.orbited_name;
	target_name = other.target_name;
	docked_to_name = other.docked_to_name;

	collision_shape_file = other.collision_shape_file;

	total_animations = other.total_animations;

	center_of_mass = other.center_of_mass;

	in_atmosphere = other.in_atmosphere;
	g_acceleration = other.g_acceleration;
	sea_level_pressure = other.sea_level_pressure;
	sea_level_temperature = other.sea_level_temperature;
	sea_level_density = other.sea_level_density;

	sunset_color = other.sunset_color;
	daylight_color = other.daylight_color;
	atmosphere_thickness = other.atmosphere_thickness;

	gimbal = other.gimbal;
	gimbal_angle = other.gimbal_angle;
	has_animations = other.has_animations;

	rcs_traslation_enabled = other.rcs_traslation_enabled;

	position = other.position;
	docked_former_position = other.docked_former_position;
	velocity = other.velocity;
	movimiento_total = other.movimiento_total;
	orbital_position = other.orbital_position;
	ground_speed_vec = other.ground_speed_vec;
	orbital_velocity = other.orbital_velocity;
	specific_angular_momentum = other.specific_angular_momentum;
	node_line = other.node_line;
	unit_vector_to_ascending_node = other.unit_vector_to_ascending_node;
	eccentricity_vector = other.eccentricity_vector;
	docked_position = other.docked_position;
	original_docked_position = other.original_docked_position;
	docked_position_b = other.docked_position_b;
	aerodynamic_controls = other.aerodynamic_controls;

	attached_position = other.attached_position;

	position_b = other.position_b;
	position_bullet = other.position_bullet;
	position_bullet_b = other.position_bullet_b;
	velocity_b = other.velocity_b;
	velocity_c = other.velocity_c;
	position_c = other.position_c;

	destroyed = other.destroyed;

	total_rotation = other.total_rotation;
	original_docked_rotation = other.original_docked_rotation;
	inherited_rotation = other.inherited_rotation;

	docked_rotation_x = other.docked_rotation_x;
	docked_rotation_y = other.docked_rotation_y;
	docked_rotation_z = other.docked_rotation_z;

	Forward_on = other.Forward_on;
	Backward_on = other.Backward_on;
	Up_on = other.Up_on;
	Down_on = other.Down_on;
	Right_on = other.Right_on;
	Left_on = other.Left_on;

	x_positive_rot_on = other.x_positive_rot_on;
	y_positive_rot_on = other.y_positive_rot_on;
	z_positive_rot_on = other.z_positive_rot_on;

	x_negative_rot_on = other.x_negative_rot_on;
	y_negative_rot_on = other.y_negative_rot_on;
	z_negative_rot_on = other.z_negative_rot_on;

	Forward_power = other.Forward_power;
	Backward_power = other.Backward_power;
	Up_power = other.Up_power;
	Down_power = other.Down_power;
	Right_power = other.Right_power;
	Left_power = other.Left_power;

	x_positive_power = other.x_positive_power;
	y_positive_power = other.y_positive_power;
	z_positive_power = other.z_positive_power;

	x_negative_power = other.x_negative_power;
	y_negative_power = other.y_negative_power;
	z_negative_power = other.z_negative_power;

	name = other.name;
	original_name = other.original_name;
	rings_texture = other.rings_texture;

	scale = other.scale;
	mass = other.mass;
	empty_mass = other.empty_mass;
	semi_mayor_axis = other.semi_mayor_axis;
	semi_minor_axis = other.semi_minor_axis;
	eccentricity = other.eccentricity;
	inclination = other.inclination;
	argument_of_perigee = other.argument_of_perigee;
	longitude_of_ascending_node = other.longitude_of_ascending_node;
	true_anomaly = other.true_anomaly;
	obliquity_to_orbit = other.obliquity_to_orbit;
	sidereal_rotation_period = other.sidereal_rotation_period;

	angulos_x = other.angulos_x;
	angulos_y = other.angulos_y;
	angulos_z = other.angulos_z;

	equatorial_radius = other.equatorial_radius;
	polar_radius = other.polar_radius;
	initial_rotation = other.initial_rotation;
	semi_latus_rectum = other.semi_latus_rectum;
	orbital_period = other.orbital_period;
	apogee = other.apogee;
	perigee = other.perigee;
	orbit_radius = other.orbit_radius;
	perigee_velocity = other.perigee_velocity;
	apogee_velocity = other.apogee_velocity;
	eccentric_anomaly = other.eccentric_anomaly;
	mean_anomaly = other.mean_anomaly;
	mean_motion = other.mean_motion;
	time_since_perigee = other.time_since_perigee;
	time_to_perigee = other.time_to_perigee;
	time_to_apogee = other.time_to_apogee;
	distance_to_apogee = other.distance_to_apogee;
	distance_to_perigee = other.distance_to_perigee;
	latitude = other.latitude;
	longitude = other.longitude;
	ground_speed = other.ground_speed;
	ME_current_power = other.ME_current_power;
	Hover_current_power = other.Hover_current_power;
	min_lift = other.min_lift;
	max_lift = other.max_lift;

	angulo_total_percent = other.angulo_total_percent;
	medio_angulo_percent = other.medio_angulo_percent;
	area_total = other.area_total;
	area_frontal = other.area_frontal;
	area_lateral = other.area_lateral;
	medio_angulo = other.medio_angulo;
	friccion = other.friccion;
	vv = other.vv;
	atmospheric_pressure = other.atmospheric_pressure;
	air_density = other.air_density;
	altitude = other.altitude;
	true_air_speed = other.true_air_speed;
	dynamic_pressure = other.dynamic_pressure;
	Drag = other.Drag;
	true_velocity = other.true_velocity;
	true_lift_vec = other.true_lift_vec;

	heading = other.heading;

	sunlit = other.sunlit;

	length_ = other.length_;
	width_ = other.width_;
	height_ = other.height_;
	FrontFriction = other.FrontFriction;
	top_area = other.top_area;
	front_area = other.front_area;
	side_area = other.side_area;

	main_engine_power = other.main_engine_power;
	rcs_power = other.rcs_power;
	hover_power = other.hover_power;
	wing_span = other.wing_span;
	approach_dist = other.approach_dist;

	fuel_capacity = other.fuel_capacity;
	fuel = other.fuel;
	rcs_capacity = other.rcs_capacity;
	rcs = other.rcs;
	o2_capacity = other.o2_capacity;
	o2 = other.o2;
	battery_capacity = other.battery_capacity;
	battery = other.battery;
	battery_charge_rate = other.battery_charge_rate;
	ground_sensor_distance = other.ground_sensor_distance;

	total_docking_ports = other.total_docking_ports;
	total_grapple_fixtures = other.total_grapple_fixtures;
	total_engines = other.total_engines;
	total_hover_engines = other.total_hover_engines;
	total_rcs_nozzles = other.total_rcs_nozzles;

	total_ForwardNozzles = other.total_ForwardNozzles;
	total_BackwardNozzles = other.total_BackwardNozzles;
	total_UpNozzles = other.total_UpNozzles;
	total_DownNozzles = other.total_DownNozzles;
	total_RightNozzles = other.total_RightNozzles;
	total_LeftNozzles = other.total_LeftNozzles;

	total_PositiveXNozzles = other.total_PositiveXNozzles;
	total_PositiveYNozzles = other.total_PositiveYNozzles;
	total_PositiveZNozzles = other.total_PositiveZNozzles;

	total_NegativeXNozzles = other.total_NegativeXNozzles;
	total_NegativeYNozzles = other.total_NegativeYNozzles;
	total_NegativeZNozzles = other.total_NegativeZNozzles;

	cockpit_camera = other.cockpit_camera;

	if (total_animations > 0)
	{
		Inititalize_animations(total_animations);
		for (int n = 0; n < total_animations; n++)
		{
			animation[n].total_moves = other.animation[n].total_moves;
			animation[n].active = other.animation[n].active;
			animation[n].name = other.animation[n].name;
			animation[n].activated = other.animation[n].activated;
			animation[n].available = other.animation[n].available;
			animation[n].modify_total = other.animation[n].modify_total;
			animation[n].time_ = other.animation[n].time_;

			if (animation[n].total_moves > 0 ) animation[n].Initizalize_moves(animation[n].total_moves);
			for (int nn = 0; nn < animation[n].total_moves; nn++)
			{
				animation[n].HandleObject[nn].end_seconds = other.animation[n].HandleObject[nn].end_seconds;
				animation[n].HandleObject[nn].object_name = other.animation[n].HandleObject[nn].object_name;
				animation[n].HandleObject[nn].rotation_x = other.animation[n].HandleObject[nn].rotation_x;
				animation[n].HandleObject[nn].rotation_y = other.animation[n].HandleObject[nn].rotation_y;
				animation[n].HandleObject[nn].rotation_z = other.animation[n].HandleObject[nn].rotation_z;

				animation[n].HandleObject[nn].start_seconds = other.animation[n].HandleObject[nn].start_seconds;

				animation[n].HandleObject[nn].traslation_x = other.animation[n].HandleObject[nn].traslation_x;
				animation[n].HandleObject[nn].traslation_y = other.animation[n].HandleObject[nn].traslation_y;
				animation[n].HandleObject[nn].traslation_z = other.animation[n].HandleObject[nn].traslation_z;
			}

			if (animation[n].modify_total > 0) animation[n].Initizalize_other_animations_modification(animation[n].modify_total);
			for (int nn = 0; nn<animation[n].modify_total; nn++)
			{
				animation[n].modify_animation[nn].animation_name = other.animation[n].modify_animation[nn].animation_name;
				animation[n].modify_animation[nn].available = other.animation[n].modify_animation[nn].available;
			}
		}
	}

	if (total_docking_ports > 0)
	{
		docking_port_position = new Vector3[total_docking_ports];
		dockingPortFree = new bool[total_docking_ports];
		docking_port_orientation = new Vector3[total_docking_ports];

		for (int n = 0; n < total_docking_ports; n++)
		{
			docking_port_position[n] = other.docking_port_position[n];
			dockingPortFree[n] = other.dockingPortFree[n];
			docking_port_orientation[n] = other.docking_port_orientation[n];
		}
	}

	if (total_grapple_fixtures > 0)
	{
		grapple_fixture_position = new Vector3[total_grapple_fixtures];
		grapple_fixture_orientation = new Vector3[total_grapple_fixtures];

		for (int n = 0; n < total_grapple_fixtures; n++)
		{
			grapple_fixture_position[n] = other.grapple_fixture_position[n];
			grapple_fixture_orientation[n] = other.grapple_fixture_orientation[n];
		}
	}

	if (total_engines > 0)
	{
		Engine = new engineStruct[total_engines];
		for (int n = 0; n < total_engines; n++)
		{
			Engine[n].position = other.Engine[n].position;
			Engine[n].size = other.Engine[n].size;
			Engine[n].depth = other.Engine[n].depth;
		}
	}

	if (total_hover_engines > 0)
	{
		Hover_Engine = new engineStruct[total_hover_engines];
		for (int n = 0; n < total_hover_engines; n++)
		{
			Hover_Engine[n].position = other.Hover_Engine[n].position;
			Hover_Engine[n].size = other.Hover_Engine[n].size;
			Hover_Engine[n].depth = other.Hover_Engine[n].depth;
		}
	}

	if (total_rcs_nozzles > 0)
	{
		RCS_Nozzle = new engineStruct[total_rcs_nozzles];
		for (int n = 0; n < total_rcs_nozzles; n++)
		{
			RCS_Nozzle[n].position = other.RCS_Nozzle[n].position;
			RCS_Nozzle[n].orientation = other.RCS_Nozzle[n].orientation;
			RCS_Nozzle[n].size = other.RCS_Nozzle[n].size;
			RCS_Nozzle[n].depth = other.RCS_Nozzle[n].depth;
		}
	}

	if (total_ForwardNozzles > 0)
	{
		ForwardNozzle = new int[total_ForwardNozzles];
		for (int n = 0; n < total_ForwardNozzles; n++)
		{
			ForwardNozzle[n] = other.ForwardNozzle[n];
		}
	}

	if (total_BackwardNozzles > 0)
	{
		BackwardNozzle = new int[total_BackwardNozzles];
		for (int n = 0; n < total_BackwardNozzles; n++)
		{
			BackwardNozzle[n] = other.BackwardNozzle[n];
		}
	}

	if (total_UpNozzles > 0)
	{
		UpNozzle = new int[total_UpNozzles];
		for (int n = 0; n < total_UpNozzles; n++)
		{
			UpNozzle[n] = other.UpNozzle[n];
		}
	}

	if (total_DownNozzles > 0)
	{
		DownNozzle = new int[total_DownNozzles];
		for (int n = 0; n < total_DownNozzles; n++)
		{
			DownNozzle[n] = other.DownNozzle[n];
		}
	}

	if (total_RightNozzles > 0)
	{
		RightNozzle = new int[total_RightNozzles];
		for (int n = 0; n < total_RightNozzles; n++)
		{
			RightNozzle[n] = other.RightNozzle[n];
		}
	}

	if (total_LeftNozzles > 0)
	{
		LeftNozzle = new int[total_LeftNozzles];
		for (int n = 0; n < total_LeftNozzles; n++)
		{
			LeftNozzle[n] = other.LeftNozzle[n];
		}
	}

	if (total_PositiveXNozzles > 0)
	{
		PositiveXnozzle = new int[total_PositiveXNozzles];
		for (int n = 0; n < total_PositiveXNozzles; n++)
		{
			PositiveXnozzle[n] = other.PositiveXnozzle[n];
		}
	}

	if (total_PositiveYNozzles > 0)
	{
		PositiveYnozzle = new int[total_PositiveYNozzles];
		for (int n = 0; n < total_PositiveYNozzles; n++)
		{
			PositiveYnozzle[n] = other.PositiveYnozzle[n];
		}
	}

	if (total_PositiveZNozzles > 0)
	{
		PositiveZnozzle = new int[total_PositiveZNozzles];
		for (int n = 0; n < total_PositiveZNozzles; n++)
		{
			PositiveZnozzle[n] = other.PositiveZnozzle[n];
		}
	}


	if (total_NegativeXNozzles > 0)
	{
		NegativeXnozzle = new int[total_NegativeXNozzles];
		for (int n = 0; n < total_NegativeXNozzles; n++)
		{
			NegativeXnozzle[n] = other.NegativeXnozzle[n];
		}
	}

	if (total_NegativeYNozzles > 0)
	{
		NegativeYnozzle = new int[total_NegativeYNozzles];
		for (int n = 0; n < total_NegativeYNozzles; n++)
		{
			NegativeYnozzle[n] = other.NegativeYnozzle[n];
		}
	}

	if (total_NegativeZNozzles > 0)
	{
		NegativeZnozzle = new int[total_NegativeZNozzles];
		for (int n = 0; n < total_NegativeZNozzles; n++)
		{
			NegativeZnozzle[n] = other.NegativeZnozzle[n];
		}
	}

	flattening = other.flattening;
	scale_b = other.scale_b;
	scale_c = other.scale_c;
	distance_to_camera = other.distance_to_camera;
	ilumination = other.ilumination;
	atmosphere_ratio = other.atmosphere_ratio;
	clouds_ratio = other.clouds_ratio;
	inner_ring = other.inner_ring;
	outer_ring = other.outer_ring;

	World_ = other.World_;

}

objectClass::~objectClass()
{
	if (total_docking_ports > 0)
	{
		SAFE_DELETE_ARRAY(docking_port_position);
		SAFE_DELETE_ARRAY(dockingPortFree);
		SAFE_DELETE_ARRAY(docking_port_orientation);
	}

	if (total_grapple_fixtures > 0)
	{
		SAFE_DELETE_ARRAY(grapple_fixture_position);
		SAFE_DELETE_ARRAY(grapple_fixture_orientation);
	}

	if (total_animations > 0)
	{
		SAFE_DELETE_ARRAY(animation);
	}

	if (total_ForwardNozzles > 0) SAFE_DELETE_ARRAY(ForwardNozzle);
	if (total_BackwardNozzles > 0) SAFE_DELETE_ARRAY(BackwardNozzle);
	if (total_UpNozzles > 0) SAFE_DELETE_ARRAY(UpNozzle);
	if (total_DownNozzles > 0) SAFE_DELETE_ARRAY(DownNozzle);
	if (total_RightNozzles > 0) SAFE_DELETE_ARRAY(RightNozzle);
	if (total_LeftNozzles > 0) SAFE_DELETE_ARRAY(LeftNozzle);

	if (total_PositiveXNozzles > 0) SAFE_DELETE_ARRAY(PositiveXnozzle);
	if (total_PositiveYNozzles > 0) SAFE_DELETE_ARRAY(PositiveYnozzle);
	if (total_PositiveZNozzles > 0) SAFE_DELETE_ARRAY(PositiveZnozzle);

	if (total_NegativeXNozzles > 0) SAFE_DELETE_ARRAY(NegativeXnozzle);
	if (total_NegativeYNozzles > 0) SAFE_DELETE_ARRAY(NegativeYnozzle);
	if (total_NegativeZNozzles > 0) SAFE_DELETE_ARRAY(NegativeZnozzle);

	if (total_engines > 0) SAFE_DELETE_ARRAY(Engine);
	if (total_hover_engines > 0) SAFE_DELETE_ARRAY(Hover_Engine);
	if (total_rcs_nozzles > 0) SAFE_DELETE_ARRAY(RCS_Nozzle);
}

void objectClass::delete_docking_ports()
{
	if (total_docking_ports > 0)
	{
		SAFE_DELETE_ARRAY(docking_port_position);
		SAFE_DELETE_ARRAY(dockingPortFree);
		SAFE_DELETE_ARRAY(docking_port_orientation);
	}
}

void objectClass::delete_grapple_fixtures()
{
	if (total_docking_ports > 0)
	{
		SAFE_DELETE_ARRAY(grapple_fixture_position);
		SAFE_DELETE_ARRAY(grapple_fixture_orientation);
	}
}


void objectClass::Rotate(quat total_rotation_of_docked_to, double dtime)
{
	quat q_temp = { 0.0, 0.0, 0.0, 1.0 };
	double angle = angulos_x *dtime;
	q_temp.x = 1.0 * sin(angle / 2.0);
	q_temp.w = cos(angle / 2.0);
	if (docked_to != -1) inherited_rotation = q_temp* inherited_rotation;
	else total_rotation = q_temp* total_rotation;

	q_temp = { 0.0, 0.0, 0.0, 1.0 };
	angle = angulos_y *dtime;
	q_temp.y = 1.0 * sin(angle / 2.0);
	q_temp.w = cos(angle / 2.0);
	if (docked_to != -1) inherited_rotation = q_temp* inherited_rotation;
	else total_rotation = q_temp* total_rotation;

	q_temp = { 0.0, 0.0, 0.0, 1.0 };
	angle = angulos_z *dtime;
	q_temp.z = 1.0 * sin(angle / 2.0);
	q_temp.w = cos(angle / 2.0);
	if (docked_to != -1) inherited_rotation = q_temp* inherited_rotation;
	else total_rotation = q_temp* total_rotation;

	if (docked_to != -1) total_rotation = inherited_rotation* total_rotation_of_docked_to;
}

void objectClass::ConvertCartesianToKeplerianElements(vec3 orbited_body_position, vec3 orbited_body_velocity, double orbited_body_mass)
{

	double central_body_gravitational_parameter = GRAVITATIONAL_CONSTANT*orbited_body_mass;
	double tolerance = 1.0e-15;
	orbital_position = position - orbited_body_position;
	orbital_velocity = velocity - orbited_body_velocity;

	specific_angular_momentum = orbital_position ^ orbital_velocity;
	semi_latus_rectum = (length(specific_angular_momentum)*length(specific_angular_momentum)) / central_body_gravitational_parameter;
	node_line = vec3{ 0.0, 0.0, 1.0 }^ specific_angular_momentum;
	unit_vector_to_ascending_node = normalized(node_line);
	eccentricity_vector = ((orbital_velocity^ specific_angular_momentum) / central_body_gravitational_parameter) - normalized(orbital_position);
	eccentricity = length(eccentricity_vector);

	if (fabs(eccentricity - 1.0) < tolerance)
	{
		semi_latus_rectum = semi_latus_rectum;
	}

	// Else the orbit is either elliptical or hyperbolic, so store the semi-major axis.
	else
	{
		semi_mayor_axis = semi_latus_rectum / (1.0 - eccentricity * eccentricity);
		semi_minor_axis = semi_mayor_axis * sqrt(1.0 - (eccentricity * eccentricity));
	}

	inclination = acos(specific_angular_momentum.z / length(specific_angular_momentum));

	double argument_of_periapsis_quadrant_condition = eccentricity_vector.z;

	// Check if the orbit is equatorial. If it is, set the vector to the line of nodes to the
	// x-axis.
	if (fabs(inclination) < tolerance)
	{
		unit_vector_to_ascending_node = { 1.0, 0.0, 0.0 };

		// If the orbit is equatorial, eccentricity_vector_.y( ) is zero, therefore the quadrant
		// condition is taken to be the y-component, eccentricity_vector_.z( ).
		argument_of_periapsis_quadrant_condition = eccentricity_vector.y;
	}

	// Compute and store the resulting longitude of ascending node.
	longitude_of_ascending_node = acos(unit_vector_to_ascending_node.x);

	// Check if the quadrant is correct.
	if (unit_vector_to_ascending_node.y < 0.0)
	{
		longitude_of_ascending_node = 2.0 * phi_d - longitude_of_ascending_node;
	}

	double true_anomaly_quadrant_condition = orbital_position* orbital_velocity;


	// Check if the orbit is equatorial. If it is, set the vector to the line of nodes to the
	// x-axis.
	if (fabs(inclination) < tolerance)
	{
		unit_vector_to_ascending_node = { 1.0, 0.0, 0.0 };

		// If the orbit is equatorial, eccentricity_vector_.y( ) is zero, therefore the quadrant
		// condition is taken to be the y-component, eccentricity_vector_.z( ).
		argument_of_periapsis_quadrant_condition = eccentricity_vector.y;
	}

	// Compute and store the resulting longitude of ascending node.
	longitude_of_ascending_node = acos(unit_vector_to_ascending_node.x);

	// Check if the quadrant is correct.
	if (unit_vector_to_ascending_node.y < 0.0)
	{
		longitude_of_ascending_node = 2.0 * phi_d - longitude_of_ascending_node;
	}

	// Compute and store argument of periapsis.
	// Define the quadrant condition for the true anomaly.
	true_anomaly_quadrant_condition = orbital_position* orbital_velocity;

	// Check if the orbit is circular. If it is, set the eccentricity vector to unit vector
	// pointing to the ascending node, i.e. set the argument of periapsis to zero.
	if (fabs(eccentricity) < tolerance)
	{
		eccentricity_vector = unit_vector_to_ascending_node;

		argument_of_perigee = 0.0;

		// Check if orbit is also equatorial and set true anomaly quadrant check condition
		// accordingly.
		if (unit_vector_to_ascending_node.x == 1.0 && unit_vector_to_ascending_node.z == 0.0 && unit_vector_to_ascending_node.y == 0.0)
		{
			// If the orbit is circular, position_.dot( velocity_ ) = 0, therefore this value
			// cannot be used as a quadrant condition. Moreover, if the orbit is equatorial,
			// position_.y( ) is also zero and therefore the quadrant condition is taken to be the
			// y-component, position_.z( ).
			true_anomaly_quadrant_condition = orbital_position.y;
		}

		else
		{
			// If the orbit is circular, position_.dot( velocity_ ) = 0, therefore the quadrant
			// condition is taken to be the z-component of the position, position_.y( ).
			true_anomaly_quadrant_condition = orbital_position.z;
		}
	}

	// Else, compute the argument of periapsis as the angle between the eccentricity vector and
	// the unit vector to the ascending node.
	else
	{
		argument_of_perigee = acos(normalized(eccentricity_vector)* unit_vector_to_ascending_node);

		// Check if the quadrant is correct.
		if (argument_of_periapsis_quadrant_condition < 0.0)
		{
			argument_of_perigee = 2.0 * phi_d - argument_of_perigee;
		}
	}

	// Compute dot-product of position and eccentricity vectors.
	double dotProductpositionAndeccentricity_vectors = normalized(orbital_position)* normalized(eccentricity_vector);

	// Check if the dot-product is one of the limiting cases: 0.0 or 1.0
	// (within prescribed tolerance).
	if (fabs(1.0 - dotProductpositionAndeccentricity_vectors) < tolerance)
	{
		dotProductpositionAndeccentricity_vectors = 1.0;
	}

	if (fabs(dotProductpositionAndeccentricity_vectors) < tolerance)
	{
		dotProductpositionAndeccentricity_vectors = 0.0;
	}

	// Compute and store true anomaly.
	true_anomaly = acos(dotProductpositionAndeccentricity_vectors);

	// Check if the quadrant is correct.
	if (true_anomaly_quadrant_condition < 0.0)
	{
		true_anomaly = 2.0 * phi_d - true_anomaly;
	}

	orbital_period = (2.0 * phi_d)*sqrt(((semi_mayor_axis * semi_mayor_axis * semi_mayor_axis) / central_body_gravitational_parameter)); // given in seconds
	apogee = (1.0 + eccentricity)*semi_mayor_axis;
	perigee = (1.0 - eccentricity)*semi_mayor_axis;

	orbit_radius = length(orbital_position);

	perigee_velocity = sqrt(((1.0 + eccentricity)*central_body_gravitational_parameter) / ((1.0 - eccentricity)*semi_mayor_axis));
	apogee_velocity = sqrt(((1.0 - eccentricity)*central_body_gravitational_parameter) / ((1.0 + eccentricity)*semi_mayor_axis));

	double qq = orbital_position* orbital_velocity;
	double ex = 1.0 - (orbit_radius / semi_mayor_axis);
	double ey = qq / sqrt(semi_mayor_axis * central_body_gravitational_parameter);
	eccentric_anomaly = atan2(ey, ex);

	mean_anomaly = eccentric_anomaly - eccentricity * sin(eccentric_anomaly);
	mean_motion = (2.0 * phi_d) / orbital_period;

	time_since_perigee = mean_anomaly * orbital_period / (2.0 * phi_d);
	if (true_anomaly > ToRadians_d(180.0)) time_since_perigee = orbital_period + time_since_perigee;

	time_to_perigee = orbital_period - time_since_perigee;
	time_to_apogee = (orbital_period / 2.0) - time_since_perigee;
	if (true_anomaly > ToRadians_d(180.0)) time_to_apogee = orbital_period + time_to_apogee;

	distance_to_apogee = length(orbital_velocity) * time_to_apogee;
	distance_to_perigee = length(orbital_velocity) * time_to_perigee;
}

void objectClass::ConvertKeplerianToCartesianElements(double central_body_gravitational_parameter)
{
	vec2 position_perifocal_;
	vec2 velocity_perifocal_;

	// Set tolerance.
	const double tolerance_ = std::numeric_limits< double >::epsilon();

	// Pre-compute sines and cosines of involved angles for efficient computation.
	double cosineOfinclination_ = cos(inclination);
	double sineOfinclination_ = sin(inclination);
	double cosineOfArgumentOfPeriapsis_ = cos(argument_of_perigee);
	double sineOfArgumentOfPeriapsis_ = sin(argument_of_perigee);
	double cosineOflongitude_of_ascending_node_ = cos(longitude_of_ascending_node);
	double sineOflongitude_of_ascending_node_ = sin(longitude_of_ascending_node);
	double cosineOftrue_anomaly_ = cos(true_anomaly);
	double sineOftrue_anomaly_ = sin(true_anomaly);

	// Declare semi-latus rectum.
	double semi_latus_rectum_ = -0.0;

	// Compute semi-latus rectum in the case it is not a parabola.
	if (fabs(eccentricity - 1.0) > tolerance_)
	{
		semi_latus_rectum_ = semi_mayor_axis * (1.0 - (eccentricity* eccentricity));
	}

	// Else set the semi-latus rectum given for a parabola as the first element in the vector
	// of Keplerian elements..
	else  { semi_latus_rectum_ = semi_mayor_axis; }

	// Definition of position in the perifocal coordinate system.
	position_perifocal_.x = semi_latus_rectum_ * cosineOftrue_anomaly_
		/ (1.0 + eccentricity * cosineOftrue_anomaly_);
	position_perifocal_.y = semi_latus_rectum_ * sineOftrue_anomaly_
		/ (1.0 + eccentricity * cosineOftrue_anomaly_);

	// Definition of velocity in the perifocal coordinate system.
	velocity_perifocal_ = {
		-sqrt(central_body_gravitational_parameter / semi_latus_rectum_) * sineOftrue_anomaly_,
		sqrt(central_body_gravitational_parameter / semi_latus_rectum_)
		* (eccentricity + cosineOftrue_anomaly_) };

	// Definition of the transformation matrix.
	double transformation_matrix_[3][2];

	// Compute the transformation matrix.
	transformation_matrix_[0][0] = cosineOflongitude_of_ascending_node_* cosineOfArgumentOfPeriapsis_ - sineOflongitude_of_ascending_node_* sineOfArgumentOfPeriapsis_ * cosineOfinclination_;
	transformation_matrix_[0][1] = -cosineOflongitude_of_ascending_node_* sineOfArgumentOfPeriapsis_ - sineOflongitude_of_ascending_node_* cosineOfArgumentOfPeriapsis_ * cosineOfinclination_;
	transformation_matrix_[1][0] = sineOflongitude_of_ascending_node_* cosineOfArgumentOfPeriapsis_ + cosineOflongitude_of_ascending_node_* sineOfArgumentOfPeriapsis_ * cosineOfinclination_;
	transformation_matrix_[1][1] = -sineOflongitude_of_ascending_node_* sineOfArgumentOfPeriapsis_ + cosineOflongitude_of_ascending_node_* cosineOfArgumentOfPeriapsis_ * cosineOfinclination_;
	transformation_matrix_[2][0] = sineOfArgumentOfPeriapsis_ * sineOfinclination_;
	transformation_matrix_[2][1] = cosineOfArgumentOfPeriapsis_ * sineOfinclination_;

	// Compute value of position in Cartesian coordinates.
	position = matrix32_vec2_multiplication(transformation_matrix_, position_perifocal_);

	// Compute value of velocity in Cartesian coordinates.
	velocity = matrix32_vec2_multiplication(transformation_matrix_, velocity_perifocal_);

}

void objectClass::UpdateCustomData(vec3 orbited_body_position, double orbited_body_scale, quat current_rotation)
{
	vec3 current_position = normalized(position - orbited_body_position)*(orbited_body_scale / 2.0);
	current_position = vector_rotation_by_quaternion(current_position, current_rotation);

	double altitud_ = length(position - orbited_body_position);
	
	vv = vs * time_acceleration;

	seconds_check += dt_a;

	if (seconds_check >= 1.0f)
	{
		seconds_check = 0.0f;

		// ground speed data
		ground_speed_vec = (current_position - former_position) / time_acceleration;
		ground_speed = length(ground_speed_vec);

		former_position = current_position;

		// vv data
		vs = former_altitud - altitud_;
		former_altitud = altitud_;
	}
}

void objectClass::InitializeDockingPorts(int total_ports)
{
	docking_port_position = new Vector3[total_ports];
	docking_port_orientation = new Vector3[total_ports];
	dockingPortFree = new bool[total_ports];

	for (int n = 0; n < total_ports; n++)
	{
		docking_port_position[n] = Vector3{ 0.f, 0.f, 0.f };
		docking_port_orientation[n] = Vector3{ 0.f, 0.f, 0.f };
		dockingPortFree[n] = true;
	}
}

void objectClass::InitializeGrappleFixtures(int total_fixtures)
{
	grapple_fixture_position = new Vector3[total_fixtures];
	grapple_fixture_orientation = new Vector3[total_fixtures];

	for (int n = 0; n < total_fixtures; n++)
	{
		grapple_fixture_position[n] = Vector3{ 0.f, 0.f, 0.f };
		grapple_fixture_orientation[n] = Vector3{ 0.f, 0.f, 0.f };
	}
}

void objectClass::InitializeEngines(int total_engines_)
{
	Engine = new engineStruct[total_engines_];

	for (int n = 0; n < total_engines_; n++)
	{
		Engine[n].position = Vector3{ 0.f, 0.f, 0.f };
		Engine[n].depth = 0.f;
		Engine[n].size = 0.f;
	}
}

void objectClass::InitializeHoverEngines(int total_hover_engines_)
{
	Hover_Engine = new engineStruct[total_hover_engines_];

	for (int n = 0; n < total_hover_engines_; n++)
	{
		Hover_Engine[n].position = Vector3{ 0.f, 0.f, 0.f };
		Hover_Engine[n].depth = 0.f;
		Hover_Engine[n].size = 0.f;
	}
}

void objectClass::InitializeRCSNozzles(int total_rcs_nozzles_)
{
	RCS_Nozzle = new engineStruct[total_rcs_nozzles_];

	for (int n = 0; n < total_rcs_nozzles_; n++)
	{
		RCS_Nozzle[n].position = Vector3{ 0.f, 0.f, 0.f };
		RCS_Nozzle[n].orientation = Vector3{ 0.f, -1.f, 0.f };
		RCS_Nozzle[n].depth = 0.f;
		RCS_Nozzle[n].size = 0.f;
	}
}

void objectClass::updateOrbitalPositionAndVelocity(vec3 orbited_body_position, vec3 orbited_body_velocity)
{
	orbital_position = position - orbited_body_position;
	orbital_velocity = velocity - orbited_body_velocity;
}

bool objectClass::Inititalize_animations(int total_animations)
{
	animation = new AnimationStruct[total_animations];
	if (!animation) return false;

	return true;
}

void objectClass::addAngulosX(double angulos)
{
	if (angulos > 0)
	{
		x_positive_rot_on = true;
		//x_positive_power = float(angulos);
		x_positive_power = max(x_positive_power, float(angulos));
	}

	else if (angulos < 0)
	{
		x_negative_rot_on = true;
		//x_negative_power = float(angulos);
		x_negative_power = min(x_negative_power, float(angulos));
	}
	
	angulos_x += angulos;
};

void objectClass::addAngulosY(double angulos)
{
	if (angulos > 0)
	{
		y_positive_rot_on = true;
		//y_positive_power = float(angulos);
		y_positive_power = max(y_positive_power, float(angulos));
	}

	else if (angulos < 0)
	{
		y_negative_rot_on = true;
		//y_negative_power = float(angulos);
		y_negative_power = min(y_negative_power, float(angulos));
	}
	
	angulos_y += angulos;
};

void objectClass::addAngulosZ(double angulos)
{
	if (angulos > 0)
	{
		z_positive_rot_on = true;
		//z_positive_power = float(angulos);
		z_positive_power = max(z_positive_power, float(angulos));
	}

	else if (angulos < 0)
	{
		z_negative_rot_on = true;
		//z_negative_power = float(angulos);
		z_negative_power = min(z_negative_power, float(angulos));
	}

	angulos_z += angulos;
};

void objectClass::setAngulosX(double angulos){ angulos_x = angulos; };
void objectClass::setAngulosY(double angulos){ angulos_y = angulos; };
void objectClass::setAngulosZ(double angulos){ angulos_z = angulos; };

void objectClass::InitializeForwardNozzles(int n_)
{
	ForwardNozzle = new int[n_];
	for (int n = 0; n < total_ForwardNozzles; n++)
	{
		ForwardNozzle[n] = 0;
	}
}

void objectClass::InitializeBackwardNozzles(int n_)
{
	BackwardNozzle = new int[n_];
	for (int n = 0; n < total_BackwardNozzles; n++)
	{
		BackwardNozzle[n] = 0;
	}
}

void objectClass::InitializeUpNozzles(int n_)
{
	UpNozzle = new int[n_];
	for (int n = 0; n < total_UpNozzles; n++)
	{
		UpNozzle[n] = 0;
	}
}

void objectClass::InitializeDownNozzles(int n_)
{
	DownNozzle = new int[n_];
	for (int n = 0; n < total_DownNozzles; n++)
	{
		DownNozzle[n] = 0;
	}
}

void objectClass::InitializeRightNozzles(int n_)
{
	RightNozzle = new int[n_];
	for (int n = 0; n < total_RightNozzles; n++)
	{
		RightNozzle[n] = 0;
	}
}

void objectClass::InitializeLeftNozzles(int n_)
{
	LeftNozzle = new int[n_];
	for (int n = 0; n < total_LeftNozzles; n++)
	{
		LeftNozzle[n] = 0;
	}
}

void objectClass::InitializePositiveXNozzles(int n_)
{
	PositiveXnozzle = new int[n_];
	for (int n = 0; n < total_PositiveXNozzles; n++)
	{
		PositiveXnozzle[n] = 0;
	}
}

void objectClass::InitializePositiveYNozzles(int n_)
{
	PositiveYnozzle = new int[n_];
	for (int n = 0; n < total_PositiveYNozzles; n++)
	{
		PositiveYnozzle[n] = 0;
	}
}

void objectClass::InitializePositiveZNozzles(int n_)
{
	PositiveZnozzle = new int[n_];
	for (int n = 0; n < total_PositiveZNozzles; n++)
	{
		PositiveZnozzle[n] = 0;
	}
}

void objectClass::InitializeNegativeXNozzles(int n_)
{
	NegativeXnozzle = new int[n_];
	for (int n = 0; n < total_NegativeXNozzles; n++)
	{
		NegativeXnozzle[n] = 0;
	}
}

void objectClass::InitializeNegativeYNozzles(int n_)
{
	NegativeYnozzle = new int[n_];
	for (int n = 0; n < total_NegativeYNozzles; n++)
	{
		NegativeYnozzle[n] = 0;
	}
}

void objectClass::InitializeNegativeZNozzles(int n_)
{
	NegativeZnozzle = new int[n_];
	for (int n = 0; n < total_NegativeZNozzles; n++)
	{
		NegativeZnozzle[n] = 0;
	}
}
