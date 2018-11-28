#include "stdafx.h"
#include "gui.h"

bool switched_shadows = true;
bool switched_object = true;
int star_object = 0;
Config_ Config;

float update_messages_interval = 0.2f;
float seconds_since_last_messages_update = 200.0f;

Gui_Class::Gui_Class(directx_input& input_, std::vector<objectClass>& object_, BulletClass& bullet_, AeroDynamicsClass& aerodynamics_,
	Matrix& ViewMatrix_, Matrix& ProjectionMatrix_, ScriptClass& script_b,
	Matrix& WorldMatrix_, int& total_objects_) :
	input(input_), object(object_), bullet_physics(bullet_), aerodynamics(aerodynamics_), ViewMatrix(ViewMatrix_),
	ProjectionMatrix(ProjectionMatrix_), script_(script_b),
	WorldMatrix(WorldMatrix_),
	total_objects(total_objects_)
{
	total_ventanas = 0;
	total_loose_buttons = 0;
	total_ventanas_rectangles = 0;
	mouse_x = 0;
	mouse_y = 0;
	mouse_original_x = 0;
	mouse_original_y = 0;
	caretTick = 0.0f;
	timer_update_a = 0.f;
	timer_update_b = 0.f;
	angulos_ = 0.0;

	degrees_ = 0.0;

	focus_on = -1;
	tb_has_focus = false;
	kill_program = false;
	escape_pressed = false;
	button_clicked = false;
	ventana_trapped = false;
	clicked = false;
	no_trap = false;
	text_box_trapped = false;
	showCaret = false;
	updated_once = false;
	re_entry_prediction = false;

	months_name[0] = "Jan";
	months_name[1] = "Feb";
	months_name[2] = "Mar";
	months_name[3] = "Apr";
	months_name[4] = "May";
	months_name[5] = "Jun";
	months_name[6] = "Jul";
	months_name[7] = "Aug";
	months_name[8] = "Sep";
	months_name[9] = "Oct";
	months_name[10] = "Nov";
	months_name[11] = "Dec";

	giro = DirectX::XMMatrixIdentity();

	horizontal_dir = { 0.0, 0.0, 0.0 };
	geodetic_coord = latlon{ 0.0, 0.0, 0.0 };
	geodetic_coord_target = latlon{ 0.0, 0.0, 0.0 };
	periapsis_map_pos = Vector3{ 0.f, 0.f, 0.f };
	cursor_position = POINT{ 0, 0 };

	texture_array[0] = nullptr;
	texture_array[1] = nullptr;
	texture_array[2] = nullptr;
	texture_array[3] = nullptr;

	kepler = nullptr;
	ventanas = nullptr;
	m_Text = nullptr;
	textos = nullptr;
	textos_screen = nullptr;
	window_rectangle_bitmap = nullptr;
	window_text_box = nullptr;
	text_box = nullptr;
	m_TextureShader = nullptr;
	dumb_shader = nullptr;
	color = nullptr;
	loose_button = nullptr;

	matriz_identidad.Identity;
}

Gui_Class::~Gui_Class()
{
	SAFE_DELETE(kepler);
	SAFE_DELETE(dumb_shader);
	SAFE_DELETE_ARRAY(loose_button);
	SAFE_RELEASE(texture_array[0]);
	SAFE_RELEASE(texture_array[1]);
	SAFE_RELEASE(texture_array[2]);
	SAFE_RELEASE(texture_array[3]);
	SAFE_DELETE_ARRAY(ventanas);
	SAFE_DELETE(textos);
	SAFE_DELETE(textos_screen);
	SAFE_DELETE(m_Text);
	SAFE_DELETE_ARRAY(window_rectangle_bitmap);
	SAFE_DELETE_ARRAY(window_text_box);
	SAFE_DELETE_ARRAY(text_box);
	SAFE_DELETE(color);
	SAFE_DELETE(m_TextureShader);
}

bool Gui_Class::Initialize_GUI(Matrix ortho_, HWND hwnd_, int screenWidth_, int screenHeight_)
{
	color = new color_class;
	if (!color) return false;

	ortho_matrix = ortho_;
	w_hwnd = hwnd_;
	screenWidth = screenWidth_;
	screenHeight = screenHeight_;

	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader->Initialize(w_hwnd)) return false;

	dumb_shader = new DumbShaderClass;
	if (!dumb_shader->Initialize()) return false;

	// Create the text object.
	m_Text = new TextClass;
	if (!m_Text->Initialize(ortho_matrix, w_hwnd, screenWidth, screenHeight)) return false;

	// Create the loose projected text object.
	textos = new TextClass;
	if (!textos->Initialize(ortho_matrix, w_hwnd, screenWidth, screenHeight)) return false;
	textos->create_sentence_array(1);

	// Create the loose screen text object.
	textos_screen = new TextClass;
	if (!textos_screen->Initialize(ortho_matrix, w_hwnd, screenWidth, screenHeight)) return false;
	textos_screen->create_sentence_array(1);

	kepler = new KeplerClass(object);
	if (!kepler) return false;

	if (!Initialize_ventanas()) return false;
	if (!Initialize_loose_buttons()) return false;

	return true;
}

bool Gui_Class::Initialize_ventanas()
{
	total_ventanas = 13;
	int vn = 0;
	int msg = 0;
	int t_box = 0;
	total_ventanas_rectangles = 3;

	ventanas = new ventana_class[total_ventanas];

	vn = 0;
	if (!ventanas[vn].initialize("Orbital Information", 1, 20, NULL, 20, 400,
		250, 190, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 4;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 62);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 5;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 62);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 6;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 79);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 7;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 79);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 8;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 96);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 9;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 96);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 10;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 113);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 11;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 113);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 12;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 130);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 13;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 130);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 14;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 147);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 15;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 147);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 16;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 164);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 17;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 164);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 18;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 181);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 19;
	ventanas[vn].set_default_message_pos_x(msg, 55);
	ventanas[vn].set_default_message_pos_y(msg, 181);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	vn = 1;
	if (!ventanas[vn].initialize("General Info", 1, 9, NULL, screenWidth - 280, 20,
		270, 110, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 4;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 67);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 5;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 67);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 6;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 84);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 7;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 84);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);
	//****

	msg = 8;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 101);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	vn = 2;
	if (!ventanas[vn].initialize("Object selection", 1, 1, 1, screenWidth / 2 - 320 / 2,
		screenHeight / 2 - 88 / 2,
		320, 60, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 40);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	t_box = 0;
	ventanas[vn].set_tb_box_color(t_box, color->dim_color(0.8f, color->Black));
	ventanas[vn].set_tb_text_color(t_box, color->dim_color(0.8f, color->Banana));
	ventanas[vn].set_tb_type(t_box, 0);
	ventanas[vn].set_tb_heigth(t_box, 28);
	ventanas[vn].set_tb_width(t_box, 227);
	ventanas[vn].set_tb_posx(t_box, 45);
	ventanas[vn].set_tb_posy(t_box, 32);
	ventanas[vn].set_tb_max_size(t_box, 100);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	vn = 3;
	if (!ventanas[vn].initialize("Docking Information", 1, 14, NULL,
		screenWidth - 775, 20, 320, 150, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 100);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 100);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 4;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 62);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 5;
	ventanas[vn].set_default_message_pos_x(msg, 100);
	ventanas[vn].set_default_message_pos_y(msg, 62);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 6;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 79);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 7;
	ventanas[vn].set_default_message_pos_x(msg, 100);
	ventanas[vn].set_default_message_pos_y(msg, 79);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 8;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 96);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 9;
	ventanas[vn].set_default_message_pos_x(msg, 100);
	ventanas[vn].set_default_message_pos_y(msg, 96);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 10;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 113);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 11;
	ventanas[vn].set_default_message_pos_x(msg, 100);
	ventanas[vn].set_default_message_pos_y(msg, 113);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 12;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 130);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 13;
	ventanas[vn].set_default_message_pos_x(msg, 100);
	ventanas[vn].set_default_message_pos_y(msg, 130);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	vn = 4;
	if (!ventanas[vn].initialize("Target selection", 1, 1, 1, screenWidth / 2 - 320 / 2,
		screenHeight / 2 - 88 / 2,
		320, 60, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 40);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	t_box = 0;
	ventanas[vn].set_tb_box_color(t_box, color->dim_color(0.8f, color->Black));
	ventanas[vn].set_tb_text_color(t_box, color->dim_color(0.8f, color->Banana));
	ventanas[vn].set_tb_type(t_box, 0);
	ventanas[vn].set_tb_heigth(t_box, 28);
	ventanas[vn].set_tb_width(t_box, 227);
	ventanas[vn].set_tb_posx(t_box, 45);
	ventanas[vn].set_tb_posy(t_box, 32);
	ventanas[vn].set_tb_max_size(t_box, 100);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// la ventana de alcance de objetivos en orbita
	vn = 5;
	if (!ventanas[vn].initialize("Orbit Interception (Rendezvous)", 1, 57, NULL,
		screenWidth - 420, 122, 420, 280, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 4;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 62);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 5;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 62);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 6;
	ventanas[vn].set_default_message_pos_x(msg, 190);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 7;
	ventanas[vn].set_default_message_pos_x(msg, 260);
	ventanas[vn].set_default_message_pos_y(msg, 28);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 8;
	ventanas[vn].set_default_message_pos_x(msg, 190);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 9;
	ventanas[vn].set_default_message_pos_x(msg, 260);
	ventanas[vn].set_default_message_pos_y(msg, 45);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 10;
	ventanas[vn].set_default_message_pos_x(msg, 190);
	ventanas[vn].set_default_message_pos_y(msg, 62);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 11;
	ventanas[vn].set_default_message_pos_x(msg, 260);
	ventanas[vn].set_default_message_pos_y(msg, 62);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 12;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 96);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 13;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 96);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 14;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 113);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 15;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 113);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 16;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 130);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 17;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 130);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 18;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 147);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 19;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 147);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 20;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 164);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 21;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 164);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 22;
	ventanas[vn].set_default_message_pos_x(msg, 190);
	ventanas[vn].set_default_message_pos_y(msg, 113);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 23;
	ventanas[vn].set_default_message_pos_x(msg, 230);
	ventanas[vn].set_default_message_pos_y(msg, 113);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 24;
	ventanas[vn].set_default_message_pos_x(msg, 320);
	ventanas[vn].set_default_message_pos_y(msg, 113);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 25;
	ventanas[vn].set_default_message_pos_x(msg, 207);
	ventanas[vn].set_default_message_pos_y(msg, 130);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 26;
	ventanas[vn].set_default_message_pos_x(msg, 200);
	ventanas[vn].set_default_message_pos_y(msg, 147);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 27;
	ventanas[vn].set_default_message_pos_x(msg, 200);
	ventanas[vn].set_default_message_pos_y(msg, 164);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 28;
	ventanas[vn].set_default_message_pos_x(msg, 200);
	ventanas[vn].set_default_message_pos_y(msg, 181);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 29;
	ventanas[vn].set_default_message_pos_x(msg, 200);
	ventanas[vn].set_default_message_pos_y(msg, 198);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 30;
	ventanas[vn].set_default_message_pos_x(msg, 230);
	ventanas[vn].set_default_message_pos_y(msg, 130);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 31;
	ventanas[vn].set_default_message_pos_x(msg, 320);
	ventanas[vn].set_default_message_pos_y(msg, 130);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 32;
	ventanas[vn].set_default_message_pos_x(msg, 230);
	ventanas[vn].set_default_message_pos_y(msg, 147);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 33;
	ventanas[vn].set_default_message_pos_x(msg, 320);
	ventanas[vn].set_default_message_pos_y(msg, 147);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 34;
	ventanas[vn].set_default_message_pos_x(msg, 230);
	ventanas[vn].set_default_message_pos_y(msg, 164);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 35;
	ventanas[vn].set_default_message_pos_x(msg, 320);
	ventanas[vn].set_default_message_pos_y(msg, 164);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 36;
	ventanas[vn].set_default_message_pos_x(msg, 230);
	ventanas[vn].set_default_message_pos_y(msg, 181);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 37;
	ventanas[vn].set_default_message_pos_x(msg, 320);
	ventanas[vn].set_default_message_pos_y(msg, 181);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 38;
	ventanas[vn].set_default_message_pos_x(msg, 230);
	ventanas[vn].set_default_message_pos_y(msg, 198);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 39;
	ventanas[vn].set_default_message_pos_x(msg, 320);
	ventanas[vn].set_default_message_pos_y(msg, 198);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 40;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 198);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 41;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 198);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 42;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 215);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 43;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 215);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 44;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 232);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 45;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 232);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 46;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 249);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 47;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 249);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 48;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 266);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 49;
	ventanas[vn].set_default_message_pos_x(msg, 70);
	ventanas[vn].set_default_message_pos_y(msg, 266);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 50;
	ventanas[vn].set_default_message_pos_x(msg, 190);
	ventanas[vn].set_default_message_pos_y(msg, 232);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 51;
	ventanas[vn].set_default_message_pos_x(msg, 240);
	ventanas[vn].set_default_message_pos_y(msg, 232);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 52;
	ventanas[vn].set_default_message_pos_x(msg, 190);
	ventanas[vn].set_default_message_pos_y(msg, 79);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 53;
	ventanas[vn].set_default_message_pos_x(msg, 260);
	ventanas[vn].set_default_message_pos_y(msg, 79);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 54;
	ventanas[vn].set_default_message_pos_x(msg, 190);
	ventanas[vn].set_default_message_pos_y(msg, 249);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 55;
	ventanas[vn].set_default_message_pos_x(msg, 275);
	ventanas[vn].set_default_message_pos_y(msg, 249);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 56;
	ventanas[vn].set_default_message_pos_x(msg, 160);
	ventanas[vn].set_default_message_pos_y(msg, 266);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Red));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// ventana de control de RCS
	vn = 6;
	if (!ventanas[vn].initialize("Reaction Control System", 1, 11, NULL, 20, 20,
		190, 270, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 35);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 57);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 79);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 101);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 4;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 123);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 5;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 146);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 6;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 168);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 7;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 190);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 8;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 212);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 9;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 236);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 10;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 258);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// ventana del piloto automatico
	vn = 7;
	if (!ventanas[vn].initialize("Auto pilot", 1, 4, 1,
		220, 120,
		250, 140,
		color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 35);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 57);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 84);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 140);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	t_box = 0;
	ventanas[vn].set_tb_box_color(t_box, color->dim_color(0.8f, color->Black));
	ventanas[vn].set_tb_text_color(t_box, color->dim_color(0.8f, color->Banana));
	ventanas[vn].set_tb_type(t_box, 1);
	ventanas[vn].set_tb_heigth(t_box, 28);
	ventanas[vn].set_tb_width(t_box, 100);
	ventanas[vn].set_tb_posx(t_box, 0);
	ventanas[vn].set_tb_posy(t_box, 105);
	ventanas[vn].set_tb_max_size(t_box, 100);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	vn = 8;
	if (!ventanas[vn].initialize("Surface Map", 2, 6, NULL, screenWidth - 435, 440,
		440, 245, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;


	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 245);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 30);
	ventanas[vn].set_default_message_pos_y(msg, 245);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 145);
	ventanas[vn].set_default_message_pos_y(msg, 245);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 175);
	ventanas[vn].set_default_message_pos_y(msg, 245);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Azure04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 4;
	ventanas[vn].set_default_message_pos_x(msg, 290);
	ventanas[vn].set_default_message_pos_y(msg, 245);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Silver));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);
	//****

	msg = 5;
	ventanas[vn].set_default_message_pos_x(msg, 320);
	ventanas[vn].set_default_message_pos_y(msg, 245);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// ventana del Flight Console
	vn = 9;
	if (!ventanas[vn].initialize("Flight Console", 1, 4, NULL, 220, 20,
		250, 60, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 35);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 150);
	ventanas[vn].set_default_message_pos_y(msg, 35);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Banana));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 52);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 150);
	ventanas[vn].set_default_message_pos_y(msg, 52);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Banana));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// ventana de los datos de vuelos atmosfericos
	vn = 10;
	if (!ventanas[vn].initialize("Atmospheric flight", 1, 14, NULL, 20, 450,
		300, 150, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 35);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 50);
	ventanas[vn].set_default_message_pos_y(msg, 35);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 52);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 50);
	ventanas[vn].set_default_message_pos_y(msg, 52);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 4;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 69);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 5;
	ventanas[vn].set_default_message_pos_x(msg, 50);
	ventanas[vn].set_default_message_pos_y(msg, 69);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 6;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 86);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 7;
	ventanas[vn].set_default_message_pos_x(msg, 50);
	ventanas[vn].set_default_message_pos_y(msg, 86);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 8;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 103);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 9;
	ventanas[vn].set_default_message_pos_x(msg, 50);
	ventanas[vn].set_default_message_pos_y(msg, 103);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 10;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 120);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 11;
	ventanas[vn].set_default_message_pos_x(msg, 50);
	ventanas[vn].set_default_message_pos_y(msg, 120);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 12;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 137);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 13;
	ventanas[vn].set_default_message_pos_x(msg, 50);
	ventanas[vn].set_default_message_pos_y(msg, 137);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// ventana del canadarm
	vn = 11;
	if (!ventanas[vn].initialize("Robotic Arm Console", 1, 9, 2,
		220, 20,
		250, 225,
		color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 30);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 1;
	ventanas[vn].set_default_message_pos_x(msg, 100);
	ventanas[vn].set_default_message_pos_y(msg, 30);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Light_golden_rod03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 2;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 55);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 3;
	ventanas[vn].set_default_message_pos_x(msg, 95);
	ventanas[vn].set_default_message_pos_y(msg, 55);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray03));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 4;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 115);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 5;
	ventanas[vn].set_default_message_pos_x(msg, 50);
	ventanas[vn].set_default_message_pos_y(msg, 115);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Gray));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, true);

	msg = 6;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 135);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 7;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 213);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	msg = 8;
	ventanas[vn].set_default_message_pos_x(msg, 106);
	ventanas[vn].set_default_message_pos_y(msg, 213);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	t_box = 0;
	ventanas[vn].set_tb_box_color(t_box, color->dim_color(0.01f, color->Black));
	ventanas[vn].set_tb_text_color(t_box, color->dim_color(0.8f, color->Banana));
	ventanas[vn].set_tb_type(t_box, 1);
	ventanas[vn].set_tb_heigth(t_box, 28);
	ventanas[vn].set_tb_width(t_box, 45);
	ventanas[vn].set_tb_posx(t_box, 50);
	ventanas[vn].set_tb_posy(t_box, 205);
	ventanas[vn].set_tb_max_size(t_box, 100);

	t_box = 1;
	ventanas[vn].set_tb_box_color(t_box, color->dim_color(0.01f, color->Black));
	ventanas[vn].set_tb_text_color(t_box, color->dim_color(0.8f, color->Banana));
	ventanas[vn].set_tb_type(t_box, 1);
	ventanas[vn].set_tb_heigth(t_box, 28);
	ventanas[vn].set_tb_width(t_box, 45);
	ventanas[vn].set_tb_posx(t_box, 156);
	ventanas[vn].set_tb_posy(t_box, 205);
	ventanas[vn].set_tb_max_size(t_box, 100);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	vn = 12;
	if (!ventanas[vn].initialize("Grapple Target selection", 1, 1, 1, screenWidth / 2 - 320 / 2,
		screenHeight / 2 - 88 / 2,
		320, 60, color->dim_color(0.8f, color->White), { 0.0f, 0.0f, 0.0f, 1.0f })) return false;

	msg = 0;
	ventanas[vn].set_default_message_pos_x(msg, 0);
	ventanas[vn].set_default_message_pos_y(msg, 40);
	ventanas[vn].set_message_color(msg, color->dim_color(0.8f, color->Dark_slate_gray04));
	ventanas[vn].set_message_size(msg, 10);
	ventanas[vn].set_message_dynamic(msg, false);

	t_box = 0;
	ventanas[vn].set_tb_box_color(t_box, color->dim_color(0.8f, color->Black));
	ventanas[vn].set_tb_text_color(t_box, color->dim_color(0.8f, color->Banana));
	ventanas[vn].set_tb_type(t_box, 0);
	ventanas[vn].set_tb_heigth(t_box, 28);
	ventanas[vn].set_tb_width(t_box, 227);
	ventanas[vn].set_tb_posx(t_box, 45);
	ventanas[vn].set_tb_posy(t_box, 32);
	ventanas[vn].set_tb_max_size(t_box, 100);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	int total_messages = 0;
	for (int n = 0; n < total_ventanas; n++) total_messages += (ventanas[n].getTotalMessages() + 1);

	m_Text->create_sentence_array(total_messages);

	int msgs = -1;
	for (int vent_ = 0; vent_ < total_ventanas; vent_++)
	{
		msgs++;
		if (!m_Text->InitializeSentence(msgs, int(ventanas[vent_].getTitle().size()),
			ventanas[vent_].getVisible())) return false;
		ventanas[vent_].set_title_sentence_number(msgs);

		for (int n = 0; n < ventanas[vent_].getTotalMessages(); n++)
		{
			msgs++;
			if (!m_Text->InitializeSentence(msgs, ventanas[vent_].get_message_size(n),
				ventanas[vent_].getVisible())) return false;
			ventanas[vent_].set_message_sentence_number(n, msgs);
		}
	}

	// por defecto seteamos todas las ventanas como no visibles
	for (int n = 0; n < total_ventanas; n++)
	{
		if (!setWindowVisible(n, false)) return false;
	}

	// creamos la lista de ventanas que son visibles inicialmente
	//if (!setWindowVisible(0, true)) return false; // la ventana de informacion orbital
	if (!setWindowVisible(1, true)) return false; // la ventana de informacion general
	if (!setWindowVisible(6, true)) return false; // la ventana del RCS
	//if (!setWindowVisible(7, true)) return false; // la ventana del piloto automatico
	//if (!setWindowVisible(8, true)) return false; // la ventana del mapa de superficie
	if (!setWindowVisible(9, true)) return false; // la ventana de la Consola de mando
	if (!setWindowVisible(10, true)) return false; // la ventana del vuelo atmosferico

	window_rectangle_bitmap = new BitmapClass[total_ventanas * total_ventanas_rectangles];

	// creamos los bitmaps que acompanhan a cada ventana
	for (int n = 0; n < total_ventanas; n++)
	{
		if (!window_rectangle_bitmap[n * total_ventanas_rectangles].Initialize(screenWidth, screenHeight,
			"textures/white.png", "textures/white.png", "textures/white.png", "textures/white.png", ventanas[n].getWidth(), 28)) return false;

		if (ventanas[n].getType() == 1)
		{
			if (!window_rectangle_bitmap[n * total_ventanas_rectangles + 1].Initialize(screenWidth, screenHeight,
				"textures/white.png", "textures/white.png", "textures/white.png", "textures/white.png", ventanas[n].getWidth(), ventanas[n].getHeight())) return false;
		}

		else // creamos las imagenes de las ventanas con fondos personalizados
		{
			if (n == 8) update_minimap_texture();
		}

		if (!window_rectangle_bitmap[n * total_ventanas_rectangles + 2].Initialize(screenWidth, screenHeight,
			"textures/gui/cw_iddle.png", "textures/gui/cw_hover.png", "textures/gui/cw_clicked.png", "textures/white.png", 20, 20)) return false;
	}


	int total_text_boxes = 0;
	for (int n = 0; n < total_ventanas; n++)
	{
		total_text_boxes += (ventanas[n].getTotalTextBoxes());
	}

	text_box = new text_box_class[total_text_boxes];
	window_text_box = new BitmapClass[total_text_boxes];

	// creamos los fondos de los cuadros de texto y los text_boxes, si la ventana tiene alguno
	int box_num_ = 0;
	for (int n = 0; n < total_ventanas; n++)
	{
		if (ventanas[n].getTotalTextBoxes()>0)
		{
			for (int nn = 0; nn < int(ventanas[n].getTotalTextBoxes()); nn++)
			{
				box_num_ = get_TBox_number(n, nn);

				// creamos el bitmap que va a servir de fondo al texto
				if (!window_text_box[box_num_].Initialize(screenWidth, screenHeight,
					"textures/white.png", "textures/white.png", "textures/white.png", "textures/white.png",
					ventanas[n].get_tb_width(nn), ventanas[n].get_tb_heigth(nn))) return false;

				// creamos el text_box y posicionamos el texto dentro del bitmap
				if (!text_box[box_num_].Initialize(ortho_matrix, w_hwnd, screenWidth, screenHeight,
					ventanas[n].get_tb_posx(nn) + 5,
					ventanas[n].get_tb_posy(nn) + 8, ventanas[n].get_tb_text_color(nn), n, ventanas[n].get_tb_max_size(nn),
					ventanas[n].get_tb_type(nn))) return false;

				text_box[box_num_].setPosX(text_box[box_num_].getOrigX() + ventanas[n].getPositionX());
				text_box[box_num_].setPosY(text_box[box_num_].getOrigY() + ventanas[n].getPositionY());
				text_box[box_num_].update();
			}
		}
	}

	return true;
}

// 	MessageBoxA(w_hwnd, "Hasta aqui llegamos.", "OK", MB_OK);

bool Gui_Class::update_ventanas(int& time_acceleration_, int& control, double& full_date_)
{
	update_minimap_texture();

	UpdateCartesianToKeplerianElements(control);

	int ventana = 0;
	std::stringstream stringstream_;
	std::string string_;

	ventana = 0;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		set_message(ventana, 0, "Ecc:");
		stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(5) << object[control].getEccentricity();
		set_message(ventana, 1, stringstream_.str());

		set_message(ventana, 2, "Per:");
		string_ = format_distance_output(object[control].getPerigee() - object[object[control].getOrbiting()].getScale() / 2.0);
		set_message(ventana, 3, string_);

		set_message(ventana, 4, "Apo:");
		string_ = format_distance_output(object[control].getApogee() - object[object[control].getOrbiting()].getScale() / 2.0);
		set_message(ventana, 5, string_);

		set_message(ventana, 6, "PeD:");
		string_ = format_distance_output(object[control].getDistance_to_perigee());
		set_message(ventana, 7, string_);

		set_message(ventana, 8, "ApD:");
		string_ = format_distance_output(object[control].getDistance_to_apogee());
		set_message(ventana, 9, string_);

		set_message(ventana, 10, "PeT:");
		string_ = format_time_output(object[control].getTime_to_perigee());
		set_message(ventana, 11, string_);

		set_message(ventana, 12, "ApT:");
		string_ = format_time_output(object[control].getTime_to_apogee());
		set_message(ventana, 13, string_);

		set_message(ventana, 14, "OrP:");
		string_ = format_time_output(object[control].getOrbitalPeriod());
		set_message(ventana, 15, string_);

		set_message(ventana, 16, "OrD:");
		string_ = format_distance_output(object[control].getOrbitRadius() - object[object[control].getOrbiting()].getScale() / 2.0);
		set_message(ventana, 17, string_);

		set_message(ventana, 18, "GSp:");
		set_message(ventana, 19, format_speed_output(object[control].getGroundSpeed()));
	}



	ventana = 1;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		set_message(ventana, 0, "Object:");
		set_message(ventana, 1, object[control].getName());
		//set_message(ventana, 1, std::to_string(robotic_arm[0].handled_part));
		//set_message(ventana, 1, std::to_string(object[control].getBulletId()));
		//set_message(ventana, 1, Config.display_info);

		if (object[control].getDockedTo() == -1)
		{
			if (object[control].getInAtmosphere()) set_message(ventana, 2, "Atmos.:");
			else set_message(ventana, 2, "Orbit:");
		}

		else
		{
			if (object[control].getDockedTo() == object[control].getOrbiting()) set_message(ventana, 2, "Ground:");
			else set_message(ventana, 2, "Dock:");
		}

		if (object[control].getDockedTo() == -1)
		{
			if (object[control].getOrbiting() >= 0) set_message(ventana, 3, object[object[control].getOrbiting()].getName());
			else set_message(ventana, 3, "N/A");
		}

		else set_message(ventana, 3, object[object[control].getDockedTo()].getName());

		set_message(ventana, 4, "Date:");

		int anho_conv = 0;
		int mes_conv = 0;
		int dia_conv = 0;
		int hora_conv = 0;
		int minuto_conv = 0;
		int segundo_conv = int(split_time(full_date_, &anho_conv, &mes_conv, &dia_conv, &hora_conv, &minuto_conv));

		std::stringstream mesillos, diillos, horillas, minutillos, segundillos;
		diillos << dia_conv;
		horillas << std::setfill('0') << std::setw(2) << hora_conv;
		minutillos << std::setfill('0') << std::setw(2) << minuto_conv;
		segundillos << std::setfill('0') << std::setw(2) << segundo_conv;
		std::string mensaje_ = " ";

		mensaje_ = months_name[mes_conv - 1] + " " + diillos.str() + ", " + std::to_string(anho_conv) + " (" + horillas.str() + ":" + minutillos.str() + ":" + segundillos.str() + " UTC" + ")";
		set_message(ventana, 5, mensaje_);

		set_message(ventana, 6, "Julian:");
		set_message(ventana, 7, std::to_string(full_date_));

		std::stringstream time_;
		time_.imbue(std::locale(""));
		time_ << std::setiosflags(std::ios::fixed) << std::setprecision(0) << time_acceleration_;
		set_message(ventana, 8, "Time x" + time_.str());
	}


	ventana = 2;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		set_message(ventana, 0, "Name:");
	}


	ventana = 3;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		int target_ = object[control].getTarget();
		int closest_port = 0;
		int closest_local_port = 0;
		int total_ports_target = object[target_].getTotalDockingPorts();
		int total_ports_control = object[control].getTotalDockingPorts();
		float distancia = 1e10f;
		float angul = 0.f;

		if (total_ports_target > 0 && total_ports_control > 0)
		{
			for (int n = 0; n < total_ports_target; n++)
			{
				for (int nn = 0; nn < total_ports_control; nn++)
				{
					float distanciab = distances_f(object[control].getDockingPortWorldPosition(nn), object[target_].getDockingPortWorldPosition(n)) / universe_scale;
					if (distanciab < distancia)
					{
						Vector3 vec_a = (object[control].getDockingPortWorldPosition(nn) - object[control].getDockingPortWorldOrientation(nn));
						Vector3 vec_b = (object[target_].getDockingPortWorldPosition(n) - object[target_].getDockingPortWorldOrientation(n));

						angul = angle_from_two_vectors_f(-vec_a, vec_b);
						distancia = distanciab;
						closest_port = n;
						closest_local_port = nn;
					}
				}
			}

			//stringstream_.str(std::string()); // clears stringstream_
			set_message(ventana, 0, "Target:");
			set_message(ventana, 1, object[target_].getName());

			set_message(ventana, 2, "Docking ports:");
			string_ = std::to_string(object[target_].getTotalDockingPorts());
			set_message(ventana, 3, string_);

			set_message(ventana, 4, "Target port #:");
			string_ = std::to_string(closest_port + 1);
			set_message(ventana, 5, string_);

			set_message(ventana, 6, "Port distance:");
			set_message(ventana, 7, format_distance_output(distancia));
			if (distancia <= 0.1f) ventanas[ventana].set_message_color(7, color->dim_color(0.8f, color->Green));
			else
			{
				ventanas[ventana].set_message_color(7, color->dim_color(0.8f, color->Red));
				object[control].setDockReady(false);
			}

			set_message(ventana, 8, "Relative angle:");
			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << ToDegrees_f(angul);
			set_message(ventana, 9, stringstream_.str() + "}");
			if (angul <= 0.01f)
			{
				ventanas[ventana].set_message_color(9, color->dim_color(0.8f, color->Green));
				if (distancia <= 0.1f) object[control].setDockReady(true);
			}

			else
			{
				ventanas[ventana].set_message_color(9, color->dim_color(0.8f, color->Red));
				object[control].setDockReady(false);
			}

			set_message(ventana, 10, "Relative speed:");
			set_message(ventana, 11, format_speed_output(length(object[target_].getVelocity() - object[control].getVelocity())));

			set_message(ventana, 12, "Status:");
			if (object[control].getDockedTo() == -1)
			{
				string_ = "NOT DOCKED";
				ventanas[ventana].set_message_color(13, color->dim_color(0.8f, color->Red));
			}

			else
			{
				if (object[control].getDockedTo() == object[control].getTarget())
				{
					string_ = "DOCKED, Local Port #" + std::to_string(closest_local_port + 1);
					ventanas[ventana].set_message_color(13, color->dim_color(0.8f, color->Green));
				}

				else
				{
					string_ = "NOT DOCKED";
					ventanas[ventana].set_message_color(13, color->dim_color(0.8f, color->Red));
				}
			}
			set_message(ventana, 13, string_);
		}

		else
		{
			object[control].setDockReady(false);

			set_message(ventana, 0, "Target:");
			if (object[control].getTarget() == 0) set_message(ventana, 1, "None");
			else set_message(ventana, 1, object[target_].getName());

			set_message(ventana, 2, "Docking ports:");
			set_message(ventana, 3, "N/A");

			set_message(ventana, 4, "Target port #:");
			set_message(ventana, 5, "N/A");

			set_message(ventana, 6, "Port distance:");
			set_message(ventana, 7, "N/A");

			set_message(ventana, 8, "Relative angle:");
			set_message(ventana, 9, "N/A");

			set_message(ventana, 10, "Relative speed:");
			set_message(ventana, 11, "N/A");

			set_message(ventana, 12, "Status:");
			set_message(ventana, 13, "N/A");
		}
	}


	ventana = 4;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		set_message(ventana, 0, "Target:");
	}


	// Rendezvous window
	ventana = 5;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		int target_ = object[control].getTarget();

		set_message(ventana, 0, "Object:");

		set_message(ventana, 2, "Inclination:");

		set_message(ventana, 4, "LAN:");

		set_message(ventana, 6, "Target:");

		set_message(ventana, 8, "Inclination:");

		set_message(ventana, 10, "LAN:");

		set_message(ventana, 12, "Rel. Incl.:");

		set_message(ventana, 14, "Rel. Vel.:");

		set_message(ventana, 16, "Rel. Alt.:");

		set_message(ventana, 18, "Distance:");

		set_message(ventana, 20, "Vert. vel.:");

		set_message(ventana, 22, "Orb.");
		set_message(ventana, 23, "Orb1-TR");
		set_message(ventana, 24, "Orb2-TR");
		set_message(ventana, 25, "0");
		set_message(ventana, 26, "+1");
		set_message(ventana, 27, "+2");
		set_message(ventana, 28, "+3");
		set_message(ventana, 29, "+4");

		set_message(ventana, 40, "DTmin.:");

		set_message(ventana, 42, "Dist. des.:");

		set_message(ventana, 44, "Dist. asc.:");

		set_message(ventana, 46, "D/N:");

		set_message(ventana, 48, "A/N:");

		set_message(ventana, 50, "Action:");

		set_message(ventana, 52, "OrD:");

		set_message(ventana, 54, "Time to Rdv:");

		if (target_ != 0)
		{
			double min_rendezvous_distance_orange = 100000; // minimum distance in meters to check to give orange OK
			double min_rendezvous_distance_green = 50000; // minimum distance in meters to check to give green OK

			set_message(ventana, 1, object[control].getName());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << ToDegrees_d(object[control].getInclination());
			set_message(ventana, 3, stringstream_.str() + "}");

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << ToDegrees_d(object[control].getLongitudeOfAscendingNode());
			set_message(ventana, 5, stringstream_.str() + "}");

			set_message(ventana, 7, object[object[control].getTarget()].getName());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << ToDegrees_d(object[target_].getInclination());
			set_message(ventana, 9, stringstream_.str() + "}");

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << ToDegrees_d(object[target_].getLongitudeOfAscendingNode());
			set_message(ventana, 11, stringstream_.str() + "}");

			stringstream_.str(std::string()); // clears stringstream_
			double rel_incl = ToDegrees_d(abs(object[control].getInclination() - object[target_].getInclination()));
			if (rel_incl > 0.5) ventanas[ventana].set_message_color(13, color->dim_color(0.8f, color->Red));
			else ventanas[ventana].set_message_color(13, color->dim_color(0.8f, color->Green));
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << rel_incl;
			set_message(ventana, 13, stringstream_.str() + "}");

			string_ = format_speed_output(length(object[control].getOrbitalVelocity() - object[target_].getOrbitalVelocity()));
			set_message(ventana, 15, string_);

			static double Altitud = 0.0;
			static double AltitudAnterior = 0.0;
			AltitudAnterior = Altitud;

			Altitud = object[control].getOrbitRadius() - object[target_].getOrbitRadius();
			string_ = format_distance_output(Altitud);
			set_message(ventana, 17, string_);

			string_ = format_distance_output(distances(object[control].getPosition(), object[target_].getPosition()));
			set_message(ventana, 19, string_);

			set_message(ventana, 21, format_speed_output(Altitud - AltitudAnterior));

			Intersection_data anomalies = angles_to_ascending_intersection_point(control, target_);

			double time_to_reach1 = 0.0;
			double time_to_reach2 = 0.0;

			if (anomalies.dtmin_asc < anomalies.dtmin_des)
			{
				time_to_reach1 = time_to_reach_true_anomaly(control, anomalies.true_an_asc1);
				time_to_reach2 = time_to_reach_true_anomaly(target_, anomalies.true_an_des2);
				if (time_to_reach1 < 0)
					time_to_reach1 += object[control].getOrbitalPeriod();
				if (time_to_reach2 < 0)
					time_to_reach2 += object[target_].getOrbitalPeriod();
			}

			else
			{
				time_to_reach1 = time_to_reach_true_anomaly(control, anomalies.true_an_des1);
				time_to_reach2 = time_to_reach_true_anomaly(target_, anomalies.true_an_asc2);
				if (time_to_reach1 < 0)
					time_to_reach1 += object[control].getOrbitalPeriod();
				if (time_to_reach2 < 0)
					time_to_reach2 += object[target_].getOrbitalPeriod();
			}

			int suma_orbita1 = 0;
			int suma_orbita2 = 0;

			if (time_to_reach1 < 0.0) suma_orbita1 = 1;
			else suma_orbita1 = 0;

			if (time_to_reach2 < 0.0) suma_orbita2 = 1;
			else suma_orbita2 = 0;

			double tiempo_a[5];
			double tiempo_b[5];

			tiempo_a[0] = time_to_reach1 + object[control].getOrbitalPeriod() * suma_orbita1;
			tiempo_b[0] = time_to_reach2 + object[target_].getOrbitalPeriod() * suma_orbita2;
			tiempo_a[1] = time_to_reach1 + object[control].getOrbitalPeriod() * (suma_orbita1 + 1);
			tiempo_b[1] = time_to_reach2 + object[target_].getOrbitalPeriod() * (suma_orbita2 + 1);
			tiempo_a[2] = time_to_reach1 + object[control].getOrbitalPeriod() * (suma_orbita1 + 2);
			tiempo_b[2] = time_to_reach2 + object[target_].getOrbitalPeriod() * (suma_orbita2 + 2);
			tiempo_a[3] = time_to_reach1 + object[control].getOrbitalPeriod() * (suma_orbita1 + 3);
			tiempo_b[3] = time_to_reach2 + object[target_].getOrbitalPeriod() * (suma_orbita2 + 3);
			tiempo_a[4] = time_to_reach1 + object[control].getOrbitalPeriod() * (suma_orbita1 + 4);
			tiempo_b[4] = time_to_reach2 + object[target_].getOrbitalPeriod() * (suma_orbita2 + 4);

			double tiempoRef = 1e20;
			int tiempo1 = 0;
			int tiempo2 = 0;

			for (int m = 0; m < 5; m++)
			{
				for (int n = 0; n < 5; n++)
				{
					if (abs(tiempo_a[m] - tiempo_b[n]) < tiempoRef)
					{
						tiempoRef = abs(tiempo_a[m] - tiempo_b[n]);
						tiempo1 = m;
						tiempo2 = n;
					}
				}
			}

			int chosen_time = 0;

			if (tiempo1 == 0)
			{
				ventanas[ventana].set_message_color(30, color->dim_color(0.8f, color->Light_golden_rod03));
				chosen_time = 0;
			}
			else ventanas[ventana].set_message_color(30, color->dim_color(0.8f, color->Silver));

			if (tiempo1 == 1)
			{
				ventanas[ventana].set_message_color(32, color->dim_color(0.8f, color->Light_golden_rod03));
				chosen_time = 1;
			}
			else ventanas[ventana].set_message_color(32, color->dim_color(0.8f, color->Silver));

			if (tiempo1 == 2)
			{
				ventanas[ventana].set_message_color(34, color->dim_color(0.8f, color->Light_golden_rod03));
				chosen_time = 2;
			}
			else ventanas[ventana].set_message_color(34, color->dim_color(0.8f, color->Silver));

			if (tiempo1 == 3)
			{
				ventanas[ventana].set_message_color(36, color->dim_color(0.8f, color->Light_golden_rod03));
				chosen_time = 3;
			}
			else ventanas[ventana].set_message_color(36, color->dim_color(0.8f, color->Silver));

			if (tiempo1 == 4)
			{
				ventanas[ventana].set_message_color(38, color->dim_color(0.8f, color->Light_golden_rod03));
				chosen_time = 4;
			}
			else ventanas[ventana].set_message_color(38, color->dim_color(0.8f, color->Silver));
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////

			if (tiempo2 == 0) ventanas[ventana].set_message_color(31, color->dim_color(0.8f, color->Light_golden_rod03));
			else ventanas[ventana].set_message_color(31, color->dim_color(0.8f, color->Silver));

			if (tiempo2 == 1) ventanas[ventana].set_message_color(33, color->dim_color(0.8f, color->Light_golden_rod03));
			else ventanas[ventana].set_message_color(33, color->dim_color(0.8f, color->Silver));

			if (tiempo2 == 2) ventanas[ventana].set_message_color(35, color->dim_color(0.8f, color->Light_golden_rod03));
			else ventanas[ventana].set_message_color(35, color->dim_color(0.8f, color->Silver));

			if (tiempo2 == 3) ventanas[ventana].set_message_color(37, color->dim_color(0.8f, color->Light_golden_rod03));
			else ventanas[ventana].set_message_color(37, color->dim_color(0.8f, color->Silver));

			if (tiempo2 == 4) ventanas[ventana].set_message_color(39, color->dim_color(0.8f, color->Light_golden_rod03));
			else ventanas[ventana].set_message_color(39, color->dim_color(0.8f, color->Silver));

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_a[0];
			set_message(ventana, 30, stringstream_.str());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_b[0];
			set_message(ventana, 31, stringstream_.str());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_a[1];
			set_message(ventana, 32, stringstream_.str());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_b[1];
			set_message(ventana, 33, stringstream_.str());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_a[2];
			set_message(ventana, 34, stringstream_.str());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_b[2];
			set_message(ventana, 35, stringstream_.str());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_a[3];
			set_message(ventana, 36, stringstream_.str());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_b[3];
			set_message(ventana, 37, stringstream_.str());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_a[4];
			set_message(ventana, 38, stringstream_.str());

			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempo_b[4];
			set_message(ventana, 39, stringstream_.str());
			/////////////////////////////////////////////////////////////////////////////////////

			if (tiempoRef > 0.25) ventanas[ventana].set_message_color(41, color->dim_color(0.8f, color->Red));
			else ventanas[ventana].set_message_color(41, color->dim_color(0.8f, color->Green));
			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << tiempoRef;
			set_message(ventana, 41, stringstream_.str());

			if (anomalies.dtmin_asc > min_rendezvous_distance_orange)
			{
				if (anomalies.dtmin_des > min_rendezvous_distance_orange) ventanas[ventana].set_message_color(43, color->dim_color(0.8f, color->Red));
				else ventanas[ventana].set_message_color(43, color->dim_color(0.8f, color->Silver));
			}

			else
			{
				if (anomalies.dtmin_asc <= min_rendezvous_distance_green) ventanas[ventana].set_message_color(43, color->dim_color(0.8f, color->Green));
				else ventanas[ventana].set_message_color(43, color->dim_color(0.8f, color->Light_golden_rod03));
			}
			set_message(ventana, 43, format_distance_output(anomalies.dtmin_asc));

			if (anomalies.dtmin_des > min_rendezvous_distance_orange)
			{
				if (anomalies.dtmin_asc > min_rendezvous_distance_orange) ventanas[ventana].set_message_color(45, color->dim_color(0.8f, color->Red));
				else ventanas[ventana].set_message_color(45, color->dim_color(0.8f, color->Silver));
			}

			else
			{
				if (anomalies.dtmin_des <= min_rendezvous_distance_green) ventanas[ventana].set_message_color(45, color->dim_color(0.8f, color->Green));
				else ventanas[ventana].set_message_color(45, color->dim_color(0.8f, color->Light_golden_rod03));
			}
			set_message(ventana, 45, format_distance_output(anomalies.dtmin_des));

			static double ang_asc = 0.0;
			static double ang_des = 0.0;
			static double former_ang_asc = 0.0;
			static double former_ang_des = 0.0;

			former_ang_asc = ang_asc;
			former_ang_des = ang_des;

			stringstream_.str(std::string()); // clears stringstream_
			ang_asc = ToDegrees_d(anomalies.an_asc1);
			Limit360(ang_asc);
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << ang_asc;
			set_message(ventana, 47, stringstream_.str() + "}");

			stringstream_.str(std::string()); // clears stringstream_
			ang_des = ToDegrees_d(anomalies.an_des1);
			Limit360(ang_des);
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << ang_des;
			set_message(ventana, 49, stringstream_.str() + "}");

			if (!Config.caption_on)
			{
				if (caretTick >= 0.5f) // cada medio segundo el caret aparece o desaparece
				{
					caretTick = 0; // reseteamos el tiempo de aparicion del caret
					showCaret = !showCaret; // cambiamos el estado del caret si se cumplio el medio segundo
				}
			}

			if (rel_incl > 0.1)
			{
				if (ang_asc<former_ang_asc && ang_asc > 5.0)
				{
					if (object[control].getNormal())
					{
						ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Silver));
						set_message(ventana, 51, "Standby...");
					}

					else
					{
						ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Light_golden_rod03));
						set_message(ventana, 51, "Set Orbit to Normal");
					}
				}

				else if (ang_des<former_ang_des && ang_des > 5.0)
				{
					if (object[control].getAnti_normal())
					{
						ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Silver));
						set_message(ventana, 51, "Standby...");
					}

					else
					{
						ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Light_golden_rod03));
						set_message(ventana, 51, "Set Orbit to anti-Normal");
					}
				}

				else
				{
					ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Silver));
					set_message(ventana, 51, "Standby...");
				}

				if (ang_asc < 5.0)
				{
					if (showCaret) ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Green));
					else ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Yellow));
					set_message(ventana, 51, "Fire engines");
				}

				else if (ang_des < 5.0)
				{
					if (showCaret) ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Green));
					else ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Yellow));
					set_message(ventana, 51, "Fire engines");
				}
			}

			else
			{
				ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Light_golden_rod03));
				if (tiempoRef > 0.5) set_message(ventana, 51, "Check DTmin.");
				else if (anomalies.dtmin_asc > min_rendezvous_distance_orange && anomalies.dtmin_des > min_rendezvous_distance_orange) set_message(ventana, 51, "Check rendesvous distance.");
				else
				{
					ventanas[ventana].set_message_color(51, color->dim_color(0.8f, color->Green));
					set_message(ventana, 51, "Rendezvous");
				}
			}

			set_message(ventana, 53, format_distance_output(object[target_].getOrbitRadius() - object[object[target_].getOrbiting()].getScale() / 2.0));

			set_message(ventana, 55, format_time_output(tiempo_a[chosen_time]));

			if (object[object[control].getOrbiting()].getHasAtmosphere())
			{
				if (object[control].getPerigee() <= (object[object[control].getOrbiting()].getScale() / 2)*object[object[control].getOrbiting()].getAtmosphereRatio())
				{
					if (showCaret) ventanas[ventana].set_message_color(56, color->dim_color(0.8f, color->Red));
					else ventanas[ventana].set_message_color(56, color->dim_color(0.8f, color->Yellow));
					set_message(ventana, 56, "PERIAPSIS BELOW ATMOSPHERE!!!");
				}
				else set_message(ventana, 56, " ");
			}

			else
			{
				if (object[control].getPerigee() <= object[object[control].getOrbiting()].getScale() / 2)
				{
					if (showCaret) ventanas[ventana].set_message_color(56, color->dim_color(0.8f, color->Red));
					else ventanas[ventana].set_message_color(56, color->dim_color(0.8f, color->Yellow));
					set_message(ventana, 56, "PERIAPSIS BELOW SURFACE!!!");
				}
				else set_message(ventana, 56, " ");
			}
		}

		else
		{
			for (int n = 0; n < ventanas[ventana].getTotalMessages(); n++)
			{
				if (ventanas[ventana].get_message_dynamic(n)) set_message(ventana, n, " ");
			}
			set_message(ventana, 7, "NO TARGET");
		}

	}


	ventana = 6;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		set_message(ventana, 0, "Horizontal level");
		set_message(ventana, 1, "Pro-grade");
		set_message(ventana, 2, "Retro-grade");
		set_message(ventana, 3, "Normal");
		set_message(ventana, 4, "Anti-normal");
		set_message(ventana, 5, "Retro-diff");
		set_message(ventana, 6, "Target H-level");
		set_message(ventana, 7, "Radial in");
		set_message(ventana, 8, "Radial out");
		set_message(ventana, 9, "Kill rotation");
		set_message(ventana, 10, "Lock to target");
	}

	// piloto automatico
	ventana = 7;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		set_message(ventana, 0, "Match target speed:");
		set_message(ventana, 1, "Approach target:");
		set_message(ventana, 2, "Approach distance (meters):");
		set_message(ventana, 3, "Current distance: " + format_distance_output(distances(object[control].getPosition(), object[object[control].getTarget()].getPosition())));
	}

	ventana = 8;
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																		//
		int orbited_ = object[control].getOrbiting();
		if (!ventanas[0].getVisible() && !ventanas[5].getVisible()) // las ventanas que actualizan los datos del objeto control
		{
			object[control].updateOrbitalPositionAndVelocity(object[orbited_].getPosition(), object[orbited_].getVelocity());
		}

		vec3 aux_v = object[control].getOrbitalPosition();
		quat aux_q = object[orbited_].getTotalRotation();
		vec3 rott = vector_rotation_by_quaternion({ aux_v.x, aux_v.z, -aux_v.y }, { aux_q.x, aux_q.z, -aux_q.y, aux_q.w });

		geodetic_coord = cartesian_to_geodetic2(orbited_, rott);
		object[control].setLatitude(geodetic_coord.lat);
		object[control].setLongitude(geodetic_coord.lon);
		object[control].set_altitude(geodetic_coord.h);

		// actualizamos los datos del latlong del target si este esta orbitando el mismo cuerpo que el objeto controlado
		int target_ = object[control].getTarget();
		if (object[target_].getOrbiting() == object[target_].getOrbiting())
		{
			object[target_].updateOrbitalPositionAndVelocity(object[orbited_].getPosition(), object[orbited_].getVelocity());

			vec3 aux_v = object[target_].getOrbitalPosition();
			quat aux_q = object[orbited_].getTotalRotation();
			vec3 rott = vector_rotation_by_quaternion({ aux_v.x, aux_v.z, -aux_v.y }, { aux_q.x, aux_q.z, -aux_q.y, aux_q.w });

			geodetic_coord_target = cartesian_to_geodetic2(orbited_, rott);

			object[target_].setLatitude(geodetic_coord_target.lat);
			object[target_].setLongitude(geodetic_coord_target.lon);
			object[target_].set_altitude(geodetic_coord_target.h);
		}
		//																																		//
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		set_message(ventana, 0, "Lat:");
		set_message(ventana, 1, convert_to_fixed_degree(ToDegrees_d(geodetic_coord.lat), 1));

		set_message(ventana, 2, "Lon:");
		set_message(ventana, 3, convert_to_fixed_degree(ToDegrees_d(geodetic_coord.lon), 2));

		set_message(ventana, 4, "Alt:");
		set_message(ventana, 5, format_distance_output(geodetic_coord.h));
	}

	// piloto automatico
	ventana = 9;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		set_message(ventana, 0, "Main Engine Power(%):");

		if (object[control].getQuickMeBurst()) set_message(ventana, 1, "100");

		else
		{
			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << object[control].getMECurrentPower();
			set_message(ventana, 1, stringstream_.str());
		}

		set_message(ventana, 2, "Hover Power(%):");

		if (object[control].getQuickHoverBurst()) set_message(ventana, 3, "100");

		else
		{
			stringstream_.str(std::string()); // clears stringstream_
			stringstream_ << setiosflags(std::ios::fixed) << std::setprecision(2) << object[control].getHoverCurrentPower();
			set_message(ventana, 3, stringstream_.str());
		}
	}

	ventana = 10;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		set_message(ventana, 0, "Alt:");
		set_message(ventana, 1, format_distance_output(object[control].getOrbitRadius() - object[object[control].getOrbiting()].getScale() / 2.0));
		set_message(ventana, 2, "Gsp:");
		set_message(ventana, 3, format_speed_output(object[control].getGroundSpeed()));
		set_message(ventana, 4, "VV:");

		if (object[control].get_vv() > 0.0) set_message(ventana, 5, "-" + format_speed_output(object[control].get_vv()));
		else set_message(ventana, 5, format_speed_output(object[control].get_vv()));

		set_message(ventana, 6, "TAS:");
		set_message(ventana, 7, format_speed_output(length(object[control].get_true_velocity())));
		set_message(ventana, 8, "KIAS:");
		set_message(ventana, 9, format_decimal_number(aerodynamics.getKIAS(control), 2) + " knots.");
		set_message(ventana, 10, "AoA:");

		if (!get_compass_dependencies_active()) horizontal_dir = getHorizontalDir(control);
		double angulos_ = ToDegrees_d(angle_from_two_vectors(horizontal_dir, -GetAxisZ(object[control].getTotalRotation())));
		if (angle_from_two_vectors(GetAxisZ(object[control].getTotalRotation()), object[control].getOrbitalPosition()) >
			(DirectX::XM_PIDIV2 - 0.001f)) // el 0.001f es para eliminar el tembleque en el display de los grados cuando estamos cerca de 0 grados
		{
			set_message(ventana, 11, format_decimal_number(angulos_, 2) + "}");
		}
		else set_message(ventana, 11, "-" + format_decimal_number(angulos_, 2) + "}");

		set_message(ventana, 12, "AHe:");
		set_message(ventana, 13, format_decimal_number(max(0.f, (aerodynamics.get_stagnation_point_temperature(control) - 273.15f)), 0) + " }C");
	}

	ventana = 11;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		if (robotic_arm[0].size < 2) script_.build_robotic_arm(control);

		set_message(ventana, 0, "Controled part:");
		set_message(ventana, 1, object[robotic_arm[0].members[robotic_arm[0].handled_part]].getName());
		set_message(ventana, 2, "Select Part:");
		set_message(ventana, 3, "Move Part:");

		set_message(ventana, 4, "Target:");

		if (object[robotic_arm[0].members[robotic_arm[0].handled_part]].getTarget() >= Config.total_initial_setup_objects &&
			(object[robotic_arm[0].members[robotic_arm[0].handled_part]].getTotalGrappleFixtures() > 0 ||
			(object[robotic_arm[0].members[robotic_arm[0].handled_part]].getTotalDockingPorts() > 0)))
		{
			if ((object[object[robotic_arm[0].members[robotic_arm[0].handled_part]].getTarget()].getTotalGrappleFixtures() > 0 && !object[robotic_arm[0].members[robotic_arm[0].handled_part]].getRoboticArmDPort()) ||
				(object[object[robotic_arm[0].members[robotic_arm[0].handled_part]].getTarget()].getTotalDockingPorts() > 0 && object[robotic_arm[0].members[robotic_arm[0].handled_part]].getRoboticArmDPort()))
			{
				ventanas[ventana].set_message_color(5, color->Green);
				set_message(ventana, 5, object[object[robotic_arm[0].members[robotic_arm[0].handled_part]].getTarget()].getName());
			}

			else
			{
				ventanas[ventana].set_message_color(5, color->Red);
				if (!object[robotic_arm[0].members[robotic_arm[0].handled_part]].getRoboticArmDPort()) set_message(ventana, 5, object[object[robotic_arm[0].members[robotic_arm[0].handled_part]].getTarget()].getName() + " (NO GF)");
				else set_message(ventana, 5, object[object[robotic_arm[0].members[robotic_arm[0].handled_part]].getTarget()].getName() + " (NO DP)");
			}
		}

		else
		{
			ventanas[ventana].set_message_color(6, color->Silver);
			set_message(ventana, 5, "N/A");
		}

		set_message(ventana, 6, "Auto grapple:");
		set_message(ventana, 7, "L. Port:");
		set_message(ventana, 8, "T. Port:");
	}


	ventana = 12;
	if (ventanas[ventana].getVisible() || !updated_once)
	{
		set_message(ventana, 0, "Target:");
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// actualizamos los mensajes de todas las ventanas
	for (int ventana_number = 0; ventana_number < total_ventanas; ventana_number++)
	{
		if (ventanas[ventana_number].getVisible())
		{
			// title
			if (!m_Text->update_sentence(ventanas[ventana_number].getTitle(),
				ventanas[ventana_number].getPositionX(), ventanas[ventana_number].getPositionY(),
				ventanas[ventana_number].getColor(), ventanas[ventana_number].get_title_sentence_number())) return false;

			for (int msg = 0; msg < ventanas[ventana_number].getTotalMessages(); msg++)
			{
				// messages
				ventanas[ventana_number].set_message_pos_x(msg,
					ventanas[ventana_number].get_default_message_pos_x(msg) + ventanas[ventana_number].getPositionX());
				ventanas[ventana_number].set_message_pos_y(msg,
					ventanas[ventana_number].get_default_message_pos_y(msg) + ventanas[ventana_number].getPositionY());

				if (!m_Text->update_sentence(ventanas[ventana_number].getMessage(msg),
					ventanas[ventana_number].get_message_pos_x(msg),
					ventanas[ventana_number].get_message_pos_y(msg), ventanas[ventana_number].get_message_color(msg),
					ventanas[ventana_number].get_message_sentence_number(msg))) return false;
			}
		}
	}

	updated_once = true;
	return true;
}



bool Gui_Class::manage_buttons_mouse_interaction(w_button* button_group, int& total_group_buttons)
{
	for (int n = 0; n < total_group_buttons; n++)
	{
		if (button_group[n].visible && button_group[n].clickable)
		{
			if (!ventana_trapped && IsPointInRect(button_group[n].position_x, button_group[n].position_y, button_group[n].width, button_group[n].heigth))
			{
				// if the mouse is hovering and the left mouse button is being clicked
				// set the button texture to the third one in the array, but don't do whatever the button is supposed to do,
				// wait until we release the mouse button to activate it
				if (input.MouseButtonDown(0))
				{
					if (button_group[n].continuous_action) perform_loose_button_action(n);
					button_clicked = true;
					button_group[n].clicked = true;
					if (button_group[n].multi_texture) button_group[n].texture->setMouseStatus(2); // si el boton tiene varias texturas, entonces
					// cambiarlas de acuerdo al mouse status sobre este
				}

				// if the mouse is hovering but no mouse button is being clicked
				else
				{
					// if the button has been clicked we can make it do whatever it is supposed to do
					if (button_group[n].clicked)
					{
						if (!button_group[n].continuous_action) perform_loose_button_action(n);
						button_group[n].clicked = false;
					}

					// if the button hasn't been clicked then set the texture to the second one
					else if (button_group[n].multi_texture) button_group[n].texture->setMouseStatus(1);  // si el boton tiene varias texturas, entonces
					// cambiarlas de acuerdo al mouse status sobre este
				}
			}

			// if the mouse is not hovering over the button then set the texture to the first one
			else
			{
				button_group[n].texture->setMouseStatus(0);
				button_group[n].clicked = false;
			}
		}
	}

	return true;
}



bool Gui_Class::manage_ventanas_mouse_interaction()
{
	int box_num = 0;

	for (int ventana_number = 0; ventana_number < total_ventanas; ventana_number++)
	{
		if (ventanas[ventana_number].getVisible())
		{
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//									check for the mouse hovering over the close window button													//
			if (!ventana_trapped &&
				IsPointInRect(
				ventanas[ventana_number].getPositionX() + window_rectangle_bitmap[ventana_number * total_ventanas_rectangles].getBitmapWidth() - 34,
				ventanas[ventana_number].getPositionY() - 6,
				window_rectangle_bitmap[ventana_number * total_ventanas_rectangles + 2].getBitmapWidth(),
				window_rectangle_bitmap[ventana_number * total_ventanas_rectangles + 2].getBitmapHeigth())
				)
			{
				// if the mouse is hovering and the left mouse button is being clicked
				// set the button texture to the third one in the array, but don't do whatever the button is supposed to do,
				// wait until we release the mouse button to activate it
				if (input.MouseButtonDown(0))
				{
					button_clicked = true;
					window_rectangle_bitmap[ventana_number * total_ventanas_rectangles + 2].setMouseStatus(2);
				}

				// if the mouse is hovering but no mouse button is being clicked
				else
				{
					// if the texture is the third one then button has been pressed, we can make it do whatever it is supposed to do
					if (window_rectangle_bitmap[ventana_number * total_ventanas_rectangles + 2].getMouseStatus() == 2)
					{
						if (!setWindowVisible(ventana_number, false)) return false;
					}

					// if the texture is not the third one, then the button has not been pressed, set the texture to the second one
					else window_rectangle_bitmap[ventana_number * total_ventanas_rectangles + 2].setMouseStatus(1);
				}
			}

			// if the mouse is not hovering over the button then set the texture to the first one
			else
			{
				window_rectangle_bitmap[ventana_number * total_ventanas_rectangles + 2].setMouseStatus(0);
			}
			//																																				//
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


			// si el boton izquierdo del mouse esta presionado entonces checkear si el puntero esta sobre alguna ventana y moverla de 
			// acuerdo al movimiento de este
			if (input.MouseButtonDown(0))
			{
				clicked = true;

				// si ya se checkearon todas las ventanas y el puntero no atrapo ninguna entonces ignorar todo el procedimiento
				if (!no_trap && !button_clicked)
				{
					if (!text_box_trapped)
					{
						// si hay alguna ventana atrapada
						if (ventana_trapped)
						{
							// mover solo la ventana que esta atrapada
							if (ventanas[ventana_number].getTrapped())
							{
								ventanas[ventana_number].set_position_x(
									ventanas[ventana_number].getVentanaOriginalPosX() + (cursor_position.x - mouse_original_x));
								ventanas[ventana_number].set_position_y(
									ventanas[ventana_number].getVentanaOriginalPosY() + (cursor_position.y - mouse_original_y));

								// make sure the window's title bar is always visible to avoid the user being unable to drag it
								if (ventanas[ventana_number].getPositionX() > screenWidth)
									ventanas[ventana_number].set_position_x(screenWidth);
								else if (ventanas[ventana_number].getPositionX() <
									-window_rectangle_bitmap[ventana_number * total_ventanas_rectangles].getBitmapWidth() + 20)
									ventanas[ventana_number].set_position_x(
									-window_rectangle_bitmap[ventana_number * total_ventanas_rectangles].getBitmapWidth() + 20);

								if (ventanas[ventana_number].getPositionY() > screenHeight)
									ventanas[ventana_number].set_position_y(screenHeight);
								else if (ventanas[ventana_number].getPositionY() <
									-window_rectangle_bitmap[ventana_number * total_ventanas_rectangles].getBitmapHeigth() + 20)
									ventanas[ventana_number].set_position_y(
									-window_rectangle_bitmap[ventana_number * total_ventanas_rectangles].getBitmapHeigth() + 20);
							}
						}

						// si no hay ninguna ventana atrapada entonces checkear todas para ver si alguna esta bajo el puntero y atraparla,
						// se atrapa a la primera que se encuentra y las demas son marcadas como ignoradas
						else if (IsPointInRect(ventanas[ventana_number].getPositionX() - 10, ventanas[ventana_number].getPositionY() - 10,
							window_rectangle_bitmap[ventana_number * total_ventanas_rectangles].getBitmapWidth(),
							window_rectangle_bitmap[ventana_number * total_ventanas_rectangles].getBitmapHeigth())
							)
						{
							// marcamos esta ventana como atrapada mientras dure el click para ir actualizando su posicion de acuerdo
							// al movimiento del puntero, tambien activamos el flag de la captura general para que no se ejecute nuevamente el checkeo
							// y se vuelva a atrapar otra ventana mientras dure el click
							ventana_trapped = true;
							ventanas[ventana_number].setTrapped(true);

							if (mouse_original_x == 0 && mouse_original_y == 0)
							{
								mouse_original_x = cursor_position.x;
								mouse_original_y = cursor_position.y;
								ventanas[ventana_number].set_ventana_original_posX(ventanas[ventana_number].getPositionX());
								ventanas[ventana_number].set_ventana_original_posY(ventanas[ventana_number].getPositionY());
							}
						}

						else if (!ventana_trapped && ventanas[ventana_number].getTotalTextBoxes() > 0)
						{
							for (int n = 0; n < ventanas[ventana_number].getTotalTextBoxes(); n++)
							{
								if (IsPointInRect(ventanas[ventana_number].getPositionX() + ventanas[ventana_number].get_tb_posx(n),
									ventanas[ventana_number].getPositionY() + ventanas[ventana_number].get_tb_posy(n),
									ventanas[ventana_number].get_tb_width(n), ventanas[ventana_number].get_tb_heigth(n)))
								{
									// limpiamos cualquier textbox que este activo
									if (tb_has_focus)
									{
										text_box[focus_on].setText(text_box[focus_on].getRealText());
										text_box[focus_on].update();

										text_box[focus_on].setHasFocus(false);
										focus_on = -1;
										tb_has_focus = false;
										caretPos = 1;
									}

									// activamos el textbox elegido
									box_num = get_TBox_number(ventana_number, n);
									showCaret = true;
									caretTick = 0.0f;
									text_box[box_num].setHasFocus(true);
									focus_on = box_num;
									tb_has_focus = true;
									text_box_trapped = true;
									caretPos = int(text_box[box_num].getRealText().size());
								}
							}
						}
					}
				}
			}

			else
			{
				no_trap = false; // reseteamos el flag de no-captura general
				button_clicked = false;
				text_box_trapped = false;
				clicked = false;

				if (ventana_trapped)
				{
					for (int nn = 0; nn < total_ventanas; nn++) ventanas[nn].setTrapped(false);
					mouse_original_x = 0;
					mouse_original_y = 0;
					ventana_trapped = false;
				}
			}

			// chequeamos si hay algun cambio de posicion en las ventanas y actualizamos la posicion de sus mensajes cuando se detecta un cambio
			if (ventanas[ventana_number].getPositionX() != ventanas[ventana_number].getPreviousPositionX() ||
				ventanas[ventana_number].getPositionY() != ventanas[ventana_number].getPreviousPositionY())
			{
				if (!update_window_position(ventana_number)) return false;
			}

		}
	} // aqui termina el contaje de ventanas


	// si ya se checkearon todas las ventanas y el puntero no atrapo ninguna entonces activar el flag para ignorar futuros checkeos mientras dure el click
	if (clicked)
	{
		if (!text_box_trapped && tb_has_focus)
		{
			text_box[focus_on].setText(text_box[focus_on].getRealText());
			text_box[focus_on].update();

			text_box[focus_on].setHasFocus(false);
			focus_on = -1;
			tb_has_focus = false;
		}

		if (!ventana_trapped) no_trap = true;
		if (!button_clicked) kill_all_depending_buttons(loose_button, total_loose_buttons);
		clicked = false;
	}

	return true;
}

bool Gui_Class::Initialize_loose_buttons()
{
	total_loose_buttons = 53;
	loose_button = new w_button[total_loose_buttons];

	int button_ = 0;

	button_ = 0;
	loose_button[button_].depends_on = -1;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 20;
	loose_button[button_].position_y = screenHeight - 70;
	loose_button[button_].width = 50;
	loose_button[button_].heigth = 50;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/gear_n.png", "textures/gui/gear_h.png", "textures/gui/gear_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 1;
	loose_button[button_].color_ = Color{ 0.0f, 0.1f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 0;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 20;
	loose_button[button_].position_y = screenHeight - 100;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/object_n.png", "textures/gui/object_h.png", "textures/gui/object_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 2;
	loose_button[button_].color_ = Color{ 0.0f, 0.2f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 1;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 116;
	loose_button[button_].position_y = screenHeight - 100;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/info_n.png", "textures/gui/info_h.png", "textures/gui/info_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 3;
	loose_button[button_].color_ = Color{ 0.0f, 0.4f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 2;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 212;
	loose_button[button_].position_y = screenHeight - 100;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/orbit_flight_n.png", "textures/gui/orbit_flight_h.png", "textures/gui/orbit_flight_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 4;
	loose_button[button_].color_ = Color{ 0.0f, 0.2f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 1;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 116;
	loose_button[button_].position_y = screenHeight - 128;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/select_n.png", "textures/gui/select_h.png", "textures/gui/select_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 5;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.1f, 1.0f };
	loose_button[button_].depends_on = 0;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 20;
	loose_button[button_].position_y = screenHeight - 128;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/general_n.png", "textures/gui/general_h.png", "textures/gui/general_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 6;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.2f, 1.0f };
	loose_button[button_].depends_on = 5;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 116;
	loose_button[button_].position_y = screenHeight - 128;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/info_n.png", "textures/gui/info_h.png", "textures/gui/info_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 7;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 0;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 20;
	loose_button[button_].position_y = screenHeight - 156;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/view_n.png", "textures/gui/view_h.png", "textures/gui/view_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 8;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 7;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 116;
	loose_button[button_].position_y = screenHeight - 156;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/orbits_off_n.png", "textures/gui/orbits_off_h.png", "textures/gui/orbits_off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 9;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 7;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 116;
	loose_button[button_].position_y = screenHeight - 184;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/stars_off_n.png", "textures/gui/stars_off_h.png", "textures/gui/stars_off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 10;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 7;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 116;
	loose_button[button_].position_y = screenHeight - 212;
	loose_button[button_].width = 125;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/constellations_off_n.png", "textures/gui/constellations_off_h.png", "textures/gui/constellations_off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 11;
	loose_button[button_].color_ = Color{ 0.0f, 0.4f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 2;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 212;
	loose_button[button_].position_y = screenHeight - 128;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/docking_n.png", "textures/gui/docking_h.png", "textures/gui/docking_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 12;
	loose_button[button_].color_ = Color{ 0.0f, 0.4f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 4;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 212;
	loose_button[button_].position_y = screenHeight - 128;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/new_object_n.png", "textures/gui/new_object_h.png", "textures/gui/new_object_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 13;
	loose_button[button_].color_ = Color{ 0.0f, 0.4f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 4;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 212;
	loose_button[button_].position_y = screenHeight - 156;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/target_n.png", "textures/gui/target_h.png", "textures/gui/target_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 14;
	loose_button[button_].color_ = Color{ 0.0f, 0.4f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 2;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 212;
	loose_button[button_].position_y = screenHeight - 156;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/rendezvous_n.png", "textures/gui/rendezvous_h.png", "textures/gui/rendezvous_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// h-level
	button_ = 15;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 30;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// pro-grade
	button_ = 16;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 52;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// retro-grade
	button_ = 17;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 74;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Normal
	button_ = 18;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 96;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// anti-normal
	button_ = 19;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 118;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// retro-diff
	button_ = 20;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 140;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// target h-level
	button_ = 21;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 162;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Radial in
	button_ = 22;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 184;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Radial out
	button_ = 23;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 206;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Kill rotation
	button_ = 24;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 228;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 25;
	loose_button[button_].color_ = Color{ 0.0f, 0.2f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 1;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 116;
	loose_button[button_].position_y = screenHeight - 156;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/control_n.png", "textures/gui/control_h.png", "textures/gui/control_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	button_ = 26;
	loose_button[button_].color_ = Color{ 0.0f, 0.4f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 25;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 212;
	loose_button[button_].position_y = screenHeight - 156;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/rcs_n.png", "textures/gui/rcs_h.png", "textures/gui/rcs_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Lock to target
	button_ = 27;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 6;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 110;
	loose_button[button_].position_orig_y = 250;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Match target speed
	button_ = 28;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 7;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 130;
	loose_button[button_].position_orig_y = 30;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Approach target
	button_ = 29;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 7;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 130;
	loose_button[button_].position_orig_y = 52;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// auto pilot
	button_ = 30;
	loose_button[button_].color_ = Color{ 0.0f, 0.4f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 25;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 212;
	loose_button[button_].position_y = screenHeight - 184;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/auto_pilot_n.png", "textures/gui/auto_pilot_h.png", "textures/gui/auto_pilot_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// La flecha verde del surface map
	button_ = 31;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].clickable = false;
	loose_button[button_].toggeable = false;
	loose_button[button_].ventana = 8;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 6; // map pointer
	loose_button[button_].position_orig_x = 210; // 457 / 203
	loose_button[button_].position_orig_y = 128;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 32;
	loose_button[button_].heigth = 32;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/arrow_green.png", "textures/white.png", "textures/white.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Blue map pointer *** DEBUG *** don't what this is used for
	button_ = 32;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].clickable = false;
	loose_button[button_].toggeable = false;
	loose_button[button_].ventana = 8;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 6; // map pointer
	loose_button[button_].position_orig_x = 194;
	loose_button[button_].position_orig_y = 98;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 32;
	loose_button[button_].heigth = 32;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/pointer_blue.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// surface map toggle button
	button_ = 33;
	loose_button[button_].color_ = Color{ 0.0f, 0.4f, 0.0f, 1.0f };
	loose_button[button_].depends_on = 2;
	loose_button[button_].visible = false;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_x = 212;
	loose_button[button_].position_y = screenHeight - 184;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/surface_map_n.png", "textures/gui/surface_map_h.png", "textures/gui/surface_map_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// green map pointer *** DEBUG *** don't know what this one is used for either
	button_ = 34;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].clickable = false;
	loose_button[button_].toggeable = false;
	loose_button[button_].ventana = 8;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 6; // map pointer
	loose_button[button_].position_orig_x = 205; // 457 / 203
	loose_button[button_].position_orig_y = 110;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 10;
	loose_button[button_].heigth = 18;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/pointer_b_green.png", "textures/white.png", "textures/white.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// red map pointer, for showing the periapsis place
	button_ = 35;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].clickable = false;
	loose_button[button_].toggeable = false;
	loose_button[button_].ventana = 8;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 6; // map pointer
	loose_button[button_].position_orig_x = 205; // 457 / 203
	loose_button[button_].position_orig_y = 110;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 10;
	loose_button[button_].heigth = 18;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/pointer_b_red.png", "textures/white.png", "textures/white.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// show re_entry button
	button_ = 36;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 8;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 400;
	loose_button[button_].position_orig_y = 25;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 25;
	loose_button[button_].heigth = 25;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/re_entry_off_n.png", "textures/gui/re_entry_off_h.png", "textures/gui/re_entry_off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// compass background
	button_ = 37;
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 10;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 155;
	loose_button[button_].position_orig_y = 40;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 100;
	loose_button[button_].heigth = 100;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/compass_back.png", "textures/gui/compass_back.png", "textures/gui/compass_back.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// compass indicator arrow
	button_ = 38;
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 10;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 155;
	loose_button[button_].position_orig_y = 40;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 100;
	loose_button[button_].heigth = 100;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/compass_pointer_a.png", "textures/gui/compass_pointer_a.png", "textures/gui/compass_pointer_a.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// compass cardinal points letters
	button_ = 39;
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 10;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 155;
	loose_button[button_].position_orig_y = 40;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 100;
	loose_button[button_].heigth = 100;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/compass_points.png", "textures/gui/compass_points.png", "textures/gui/compass_points.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// button of previous selection
	button_ = 40;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 5;
	loose_button[button_].position_orig_y = 75;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 28;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/down_n.png", "textures/gui/down_h.png", "textures/gui/down_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// button of next selection
	button_ = 41;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 35;
	loose_button[button_].position_orig_y = 75;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 28;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/up_n.png", "textures/gui/up_h.png", "textures/gui/up_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// button of forward movement
	button_ = 42;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].continuous_action = true;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 95;
	loose_button[button_].position_orig_y = 75;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 28;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/left_n.png", "textures/gui/left_h.png", "textures/gui/left_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// button of backward movement
	button_ = 43;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].continuous_action = true;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 125;
	loose_button[button_].position_orig_y = 75;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 28;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/right_n.png", "textures/gui/right_h.png", "textures/gui/right_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// button of focus on robotic arm part
	button_ = 44;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 200;
	loose_button[button_].position_orig_y = 50;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 28;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/eye_n.png", "textures/gui/eye_h.png", "textures/gui/eye_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Boton de modo de camara
	button_ = 45;
	loose_button[button_].depends_on = -1;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].toggeable = true;
	loose_button[button_].visible = Config.camera_locked;
	loose_button[button_].position_x = 90;
	loose_button[button_].position_y = screenHeight - 55;
	loose_button[button_].width = 42;
	loose_button[button_].heigth = 23;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/camera_locked_n.png", "textures/gui/camera_locked_h.png", "textures/gui/camera_locked_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Boton de modo de camara
	button_ = 46;
	loose_button[button_].depends_on = -1;
	loose_button[button_].tipo_ = 3;
	loose_button[button_].visible = !Config.camera_locked;
	loose_button[button_].position_x = 90;
	loose_button[button_].position_y = screenHeight - 55;
	loose_button[button_].width = 42;
	loose_button[button_].heigth = 23;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/camera_unlocked_n.png", "textures/gui/camera_unlocked_h.png", "textures/gui/camera_unlocked_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Auto grapple
	button_ = 47;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 90;
	loose_button[button_].position_orig_y = 130;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 44;
	loose_button[button_].heigth = 20;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/off_n.png", "textures/gui/off_h.png", "textures/gui/off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Manual Grapple
	button_ = 48;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 200;
	loose_button[button_].position_orig_y = 80;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 28;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/grapple_n.png", "textures/gui/grapple_h.png", "textures/gui/grapple_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Select grapple target
	button_ = 49;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 200;
	loose_button[button_].position_orig_y = 110;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 28;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/crosshair_n.png", "textures/gui/crosshair_h.png", "textures/gui/crosshair_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// boton de usar grapple fixtures o docking ports
	button_ = 50;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 0;
	loose_button[button_].position_orig_y = 165;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/dport_off_n.png", "textures/gui/dport_off_h.png", "textures/gui/dport_off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// boton de visual helpers del robotic arm
	button_ = 51;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 101;
	loose_button[button_].position_orig_y = 165;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 96;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/v_helpers_off_n.png", "textures/gui/v_helpers_off_h.png", "textures/gui/v_helpers_off_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	// Anchor to target
	button_ = 52;
	loose_button[button_].toggeable = true;
	loose_button[button_].toggled_on = false;
	loose_button[button_].color_ = Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	loose_button[button_].depends_on = -1;
	loose_button[button_].ventana = 11;
	loose_button[button_].visible = ventanas[loose_button[button_].ventana].getVisible();
	loose_button[button_].tipo_ = 3;
	loose_button[button_].position_orig_x = 200;
	loose_button[button_].position_orig_y = 140;
	loose_button[button_].position_x = loose_button[button_].position_orig_x;
	loose_button[button_].position_y = loose_button[button_].position_orig_y;
	loose_button[button_].width = 28;
	loose_button[button_].heigth = 28;
	if (!loose_button[button_].texture->Initialize(screenWidth, screenHeight,
		"textures/gui/anchor_n.png", "textures/gui/anchor_h.png", "textures/gui/anchor_c.png", "textures/white.png",
		loose_button[button_].width, loose_button[button_].heigth)) return false;

	return true;
}

void Gui_Class::perform_loose_button_action(int n)
{
	if (n == 0)
	{
		toggle_depending_buttons(loose_button, total_loose_buttons, n);
	}

	else if (n == 1)
	{
		toggle_depending_buttons(loose_button, total_loose_buttons, n);
	}

	else if (n == 2)
	{
		toggle_depending_buttons(loose_button, total_loose_buttons, n);
	}

	else if (n == 3)
	{
		if (object[control].getInAtmosphere()) setWindowVisible(10, true);
		else setWindowVisible(0, true);
		kill_all_depending_buttons(loose_button, total_loose_buttons);
	}

	else if (n == 4)
	{
		toggle_depending_buttons(loose_button, total_loose_buttons, n);
	}

	else if (n == 5)
	{
		toggle_depending_buttons(loose_button, total_loose_buttons, n);
	}

	else if (n == 6)
	{
		setWindowVisible(1, true);
		kill_all_depending_buttons(loose_button, total_loose_buttons);
	}

	else if (n == 7)
	{
		toggle_depending_buttons(loose_button, total_loose_buttons, n);
	}

	else if (n == 8)
	{
		Config.draw_orbits = !Config.draw_orbits;
		Config.create_projected_sentences = true;
		loose_button[8].toggled_on = !loose_button[8].toggled_on;

		if (!loose_button[8].toggled_on)
		{
			loose_button[8].texture->~BitmapClass();
			loose_button[8].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/orbits_off_n.png", "textures/gui/orbits_off_h.png", "textures/gui/orbits_off_c.png", "textures/white.png",
				loose_button[8].width, loose_button[8].heigth);
		}

		else
		{
			loose_button[8].texture->~BitmapClass();
			loose_button[8].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/orbits_on_n.png", "textures/gui/orbits_on_h.png", "textures/gui/orbits_on_c.png", "textures/white.png",
				loose_button[8].width, loose_button[8].heigth);
		}

		//kill_all_buttons(loose_button, total_loose_buttons);
	}

	else if (n == 9)
	{
		Config.draw_stars = !Config.draw_stars;
		Config.create_projected_sentences = true;
		loose_button[9].toggled_on = !loose_button[9].toggled_on;

		if (!loose_button[9].toggled_on)
		{
			loose_button[9].texture->~BitmapClass();
			loose_button[9].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/stars_off_n.png", "textures/gui/stars_off_h.png", "textures/gui/stars_off_c.png", "textures/white.png",
				loose_button[9].width, loose_button[9].heigth);
		}

		else
		{
			loose_button[9].texture->~BitmapClass();
			loose_button[9].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/stars_on_n.png", "textures/gui/stars_on_h.png", "textures/gui/stars_on_c.png", "textures/white.png",
				loose_button[9].width, loose_button[9].heigth);
		}

		//kill_all_buttons(loose_button, total_loose_buttons);
	}

	else if (n == 10)
	{
		Config.draw_constellations = !Config.draw_constellations;
		loose_button[10].toggled_on = !loose_button[10].toggled_on;

		if (!loose_button[10].toggled_on)
		{
			loose_button[10].texture->~BitmapClass();
			loose_button[10].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/constellations_off_n.png", "textures/gui/constellations_off_h.png", "textures/gui/constellations_off_c.png", "textures/white.png",
				loose_button[10].width, loose_button[10].heigth);
		}

		else
		{
			loose_button[10].texture->~BitmapClass();
			loose_button[10].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/constellations_on_n.png", "textures/gui/constellations_on_h.png", "textures/gui/constellations_on_c.png", "textures/white.png",
				loose_button[10].width, loose_button[10].heigth);
		}

		//kill_all_buttons(loose_button, total_loose_buttons);
	}

	else if (n == 11)
	{
		setWindowVisible(3, true);
		kill_all_depending_buttons(loose_button, total_loose_buttons);
	}


	else if (n == 12)
	{
		int ventana = 2;
		setWindowVisible(ventana, true);
		focus_on = get_TBox_number(ventana, 0);
		text_box[focus_on].setHasFocus(true);
		tb_has_focus = true;
		kill_all_depending_buttons(loose_button, total_loose_buttons);
	}

	else if (n == 13)
	{
		int ventana = 4;
		setWindowVisible(ventana, true);
		focus_on = get_TBox_number(ventana, 0);
		text_box[focus_on].setHasFocus(true);
		tb_has_focus = true;
		kill_all_depending_buttons(loose_button, total_loose_buttons);
	}

	// boton de ventana de rendezvous
	else if (n == 14)
	{
		setWindowVisible(5, true);
		kill_all_depending_buttons(loose_button, total_loose_buttons);
	}

	// h-level
	else if (n == 15 && !object[control].getAerodynamicControls())
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getH_level();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setH_level(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// pro-grade
	else if (n == 16 && !object[control].getAerodynamicControls())
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getPro_grade();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setPro_grade(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// retro-grade
	else if (n == 17 && !object[control].getAerodynamicControls())
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getRetro_grade();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setRetro_grade(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// Normal
	else if (n == 18 && !object[control].getAerodynamicControls())
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getNormal();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setNormal(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// Anti-normal
	else if (n == 19 && !object[control].getAerodynamicControls())
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getAnti_normal();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setAnti_Normal(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// Retro-diff
	else if (n == 20 && !object[control].getAerodynamicControls() && object[control].getTarget() != 0)
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getRetroDiff();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setRetroDiff(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// Target H-level
	else if (n == 21 && !object[control].getAerodynamicControls() && object[control].getTarget() != 0)
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getAlignToTargetsUp();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setAlignToTargetsUp(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// Radial in
	else if (n == 22 && !object[control].getAerodynamicControls())
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getRadialIn();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setRadialIn(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// Radial out
	else if (n == 23 && !object[control].getAerodynamicControls())
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getRadialOut();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setRadialOut(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// Kill rotation
	else if (n == 24 && !object[control].getAerodynamicControls())
	{
		object[control].setKillRotation(!object[control].getKillRotation());

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	else if (n == 25)
	{
		toggle_depending_buttons(loose_button, total_loose_buttons, n);
	}

	else if (n == 26)
	{
		setWindowVisible(6, true);
		kill_all_depending_buttons(loose_button, total_loose_buttons);
	}

	// Lock to target
	else if (n == 27 && !object[control].getAerodynamicControls() && object[control].getTarget() != 0)
	{
		Config.changed_attitude = true;
		bool status_ = !object[control].getLock_Target();
		object[control].KillRCS();
		object[control].setKillRotation(true);
		object[control].setLock_Target(status_);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// Match speed of target
	else if (n == 28 && !object[control].getAerodynamicControls() && object[control].getTarget() != 0)
	{
		object[control].setMatchSpeed(!object[control].getMatchSpeed());
		object[control].setApproach(false);

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	// Approach target
	else if (n == 29 && !object[control].getAerodynamicControls() && object[control].getTarget() != 0)
	{
		object[control].setApproach(!object[control].getApproach());
		object[control].setMatchSpeed(false);

		if (object[control].getApproachDist() == 0)
		{
			object[control].setApproachDist(float(object[control].getScale() / 2 + object[object[control].getTarget()].getScale() / 2));
			std::stringstream texto_formateado;
			texto_formateado << std::setiosflags(std::ios::fixed) << std::setprecision(2) << object[control].getApproachDist();
			text_box[2].setText(texto_formateado.str());
			text_box[2].setRealText(text_box[2].getText());
			text_box[2].update();
		}

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");
	}

	else if (n == 30)
	{
		setWindowVisible(7, true);
		kill_all_depending_buttons(loose_button, total_loose_buttons);
	}

	// boton de surface map
	else if (n == 33)
	{
		setWindowVisible(8, true);
		kill_all_depending_buttons(loose_button, total_loose_buttons);
	}

	// re_entry prediction
	else if (n == 36)
	{
		re_entry_prediction = !re_entry_prediction;
		loose_button[n].toggled_on = re_entry_prediction;
		toggle_on_off_button(n, "textures/gui/re_entry_");
	}

	// button of next robotic arm part
	else if (n == 40)
	{
		if (robotic_arm[0].handled_part + 1 < robotic_arm[0].size)
		{
			robotic_arm[0].handled_part++;
		}
	}

	// button of previous robotic arm part
	else if (n == 41)
	{
		if (robotic_arm[0].handled_part - 1 >= 0)
		{
			robotic_arm[0].handled_part--;
		}
	}

	// button of increase movement of robotic arm part
	else if (n == 42)
	{
		double speed = 0.02;
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];

		// si es un hinge
		if (object[robotic_arm[0].members[robotic_arm[0].handled_part]].getConstraintType() == 1)
		{
			if (object[controled].getConstraintAxis() == 'x') bullet_physics.setAngularVel_X(controled, speed);
			else if (object[controled].getConstraintAxis() == 'y') bullet_physics.setAngularVel_Y(controled, speed);
			else if (object[controled].getConstraintAxis() == 'z') bullet_physics.setAngularVel_Z(controled, speed);
		}

		// si es un slider
		else if (object[controled].getConstraintType() == 2)
		{
			object[controled].setCurrentConstraintMove(object[controled].getCurrentConstraintMove() + 0.005);

			if (object[controled].getCurrentConstraintMove() <= object[controled].getMaxConstraintMove())
			{
				if (object[controled].getConstraintAxis() == 'x') object[controled].setDocked_position(object[controled].getOriginalDockedPosition() + GetAxisX(object[controled].getOriginalDockedRotation())*object[controled].getCurrentConstraintMove());
				if (object[controled].getConstraintAxis() == 'y') object[controled].setDocked_position(object[controled].getOriginalDockedPosition() + GetAxisY(object[controled].getOriginalDockedRotation())*object[controled].getCurrentConstraintMove());
				if (object[controled].getConstraintAxis() == 'z') object[controled].setDocked_position(object[controled].getOriginalDockedPosition() + GetAxisZ(object[controled].getOriginalDockedRotation())*object[controled].getCurrentConstraintMove());
			}

			else object[controled].setCurrentConstraintMove(object[controled].getMaxConstraintMove());
		}
	}

	// button of decrease movement of robotic arm part
	else if (n == 43)
	{
		double speed = 0.02;
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];

		// si es un hinge
		if (object[controled].getConstraintType() == 1)
		{
			if (object[controled].getConstraintAxis() == 'x') bullet_physics.setAngularVel_X(controled, -speed);
			else if (object[controled].getConstraintAxis() == 'y') bullet_physics.setAngularVel_Y(controled, -speed);
			else if (object[controled].getConstraintAxis() == 'z') bullet_physics.setAngularVel_Z(controled, -speed);
		}

		// si es un slider
		else if (object[controled].getConstraintType() == 2)
		{
			object[controled].setCurrentConstraintMove(object[controled].getCurrentConstraintMove() - 0.005);

			if (object[controled].getCurrentConstraintMove() >= object[controled].getMinConstraintMove())
			{
				if (object[controled].getConstraintAxis() == 'x') object[controled].setDocked_position(object[controled].getOriginalDockedPosition() + GetAxisX(object[controled].getOriginalDockedRotation())*object[controled].getCurrentConstraintMove());
				if (object[controled].getConstraintAxis() == 'y') object[controled].setDocked_position(object[controled].getOriginalDockedPosition() + GetAxisY(object[controled].getOriginalDockedRotation())*object[controled].getCurrentConstraintMove());
				if (object[controled].getConstraintAxis() == 'z') object[controled].setDocked_position(object[controled].getOriginalDockedPosition() + GetAxisZ(object[controled].getOriginalDockedRotation())*object[controled].getCurrentConstraintMove());
			}

			else object[controled].setCurrentConstraintMove(object[controled].getMinConstraintMove());
		}
	}

	// select controled as main focus
	else if (n == 44)
	{
		control = robotic_arm[0].members[robotic_arm[0].handled_part];
	}

	// button of camera mode
	else if (n == 45)
	{
		Config.camera_locked = !Config.camera_locked;
		Config.camera_changed = true;

		loose_button[n].visible = false;
		loose_button[46].visible = true;
	}

	// button of camera mode
	else if (n == 46)
	{
		Config.camera_locked = !Config.camera_locked;
		Config.camera_changed = true;

		loose_button[n].visible = false;
		loose_button[45].visible = true;
	}

	// Auto Grapple
	else if (n == 47)
	{
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];
		if (object[controled].getTarget() >= Config.total_initial_setup_objects
			&&
			(
			(!object[controled].getRoboticArmDPort() && object[controled].getTotalGrappleFixtures() > 0)
			||
			(object[controled].getRoboticArmDPort() && object[controled].getTotalDockingPorts() > 0)
			)
			&&
			(
			(!object[controled].getRoboticArmDPort() && object[object[controled].getTarget()].getTotalGrappleFixtures() > 0)
			||
			(object[controled].getRoboticArmDPort() && object[object[controled].getTarget()].getTotalDockingPorts() > 0)
			)
			)
		{
			object[controled].setAutoGrapple(!object[controled].getAutoGrapple());
		}

		loose_button[n].toggled_on = !loose_button[n].toggled_on;
		toggle_on_off_button(n, "textures/gui/");

		// Si apagamos el auto grapple, reseteamos todos las rotaciones y valores
		if (!loose_button[n].toggled_on) script_.stop_robotic_arm_rotations();
	}

	// release grapple
	else if (n == 48)
	{
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];

		for (int nn = 0; nn<total_objects; nn++)
		{
			if (object[nn].getSubType() != "Robotic Arm Part" && object[nn].getDockedTo() == controled) script_.undock_object(nn);
		}

		loose_button[n].toggled_on = false;
		loose_button[n].texture->~BitmapClass();
		loose_button[n].texture->Initialize(screenWidth, screenHeight,
			"textures/gui/grapple_n.png", "textures/gui/grapple_h.png", "textures/gui/grapple_c.png", "textures/white.png",
			loose_button[n].width, loose_button[n].heigth);
	}

	// select grapple target
	else if (n == 49)
	{
		int ventana = 12;
		setWindowVisible(ventana, true);
		focus_on = get_TBox_number(ventana, 0);
		text_box[focus_on].setHasFocus(true);
		tb_has_focus = true;
	}

	// robotic arm using grapple fixtures or docking ports
	else if (n == 50)
	{
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];
		object[controled].setAutoGrapple(false);
		object[controled].setRoboticArmDPort(!object[controled].getRoboticArmDPort());
		loose_button[n].toggled_on = object[controled].getRoboticArmDPort();

		if (!loose_button[n].toggled_on)
		{
			loose_button[n].texture->~BitmapClass();
			loose_button[n].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/dport_off_n.png", "textures/gui/dport_off_h.png", "textures/gui/dport_off_c.png", "textures/white.png",
				loose_button[n].width, loose_button[n].heigth);
		}

		else
		{
			loose_button[n].texture->~BitmapClass();
			loose_button[n].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/dport_on_n.png", "textures/gui/dport_on_h.png", "textures/gui/dport_on_c.png", "textures/white.png",
				loose_button[n].width, loose_button[n].heigth);
		}
	}

	// robotic arm visual helpers
	else if (n == 51)
	{
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];
		object[controled].setVisualHelpers(!object[controled].getVisualHelpers());
		loose_button[n].toggled_on = object[controled].getVisualHelpers();

		if (!loose_button[n].toggled_on)
		{
			loose_button[n].texture->~BitmapClass();
			loose_button[n].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/v_helpers_off_n.png", "textures/gui/v_helpers_off_h.png", "textures/gui/v_helpers_off_c.png", "textures/white.png",
				loose_button[n].width, loose_button[n].heigth);
		}

		else
		{
			loose_button[n].texture->~BitmapClass();
			loose_button[n].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/v_helpers_on_n.png", "textures/gui/v_helpers_on_h.png", "textures/gui/v_helpers_on_c.png", "textures/white.png",
				loose_button[n].width, loose_button[n].heigth);
		}
	}

	// robotic arm anchor
	else if (n == 52)
	{
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];
		int parent = object[controled].getTopAbsoluteParent();

		object[parent].setTarget(object[controled].getTarget());
		object[parent].setMatchSpeed(!object[parent].getMatchSpeed());		

		loose_button[n].toggled_on = object[parent].getMatchSpeed();

		if (!loose_button[n].toggled_on)
		{
			loose_button[n].texture->~BitmapClass();
			loose_button[n].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/anchor_n.png", "textures/gui/anchor_h.png", "textures/gui/anchor_c.png", "textures/white.png",
				loose_button[n].width, loose_button[n].heigth);
		}

		else
		{
			loose_button[n].texture->~BitmapClass();
			loose_button[n].texture->Initialize(screenWidth, screenHeight,
				"textures/gui/anchor_on_n.png", "textures/gui/anchor_on_h.png", "textures/gui/anchor_on_c.png", "textures/white.png",
				loose_button[n].width, loose_button[n].heigth);
		}
	}


	if (n != 0) dimm_other_buttons(loose_button, total_loose_buttons, n); // atenuamos los botones del mismo rango que no fueron seleccionados

	return;
}

void Gui_Class::kill_all_depending_buttons(w_button* button_group, int& total_group_buttons)
{
	for (int n = 0; n < total_group_buttons; n++)
	{
		if (loose_button[n].depends_on != -1)
		{
			loose_button[n].visible = false;
			loose_button[n].dimmed = 0.0f;
		}
	}
}

void Gui_Class::kill_depending_buttons(w_button* button_group, int& total_group_buttons, int n)
{
	for (int nn = 1; nn < total_group_buttons; nn++)
	{
		if (button_group[nn].depends_on == n)
		{
			button_group[nn].visible = false;
			loose_button[nn].dimmed = 0.0f;
			kill_depending_buttons(button_group, total_group_buttons, nn);
		}
	}
}

void Gui_Class::toggle_depending_buttons(w_button* button_group, int& total_group_buttons, int n)
{
	clean_up_buttons(button_group, total_group_buttons, n);
	for (int nn = 1; nn < total_loose_buttons; nn++)
	{
		if (loose_button[nn].depends_on == n)
		{
			loose_button[nn].visible = !loose_button[nn].visible;
			loose_button[nn].dimmed = 0.0f;
			kill_depending_buttons(button_group, total_group_buttons, nn);
		}
	}
}

void Gui_Class::clean_up_buttons(w_button* button_group, int& total_group_buttons, int n)
{
	for (int nn = 1; nn < total_loose_buttons; nn++)
	{
		if (loose_button[nn].depends_on == loose_button[n].depends_on && nn != n)
		{
			kill_depending_buttons(button_group, total_group_buttons, nn);
		}
	}
}

void Gui_Class::dimm_other_buttons(w_button* button_group, int& total_group_buttons, int n)
{
	for (int nn = 1; nn < total_loose_buttons; nn++)
	{
		if (button_group[nn].depends_on == button_group[n].depends_on && nn != n && button_group[n].depends_on != -1)
		{
			button_group[n].dimmed = 0.0f;
			if (!button_group[nn].toggeable) button_group[nn].dimmed = 2.0f;
		}
	}
}

bool Gui_Class::update_window_position(int ventana_number)
{
	ventanas[ventana_number].set_previous_position_x(ventanas[ventana_number].getPositionX());
	ventanas[ventana_number].set_previous_position_y(ventanas[ventana_number].getPositionY());

	// title
	if (!m_Text->update_sentence(ventanas[ventana_number].getTitle(), ventanas[ventana_number].getPositionX(),
		ventanas[ventana_number].getPositionY(), ventanas[ventana_number].getColor(),
		ventanas[ventana_number].get_title_sentence_number())) return false;

	for (int msg = 0; msg < ventanas[ventana_number].getTotalMessages(); msg++)
	{
		ventanas[ventana_number].set_message_pos_x(msg,
			ventanas[ventana_number].get_default_message_pos_x(msg) + ventanas[ventana_number].getPositionX());
		ventanas[ventana_number].set_message_pos_y(msg,
			ventanas[ventana_number].get_default_message_pos_y(msg) + ventanas[ventana_number].getPositionY());

		// messages
		if (!m_Text->update_sentence(ventanas[ventana_number].getMessage(msg),
			ventanas[ventana_number].get_message_pos_x(msg), ventanas[ventana_number].get_message_pos_y(msg),
			ventanas[ventana_number].get_message_color(msg), ventanas[ventana_number].get_message_sentence_number(msg))) return false;
	}



	int box_num = 0;
	for (int ventana_number = 0; ventana_number < total_ventanas; ventana_number++)
	{
		if (ventanas[ventana_number].getVisible())
		{
			if (ventanas[ventana_number].getTotalTextBoxes()>0)
			{
				for (int n = 0; n < ventanas[ventana_number].getTotalTextBoxes(); n++)
				{
					box_num = get_TBox_number(ventana_number, n);
					text_box[box_num].setPosX(text_box[box_num].getOrigX() + ventanas[ventana_number].getPositionX());
					text_box[box_num].setPosY(text_box[box_num].getOrigY() + ventanas[ventana_number].getPositionY());
					text_box[box_num].update();
				}
			}
		}
	}

	return true;
}

bool Gui_Class::render_ventanas()
{
	// render the windows' rectangles
	for (int n = 0; n < total_ventanas; n++)
	{
		if (ventanas[n].getVisible())
		{
			if (!window_rectangle_bitmap[n * total_ventanas_rectangles].Render(ventanas[n].getPositionX() - 10,
				ventanas[n].getPositionY() - 10)) return false;
			if (!m_TextureShader->Render(window_rectangle_bitmap[n].GetIndexCount(),
				matriz_identidad, matriz_identidad, ortho_matrix, window_rectangle_bitmap[n].GetTexture(), 1,
				window_rectangle_bitmap[n * total_ventanas_rectangles].getMouseStatus(), ventanas[n].getWindowColor(), { 0.f, 0.f, 0.f },
				ventanas[n].getDimmed())) return false;

			if (!window_rectangle_bitmap[n * total_ventanas_rectangles + 1].Render(ventanas[n].getPositionX() - 10,
				ventanas[n].getPositionY() + 19)) return false;

			if (ventanas[n].getType() == 1)
			{
				if (!m_TextureShader->Render(window_rectangle_bitmap[n * total_ventanas_rectangles + 1].GetIndexCount(),
					matriz_identidad, matriz_identidad, ortho_matrix, window_rectangle_bitmap[n * total_ventanas_rectangles + 1].GetTexture(), 2,
					window_rectangle_bitmap[n * total_ventanas_rectangles + 1].getMouseStatus(), ventanas[n].getWindowColor(), { 0.f, 0.f, 0.f },
					ventanas[n].getDimmed())) return false;
			}

			else if (ventanas[n].getType() == 2) // ventana con fondo de textura y mapa de luz solar
			{
				int orbited_ = 0;
				if (object[control].getType() == 4) orbited_ = object[control].getOrbiting();
				else orbited_ = control;

				vec3 aux_v = object[star_object].getPosition() - object[orbited_].getPosition(); // the orbital position
				quat aux_q = quat{ 0.0, 0.0, 0.0, 1.0 } / object[orbited_].getTotalRotation();

				vec3 rott = vector_rotation_by_quaternion(aux_v, ~aux_q);

				if (!m_TextureShader->Render(window_rectangle_bitmap[n * total_ventanas_rectangles + 1].GetIndexCount(),
					matriz_identidad, matriz_identidad, ortho_matrix, window_rectangle_bitmap[n * total_ventanas_rectangles + 1].GetTexture(), 7,
					window_rectangle_bitmap[n * total_ventanas_rectangles + 1].getMouseStatus(), ventanas[n].getWindowColor(),
					vec_to_d3d({ rott.z, -rott.y, rott.x }),
					ventanas[n].getDimmed())) return false;
			}

			else if (ventanas[n].getType() == 3) // ventana con fondo de textura
			{
				int orbited_ = 0;
				if (object[control].getType() == 4) orbited_ = object[control].getOrbiting();
				else orbited_ = control;

				vec3 aux_v = object[star_object].getPosition() - object[orbited_].getPosition(); // the orbital position
				quat aux_q = quat{ 0.0, 0.0, 0.0, 1.0 } / object[orbited_].getTotalRotation();

				vec3 rott = vector_rotation_by_quaternion(aux_v, ~aux_q);

				if (!m_TextureShader->Render(window_rectangle_bitmap[n * total_ventanas_rectangles + 1].GetIndexCount(),
					matriz_identidad, matriz_identidad, ortho_matrix, window_rectangle_bitmap[n * total_ventanas_rectangles + 1].GetTexture(), 10,
					window_rectangle_bitmap[n * total_ventanas_rectangles + 1].getMouseStatus(), ventanas[n].getWindowColor(),
					vec_to_d3d({ rott.z, -rott.y, rott.x }),
					ventanas[n].getDimmed())) return false;
			}

			if (!window_rectangle_bitmap[n * total_ventanas_rectangles + 2].Render(
				ventanas[n].getPositionX() + window_rectangle_bitmap[n * total_ventanas_rectangles].getBitmapWidth() - 34, ventanas[n].getPositionY() - 6))
				return false;
			if (!m_TextureShader->Render(window_rectangle_bitmap[n * total_ventanas_rectangles + 2].GetIndexCount(),
				matriz_identidad, matriz_identidad, ortho_matrix, window_rectangle_bitmap[n * total_ventanas_rectangles + 2].GetTexture(), 3,
				window_rectangle_bitmap[n * total_ventanas_rectangles + 2].getMouseStatus(), ventanas[n].getWindowColor(), { 0.f, 0.f, 0.f },
				ventanas[n].getDimmed())) return false;
		}
	}


	// render the windows' text boxes
	int box_num = 0;
	for (int n = 0; n < total_ventanas; n++)
	{
		if (ventanas[n].getVisible() && ventanas[n].getTotalTextBoxes()>0)
		{
			for (int nn = 0; nn < ventanas[n].getTotalTextBoxes(); nn++)
			{
				box_num = get_TBox_number(n, nn);
				if (!window_text_box[box_num].Render(ventanas[n].getPositionX() + ventanas[n].get_tb_posx(nn),
					ventanas[n].getPositionY() + ventanas[n].get_tb_posy(nn))) return false;


				if (!m_TextureShader->Render(window_text_box[box_num].GetIndexCount(),
					matriz_identidad, matriz_identidad, ortho_matrix, window_text_box[box_num].GetTexture(), 5,
					window_text_box[box_num].getMouseStatus(), ventanas[n].get_tb_box_color(nn), { 0.f, 0.f, 0.f },
					ventanas[n].getDimmed())) return false;

				if (!text_box[box_num].render()) return false;
			}
		}
	}

	// render the text inside the windows
	if (!m_Text->Render()) return false;

	return true;
}

bool Gui_Class::IsPointInRect(int X, int Y, int W, int H)
{
	mouse_x = cursor_position.x;
	mouse_y = cursor_position.y;

	if (mouse_x >= X && mouse_x <= W + X && mouse_y >= Y && mouse_y <= H + Y) return true;

	else return false;
}

bool Gui_Class::setWindowVisible(int ventana_, bool visible_)
{
	ventanas[ventana_].set_visible(visible_);

	if (!m_Text->InitializeSentence(ventanas[ventana_].get_title_sentence_number(), int(ventanas[ventana_].getTitle().size()),
		ventanas[ventana_].getVisible())) return false;

	for (int n = 0; n < ventanas[ventana_].getTotalMessages(); n++)
	{
		// set the message size to a minimum of 1 character and maximum of 100 in case the message has not been initialized yet
		if (!m_Text->InitializeSentence(ventanas[ventana_].get_message_sentence_number(n),
			min(100, max(1, ventanas[ventana_].get_message_size(n))),
			ventanas[ventana_].getVisible())) return false;
	}

	return true;
}

bool Gui_Class::getWindowVisible(int ventana_)
{
	return ventanas[ventana_].getVisible();
}

bool Gui_Class::real_time_updates()
{
	if (!manage_buttons_mouse_interaction(loose_button, total_loose_buttons)) return false;

	return true;
}

bool Gui_Class::render()
{
	GetCursorPos(&cursor_position);
	ScreenToClient(w_hwnd, &cursor_position);

	// checkeamos las funciones que usan el caret tick
	if (Config.caption_on || ventanas[5].getVisible()) caretTick += dt_a; // sumamos el tiempo delta al tiempo de aparicion del caret
	if (ventanas[8].getVisible())
	{
		timer_update_a += dt_a; // sumamos el timer a que controla el tiempo de actualizacion de la trayectoria de superficie
		timer_update_b += dt_a; // sumamos el timer a que controla el tiempo de actualizacion del icono de medi orbita
	}

	// calculos que no pueden esperar el update de los mensajes
	if (!update_priority_parameters()) return false;

	// esta instruccion la pasamos al showmaster junto con el update del autopilot
	//if (!manage_buttons_mouse_interaction(loose_button, total_loose_buttons)) return false;
	if (!manage_ventanas_mouse_interaction()) return false;
	if (!render_ventanas()) return false;
	if (!manage_tboxes()) return false;
	if (!render_window_depending_loose_buttons(loose_button, total_loose_buttons)) return false;
	if (!render_independent_loose_buttons(loose_button, total_loose_buttons)) return false;

	if (escape_pressed) kill_program = true;

	return true;
}

void Gui_Class::JdToYmd(const long lJD, int *piYear, int *piMonth, int *piDay)
{
#ifndef JULDATE_USE_ALTERNATE_METHOD

	long a, b, c, d, e, z, alpha;

	z = lJD;
	if (z < 2299161L)
		a = z;
	else
	{
		alpha = (long)((z - 1867216.25) / 36524.25);
		a = z + 1 + alpha - alpha / 4;
	}
	b = a + 1524;
	c = (long)((b - 122.1) / 365.25);
	d = (long)(365.25 * c);
	e = (long)((b - d) / 30.6001);
	*piDay = (int)b - d - (long)(30.6001 * e);
	*piMonth = (int)(e < 13.5) ? e - 1 : e - 13;
	*piYear = (int)(*piMonth > 2.5) ? (c - 4716) : c - 4715;
	if (*piYear <= 0)
		*piYear -= 1;

#else

	long t1, t2, yr, mo;

	t1 = lJD + 68569L;
	t2 = 4L * t1 / 146097L;
	t1 = t1 - (146097L * t2 + 3L) / 4L;
	yr = 4000L * (t1 + 1L) / 1461001L;
	t1 = t1 - 1461L * yr / 4L + 31L;
	mo = 80L * t1 / 2447L;
	*piDay = (int)(t1 - 2447L * mo / 80L);
	t1 = mo / 11L;
	*piMonth = (int)(mo + 2L - 12L * t1);
	*piYear = (int)(100L * (t2 - 49L) + yr + t1);

	// Correct for BC years
	if (*piYear <= 0)
		*piYear -= 1;

#endif

	return;
}


double Gui_Class::split_time(double jd, int *year, int *month, int *day,
	int *hr, int *min)
{
	long int_jd;
	double seconds;

	jd += .5;
	int_jd = (long)floor(jd);
	jd = (jd - (double)int_jd) * 1440.;     /* t is now in minutes */
	*min = (int)jd;
	if (*min == 1440)       /* evade rounding errors: */
	{
		int_jd++;
		jd = 0.;
		*min = 0;
	}
	seconds = (jd - (double)*min) * 60.;
	JdToYmd(int_jd, year, month, day);
	*hr = *min / 60;
	*min %= 60;
	return(seconds);
}

bool Gui_Class::set_message(int vn, int msg, std::string messag_)
{
	if (ventanas[vn].getVisible() || !updated_once)
	{
		if (ventanas[vn].get_message_dynamic(msg) || !updated_once)
		{
			ventanas[vn].set_message_(msg, messag_);

			// If the message size changes then resize it accordingly.
			// This will dynamically shrink or expand the size according to the message being displayed.
			// Note this means that if the new message size is longer than the window's length then it will 
			// surpass the window's border and look weird to the user.
			// However one of the biggest advantages of this method is that it will only draw as many quads as needed,
			// therefore increaing perfomance. It will also display the whole message without clamping it to
			// a fixed size.
			if (ventanas[vn].getMessage(msg).size() != ventanas[vn].get_message_size(msg))
			{
				ventanas[vn].set_message_size(msg, int(ventanas[vn].getMessage(msg).size()));

				if (!m_Text->InitializeSentence(ventanas[vn].get_message_sentence_number(msg), int(ventanas[vn].getMessage(msg).size()),
					ventanas[vn].getVisible())) return false;
			}

			// In case we change our mind and want to actually clamp the message size to a predetermined one
			// to prevent it from surpassing the window borders.
			/*
			{
			messag_.resize(10); // new message length
			ventanas[vn].set_message_(msg, messag_);
			}
			*/
		}
	}

	return true;
}

bool Gui_Class::render_independent_loose_buttons(w_button* button_group, int& total_group_buttons)
{
	for (int n = 0; n < total_group_buttons; n++)
	{
		// botones que dependen de ventanas
		if (button_group[n].ventana == -1)
		{
			if (button_group[n].visible)
			{
				if (!button_group[n].texture->Render(button_group[n].position_x, button_group[n].position_y)) return false;
				if (!m_TextureShader->Render(button_group[n].texture->GetIndexCount(),
					matriz_identidad, matriz_identidad, ortho_matrix, button_group[n].texture->GetTexture(), button_group[n].tipo_,
					button_group[n].texture->getMouseStatus(), button_group[n].color_, { 0.f, 0.f, 0.f }, button_group[n].dimmed)) return false;
			}
		}
	}

	return true;
}

bool Gui_Class::render_window_depending_loose_buttons(w_button* button_group, int& total_group_buttons)
{
	for (int n = 0; n < total_group_buttons; n++)
	{
		// botones que dependen de ventanas
		if (button_group[n].ventana != -1)
		{
			if (ventanas[button_group[n].ventana].getVisible())
			{
				if (button_group[n].tipo_ == 6 || button_group[n].tipo_ == 8) handle_moving_button(button_group, n); // botones no clickeables

				else
				{
					button_group[n].visible = true;

					button_group[n].position_x = button_group[n].position_orig_x + ventanas[button_group[n].ventana].getPositionX();
					button_group[n].position_y = button_group[n].position_orig_y + ventanas[button_group[n].ventana].getPositionY();

					if (n == 38) // el pointer rotante de la brujula
					{
						if (!button_group[n].texture->Render(screenWidth / 2 - button_group[n].width / 2, screenHeight / 2 - button_group[n].heigth / 2)) return false;

						Matrix tras_;
						tras_.Translation(Vector3{
							float(button_group[n].position_x) + button_group[n].width / 2 - screenWidth / 2,
							-(float(button_group[n].position_y) + button_group[n].heigth / 2 - screenHeight / 2),
							0.f });

							tras_ = giro*tras_;

							if (!m_TextureShader->Render(button_group[n].texture->GetIndexCount(),
								tras_, matriz_identidad, ortho_matrix, button_group[n].texture->GetTexture(), button_group[n].tipo_,
								button_group[n].texture->getMouseStatus(), button_group[n].color_, { 0.f, 0.f, 0.f }, button_group[n].dimmed)) return false;

							//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							//										Dibujamos los grados dentro de la brujula												//
							static bool compass_init = false;

							if (!compass_init)
							{
								Config.create_screen_sentences = true;
								compass_init = true;
							}

							std::string texto = std::to_string(int(degrees_)) + "}";
							if (texto.length() >= 2 && texto.length() <= 4)
							{
								if (texto.length() == 2) texto = "  " + texto;
								else if (texto.length() == 3) texto = " " + texto;
							}

							else  texto = "Erro";

							Vector2 pos_ = { float(button_group[37].position_x + 37), float(button_group[37].position_y + 45) };
							draw_screen_text(&pos_, &texto, &color->White, 1);
							//																																//
							//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					}

					else
					{
						if (!button_group[n].texture->Render(button_group[n].position_x, button_group[n].position_y)) return false;
						if (!m_TextureShader->Render(button_group[n].texture->GetIndexCount(),
							matriz_identidad, matriz_identidad, ortho_matrix, button_group[n].texture->GetTexture(), button_group[n].tipo_,
							button_group[n].texture->getMouseStatus(), button_group[n].color_, { 0.f, 0.f, 0.f }, button_group[n].dimmed)) return false;
					}
				}
			}

			else button_group[n].visible = false;
		}
	}

	return true;
}

int Gui_Class::get_TBox_number(int ventana, int tbox_)
{
	int box_num = 0;
	for (int n = 0; n < total_ventanas; n++)
	{
		if (ventanas[n].getTotalTextBoxes()>0)
		{
			for (int nn = 0; nn < ventanas[n].getTotalTextBoxes(); nn++)
			{
				if (n == ventana && nn == tbox_) return box_num;
				box_num++;
			}
		}
	}

	return 0;
}

bool Gui_Class::manage_tboxes()
{
	// si algun textbox esta activo
	if (focus_on != -1)
	{
		//MessageBoxA(m_hwnd, "OK", "Error", MB_OK);

		Config.caption_on = true; // comunicamos al resto del proyecto que un textbox esta activo, asi evitamos que los controles usen las teclas presionadas

		if (caretTick >= 0.5f) // cada medio segundo el caret aparece o desaparece
		{
			caretTick = 0; // reseteamos el tiempo de aparicion del caret
			showCaret = !showCaret; // cambiamos el estado del caret si se cumplio el medio segundo
		}

		if (key_pressed != 0)
		{
			if (key_pressed == VK_BACK)
			{

				if (text_box[focus_on].getRealText().size() > 0)
				{
					text_box[focus_on].getRealText().erase(text_box[focus_on].getRealText().size() - 1, 1);
					showCaret = true;
					caretTick = 0;
				}
			}

			else if (key_pressed == VK_RETURN)
			{
				if (text_box[focus_on].getRealText().size() > 0)
				{
					text_box[focus_on].setText(text_box[focus_on].getRealText());
					text_box[focus_on].update();

					text_box[focus_on].setHasFocus(false);
					tb_has_focus = false;

					key_pressed = 0; // reseteamos la ultima tecla presionada para evitar que su valor sea usado por los controles
					Config.caption_on = false; // indicamos al resto del programa que ningun textbox esta activo, asi los controles pueden ejecutarse normalmente otra vez

					perform_textbox_action(focus_on);
					focus_on = -1;

					return true;
				}
			}

			else if (key_pressed == VK_ESCAPE)
			{
				text_box[focus_on].setText(text_box[focus_on].getRealText());
				text_box[focus_on].update();

				text_box[focus_on].setHasFocus(false);
				focus_on = -1;
				tb_has_focus = false;

				key_pressed = 0; // reseteamos la ultima tecla presionada para evitar que su valor sea usado por los controles
				Config.caption_on = false; // indicamos al resto del programa que ningun textbox esta activo, asi los controles pueden ejecutarse normalmente otra vez

				return true;
			}

			// agregamos la ultima letra ingresada si el largo del texto no es mayor al limite establecido ni al recuadro de texto
			else if (true)
				//(int(text_box[focus_on].getRealText().size()) + 1 < text_box[focus_on].getMaxSize() &&
				//text_box[focus_on].get_last_pixel_position(0) + screenWidth / 2 <
				//window_text_box[focus_on].getBitmapWidth() + ventanas[text_box[focus_on].getDependsOn()].getPositionX()
				//+ ventanas[text_box[focus_on].getDependsOn()].get_tb_posx(get_TBox_number(text_box[focus_on].getDependsOn(), focus_on)) - 15)
			{
				if (text_box[focus_on].getType() == 0) text_box[focus_on].setRealText(text_box[focus_on].getRealText() + key_pressed);
				else if (text_box[focus_on].getType() == 1) // solo se permiten numeros
				{
					if (key_pressed == '0' ||
						key_pressed == '1' ||
						key_pressed == '2' ||
						key_pressed == '3' ||
						key_pressed == '4' ||
						key_pressed == '5' ||
						key_pressed == '6' ||
						key_pressed == '7' ||
						key_pressed == '8' ||
						key_pressed == '9' ||
						key_pressed == '.' ||
						key_pressed == ',' ||
						key_pressed == '-')

						text_box[focus_on].setRealText(text_box[focus_on].getRealText() + key_pressed);
				}
			}

			key_pressed = 0;
		}

		if (showCaret) text_box[focus_on].setText(text_box[focus_on].getRealText() + "|"); // agregamos el caret al texto que se muesta en el textbox
		else text_box[focus_on].setText(text_box[focus_on].getRealText()); // reseteamos el texto del textbox a su texto original


		text_box[focus_on].update(); // actualizamos el grafico del texto que se va a mostrar en la pantalla
	}

	// si ningun textbox esta activo
	else
	{
		key_pressed = 0; // reseteamos la ultima tecla presionada para evitar que su valor sea usado por los controles
		Config.caption_on = false; // indicamos al resto del programa que ningun textbox esta activo, asi los controles pueden volver a ejecutarse normalmente
	}


	return true;
}

void Gui_Class::perform_textbox_action(int tb_)
{
	// Cambiar el objeto controlado
	if (tb_ == 0)
	{
		for (int n = 0; n < int(object.size()); n++)
		{
			if (text_box[tb_].getRealText() == object[n].getName())
			{
				Config.create_projected_sentences = true;
				control = n;
				universe_scale = 1.0f / float(object[control].getScale());
				switched_shadows = true;
				setWindowVisible(text_box[tb_].getDependsOn(), false);
				switched_object = true;
			}
		}
	}

	// Cambiar el objetivo
	else if (tb_ == 1)
	{
		for (int n = 0; n < total_objects; n++)
		{
			if (text_box[tb_].getRealText() == object[n].getName())
			{
				Config.create_projected_sentences = true;
				object[control].setTarget(n);
				setWindowVisible(text_box[tb_].getDependsOn(), false);
				//switched_object = true; // *** DEBUG *** see why this line was important, i think it had something to do with the surface map updating
				// the target path
			}
		}
	}

	// Establecer la distancia de acercamiento
	else if (tb_ == 2)
	{
		object[control].setApproachDist(stof(text_box[tb_].getRealText()));
		if (text_box[tb_].getRealText() == "") object[control].setApproachDist(0.f);
	}

	// Establecer el puerto local del robotic arm
	else if (tb_ == 3)
	{
		int selection = stoi(text_box[tb_].getRealText());
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];

		if (object[controled].getRoboticArmDPort())
		{
			if (selection >= object[controled].getTotalDockingPorts()) selection = object[controled].getTotalDockingPorts() - 1;
			else if (selection < 0) selection = 0;
		}

		else
		{
			if (selection >= object[controled].getTotalGrappleFixtures()) selection = object[controled].getTotalGrappleFixtures() - 1;
			else if (selection < 0) selection = 0;
		}

		if ((object[controled].getRoboticArmDPort() && object[controled].getTotalDockingPorts() > 0) ||
			(!object[controled].getRoboticArmDPort() && object[controled].getTotalGrappleFixtures() > 0))
		{
			object[controled].setLocalFixture(selection);
		}

		else selection = 0;

		if (text_box[tb_].getRealText() == "") object[controled].setLocalFixture(0);
		text_box[tb_].setRealText(std::to_string(selection));
		text_box[tb_].setText(std::to_string(selection));
		text_box[tb_].update();
	}

	// Establecer el puerto del target del robotic arm
	else if (tb_ == 4)
	{
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];

		int selection = stoi(text_box[tb_].getRealText());
		int target_ = object[controled].getTarget();

		if (object[controled].getRoboticArmDPort())
		{
			if (selection >= object[target_].getTotalDockingPorts()) selection = object[target_].getTotalDockingPorts() - 1;
			else if (selection < 0) selection = 0;
		}

		else
		{
			if (selection >= object[target_].getTotalGrappleFixtures()) selection = object[target_].getTotalGrappleFixtures() - 1;
			else if (selection < 0) selection = 0;
		}

		if ((object[controled].getRoboticArmDPort() && object[target_].getTotalDockingPorts() > 0) ||
			(!object[controled].getRoboticArmDPort() && object[target_].getTotalGrappleFixtures() > 0))
		{
			object[controled].setTargetFixture(selection);
		}

		else selection = 0;

		if (text_box[tb_].getRealText() == "") object[controled].setTargetFixture(0);
		text_box[tb_].setRealText(std::to_string(selection));
		text_box[tb_].setText(std::to_string(selection));
		text_box[tb_].update();
	}

	// Cambiar el objetivo del grapple
	else if (tb_ == 5)
	{
		int controled = robotic_arm[0].members[robotic_arm[0].handled_part];

		for (int n = 0; n < total_objects; n++)
		{
			if (text_box[tb_].getRealText() == object[n].getName())
			{
				Config.create_projected_sentences = true;
				object[controled].setTarget(n);
				setWindowVisible(text_box[tb_].getDependsOn(), false);
			}
		}
	}
}

bool Gui_Class::draw_line_strip(Vector3* buffer_, int buffer_size, Color color_)
{
	D3D_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	LineClass line_;
	if (!line_.create_points_buffer(buffer_, buffer_size)) return false;

	line_.set_buffers();

	dumb_shader->set_dumb_shader();

	if (!dumb_shader->SetShaderParameters(matriz_identidad, ViewMatrix, ProjectionMatrix, color_)) return false;

	D3D_context->Draw(buffer_size, 0);


	return true;
}

bool Gui_Class::draw_2D_line_strip(Vector3* buffer_, int buffer_size, Color color_)
{
	D3D_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	LineClass line_;
	if (!line_.create_points_buffer(buffer_, buffer_size)) return false;

	line_.set_buffers();

	dumb_shader->set_dumb_shader();

	if (!dumb_shader->SetShaderParameters(matriz_identidad, matriz_identidad, ortho_matrix, color_)) return false;

	D3D_context->Draw(buffer_size, 0);


	return true;
}

bool Gui_Class::draw_line_list(Vector3* buffer_, int buffer_size, Color color_)
{
	D3D_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	LineClass line_;
	if (!line_.create_points_buffer(buffer_, buffer_size)) return false;

	line_.set_buffers();

	dumb_shader->set_dumb_shader();

	if (!dumb_shader->SetShaderParameters(matriz_identidad, ViewMatrix, matriz_identidad, color_)) return false;

	D3D_context->Draw(buffer_size, 0);

	return true;
}

bool Gui_Class::draw_2D_line_list(Vector3* buffer_, int buffer_size, Color color_)
{
	D3D_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	LineClass line_;
	if (!line_.create_points_buffer(buffer_, buffer_size)) return false;

	line_.set_buffers();

	dumb_shader->set_dumb_shader();

	if (!dumb_shader->SetShaderParameters(matriz_identidad, matriz_identidad, ortho_matrix, color_)) return false;

	D3D_context->Draw(buffer_size, 0);

	return true;
}

bool Gui_Class::draw_projected_text(Vector3* position_buffer, std::string* text_buffer, Color* color_buffer, int buffer_size)
{
	if (buffer_size > 0)
	{
		bool visible = true;
		Vector2* screen_pos = new Vector2[buffer_size];

		for (int n = 0; n < buffer_size; n++)
		{
			WorldToScreen(position_buffer[n], &screen_pos[n]);
			//screen_pos[n].x -= 4;
			screen_pos[n].y -= 5;
		}

		if (Config.create_projected_sentences) textos->create_sentence_array(buffer_size);

		for (int n = 0; n < buffer_size; n++)
		{
			if (int(screen_pos[n].x) < 0) textos->set_visible(n, false);
			else textos->set_visible(n, true);

			if (Config.create_projected_sentences)
			{
				if (!textos->InitializeSentence(n, int(text_buffer[n].size()),
					true)) return false;
			}

			if (!textos->update_sentence(text_buffer[n],
				int(screen_pos[n].x), int(screen_pos[n].y),
				color_buffer[n], n)) return false;
		}

		if (!textos->Render()) return false;

		SAFE_DELETE_ARRAY(screen_pos);
	}

	if (Config.create_projected_sentences) Config.create_projected_sentences = false;

	return true;
}


bool Gui_Class::draw_screen_text(Vector2* screen_pos, std::string* text_buffer, Color* color_buffer, int buffer_size)
{
	if (buffer_size > 0)
	{
		bool visible = true;

		if (Config.create_screen_sentences) textos_screen->create_sentence_array(buffer_size);

		for (int n = 0; n < buffer_size; n++)
		{
			if (int(screen_pos[n].x) < 0) textos_screen->set_visible(n, false);
			else textos_screen->set_visible(n, true);

			if (Config.create_screen_sentences)
			{
				if (!textos_screen->InitializeSentence(n, int(text_buffer[n].size()),
					true)) return false;
			}

			if (!textos_screen->update_sentence(text_buffer[n],
				int(screen_pos[n].x), int(screen_pos[n].y),
				color_buffer[n], n)) return false;
		}

		if (!textos_screen->Render()) return false;
	}

	if (Config.create_screen_sentences) Config.create_screen_sentences = false;

	return true;
}

bool Gui_Class::WorldToScreen(DirectX::XMVECTOR pV, Vector2* pOut2d)
{

	DirectX::XMMATRIX pWorld = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX pProjection = ProjectionMatrix;
	DirectX::XMMATRIX pView = ViewMatrix;

	Matrix m1, m2;
	Vector3 vec;
	DirectX::XMFLOAT3 pOut3d;

	m1 = pWorld* pView;
	m2 = m1*pProjection;
	vec = DirectX::XMVector3TransformCoord(pV, m2);
	pOut3d.x = (1.0f + vec.x) * screenWidth / 2.0f;
	pOut3d.y = (1.0f - vec.y) * screenHeight / 2.0f;
	pOut3d.z = vec.z;

	if (pOut3d.z < 1.0f)
	{
		pOut2d->x = pOut3d.x; pOut2d->y = pOut3d.y;
		return true;
	}
	else pOut2d->x = -1;

	return false;
}

void Gui_Class::set_angulos_(double angulos_b)
{
	angulos_ = angulos_b;
}

Gui_Class::Intersection_data Gui_Class::angles_to_ascending_intersection_point(int cuerpo1, int cuerpo2)
{
	static double angulo_anterior1 = 0.0;

	vec3 x = normalized(object[cuerpo2].getSpecificAngularMomentum() ^ object[cuerpo1].getSpecificAngularMomentum());
	vec3 vector_to_cuerpo = normalized(object[cuerpo1].getOrbitalPosition());
	double angulo_asc1 = angle_from_two_vectors(x, vector_to_cuerpo);
	double angulo_des1 = abs(angulo_asc1 - phi_d);
	double true_anomaly_at_intersection_asc1 = 0;

	if (angulo_anterior1 < angulo_asc1)
	{
		if (object[cuerpo1].getTrueAnomaly()>angulo_asc1)
			true_anomaly_at_intersection_asc1 = object[cuerpo1].getTrueAnomaly() - angulo_asc1;
		else
			true_anomaly_at_intersection_asc1 = 2.0 * phi_d + object[cuerpo1].getTrueAnomaly() - angulo_asc1;
	}

	else if (angulo_anterior1 > angulo_asc1)
	{
		true_anomaly_at_intersection_asc1 = object[cuerpo1].getTrueAnomaly() + angulo_asc1;
	}

	angulo_anterior1 = angulo_asc1;

	double dist_asc1 = (object[cuerpo1].getSemiMayorAxis() * (1 - (object[cuerpo1].getEccentricity() * object[cuerpo1].getEccentricity()))) / (1.0 + object[cuerpo1].getEccentricity() * cos(true_anomaly_at_intersection_asc1));
	double true_anomaly_at_intersection_des1 = abs(phi_d + true_anomaly_at_intersection_asc1);
	Limit360(true_anomaly_at_intersection_des1);
	double dist_des1 = (object[cuerpo1].getSemiMayorAxis() * (1.0 - (object[cuerpo1].getEccentricity() * object[cuerpo1].getEccentricity()))) / (1.0 + object[cuerpo1].getEccentricity() * cos(true_anomaly_at_intersection_des1));


	////////////////////////////////////////////////////
	static double angulo_anterior2 = 0.0;

	vec3 x2 = normalized(object[cuerpo1].getSpecificAngularMomentum() ^ object[cuerpo2].getSpecificAngularMomentum());
	vec3 vector_to_cuerpo2 = normalized(object[cuerpo2].getOrbitalPosition());
	double angulo_asc2 = angle_from_two_vectors(x2, vector_to_cuerpo2);
	double angulo_des2 = abs(angulo_asc2 - phi_d);
	double true_anomaly_at_intersection_asc2 = 0;

	if (angulo_anterior2 < angulo_asc2)
	{
		if (object[cuerpo2].getTrueAnomaly()>angulo_asc2)
			true_anomaly_at_intersection_asc2 = object[cuerpo2].getTrueAnomaly() - angulo_asc2;
		else
			true_anomaly_at_intersection_asc2 = 2 * phi_d + object[cuerpo2].getTrueAnomaly() - angulo_asc2;
	}

	else if (angulo_anterior2 > angulo_asc2)
	{
		true_anomaly_at_intersection_asc2 = object[cuerpo2].getTrueAnomaly() + angulo_asc2;
	}

	angulo_anterior2 = angulo_asc2;

	double dist_asc2 = (object[cuerpo2].getSemiMayorAxis() * (1.0 - (object[cuerpo2].getEccentricity() * object[cuerpo2].getEccentricity()))) / (1.0 + object[cuerpo2].getEccentricity() * cos(true_anomaly_at_intersection_asc2));
	double true_anomaly_at_intersection_des2 = abs(phi_d + true_anomaly_at_intersection_asc2);
	Limit360(true_anomaly_at_intersection_des2);
	double dist_des2 = (object[cuerpo2].getSemiMayorAxis() * (1.0 - (object[cuerpo2].getEccentricity() * object[cuerpo2].getEccentricity()))) / (1.0 + object[cuerpo2].getEccentricity() * cos(true_anomaly_at_intersection_des2));

	double DTmin_asc = abs(dist_asc1 - dist_asc2);
	double DTmin_des = abs(dist_des1 - dist_des2);

	return{ angulo_asc1, angulo_asc2, angulo_des1, angulo_des2, true_anomaly_at_intersection_asc1, true_anomaly_at_intersection_asc2, true_anomaly_at_intersection_des1, true_anomaly_at_intersection_des2, DTmin_asc, DTmin_des };
}

double Gui_Class::time_to_reach_true_anomaly(int cuerpo, double trueAnomaly)
{
	double ecc_anom = convertTrueAnomalyToEllipticalEccentricAnomaly(trueAnomaly, object[cuerpo].getEccentricity());
	double mean_anom = convertEllipticalEccentricAnomalyToMeanAnomaly(ecc_anom, object[cuerpo].getEccentricity());
	return (mean_anom / sqrt((GRAVITATIONAL_CONSTANT*object[object[cuerpo].getOrbiting()].getMass()) / (object[cuerpo].getSemiMayorAxis() * object[cuerpo].getSemiMayorAxis() * object[cuerpo].getSemiMayorAxis()))) - object[cuerpo].getTimeSincePerigee();
}

double Gui_Class::convertTrueAnomalyToEllipticalEccentricAnomaly(const double trueAnomaly,
	const double eccentricity)
{
	// Declare and compute sine and cosine of eccentric anomaly.
	double sineOfEccentricAnomaly_ = sqrt(1.0 - pow(eccentricity, 2.0))
		* sin(trueAnomaly) / (1.0 + eccentricity * cos(trueAnomaly));
	double cosineOfEccentricAnomaly_ = (eccentricity + cos(trueAnomaly))
		/ (1.0 + eccentricity * cos(trueAnomaly));

	// Return elliptic eccentric anomaly.
	return atan2(sineOfEccentricAnomaly_, cosineOfEccentricAnomaly_);
}


double Gui_Class::convertEllipticalEccentricAnomalyToMeanAnomaly(const double ellipticalEccentricAnomaly,
	const double eccentricity)
{
	return ellipticalEccentricAnomaly - eccentricity * sin(ellipticalEccentricAnomaly);
}

// Real time toggeable buttons status check
void Gui_Class::update_buttons()
{
	for (int n = 0; n < total_loose_buttons; n++)
	{
		// ventana del rcs
		if (ventanas[6].getVisible())
		{
			if (n == 15)
			{
				if (object[control].getH_level()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 16)
			{
				if (object[control].getPro_grade()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 17)
			{
				if (object[control].getRetro_grade()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 18)
			{
				if (object[control].getNormal()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 19)
			{
				if (object[control].getAnti_normal()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 20)
			{
				if (object[control].getRetroDiff()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 21)
			{
				if (object[control].getAlignToTargetsUp()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 22)
			{
				if (object[control].getRadialIn()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 23)
			{
				if (object[control].getRadialOut()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 24)
			{
				if (object[control].getKillRotation()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 27)
			{
				if (object[control].getLock_Target()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}
		}

		// ventana del autopilot
		if (ventanas[7].getVisible())
		{
			if (n == 28)
			{
				if (object[control].getMatchSpeed()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			if (n == 29)
			{
				if (object[control].getApproach()) loose_button[n].toggled_on = true;
				else loose_button[n].toggled_on = false;
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}
		}

		if (ventanas[11].getVisible())
		{
			// auto grapple button
			if (n == 47)
			{
				int controled = robotic_arm[0].members[robotic_arm[0].handled_part];
				loose_button[n].toggled_on = object[controled].getAutoGrapple();
				if (loose_button[n].last_toggle_status != loose_button[n].toggled_on) toggle_on_off_button(n, "textures/gui/");
			}

			// make sure the grapple button is on if the handled part's target is grappled
			if (n == 48)
			{
				int controled = robotic_arm[0].members[robotic_arm[0].handled_part];

				if (object[object[controled].getTarget()].getDockedTo() == controled && !loose_button[n].toggled_on)
				{
					loose_button[n].toggled_on = true;

					loose_button[n].texture->~BitmapClass();
					loose_button[n].texture->Initialize(screenWidth, screenHeight,
						"textures/gui/grapple_on_n.png", "textures/gui/grapple_on_h.png", "textures/gui/grapple_on_c.png", "textures/white.png",
						loose_button[n].width, loose_button[n].heigth);

					// desactivamos el anchor
					if (loose_button[52].toggled_on)
					{
						if (object[object[controled].getTopAbsoluteParent()].getTarget() == object[controled].getTarget() &&
							object[object[controled].getTopAbsoluteParent()].getMatchSpeed())
						{
							object[object[controled].getTopAbsoluteParent()].setMatchSpeed(false);
						}

						loose_button[52].toggled_on = false;
						loose_button[52].texture->~BitmapClass();
						loose_button[52].texture->Initialize(screenWidth, screenHeight,
							"textures/gui/anchor_n.png", "textures/gui/anchor_h.png", "textures/gui/anchor_c.png", "textures/white.png",
							loose_button[52].width, loose_button[52].heigth);
					}
				}

				// desactivamos el grappled button si el target no esta acoplado
				else if (object[object[controled].getTarget()].getDockedTo() != controled && loose_button[n].toggled_on)
				{
					loose_button[n].toggled_on = false;

					loose_button[n].texture->~BitmapClass();
					loose_button[n].texture->Initialize(screenWidth, screenHeight,
						"textures/gui/grapple_n.png", "textures/gui/grapple_h.png", "textures/gui/grapple_c.png", "textures/white.png",
						loose_button[n].width, loose_button[n].heigth);
				}
			}
		}
	}
}

void Gui_Class::toggle_on_off_button(int n, std::string texture_path)
{
	loose_button[n].last_toggle_status = loose_button[n].toggled_on;
	if (!loose_button[n].toggled_on)
	{
		loose_button[n].texture->~BitmapClass();
		loose_button[n].texture->Initialize(screenWidth, screenHeight,
			texture_path + "off_n.png", texture_path + "off_h.png", texture_path + "off_c.png", "textures/white.png",
			loose_button[n].width, loose_button[n].heigth);
	}

	else
	{
		loose_button[n].texture->~BitmapClass();
		loose_button[n].texture->Initialize(screenWidth, screenHeight,
			texture_path + "on_n.png", texture_path + "on_h.png", texture_path + "on_c.png", "textures/white.png",
			loose_button[n].width, loose_button[n].heigth);
	}
}

std::string Gui_Class::convert_to_fixed_degree(double grados, int ll)
{
	// si ll=1 el valor es una latitud
	// si ll=2 el valor es una longitud
	// si ll es otro valor entonces ignoramos los puntos cardinales
	std::string retornar;
	std::string cardinal;

	int degree_conv = (int)grados;
	if (ll == 1)
	{
		if (degree_conv < 0) cardinal = " (S)";
		else cardinal = " (N)";
	}
	else if (ll == 2)
	{
		if (degree_conv < 0) cardinal = " (W)";
		else cardinal = " (E)";
	}
	if (abs(degree_conv) < 100) retornar += " ";
	if (abs(degree_conv) < 10) retornar += " ";

	if (degree_conv < 0 && ll != 1 && ll != 2) retornar += "-";

	retornar += std::to_string(abs(degree_conv)) + "}";

	int minutes_conv = (int)((grados - (double)degree_conv) * 60.0);
	//if (abs(minutes_conv) < 100) retornar += " ";
	if (abs(minutes_conv) < 10) retornar += " ";
	retornar += " " + std::to_string(abs(minutes_conv)) + "'";

	int seconds_conv = (int)((grados - (double)degree_conv - (double)minutes_conv / 60.0) * 60.0 * 60.0);
	//if (abs(seconds_conv) < 100) retornar += " ";
	if (abs(seconds_conv) < 10) retornar += " ";
	retornar += " " + std::to_string(abs(seconds_conv)) + "\"" + cardinal;

	return retornar;
}

Gui_Class::latlon Gui_Class::cartesian_to_geodetic2(int objeto_orbitado, vec3 posic)
{
	int orbited_ = object[control].getOrbiting();
	double lat = 0, lon = 0, alt = 0;
	double x = posic.x;
	double y = posic.y;
	double z = posic.z;
	double semi_major_axis = 0; // semi-major axis

	if (object[orbited_].getFlattening() > 0.0)
	{
		if (object[orbited_].getEquatorialRadius() < object[orbited_].getPolarRadius())
			semi_major_axis = object[orbited_].getPolarRadius() * 1000;

		else semi_major_axis = object[orbited_].getEquatorialRadius() * 1000;
	}

	else
	{
		semi_major_axis = object[orbited_].getScale() / 2;
	}

	double _f = object[orbited_].getFlattening();
	// flatenning = 1 - (semi_minor_axis / semi_major_axis);

	const double eps_h = 1e-3;
	const double eps_b = 1e-10;
	const int imax = 1;  // default: 30

	double q = sqrt(x * x + y * y);
	lon = atan2(y, x);
	alt = 0.;

	double N = semi_major_axis;
	double _e2 = _f * (2. - _f);
	for (int i = 0; i < imax; i++)
	{
		double h0 = alt;
		double b0 = lat;
		lat = atan(z / q / (1 - _e2 * N / (N + alt)));
		double sin_b = sin(lat);
		N = semi_major_axis / sqrt(1. - _e2 * sin_b * sin_b);
		alt = q / cos(lat) - N;
		//cout << i << ": "<< b << ", " << h << endl;
		if (fabs(alt - h0) < eps_h && fabs(lat - b0) < eps_b) return{ -lat, lon, alt }; // invertimos la latitud no se por que, pero asi salen bien los calculos
	}

	return{ -lat, lon, alt }; // invertimos la latitud no se por que, pero asi salen bien los calculos
}

vec3 Gui_Class::geodetic_to_cartesian(int orbited_, double lati, double longi, double alti)
{
	longi *= -1; // invertimos la longitud no se por que, pero asi salen bien los calculos
	lati = ToRadians_d(lati);
	longi = ToRadians_d(longi);
	double xx;
	double yy;
	double zz;
	double semi_major_axis = 0;

	if (object[orbited_].getFlattening() > 0.0)
	{
		if (object[orbited_].getEquatorialRadius() < object[orbited_].getPolarRadius())
			semi_major_axis = object[orbited_].getPolarRadius() * 1000;

		else semi_major_axis = object[orbited_].getEquatorialRadius() * 1000;
	}

	else
	{
		semi_major_axis = object[orbited_].getScale() / 2;
	}

	double slat = sin(lati);
	double clat = cos(lati);

	double r = semi_major_axis / sqrt(1.0 - (object[orbited_].getFlattening() * (2 - object[orbited_].getFlattening())) * slat * slat);

	xx = (r + alti) * clat * cos(longi);
	yy = (r + alti) * clat * sin(longi);
	zz = (r * (1 - (object[orbited_].getFlattening() * (2 - object[orbited_].getFlattening()))) + alti) * slat;

	vec3 rott = vector_rotation_by_quaternion(
	{ xx, zz, -yy },
	~object[orbited_].getTotalRotation());

	return rott;
}

Vector3 Gui_Class::MapPositionPrediction(int cuerpo, int seconds_)
{
	int orbited_ = object[cuerpo].getOrbiting();

	vec3 kep_ = kepler->positionAtE(cuerpo, seconds_); // prediccion de Kepler de la posicion orbital en el tiempo especificado

	vec3 aux_v = kep_; // the orbital position
	quat aux_q = object[orbited_].getTotalRotation();

	// orbited body rotation prediction
	quat q_temp = { 0.0, 0.0, 0.0, 1.0 };
	double angle = (bullet_physics.getAngulosY(orbited_) * seconds_);
	q_temp.y = 1.0 * sin(angle / 2.0);
	q_temp.w = cos(angle / 2.0);
	aux_q = q_temp * aux_q;

	vec3 rott = vector_rotation_by_quaternion({ aux_v.x, aux_v.z, -aux_v.y }, { aux_q.x, aux_q.z, -aux_q.y, aux_q.w });

	latlon kepler_lat = cartesian_to_geodetic2(orbited_, rott);

	Vector3 pos_ = { ToDegrees_f(float(kepler_lat.lon))*1.21f,
		ToDegrees_f(float(kepler_lat.lat))*1.21f, float(kepler_lat.h) };

	return pos_;
}

void Gui_Class::create_trajectory_buffer(int total_lines, int total_orbits, int objeto, Vector3* vec_buf_a)
{
	for (int l = 0; l < total_lines / 2; l++)
	{
		vec_buf_a[l] = MapPositionPrediction(objeto, l * int(object[objeto].getOrbitalPeriod() / (total_lines / 2)*total_orbits));;
	}

	for (int l = total_lines / 2; l < total_lines - 2; l++)
	{
		vec_buf_a[l] = vec_buf_a[l - (total_lines / 2 - 1)];
	}

	for (int l = 0; l < total_lines; l += 2)
	{
		if (fabs(vec_buf_a[l].x - vec_buf_a[l + 1].x)>200 ||
			fabs(vec_buf_a[l].y - vec_buf_a[l + 1].y)>200 ||
			vec_buf_a[l].z <= 0)
		{
			vec_buf_a[l + 1] = vec_buf_a[l];
		}
	}
}

bool Gui_Class::update_minimap_texture()
{
	static int orbited = 0;
	static int previos_orbited = -1;
	static int update_num = 0;

	if (object[control].getType() == 4) orbited = object[control].getOrbiting();
	else orbited = control;

	if (orbited != previos_orbited)
	{
		update_num++;

		previos_orbited = orbited;
		window_rectangle_bitmap[8 * total_ventanas_rectangles + 1].~BitmapClass(); // liberar la memoria anterior

		if (object[orbited].getName() == "Sun") ventanas[8].setImage("textures/gui/minimaps/sun.png");
		else if (object[orbited].getName() == "Mercury") ventanas[8].setImage("textures/gui/minimaps/mercury.png");
		else if (object[orbited].getName() == "Venus") ventanas[8].setImage("textures/gui/minimaps/venus.png");
		else if (object[orbited].getName() == "Earth") ventanas[8].setImage("textures/gui/minimaps/earth.png");
		else if (object[orbited].getName() == "Mars") ventanas[8].setImage("textures/gui/minimaps/mars.png");
		else if (object[orbited].getName() == "Jupiter") ventanas[8].setImage("textures/gui/minimaps/jupiter.png");
		else if (object[orbited].getName() == "Saturn") ventanas[8].setImage("textures/gui/minimaps/saturn.png");
		else if (object[orbited].getName() == "Uranus") ventanas[8].setImage("textures/gui/minimaps/uranus.png");
		else if (object[orbited].getName() == "Neptune") ventanas[8].setImage("textures/gui/minimaps/neptune.png");
		else if (object[orbited].getName() == "Pluto") ventanas[8].setImage("textures/gui/minimaps/pluto.png");

		else if (object[orbited].getName() == "Moon") ventanas[8].setImage("textures/gui/minimaps/moon.png");

		else if (object[orbited].getName() == "Europa") ventanas[8].setImage("textures/gui/minimaps/europa.png");


		else ventanas[8].setImage("textures/white.png");

		if (!window_rectangle_bitmap[8 * total_ventanas_rectangles + 1].Initialize(screenWidth, screenHeight,
			ventanas[8].getImage(), "textures/white.png", "textures/white.png", "textures/white.png", ventanas[8].getWidth(), ventanas[8].getHeight())) return false;
	}

	return true;
}

bool Gui_Class::handle_moving_button(w_button* button_group, int n)
{
	// map pointer
	if ((n == 31 || n == 32) && object[control].getType() == 4)
	{
		// boton 31 es el green pointer
		// boton 32 es el blue pointer

		static int objeto = 0;
		static Color color_;

		if (n == 31)
		{
			objeto = control;
			color_ = color->Green;
			button_group[n].visible = true;
		}

		else if (n == 32)
		{
			objeto = object[control].getTarget();
			color_ = color->dim_color(0.7f, color->Blue);
			// se dibuja el segundo pointer solo si el target esta orbitando el mismo cuerpo que el objeto controlado
			if (object[objeto].getOrbiting() == object[control].getOrbiting()) button_group[n].visible = true;
			else button_group[n].visible = false;
		}

		if (button_group[n].visible)
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/////																															/////
			/////											Dibujo de la trayectoria de superficie											/////
			/////																															/////

			draw_surface_trajectory(button_group, n, objeto, color_);

			/////																															/////
			/////																															/////
			/////																															/////
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



			// seteamos la posicion del boton
			if (n != 31) // el boton 31 gira entonces su posicion se calcula de forma diferente
			{
				button_group[n].position_x = button_group[n].position_orig_x + ventanas[button_group[n].ventana].getPositionX()
					+ int(ToDegrees_f(object[objeto].getLongitude())*1.21);
				button_group[n].position_y = button_group[n].position_orig_y + ventanas[button_group[n].ventana].getPositionY()
					- int(ToDegrees_f(object[objeto].getLatitude())*1.21);
			}

			// dibujamos el boton
			if (n == 31) // el boton 31 gira entonces su posicion se calcula de forma diferente
			{
				if (!button_group[n].texture->Render(screenWidth / 2 - button_group[n].width / 2, screenHeight / 2 - button_group[n].heigth / 2)) return false;

				Matrix tras_;
				tras_.Translation(Vector3{
					float(button_group[n].position_orig_x) + ventanas[button_group[n].ventana].getPositionX() - screenWidth / 2
					+ int(ToDegrees_f(object[objeto].getLongitude())*1.21),

					-(float(button_group[n].position_orig_y) + ventanas[button_group[n].ventana].getPositionY() - screenHeight / 2)
					+ int(ToDegrees_f(object[objeto].getLatitude())*1.21),
					0.f });

					tras_ = giro*tras_;

					if (!m_TextureShader->Render(button_group[n].texture->GetIndexCount(),
						tras_, matriz_identidad, ortho_matrix, button_group[n].texture->GetTexture(), button_group[n].tipo_,
						button_group[n].texture->getMouseStatus(), button_group[n].color_, { 0.f, 0.f, 0.f }, button_group[n].dimmed)) return false;
			}

			else
			{
				if (!button_group[n].texture->Render(button_group[n].position_x, button_group[n].position_y)) return false;
				if (!m_TextureShader->Render(button_group[n].texture->GetIndexCount(),
					matriz_identidad, matriz_identidad, ortho_matrix, button_group[n].texture->GetTexture(), button_group[n].tipo_,
					button_group[n].texture->getMouseStatus(), button_group[n].color_, { 0.f, 0.f, 0.f }, button_group[n].dimmed)) return false;
			}
		}
	}

	else if (n == 34) // icono de media orbita
	{
		//if (object[control].getType() == 4 && object[control].getPerigee() > object[object[control].getOrbiting()].getScale() / 2 && re_entry_prediction) // altitud bajo el nivel del mar
		if (object[control].getType() == 4 && object[control].getPerigee() > (object[object[control].getOrbiting()].getScale() / 2)*object[object[control].getOrbiting()].getAtmosphereRatio()
			&& re_entry_prediction) // entrada en la atmosfera
		{
			button_group[n].visible = true;
			static Vector3 pos_ = { 0.f, 0.f, 0.f };

			// solo actualizamos la posicion del icono de media orbita si paso un segundo desde la ultima actualizacion o la
			// aceleracion del tiempo es mayor a 1, esto porque el muy pesado caluclar las predicciones de orbita de Kepler
			if (timer_update_b > 1.f || time_acceleration > 1)
			{
				timer_update_b = 0.f;
				pos_ = MapPositionPrediction(control, int(object[control].getOrbitalPeriod() / 2));
			}

			button_group[n].position_x = button_group[n].position_orig_x + ventanas[button_group[n].ventana].getPositionX() + int(pos_.x);
			button_group[n].position_y = button_group[n].position_orig_y + ventanas[button_group[n].ventana].getPositionY() - int(pos_.y);

			if (!button_group[n].texture->Render(button_group[n].position_x, button_group[n].position_y)) return false;
			if (!m_TextureShader->Render(button_group[n].texture->GetIndexCount(),
				matriz_identidad, matriz_identidad, ortho_matrix, button_group[n].texture->GetTexture(), button_group[n].tipo_,
				button_group[n].texture->getMouseStatus(), button_group[n].color_, { 0.f, 0.f, 0.f }, button_group[n].dimmed)) return false;
		}
	}

	else if (n == 35) // icono de altitud periapsis
	{
		if (object[control].getType() == 4 && object[control].getDockedTo() == -1 && re_entry_prediction)
		{
			button_group[n].visible = true;
			button_group[n].position_x = button_group[n].position_orig_x + ventanas[button_group[n].ventana].getPositionX() + int(periapsis_map_pos.x);
			button_group[n].position_y = button_group[n].position_orig_y + ventanas[button_group[n].ventana].getPositionY() - int(periapsis_map_pos.y);

			if (!button_group[n].texture->Render(button_group[n].position_x, button_group[n].position_y)) return false;
			if (!m_TextureShader->Render(button_group[n].texture->GetIndexCount(),
				matriz_identidad, matriz_identidad, ortho_matrix, button_group[n].texture->GetTexture(), button_group[n].tipo_,
				button_group[n].texture->getMouseStatus(), button_group[n].color_, { 0.f, 0.f, 0.f }, button_group[n].dimmed)) return false;
		}
	}

	return true;
}

void Gui_Class::to_screen_coords(Vector3* vec_)
{
	vec_->x = ((screenWidth / 2) * -1) + vec_->x;
	vec_->y = (screenHeight / 2) - vec_->y;
}

void Gui_Class::UpdateCartesianToKeplerianElements(int n)
{
	if (seconds_since_last_messages_update >= update_messages_interval)  // condicion para prevenir actualizaciones muy frecuentes
	{
		if (ventanas[0].getVisible() || ventanas[5].getVisible() || ventanas[8].getVisible() || n == control)
		{
			object[n].ConvertCartesianToKeplerianElements(
				object[object[n].getOrbiting()].getPosition(),
				object[object[n].getOrbiting()].getVelocity(),
				object[object[n].getOrbiting()].getMass());
		}

		int target_ = object[n].getTarget();

		if (ventanas[5].getVisible() || (ventanas[8].getVisible() && (object[target_].getOrbiting() == object[n].getOrbiting()))
			|| n == target_)
		{
			object[target_].ConvertCartesianToKeplerianElements(
				object[object[target_].getOrbiting()].getPosition(),
				object[object[target_].getOrbiting()].getVelocity(),
				object[object[target_].getOrbiting()].getMass());
		}
	}
}

void Gui_Class::UpdateCartesianToKeplerianElements_inconditional(int n) // n indica el update que tiene que efectuarse incondicionalmente
{
	if (seconds_since_last_messages_update >= update_messages_interval)  // condicion para prevenir actualizaciones muy frecuentes
	{
		object[n].ConvertCartesianToKeplerianElements(
			object[object[n].getOrbiting()].getPosition(),
			object[object[n].getOrbiting()].getVelocity(),
			object[object[n].getOrbiting()].getMass());

		int target_ = object[n].getTarget();

		object[target_].ConvertCartesianToKeplerianElements(
			object[object[target_].getOrbiting()].getPosition(),
			object[object[target_].getOrbiting()].getVelocity(),
			object[object[target_].getOrbiting()].getMass());
	}
}

void Gui_Class::update_compass(int n)
{
	int orbited_ = object[n].getOrbiting();

	quat total_rotation = object[orbited_].getTotalRotation();

	quat unrotated = total_rotation / quat{ 0.0, 0.0, 0.0, 1.0 };
	vec3 orbital_position = object[n].getPosition() - object[orbited_].getPosition();
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

	vec3 east = (vector_rotation_by_quaternion(rotated_position, ~total_rotation) - orbital_position)*universe_scale;
	vec3 west = -east;
	vec3 north = east ^ normalized(orbital_position);
	vec3 south = -north;

	horizontal_dir = getHorizontalDir(n);
	object[n].setHorizontalDir(horizontal_dir);

	//draw_loose_line({ 0, 0, 0 }, vec_to_d3d(horizontal_dir), color->Yellow);

	double degrees_east = angle_from_two_vectors(horizontal_dir, east);
	double degrees_west = angle_from_two_vectors(horizontal_dir, west);
	double degrees = angle_from_two_vectors(horizontal_dir, north);

	if (degrees_east > degrees_west)
	{
		degrees_ = 360.0 - ToDegrees_d(degrees);
		degrees *= -1;
	}

	else degrees_ = ToDegrees_d(degrees);
	giro = Mat4d_to_D3DXMATRIX(Mat4Zrotation(degrees));

}

bool Gui_Class::draw_loose_line(Vector3 origin_, Vector3 target_, Color color_)
{
	int buffer_size = 2;
	Vector3* buffer_ = new Vector3[buffer_size];

	buffer_[0] = origin_;
	buffer_[1] = target_;

	if (!draw_line_strip(buffer_, buffer_size, color_)) return false;

	SAFE_DELETE_ARRAY(buffer_);

	return true;
}

vec3 Gui_Class::getHorizontalDir(int n)
{
	vec3 hypotenuse = -GetAxisZ(object[n].getTotalRotation());
	vec3 opposite = -normalized(d3d_to_vec(object[object[n].getOrbiting()].getPosition_b()));
	opposite *= (cos(angle_from_two_vectors(hypotenuse, opposite))*length(hypotenuse));
	return hypotenuse - opposite;
}

bool Gui_Class::get_compass_dependencies_active()
{
	if (ventanas[8].getVisible()) return true;

	return false;
}

bool Gui_Class::update_priority_parameters()
{
	// cerramos la ventana del orbital information si estamos dentro de una atmosfera
	if (ventanas[0].getVisible() && object[control].getInAtmosphere())
	{
		if (!setWindowVisible(10, true)) return false; // la ventana del vuelo atmosferico
		if (!setWindowVisible(0, false)) return false; // la ventana del orbital information
	}

	// cerramos la ventana del atmospheric flight si no estamos dentro de una atmosfera
	if (ventanas[10].getVisible() && !object[control].getInAtmosphere())
	{
		if (!setWindowVisible(10, false)) return false; // la ventana del vuelo atmosferico
		if (!setWindowVisible(0, true)) return false; // la ventana del orbital information
	}

	if (get_compass_dependencies_active()) update_compass(control);

	return true;
}

bool Gui_Class::draw_surface_trajectory(w_button* button_group, int n, int objeto, Color color_)
{
	static const int total_lines = 160;
	static const int total_orbits_to_draw = 3;
	static Vector3 vec_buf_control[160];
	static Vector3 vec_buf_target[160];
	static Vector3 vec_buf_control_b[160];
	static Vector3 vec_buf_target_b[160];
	static double former_longitude_control = 0.0;
	static double former_latitude_control = 0.0;
	static double former_longitude_target = 0.0;
	static double former_latitude_target = 0.0;

	if (objeto == control && !object[objeto].getLanded())
	{
		// Como los calculos para crear la trayectoria de superficie son pesadisimos al igual que cualquier calculo
		// que utilize la preciccion de orbita de Kepler, entonces debemos actualizarlos el menor
		// numero de veces posible. Aca ponemos como condicion de actualizacion que el objeto sea movido por el usuario y
		// que haya pasado un segundo desde su ultima actualizacion, o que el objeto haya cruzado el final o el medio del mapa
		if ((object[objeto].getVelocityModified() && timer_update_a > 1.f)
			|| sign(object[objeto].getLongitude()) != sign(former_longitude_control)
			|| sign(object[objeto].getLatitude()) != sign(former_latitude_control)
			|| switched_object)
		{
			timer_update_a = 0.f;
			create_trajectory_buffer(total_lines, total_orbits_to_draw, objeto, vec_buf_control);

			// solo actualizamos la posicion del icono de periapsis
			periapsis_map_pos = MapPositionPrediction(control, int(object[control].getTime_to_perigee()));

			for (int l = 0; l < total_lines; l++)
			{
				//vec_buf_control_b[l] = { ventanas[8].getPositionX() - ventanas[8].getWidth() + vec_buf_control[l].x,
				//	-ventanas[8].getPositionY() + ventanas[8].getHeight() + vec_buf_control[l].y, vec_buf_control[l].z };

				vec_buf_control_b[l].x = (ventanas[8].getWidth() / 2 - 10) + ventanas[button_group[n].ventana].getPositionX() + vec_buf_control[l].x;
				vec_buf_control_b[l].y = (ventanas[8].getHeight() / 2 + 7) + ventanas[button_group[n].ventana].getPositionY() - vec_buf_control[l].y;
				vec_buf_control_b[l].z = vec_buf_control[l].z;
				//vec_buf_control_b[l] = d3d_to_scientific_coords_f(vec_buf_control_b[l]);
				to_screen_coords(&vec_buf_control_b[l]);

				//button_group[n].position_x = button_group[n].position_orig_x + ventanas[button_group[n].ventana].getPositionX() + int(pos_.x);
				//button_group[n].position_y = button_group[n].position_orig_y + ventanas[button_group[n].ventana].getPositionY() - int(pos_.y);
			}
		}
		former_longitude_control = object[objeto].getLongitude();
		former_latitude_control = object[objeto].getLatitude();

		// creamos el buffer a ser dibujado, actualizado con la posicion de las ventanas
		if (ventanas[8].getTrapped()) // si la ventana esta atrapada entonces mover los graficos de acuerdo a la nueva posicion
		{
			for (int l = 0; l < total_lines; l++)
			{
				vec_buf_control_b[l].x = (ventanas[8].getWidth() / 2 - 10) + ventanas[button_group[n].ventana].getPositionX() + vec_buf_control[l].x;
				vec_buf_control_b[l].y = (ventanas[8].getHeight() / 2 + 7) + ventanas[button_group[n].ventana].getPositionY() - vec_buf_control[l].y;
				vec_buf_control_b[l].z = vec_buf_control[l].z;
				to_screen_coords(&vec_buf_control_b[l]);
			}

			// dibujamos las lineas de la trayectoria de superficie
			draw_2D_line_list(vec_buf_control_b, total_lines, color_);
		}

		else draw_2D_line_list(vec_buf_control_b, total_lines, color_);
	}

	else if (objeto == object[control].getTarget() && !object[objeto].getLanded())
	{
		// como los calculos para crear la trayectoria de superficie son pesadisimos, entonces debemos actualizarlos el menor
		// numero de veces posible. Aca ponemos como condicion de actualizacion que el objeto sea movido por el usuario y
		// que haya pasado un segundo desde su ultima actualizacion, o que el objeto haya cruzado el final o el medio del mapa
		if ((object[objeto].getVelocityModified() && timer_update_a > 1.f)
			|| sign(object[objeto].getLongitude()) != sign(former_longitude_target)
			|| sign(object[objeto].getLatitude()) != sign(former_latitude_target)
			|| switched_object)
		{
			timer_update_a = 0.f;
			create_trajectory_buffer(total_lines, total_orbits_to_draw, objeto, vec_buf_target);
			for (int l = 0; l < total_lines; l++)
			{
				vec_buf_target_b[l].x = (ventanas[8].getWidth() / 2 - 10) + ventanas[button_group[n].ventana].getPositionX() + vec_buf_target[l].x;
				vec_buf_target_b[l].y = (ventanas[8].getHeight() / 2 + 7) + ventanas[button_group[n].ventana].getPositionY() - vec_buf_target[l].y;
				vec_buf_target_b[l].z = vec_buf_target[l].z;
				to_screen_coords(&vec_buf_target_b[l]);
			}

		}
		former_longitude_target = object[objeto].getLongitude();
		former_latitude_target = object[objeto].getLatitude();

		// creamos el buffer a ser dibujado, actualizado con la posicion de las ventanas
		if (ventanas[8].getTrapped()) // si la ventana esta atrapada entonces mover los graficos de acuerdo a la nueva posicion
		{
			for (int l = 0; l < total_lines; l++)
			{
				vec_buf_target_b[l].x = (ventanas[8].getWidth() / 2 - 10) + ventanas[button_group[n].ventana].getPositionX() + vec_buf_target[l].x;
				vec_buf_target_b[l].y = (ventanas[8].getHeight() / 2 + 7) + ventanas[button_group[n].ventana].getPositionY() - vec_buf_target[l].y;
				vec_buf_target_b[l].z = vec_buf_target[l].z;
				to_screen_coords(&vec_buf_target_b[l]);
			}

			// dibujamos las lineas de la trayectoria de superficie
			draw_2D_line_list(vec_buf_target_b, total_lines, color_);
		}

		else draw_2D_line_list(vec_buf_target_b, total_lines, color_);
	}

	return true;
}