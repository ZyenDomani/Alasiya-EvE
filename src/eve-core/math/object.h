#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "stdafx.h"

class objectClass
{
public:

	objectClass();

	objectClass(const objectClass& other);
	~objectClass();

	struct animation_modification
	{
		animation_modification()
		{
			animation_name = "N/A";
			available = false;
		}

		std::string animation_name;
		bool available;
	};

	struct AnimationStruct
	{
		AnimationStruct()
		{
			name = "N/A";
			total_moves = 0;
			modify_total = 0;
			active = false;
			available = true;
			activated = false;
			time_ = 0.f;
			HandleObject = nullptr;
			modify_animation = nullptr;
		}

		~AnimationStruct()
		{
			if (total_moves > 0 ) SAFE_DELETE_ARRAY(HandleObject);
			if (modify_total > 0) SAFE_DELETE_ARRAY(modify_animation);
		}

		bool Initizalize_moves(int total_moves)
		{
			HandleObject = new HandleMovingPart[total_moves];
			if (!HandleObject) return false;

			return true;
		}

		bool Initizalize_other_animations_modification(int modifications_total)
		{
			modify_animation = new animation_modification[modify_total];
			if (!modify_animation) return false;

			return true;
		}

		std::string name;
		int total_moves, modify_total;
		bool active, available, activated;
		float time_;
		HandleMovingPart* HandleObject;
		animation_modification* modify_animation;
	};

	AnimationStruct* animation;
	bool Inititalize_animations(int total_animations);

	void setTotalAnimations(int ta_){ total_animations = ta_; };
	void setRoboticArmDPort(bool ragf){ robotic_arm_dport = ragf; };
	void setVisualHelpers(bool vh_){ visual_helpers = vh_; };
	void setLocalFixture(int lf_){ local_fixture = lf_; };
	void setTargetFixture(int tf_){ target_fixture = tf_; };

	void Rotate(quat total_rotation_of_docked_to, double dtime);
	void ConvertCartesianToKeplerianElements(vec3 orbiting_body_position, vec3 orbiting_body_velocity, double orbiting_body_mass);
	void ConvertKeplerianToCartesianElements(double central_body_gravitational_parameter);

	void setHasAnimations(bool ha_){ has_animations = ha_; };

	void setRCSTraslationEnabled(bool rte_){ rcs_traslation_enabled = rte_; };

	void setModel(int model_number) { model = model_number; };

	void setVelocity(vec3 vel){ velocity = vel; };

	void setPosition(vec3 pos){ position = pos; };
	void setDockedFormerPosition(vec3 pos){ docked_former_position = pos; };
	void setVelocity_b(Vector3 vel){ velocity_b = vel; };
	void setVelocity_c(Vector3 vel){ velocity_c = vel; };
	void setPosition_b(Vector3 pos){ position_b = pos; };
	void setPosition_c(Vector3 pos){ position_c = pos; };
	void setPosition_bullet(Vector3 pos){ position_bullet = pos; };
	void setPosition_bullet_b(Vector3 pos){ position_bullet_b = pos; };
	void setTotalRotation(quat tot_rot){ total_rotation = tot_rot; };
	void setOriginalDockedRotation(quat tot_rot){ original_docked_rotation = tot_rot; };

	void addAngulosX(double add_angulos);
	void addAngulosY(double add_angulos);
	void addAngulosZ(double add_angulos);

	void setAngulosX(double add_angulos);
	void setAngulosY(double add_angulos);
	void setAngulosZ(double add_angulos);

	void setLatitude(double lat_){ latitude = lat_; };
	void setLongitude(double lon_){ longitude = lon_; };
	void setHeading(double hea_){ heading = hea_; };
	void setDockedTo(int docked_){ docked_to = docked_; };
	void setAttachedTo(std::string attached_){ attached_to = attached_; };
	void setScale_b(float scaled){ scale_b = scaled; };
	void setLength(float le_){ length_ = le_; };
	void setWidth(float wi_){ width_ = wi_; };
	void setHeight(float ht_){ height_ = ht_; };
	void setWingSpan(float ws_){ wing_span = ws_; };
	void setFrontFriction(float af_){ FrontFriction = af_; };
	void setMinLift(float ml_){ min_lift = ml_; };
	void setMaxLift(float ml_){ max_lift = ml_; };
	void setAerodynamicControls(bool ac_){ aerodynamic_controls = ac_; };
	void setInBullet(bool bl_){ in_bullet = bl_; };
	void setInVirtualBullet(bool vb_){ in_virtual_bullet = vb_; };
	void setBulletId(int bi_){ bullet_id = bi_; };
	void setBulletConstrained(bool bc_){ bullet_constrained = bc_; };
	void setBulletConstrainedTo(int bct_){ bullet_constrained_to = bct_; };
	void setMinConstraintMove(double mcm){ min_constraint_move = mcm; };
	void setMaxConstraintMove(double mcm){ max_constraint_move = mcm; };
	void setCurrentConstraintMove(double ccm){ current_constraint_move = ccm; };

	void setEngineColorStart(Color ec_){ engine_color_start = ec_; };
	void setEngineColorEnd(Color ec_){ engine_color_end = ec_; };

	void setAttachedPosition(vec3 pos_){ attached_position = pos_; };
	void setHorizontalDir(vec3 hd_){ horizontal_dir = hd_; };
	void setDestroyed(bool d_){ destroyed = d_; };
	void setCenterOfMass(vec3 cm_){ center_of_mass = cm_; };
	void setIsParent(bool ip_){ is_parent = ip_; };

	void setGimbal(bool gim_){ gimbal = gim_; };
	void setGimbalAngle(double angle_){ gimbal_angle = angle_; };

	void setStagesFile(std::string file_){ stages_file = file_; };
	void setStage(int st_){ stage = st_; };
	void setTopAbsoluteParent(int tp_){ top_absolute_parent = tp_; };
	void setTopArtificialParent(int tp_){ top_artificial_parent = tp_; };
	void setFuelConsumption(double fc_){ fuel_consumption = fc_; };

	void set_angulo_total_percent(float atp_){ angulo_total_percent = atp_; };
	void set_medio_angulo_percent(float map_){ medio_angulo_percent = map_; };
	void set_area_total(float at_){ area_total = at_; };
	void set_area_frontal(float af_){ area_frontal = af_; };
	void set_area_lateral(float al_){ area_lateral = al_; };
	void set_medio_angulo(float ma_){ medio_angulo = ma_; };
	void set_friccion(float f_){ friccion = f_; };
	void set_vv(double vv_){ vv = vv_; };
	void set_atmospheric_pressure(float ap_){ atmospheric_pressure = ap_; };
	void set_air_density(float ad_){ air_density = ad_; };
	void set_altitude(double alt_){ altitude = alt_; };
	void set_true_air_speed(float tas_){ true_air_speed = tas_; };
	void set_dynamic_pressure(vec3 dp_){ dynamic_pressure = dp_; };
	void set_Drag(vec3 d_){ Drag = d_; };
	void set_true_velocity(vec3 tv_){ true_velocity = tv_; };
	void set_true_lift_vec(vec3 tlv_){ true_lift_vec = tlv_; };

	void delete_docking_ports();
	void delete_grapple_fixtures();

	void setRocketEngine(bool re_){ rocket_engine = re_; };

	void setSunsetColor(Vector3 sc_){ sunset_color = sc_; };
	void setDayLightColor(Vector3 dc_){ daylight_color = dc_; };
	void setAtmosphereThickness(float at_){ atmosphere_thickness = at_; };

	void setTopArea(float ta_){ top_area = ta_; };
	void setFrontArea(float fa_){ front_area = fa_; };
	void setSideArea(float sa_){ side_area = sa_; };

	void setScale_c(float scaled){ scale_c = scaled; };
	void setScale(double scaled){ scale = scaled; };
	void setType(int tipo){ type = tipo; };
	void setSubType(std::string stipo){ sub_type = stipo; };
	void setCollisionShapeType(int cst_){ collision_shape_type = cst_; };
	void setCollisionShapeFile(std::string csf_){ collision_shape_file = csf_; };
	void setTexture1(int tex_){ texture1 = tex_; };
	void setTexture2(int tex_){ texture2 = tex_; };
	void setTexture3(int tex_){ texture3 = tex_; };
	void setIlumination(float ilum_){ ilumination = ilum_; };
	void setAffects(int affects_){ affects = affects_; };
	void setCopyNumber(int cn_){ copy_number = cn_; };
	void setMass(double mass_){ mass = mass_; };
	void setEmptyMass(double mass_){ empty_mass = mass_; };
	void setTotalMovement(vec3 mov_){ movimiento_total = mov_; };
	void setName(std::string name_){ name = name_; };
	void setOriginalName(std::string name_){ original_name = name_; };
	void setOrbiting(int orbit_){ orbiting = orbit_; };
	void setCastsShadows(bool casts_){ casts_shadow = casts_; };
	void setSemiMayorAxis(double semi_major_){ semi_mayor_axis = semi_major_; };
	void setEccentricity(double ecce_){ eccentricity = ecce_; };
	void setInclination(double incl_){ inclination = incl_; };
	void setArgumentOfPerigee(double argu_){ argument_of_perigee = argu_; };
	void setLongitudeOfAscendingNode(double longi_){ longitude_of_ascending_node = longi_; };
	void setTrueAnomaly(double true_a){ true_anomaly = true_a; };
	void setObliquityToOrbit(double obli_){ obliquity_to_orbit = obli_; };
	void setInitialRotation(double init_){ initial_rotation = init_; };
	void setSiderealRotationPeriod(double side_){ sidereal_rotation_period = side_; };
	void setEquatorialRadius(double equa_){ equatorial_radius = equa_; };
	void setPolarRadius(double polar_){ polar_radius = polar_; };
	void setFlattening(float flat_){ flattening = flat_; };
	void setInheritedRotation(quat inhe_){ inherited_rotation = inhe_; };
	void setEphemeris_OK(bool eph_){ ephemeris_OK = eph_; };
	void setDistanceToCamera(float dist_){ distance_to_camera = dist_; };
	void setVisible(bool visible_){ visible = visible_; };
	void setHasAtmosphere(bool atmos_){ has_atmosphere = atmos_; };
	void setInAtmosphere(bool ia_){ in_atmosphere = ia_; };
	void setAtmosphereRatio(float ratio_){ atmosphere_ratio = ratio_; };
	void setHasRings(bool rings_){ has_rings = rings_; };
	void setInnerRing(float i_ring_){ inner_ring = i_ring_; };
	void setOuterRing(float o_ring_){ outer_ring = o_ring_; };
	void setRingsTexture(std::string r_text_){ rings_texture = r_text_; };
	void setRingObject(int ring_){ ring_object = ring_; };
	void setCloudsObject(int ring_){ clouds_object = ring_; };
	void setTarget(int n){ target_ = n; };
	void setHighlighted(bool hl){ highlighted = hl; };
	void setConstraintType(int ct_){ constraint_type = ct_; };
	void setConstraintAxis(char ca_){ constraint_axis = ca_; };
	void setDockReady(bool dr_){ dock_ready = dr_; };
	void setLanded(bool ld_){ landed = ld_; };
	void setQuickMeBurst(bool qmeb_){ quick_Me_Burst = qmeb_; };
	void setQuickHoverBurst(bool qhb_){ quick_Hover_Burst = qhb_; };

	void setMainEnginePower(double mp_){ main_engine_power = mp_; };
	void setMEcurrentPower(double mecp_){ ME_current_power = mecp_; };
	void setHoverCurrentPower(double rcp_){ Hover_current_power = rcp_; };
	void setRCSpower(double rcs_p){ rcs_power = rcs_p; };
	void setHoverPower(double hover_){ hover_power = hover_; };
	void setTotalDockingPorts(int tot_d){ total_docking_ports = tot_d; };
	void setTotalGrappleFixtures(int tot_g){ total_grapple_fixtures = tot_g; };

	void setTotalEngines(int tot_e){ total_engines = tot_e; };
	void setTotalHoverEngines(int tot_he){ total_hover_engines = tot_he; };
	void setTotalRCSNozzles(int tot_n){ total_rcs_nozzles = tot_n; };

	void setDockingPortPosition(int dp_, Vector3 pos_){ docking_port_position[dp_] = pos_; };
	void setGrappleFixturePosition(int gf_, Vector3 pos_){ grapple_fixture_position[gf_] = pos_; };

	void setEnginePosition(int engine_, Vector3 pos_){ Engine[engine_].position = pos_; };
	void setEngineDepth(int engine_, float d_){ Engine[engine_].depth = d_; };
	void setHoverEngineDepth(int engine_, float d_){ Hover_Engine[engine_].depth = d_; };
	void setRCSNozzleDepth(int nozzle_, float d_){ RCS_Nozzle[nozzle_].depth = d_; };
	void setHoverEnginePosition(int hengine_, Vector3 pos_){ Hover_Engine[hengine_].position = pos_; };
	void setRCSNozzlePosition(int nozzle_, Vector3 pos_){ RCS_Nozzle[nozzle_].position = pos_; };
	void setRCSNozzleOrientation(int nozzle_, Vector3 orient_){ RCS_Nozzle[nozzle_].orientation = orient_; };

	void setEngineSize(int engine_, float size_){ Engine[engine_].size = size_; };
	void setHoverEngineSize(int hengine_, float size_){ Hover_Engine[hengine_].size = size_; };
	void setRCSNozzleSize(int nozzle_, float size_){ RCS_Nozzle[nozzle_].size = size_; };

	void setDockingPortFree(int dp_, bool status_){ dockingPortFree[dp_] = status_; };
	void setDockingPortOrientation(int dp_, Vector3 ori_){ docking_port_orientation[dp_] = ori_; };
	void setGrappleFixtureOrientation(int gf_, Vector3 ori_){ grapple_fixture_orientation[gf_] = ori_; };
	void setFuelCapacity(float cap_){ fuel_capacity = cap_; };
	void setFuel(float fu_){ fuel = fu_; };
	void setRCSCapacity(float cap_){ rcs_capacity = cap_; };
	void setRCS(float rc_s){ rcs = rc_s; };
	void setO2Capacity(float ocap_){ o2_capacity = ocap_; };
	void setO2(float o2_){ o2 = o2_; };
	void setBatteryCapacity(float bat_cap){ battery_capacity = bat_cap; };
	void setBattery(float bat_){ battery = bat_; };
	void setBatteryChargeRate(float bat_r){ battery_charge_rate = bat_r; };
	void setGroundSensorDistance(float gsd_){ ground_sensor_distance = gsd_; };
	void setCockpitCameraPosition(Vector3 c_pos){ cockpit_camera = c_pos; };

	void set_docked_to_name(std::string dtn_){ docked_to_name = dtn_; };
	void set_orbiting_name(std::string on_){ orbited_name = on_; };
	void set_target_name(std::string tn_){ target_name = tn_; };

	void setH_level(bool hl){ h_level = hl; };
	void setRadialIn(bool ri_){ radial_in = ri_; };
	void setRadialOut(bool ro_){ radial_out = ro_; };
	void setLandingGear(bool lg_){ landing_gear = lg_; };
	void setPro_grade(bool pro_){ pro_grade = pro_; };
	void setRetro_grade(bool retro){ retro_grade = retro; };
	void setNormal(bool nor_){ normal = nor_; };
	void setAnti_Normal(bool an_){ anti_normal = an_; };
	void setLock_Target(bool lt_){ lock_target = lt_; };
	void setDocked_position(vec3 pos_){ docked_position = pos_; };
	void setDocked_position_b(vec3 pos_){ docked_position_b = pos_; };
	void setOriginalDockedPosition(vec3 pos_){ original_docked_position = pos_; };
	void setDockingDataUpdated(bool ddu_){ docking_data_updated=ddu_; };

	void setCameraPreferedAxis(Vector3 ax_){ camera_prefered_axis = ax_; };

	void setDocked_rotation_x(float angle_){ docked_rotation_x = angle_; }
	void setDocked_rotation_y(float angle_){ docked_rotation_y = angle_; }
	void setDocked_rotation_z(float angle_){ docked_rotation_z = angle_; }

	void setGroundSpeed(double gs_){ ground_speed = gs_; };

	void setHasClouds(bool clouds_){ has_clouds = clouds_; };
	void setCloudsRatio(float ratio_){ clouds_ratio = ratio_; };

	void setHasLightMap(bool has_light_){ has_light_map = has_light_; };
	void setHasNightMap(bool has_night_){ has_night_map = has_night_; };
	void setHasSpecularMap(bool has_night_){ has_night_map = has_night_; };
	void setLoaded_tm(bool loaded_){ loaded_tm = loaded_; };
	void setKillRotation(bool kill_){ kill_rotation = kill_; };
	void setAutoGrapple(bool ag_){ auto_grapple = ag_; };
	void KillAutopilot()
	{
		retro_diff = false;
		radial_out = false;
		radial_in = false;
		h_level = false;
		pro_grade = false;
		retro_grade = false;
		normal = false;
		anti_normal = false;
		lock_target = false;
		AlignToTargetsUp = false;
		match_speed = false;
		approach = false;
	}

	void KillRCS()
	{
		retro_diff = false;
		radial_out = false;
		radial_in = false;
		h_level = false;
		pro_grade = false;
		retro_grade = false;
		normal = false;
		anti_normal = false;
		lock_target = false;
		AlignToTargetsUp = false;
	}

	void setKillRotationX(bool kill_){ kill_rotationX = kill_; };
	void setKillRotationY(bool kill_){ kill_rotationY = kill_; };
	void setKillRotationZ(bool kill_){ kill_rotationZ = kill_; };

	void setMatchSpeed(bool ms_){ match_speed = ms_; };
	void setApproach(bool ap_){ approach = ap_; };
	void setApproachDist(float ad_){ approach_dist = ad_; };
	void setRetroDiff(bool rd_){ retro_diff = rd_; };
	void setAlignToTargetsUp(bool atu_) { AlignToTargetsUp = atu_; };

	void setG_Acceleration(double accel_){ g_acceleration = accel_; };
	void setSeaLevelPressure(float slp_){ sea_level_pressure = slp_; };
	void setSeaLevelTemperature(float slt_){ sea_level_temperature = slt_; };
	void setSeaLevelDensity(float sld_){ sea_level_density = sld_; };
	void setSunlit(bool sl_){ sunlit = sl_; };

	void setInOrbit(bool io_){ in_orbit = io_; };

	void setWorld(Matrix wo_){ World_ = wo_; };
	void setVelocityModified(bool vm_){ velocity_modified = vm_; };

	double getAngulosX_() const { return angulos_x; };
	double getAngulosY_() const { return angulos_y; };
	double getAngulosZ_() const { return angulos_z; };

	bool getHighlighted(){ return highlighted; };
	bool getRoboticArmDPort(){ return robotic_arm_dport; };
	bool getVisualHelpers(){ return visual_helpers; };

	double getLatitude() const { return latitude; };
	double getLongitude() const { return longitude; };
	double getHeading() const { return heading; };

	double getMainEnginePower() const { return main_engine_power; };
	double getMECurrentPower() const { return ME_current_power; };
	double getHoverCurrentPower() const { return Hover_current_power; };
	double getRCSpower() const { return rcs_power; };
	double getHoverPower() const { return hover_power; };
	int getTotalDockingPorts() const { return total_docking_ports; };
	int getTotalGrappleFixtures() const { return total_grapple_fixtures; };

	Vector3 getEnginePosition(int engine_){ return Engine[engine_].position; };
	float getEngineSize(int engine_){ return Engine[engine_].size; };
	float getEngineDepth(int engine_){ return Engine[engine_].depth; };

	vec3 getCenterOfMass(){ return center_of_mass; };
	bool getIsParent(){ return is_parent; };

	std::string GetStagesFile(){ return stages_file; };
	int getStage(){ return stage; };
	int getTopAbsoluteParent(){ return top_absolute_parent; };
	int getTopArtificialParent(){ return top_artificial_parent; };
	double getFuelConsumption(){ return fuel_consumption; };
	bool getGimbal(){ return gimbal; };
	double getGimbalAngle(){ return gimbal_angle; };

	bool getInBullet(){ return in_bullet; };
	bool getInVirtualBullet(){ return in_virtual_bullet; };
	int getBulletId(){ return bullet_id; };
	bool getBulletConstrained(){ return bullet_constrained; };
	int getBulletConstrainedTo(){ return bullet_constrained_to; };
	int getLocalFixture(){ return local_fixture; };
	int getTargetFixture(){ return target_fixture; };

	vec3 getHorizontalDir(){ return horizontal_dir; };

	Vector3 getHoverEnginePosition(int engine_){ return Hover_Engine[engine_].position; };
	float getHoverEngineSize(int engine_){ return Hover_Engine[engine_].size; };
	float getHoverEngineDepth(int engine_){ return Hover_Engine[engine_].depth; };

	Color getEngineColorStart(){ return engine_color_start; };
	Color getEngineColorEnd(){ return engine_color_end; };

	Vector3 getRCSNozzlePosition(int nozzle_){ return RCS_Nozzle[nozzle_].position; };
	Vector3 getRCSNozzleOrientation(int nozzle_){ return RCS_Nozzle[nozzle_].orientation; };
	float getRCSNozzleSize(int nozzle_){ return RCS_Nozzle[nozzle_].size; };
	float getRCSNozzleDepth(int nozzle_){ return RCS_Nozzle[nozzle_].depth; };

	int getTotalEngines() const { return total_engines; };
	int getTotalHoverEngines() const { return total_hover_engines; };
	int getTotalRCSNozzles() const { return total_rcs_nozzles; };

	bool getDockingDataUpdated(){ return docking_data_updated; };

	float get_angulo_total_percent() const { return angulo_total_percent; };
	float get_medio_angulo_percent() const { return medio_angulo_percent; };
	float get_area_total() const { return area_total; };
	float get_area_frontal() const { return area_frontal; };
	float get_area_lateral() const { return area_lateral; };
	float get_medio_angulo() const { return medio_angulo; };
	float get_friccion() const { return friccion; };
	double get_vv() const { return vv; };
	float get_atmospheric_pressure() const { return atmospheric_pressure; };
	float get_air_density() const { return air_density; };
	double get_altitude() const { return altitude; };
	float get_true_air_speed() const { return true_air_speed; };
	vec3 get_dynamic_pressure() const { return dynamic_pressure; };
	vec3 get_Drag() const { return Drag; };
	vec3 get_true_velocity() const { return true_velocity; };
	vec3 get_true_lift_vec() const { return true_lift_vec; };

	vec3 getAttachedPosition(){ return attached_position; };

	float getDocked_rotation_x(){ return docked_rotation_x; }
	float getDocked_rotation_y(){ return docked_rotation_y; }
	float getDocked_rotation_z(){ return docked_rotation_z; }

	Vector3 getDockingPortPosition(int dp_){ return docking_port_position[dp_]; };
	Vector3 getGrappleFixturePosition(int gf_){ return grapple_fixture_position[gf_]; };
	bool getDockingPortFree(int dp_){ return dockingPortFree[dp_]; };
	Vector3 getDockingPortWorldPosition(int dp_){ return vec_to_d3d(RowVec3byMat4d(d3d_to_vec(docking_port_position[dp_]), D3DXMATRIX_to_Mat4d(World_))) + position_b; };
	Vector3 getDockingPortWorldOrientation(int dp_){ return vec_to_d3d(RowVec3byMat4d(d3d_to_vec(docking_port_orientation[dp_]), D3DXMATRIX_to_Mat4d(World_))) + position_b; };

	Vector3 getGrappleFixtureWorldPosition(int gf_){ return vec_to_d3d(RowVec3byMat4d(d3d_to_vec(grapple_fixture_position[gf_]), D3DXMATRIX_to_Mat4d(World_))) + position_b; };
	Vector3 getGrappleFixtureWorldOrientation(int gf_){ return vec_to_d3d(RowVec3byMat4d(d3d_to_vec(grapple_fixture_orientation[gf_]), D3DXMATRIX_to_Mat4d(World_))) + position_b; };

	vec3 getWorldPosition(vec3 vec_){ return RowVec3byMat4d(vec_, D3DXMATRIX_to_Mat4d(World_)) + d3d_to_vec(position_b); };

	Vector3 getCameraPreferedAxis(){ return camera_prefered_axis; };

	Vector3 getSunsetColor() const { return sunset_color; };
	Vector3 getDayLightColor() const { return daylight_color; };
	float getAtmosphereThickness() const { return atmosphere_thickness; };

	Vector3 getDockingPortOrientation(int dp_){ return docking_port_orientation[dp_]; };
	Vector3 getGrappleFixtureOrientation(int gf_){ return grapple_fixture_orientation[gf_]; };
	float getFuelCapacity() const { return fuel_capacity; };
	float getFuel() const { return fuel; };
	float getRCSCapacity() const { return rcs_capacity; };
	float getRCS() const { return rcs; };
	float getO2Capacity() const { return o2_capacity; };
	float getO2() const { return o2; };
	float getBatteryCapacity() const { return battery_capacity; };
	float getBattery() const { return battery; };
	float getBatteryChargeRate() const { return battery_charge_rate; };
	float getGroundSensorDistance() const { return ground_sensor_distance; };
	Vector3 getCockpitCameraPosition() const { return cockpit_camera; };
	bool getDockReady() const { return dock_ready; };
	bool getVelocityModified() const { return velocity_modified; };
	float getMinLift() const { return min_lift; };
	float getMaxLift() const { return max_lift; };

	bool getRocketEngine() const { return rocket_engine; };
	bool getSunlit() const { return sunlit; };
	bool getDestroyed() const { return destroyed; };

	int getModel() const { return model; };
	vec3 getVelocity() const { return velocity; };
	vec3 getPosition() const { return position; };
	vec3 getDockedFormerPosition() const { return docked_former_position; };
	vec3 getSpecificAngularMomentum() const { return specific_angular_momentum; };
	Vector3 getVelocity_b() const { return velocity_b; };
	Vector3 getVelocity_c() const { return velocity_c; };
	Vector3 getPosition_b() const { return position_b; };
	Vector3 getPosition_bullet() const { return position_bullet; };
	Vector3 getPosition_bullet_b() const { return position_bullet_b; };
	Vector3 getPosition_c() const { return position_c; };
	float getScale_b() const { return scale_b; };
	float getScale_c() const { return scale_c; };
	double getScale() const { return scale; };

	float  getLength() const { return length_; };
	float  getWidth() const { return width_; };
	float  getHeight() const { return height_; };
	float getWingSpan() const { return wing_span; };
	float getFrontFriction() const { return FrontFriction; };
	float getTopArea() const { return top_area; };
	float getFrontArea() const { return front_area; };
	float getSideArea() const { return side_area; };

	quat getTotalRotation() const { return total_rotation; };
	quat getOriginalDockedRotation() const { return original_docked_rotation; };
	int getType() const { return type; };
	std::string getSubType() const { return sub_type; };
	std::string getCollisionShapeFile(){ return collision_shape_file; };
	int getCollisionShapeType(){ return collision_shape_type; };
	int getTexture1() const { return texture1; };
	int getTexture2() const { return texture2; };
	int getTexture3() const { return texture3; };
	float getIlumination() const { return ilumination; };
	int getAffects() const { return affects; };
	int getCopyNumber() const { return copy_number; };
	double getMass() const { return mass; };
	double getEmptyMass() const { return empty_mass; };
	vec3 getTotalMovement() const { return movimiento_total; };
	int getDockedTo() const { return docked_to; };
	std::string getAttachedTo() const { return attached_to; };
	std::string getName() const { return name; };
	std::string getOriginalName() const { return original_name; };
	std::string get_docked_to_name() const { return docked_to_name; };
	std::string get_orbiting_name() const { return orbited_name; };
	std::string get_target_name() const { return target_name; };
	int getOrbiting() const { return orbiting; };
	bool getCastsShadows() const { return casts_shadow; };
	double getSemiMayorAxis() const { return semi_mayor_axis; };
	double getEccentricity() const { return eccentricity; };
	double getInclination() const { return inclination; };
	double getArgumentOfPerigee() const { return argument_of_perigee; };
	double getLongitudeOfAscendingNode() const { return longitude_of_ascending_node; };
	double getTrueAnomaly() const { return true_anomaly; };
	double getMeanAnomaly() const { return mean_anomaly; };
	double getMeanMotion() const { return mean_motion; };
	double getObliquityToOrbit() const { return obliquity_to_orbit; };
	double getInitialRotation() const { return initial_rotation; };
	double getSiderealRotationPeriod() const { return sidereal_rotation_period; };
	double getEquatorialRadius() const { return equatorial_radius; };
	double getPolarRadius() const { return polar_radius; };
	float getFlattening() const { return flattening; };
	quat getInheritedRotation() const { return inherited_rotation; };
	bool getEphemeris_OK() const { return ephemeris_OK; };
	float getDistanceToCamera() const { return distance_to_camera; };
	bool getVisible() const { return visible; };
	bool getHasAtmosphere() const { return has_atmosphere; };
	bool getInAtmosphere() const { return in_atmosphere; };
	float getAtmosphereRatio() const { return atmosphere_ratio; };
	bool getHasRings() const { return has_rings; };
	float getInnerRing() const { return inner_ring; };
	float getOuterRing() const { return outer_ring; };
	double getTime_to_perigee() const { return time_to_perigee; };
	double getTime_to_apogee() const { return time_to_apogee; };
	double getOrbitalPeriod() const { return orbital_period; };
	vec3 getOrbitalVelocity() const { return orbital_velocity; };
	vec3 getOrbitalPosition() const { return orbital_position; };
	double getOrbitRadius() const { return orbit_radius; };
	double getDistance_to_perigee() const { return distance_to_perigee; };
	double getPerigee() const { return perigee; };
	double getDistance_to_apogee() const { return distance_to_apogee; };
	double getApogee() const { return apogee; };
	std::string getRingsTexture() const { return rings_texture; };
	int getRingObject() const { return ring_object; };
	int getCloudsObject() const { return clouds_object; };
	double getGroundSpeed() const { return ground_speed; };
	vec3 getGroundSpeedVec() const { return ground_speed_vec; };
	double getTimeSincePerigee() const { return time_since_perigee; };
	double getG_Acceleration() const { return g_acceleration; };
	float getSeaLevelPressure() const { return sea_level_pressure; };
	float getSeaLevelTemperature() const { return sea_level_temperature; };
	float getSeaLevelDensity() const { return sea_level_density; };

	bool getHasAnimations(){ return has_animations; };
	bool getRCSTraslationEnabled(){ return rcs_traslation_enabled; };
	bool getAerodynamicControls() const { return aerodynamic_controls; };

	int getTotalAnimations(){ return total_animations; };

	bool getQuickMeBurst() const { return quick_Me_Burst; };
	bool getQuickHoverBurst() const { return quick_Hover_Burst; };

	bool getInOrbit() const { return in_orbit; };

	bool getHasClouds() const { return has_clouds; };
	float getCloudsRatio() const { return clouds_ratio; };

	bool getHasLightMap() const { return has_light_map; };
	bool getHasNightMap() const { return has_night_map; };
	bool getHasSpecularMap() const { return has_night_map; };
	bool getLoaded_tm() const { return loaded_tm; };
	bool getKillRotation() const { return kill_rotation; };
	bool getAutoGrapple() const { return auto_grapple; };

	bool getKillRotationX() const { return kill_rotationX; };
	bool getKillRotationY() const { return kill_rotationY; };
	bool getKillRotationZ() const { return kill_rotationZ; };

	Matrix getWorld() const { return World_; };

	bool getH_level() const { return h_level; };
	bool getRadialIn() const { return radial_in; };
	bool getRadialOut() const { return radial_out; };
	bool getLandingGear() const { return landing_gear; };
	bool getPro_grade() const { return pro_grade; };
	bool getRetro_grade() const { return retro_grade; };
	bool getNormal() const { return normal; };
	bool getAnti_normal() const { return anti_normal; };
	bool getLock_Target() const { return lock_target; };
	bool getMatchSpeed() const { return match_speed; };
	bool getApproach() const { return approach; };
	float getApproachDist() const { return approach_dist; };
	bool getRetroDiff() const { return retro_diff; };
	bool getLanded() const { return landed; };
	vec3 getDocked_position() const { return docked_position; };
	vec3 getDocked_position_b() const { return docked_position_b; };
	vec3 getOriginalDockedPosition() const { return original_docked_position; };
	bool getAlignToTargetsUp() const { return AlignToTargetsUp; };
	int getTarget() const { return target_; };
	int getConstraintType(){ return constraint_type; };
	char getConstraintAxis(){ return constraint_axis; };
	double getMinConstraintMove(){ return min_constraint_move; };
	double getMaxConstraintMove(){ return max_constraint_move; };
	double getCurrentConstraintMove(){ return current_constraint_move; };
	void setOrbitalPosition(vec3 op_){ orbital_position = op_; };
	void setOrbitalVelocity(vec3 vel_){ orbital_velocity = vel_; };
	void UpdateCustomData(vec3 orbited_body_position, double orbited_body_scale, quat orbited_rotation);
	void updateOrbitalPositionAndVelocity(vec3 orbited_body_position, vec3 orbited_body_velocity);

	void InitializeDockingPorts(int total_ports);
	void InitializeGrappleFixtures(int total_fixtures);
	void InitializeEngines(int total_engines_);
	void InitializeHoverEngines(int total_hover_engines_);
	void InitializeRCSNozzles(int total_rcs_nozzles_);

	void setTotalForwardNozzles(int pn_) { total_ForwardNozzles = pn_; };
	void setTotalBackwardNozzles(int pn_) { total_BackwardNozzles = pn_; };
	void setTotalUpNozzles(int pn_) { total_UpNozzles = pn_; };
	void setTotalDownNozzles(int pn_) { total_DownNozzles = pn_; };
	void setTotalRightNozzles(int pn_) { total_RightNozzles = pn_; };
	void setTotalLeftNozzles(int pn_) { total_LeftNozzles = pn_; };

	int getTotalForwardNozzles() const { return total_ForwardNozzles; };
	int getTotalBackwardNozzles() const { return total_BackwardNozzles; };
	int getTotalUpNozzles() const { return total_UpNozzles; };
	int getTotalDownNozzles() const { return total_DownNozzles; };
	int getTotalRightNozzles() const { return total_RightNozzles; };
	int getTotalLeftNozzles() const { return total_LeftNozzles; };

	void InitializeForwardNozzles(int n_);
	void InitializeBackwardNozzles(int n_);
	void InitializeUpNozzles(int n_);
	void InitializeDownNozzles(int n_);
	void InitializeRightNozzles(int n_);
	void InitializeLeftNozzles(int n_);

	void setForwardNozzle(int nozzle_n, int n_) { ForwardNozzle[nozzle_n] = n_; };
	void setBackwardNozzle(int nozzle_n, int n_) { BackwardNozzle[nozzle_n] = n_; };
	void setUpNozzle(int nozzle_n, int n_) { UpNozzle[nozzle_n] = n_; };
	void setDownNozzle(int nozzle_n, int n_) { DownNozzle[nozzle_n] = n_; };
	void setRightNozzle(int nozzle_n, int n_) { RightNozzle[nozzle_n] = n_; };
	void setLeftNozzle(int nozzle_n, int n_) { LeftNozzle[nozzle_n] = n_; };

	void setTotalPositiveXNozzles(int pn_) { total_PositiveXNozzles = pn_; };
	void setTotalPositiveYNozzles(int pn_) { total_PositiveYNozzles = pn_; };
	void setTotalPositiveZNozzles(int pn_) { total_PositiveZNozzles = pn_; };

	void setTotalNegativeXNozzles(int nn_) { total_NegativeXNozzles = nn_; };
	void setTotalNegativeYNozzles(int nn_) { total_NegativeYNozzles = nn_; };
	void setTotalNegativeZNozzles(int nn_) { total_NegativeZNozzles = nn_; };

	int getTotalPositiveXNozzles() const { return total_PositiveXNozzles; };
	int getTotalPositiveYNozzles() const { return total_PositiveYNozzles; };
	int getTotalPositiveZNozzles() const { return total_PositiveZNozzles; };

	int getTotalNegativeXNozzles() const { return total_NegativeXNozzles; };
	int getTotalNegativeYNozzles() const { return total_NegativeYNozzles; };
	int getTotalNegativeZNozzles() const { return total_NegativeZNozzles; };

	int getForwardNozzle(int nozzle_n) const { return ForwardNozzle[nozzle_n]; };
	int getBackwardNozzle(int nozzle_n) const { return BackwardNozzle[nozzle_n]; };
	int getUpNozzle(int nozzle_n) const { return UpNozzle[nozzle_n]; };
	int getDownNozzle(int nozzle_n) const { return DownNozzle[nozzle_n]; };
	int getRightNozzle(int nozzle_n) const { return RightNozzle[nozzle_n]; };
	int getLeftNozzle(int nozzle_n) const { return LeftNozzle[nozzle_n]; };

	int getPositiveXNozzle(int nozzle_n) { return PositiveXnozzle[nozzle_n]; };
	int getPositiveYNozzle(int nozzle_n) { return PositiveYnozzle[nozzle_n]; };
	int getPositiveZNozzle(int nozzle_n) { return PositiveZnozzle[nozzle_n]; };

	int getNegativeXNozzle(int nozzle_n) { return NegativeXnozzle[nozzle_n]; };
	int getNegativeYNozzle(int nozzle_n) { return NegativeYnozzle[nozzle_n]; };
	int getNegativeZNozzle(int nozzle_n) { return NegativeZnozzle[nozzle_n]; };

	void InitializePositiveXNozzles(int n_);
	void InitializePositiveYNozzles(int n_);
	void InitializePositiveZNozzles(int n_);

	void InitializeNegativeXNozzles(int n_);
	void InitializeNegativeYNozzles(int n_);
	void InitializeNegativeZNozzles(int n_);

	void setPositiveXNozzle(int nozzle_n, int n_) { PositiveXnozzle[nozzle_n] = n_; };
	void setPositiveYNozzle(int nozzle_n, int n_) { PositiveYnozzle[nozzle_n] = n_; };
	void setPositiveZNozzle(int nozzle_n, int n_) { PositiveZnozzle[nozzle_n] = n_; };

	void setNegativeXNozzle(int nozzle_n, int n_) { NegativeXnozzle[nozzle_n] = n_; };
	void setNegativeYNozzle(int nozzle_n, int n_) { NegativeYnozzle[nozzle_n] = n_; };
	void setNegativeZNozzle(int nozzle_n, int n_) { NegativeZnozzle[nozzle_n] = n_; };

	bool get_Forward_on() const { return Forward_on; };
	bool get_Backward_on() const { return Backward_on; };
	bool get_Up_on() const { return Up_on; };
	bool get_Down_on() const { return Down_on; };
	bool get_Right_on() const { return Right_on; };
	bool get_Left_on() const { return Left_on; };

	void set_Forward_on(bool s_){ Forward_on = s_; };
	void set_Backward_on(bool s_){ Backward_on = s_; };
	void set_Up_on(bool s_){ Up_on = s_; };
	void set_Down_on(bool s_){ Down_on = s_; };
	void set_Right_on(bool s_){ Right_on = s_; };
	void set_Left_on(bool s_){ Left_on = s_; };

	float get_Forward_power() const { return Forward_power; };
	float get_Backward_power() const { return Backward_power; };
	float get_Up_power() const { return Up_power; };
	float get_Down_power() const { return Down_power; };
	float get_Right_power() const { return Right_power; };
	float get_Left_power() const { return Left_power; };

	void set_Forward_power(float powe_){ Forward_power = powe_; };
	void set_Backward_power(float powe_){ Backward_power = powe_; };
	void set_Up_power(float powe_){ Up_power = powe_; };
	void set_Down_power(float powe_){ Down_power = powe_; };
	void set_Right_power(float powe_){ Right_power = powe_; };
	void set_Left_power(float powe_){ Left_power = powe_; };

	bool get_x_positive_rot_on() const { return x_positive_rot_on; };
	bool get_y_positive_rot_on() const { return y_positive_rot_on; };
	bool get_z_positive_rot_on() const { return z_positive_rot_on; };

	bool get_x_negative_rot_on() const { return x_negative_rot_on; };
	bool get_y_negative_rot_on() const { return y_negative_rot_on; };
	bool get_z_negative_rot_on() const { return z_negative_rot_on; };

	void set_x_positive_rot_on(bool s_){ x_positive_rot_on = s_; };
	void set_y_positive_rot_on(bool s_){ y_positive_rot_on = s_; };
	void set_z_positive_rot_on(bool s_){ z_positive_rot_on = s_; };

	void set_x_negative_rot_on(bool s_){ x_negative_rot_on = s_; };
	void set_y_negative_rot_on(bool s_){ y_negative_rot_on = s_; };
	void set_z_negative_rot_on(bool s_){ z_negative_rot_on = s_; };

	float get_x_positive_power() const { return x_positive_power; };
	float get_y_positive_power() const { return y_positive_power; };
	float get_z_positive_power() const { return z_positive_power; };

	float get_x_negative_power() const { return x_negative_power; };
	float get_y_negative_power() const { return y_negative_power; };
	float get_z_negative_power() const { return z_negative_power; };

	void set_x_positive_power(float powe_){ x_positive_power = powe_; };
	void set_y_positive_power(float powe_){ y_positive_power = powe_; };
	void set_z_positive_power(float powe_){ z_positive_power = powe_; };

	void set_x_negative_power(float powe_){ x_negative_power = powe_; };
	void set_y_negative_power(float powe_){ y_negative_power = powe_; };
	void set_z_negative_power(float powe_){ z_negative_power = powe_; };

	Color engine_color_start, engine_color_end;

	engineStruct* Engine;
	engineStruct* Hover_Engine;
	engineStruct* RCS_Nozzle;

	int* ForwardNozzle;
	int* BackwardNozzle;
	int* UpNozzle;
	int* DownNozzle;
	int* RightNozzle;
	int* LeftNozzle;

	int* PositiveXnozzle;
	int* NegativeXnozzle;
	int* PositiveYnozzle;
	int* NegativeYnozzle;
	int* PositiveZnozzle;
	int* NegativeZnozzle;

private:

	std::string attached_to, collision_shape_file;

	int texture1, texture2, texture3, type, affects, orbiting, model, docked_to, ring_object, clouds_object, target_, copy_number, collision_shape_type;
	int total_PositiveXNozzles, total_NegativeXNozzles, total_PositiveYNozzles, total_NegativeYNozzles, total_PositiveZNozzles, total_NegativeZNozzles;
	int total_ForwardNozzles, total_BackwardNozzles, total_UpNozzles, total_DownNozzles, total_RightNozzles, total_LeftNozzles;
	int stage, top_absolute_parent, top_artificial_parent;
	int constraint_type;
	char constraint_axis;
	double min_constraint_move, max_constraint_move, current_constraint_move;
	std::string sub_type;

	double fuel_consumption;

	bool highlighted;
	bool casts_shadow, landed, ephemeris_OK, visible, has_atmosphere, has_clouds, has_light_map, has_night_map, has_specular_map;
	bool has_rings, loaded_tm, kill_rotation, h_level, pro_grade, retro_grade, normal, anti_normal, lock_target, match_speed, retro_diff, AlignToTargetsUp;
	bool radial_in, radial_out, approach, in_orbit, auto_grapple;
	bool kill_rotationX, kill_rotationY, kill_rotationZ;
	bool dock_ready, landing_gear, quick_Me_Burst, quick_Hover_Burst;
	bool velocity_modified, in_atmosphere, aerodynamic_controls;
	bool rocket_engine;
	bool Forward_on, Backward_on, Up_on, Down_on, Right_on, Left_on;
	bool x_positive_rot_on, y_positive_rot_on, z_positive_rot_on;
	bool x_negative_rot_on, y_negative_rot_on, z_negative_rot_on;
	bool sunlit, gimbal;

	bool has_animations;
	double gimbal_angle;

	std::string stages_file;

	float docked_rotation_x, docked_rotation_y, docked_rotation_z;

	vec3 position, velocity, movimiento_total;
	vec3 attached_position;

	Vector3 camera_prefered_axis;
	
	vec3 former_position, docked_former_position;
	float seconds_check, seconds_check_b;
	double vs, former_altitud;

	vec3 orbital_position, orbital_velocity, specific_angular_momentum, node_line, unit_vector_to_ascending_node,
		eccentricity_vector, docked_position, docked_position_b, original_docked_position, ground_speed_vec;

	int total_animations;

	Vector3 position_b, position_bullet, position_bullet_b, velocity_b, velocity_c, position_c, sunset_color, daylight_color;

	float atmosphere_thickness;
	float Forward_power, Backward_power, Up_power, Down_power, Right_power, Left_power;
	float x_positive_power, y_positive_power, z_positive_power, x_negative_power, y_negative_power, z_negative_power;

	quat total_rotation, inherited_rotation, original_docked_rotation;
	bool is_parent;

	std::string name, original_name, rings_texture;
	std::string orbited_name, target_name, docked_to_name;

	double scale, mass, empty_mass, semi_mayor_axis, semi_minor_axis, eccentricity, inclination, argument_of_perigee, longitude_of_ascending_node,
		true_anomaly, obliquity_to_orbit, sidereal_rotation_period, equatorial_radius, polar_radius,
		initial_rotation, semi_latus_rectum, orbital_period, apogee,
		perigee, orbit_radius, perigee_velocity, apogee_velocity, eccentric_anomaly, mean_anomaly, mean_motion, time_since_perigee,
		time_to_perigee, time_to_apogee, distance_to_apogee, distance_to_perigee, latitude, longitude, altitude, ground_speed, g_acceleration;

	double angulos_x, angulos_y, angulos_z;

	float angulo_total_percent, medio_angulo_percent;
	float area_total; // el area de la superficie que enfrenta al flujo
	float area_frontal;
	float area_lateral;
	float medio_angulo;
	float friccion;
	bool robotic_arm_dport, visual_helpers;
	int local_fixture, target_fixture;

	double vv;
	vec3 center_of_mass, horizontal_dir;

	float atmospheric_pressure, air_density, true_air_speed;
	vec3 dynamic_pressure, Drag, true_velocity, true_lift_vec;

	bool docking_data_updated;
	bool destroyed, rcs_traslation_enabled, bullet_constrained;
	int bullet_constrained_to;
	bool in_bullet, in_virtual_bullet;
	int bullet_id;

	double heading;

	double main_engine_power, rcs_power, hover_power, ME_current_power, Hover_current_power;
	float fuel_capacity, fuel, rcs_capacity, rcs, o2_capacity, o2, battery_capacity, battery, battery_charge_rate, ground_sensor_distance;
	float approach_dist, sea_level_pressure, sea_level_temperature, sea_level_density, min_lift, max_lift;
	int total_docking_ports, total_grapple_fixtures, total_engines, total_hover_engines, total_rcs_nozzles;
	bool* dockingPortFree;
	Vector3* docking_port_position;
	Vector3* docking_port_orientation;
	Vector3* grapple_fixture_position;
	Vector3* grapple_fixture_orientation;
	Vector3 cockpit_camera;

	Matrix World_;

	float flattening, scale_b, scale_c, distance_to_camera, ilumination, atmosphere_ratio, clouds_ratio, inner_ring, outer_ring;
	float length_, width_, height_, wing_span, FrontFriction;
	float front_area, top_area, side_area; // para calculos de friccion aerodinamica
};

#endif

