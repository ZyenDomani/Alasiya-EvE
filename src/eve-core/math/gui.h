#ifndef _GUI_H_
#define _GUI_H_

#include "stdafx.h"
#include "ventana.h"
#include "bitmap.h"
#include "textureshaderclass.h"
#include "dumb_shader.h"
#include "object.h"
#include "directx_input.h"
#include "line_class.h"
#include "kepler.h"
#include "aerodynamics.h"
#include "bullet.h"
#include "script.h"

class Gui_Class
{
	struct w_button
	{
		w_button()
		{
			dimmed = false;
			color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
			depends_on = 0;
			visible = true;
			multi_texture = true;
			toggeable = false;
			toggled_on = false;
			last_toggle_status = false;
			continuous_action = false;
			tipo_ = 3;
			clickable = true;
			clicked = false;
			ventana = -1;
			texture = new BitmapClass;
		};

		~w_button(){ SAFE_DELETE(texture); };

		int position_x, position_y, position_orig_x, position_orig_y, width, heigth, tipo_, depends_on, ventana;
		BitmapClass* texture;
		bool visible, clicked, multi_texture, toggeable, toggled_on, last_toggle_status, clickable, continuous_action;
		float dimmed;
		Color color_;
	};

	struct Intersection_data
	{
		double an_asc1, an_asc2, an_des1, an_des2, true_an_asc1, true_an_asc2, true_an_des1, true_an_des2, dtmin_asc, dtmin_des;
	};

	struct latlon { double lat, lon, h; };

public:
	Gui_Class(directx_input& input_, std::vector<objectClass>& object_, BulletClass& bullet_, AeroDynamicsClass& aerodynamics_,
		Matrix& ViewMatrix_, Matrix& ProjectionMatrix_, ScriptClass& script_b,
		Matrix& WorldMatrix_, int& total_objects_);
	~Gui_Class();

	bool draw_surface_trajectory(w_button* button_group, int n, int objeto, Color color_);
	bool Initialize_GUI(Matrix ortho_, HWND hwnd_, int screenWidth_, int screenHeight_);
	bool Initialize_ventanas();
	bool Initialize_loose_buttons();
	bool update_ventanas(int& time_acceleration_, int& control_, double& full_date_);
	void update_buttons();
	bool render();
	bool render_ventanas();
	bool render_independent_loose_buttons(w_button* button_group, int& total_group_buttons);
	bool render_window_depending_loose_buttons(w_button* button_group, int& total_group_buttons);
	bool IsPointInRect(int X, int Y, int W, int H);
	bool setWindowVisible(int ventana_, bool visible_);
	bool getWindowVisible(int ventana_);
	void set_angulos_(double angulos_);
	void update_compass(int n);
	bool draw_loose_line(Vector3 origin_, Vector3 target_, Color color_);
	bool update_priority_parameters();

	bool draw_line_strip(Vector3* buffer_, int buffer_size, Color color_);
	bool draw_2D_line_strip(Vector3* buffer_, int buffer_size, Color color_);
	bool draw_line_list(Vector3* buffer_, int buffer_size, Color color_);
	bool draw_2D_line_list(Vector3* buffer_, int buffer_size, Color color_);
	bool draw_projected_text(Vector3* position_buffer, std::string* tetx_buffer, Color* color_buffer, int buffer_size);
	bool draw_screen_text(Vector2* position_buffer, std::string* text_buffer, Color* color_buffer, int buffer_size);
	bool WorldToScreen(DirectX::XMVECTOR pV, Vector2* pOut2d);
	bool real_time_updates();

	latlon cartesian_to_geodetic2(int objeto_orbitado, vec3 posic);
	vec3 geodetic_to_cartesian(int objeto_orbitado, double lati, double longi, double alti);
	vec3 getHorizontalDir(int n);

	void UpdateCartesianToKeplerianElements(int n);
	void UpdateCartesianToKeplerianElements_inconditional(int n); // n indica el update que tiene que efectuarse incondicionalmente

	bool escape_pressed, kill_program;

private:

	void perform_loose_button_action(int n);
	void perform_textbox_action(int n);
	void kill_all_depending_buttons(w_button* button_group, int& total_group_buttons);
	void toggle_depending_buttons(w_button* button_group, int& total_group_buttons, int n);
	void kill_depending_buttons(w_button* button_group, int& total_group_buttons, int n);
	void clean_up_buttons(w_button* button_group, int& total_group_buttons, int n);
	void JdToYmd(const long lJD, int *piYear, int *piMonth, int *piDay);
	double split_time(double jd, int *year, int *month, int *day, int *hr, int *min);
	void dimm_other_buttons(w_button* button_group, int& total_group_buttons, int n);
	bool update_window_position(int ventana_number);
	bool manage_ventanas_mouse_interaction();
	bool manage_buttons_mouse_interaction(w_button* button_group, int& total_group_buttons);
	bool update_minimap_texture();
	int get_TBox_number(int ventana, int tbox_);
	bool manage_tboxes();
	Intersection_data angles_to_ascending_intersection_point(int cuerpo1, int cuerpo2);
	double time_to_reach_true_anomaly(int cuerpo, double trueAnomaly);
	double convertTrueAnomalyToEllipticalEccentricAnomaly(const double trueAnomaly,
		const double eccentricity);
	double convertEllipticalEccentricAnomalyToMeanAnomaly(const double ellipticalEccentricAnomaly,
		const double eccentricity);
	void toggle_on_off_button(int n, std::string texture_path);
	std::string convert_to_fixed_degree(double grados, int ll);
	Vector3 MapPositionPrediction(int cuerpo, int seconds_);
	void create_trajectory_buffer(int total_lines, int total_orbits, int objeto, Vector3* vec_buf_a);
	void to_screen_coords(Vector3* vec_);
	bool get_compass_dependencies_active();

	bool set_message(int vn, int msg, std::string messag_);
	bool handle_moving_button(w_button* button_group, int n);
	double angulos_;

	int total_ventanas, total_loose_buttons, total_ventanas_rectangles, mouse_x, mouse_y, mouse_original_x, mouse_original_y;

	std::string months_name[12];

	bool ventana_trapped, clicked, no_trap, button_clicked, text_box_trapped, tb_has_focus, showCaret, updated_once;
	bool re_entry_prediction;
	int focus_on, caretPos;
	float caretTick, timer_update_a, timer_update_b;
	double degrees_;

	POINT cursor_position;
	Vector3 periapsis_map_pos;
	vec3 horizontal_dir;

	Matrix ortho_matrix, matriz_identidad;
	HWND w_hwnd;
	int screenWidth, screenHeight;

	latlon geodetic_coord, geodetic_coord_target;

	ID3D11ShaderResourceView* texture_array[4];
	TextClass* m_Text;
	TextClass* textos;
	TextClass* textos_screen;
	ventana_class* ventanas;
	BitmapClass* window_rectangle_bitmap;
	BitmapClass* window_text_box;
	text_box_class* text_box;
	w_button* loose_button;
	TextureShaderClass* m_TextureShader;
	DumbShaderClass* dumb_shader;
	color_class* color;
	KeplerClass* kepler;
	Matrix giro;

	int& total_objects;
	Matrix& WorldMatrix;
	Matrix& ViewMatrix;
	Matrix& ProjectionMatrix;
	directx_input& input;
	std::vector<objectClass>& object;
	AeroDynamicsClass& aerodynamics;
	BulletClass& bullet_physics;
	ScriptClass& script_;
};


#endif