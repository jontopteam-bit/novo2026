#pragma once
#include <fstream>
#include "utils.h"
#include "json.hpp"
#include "auto_updater.hpp"

using json = nlohmann::json;

namespace config {
	const std::string file_path = "config.json";

	extern bool read();
	extern void save();

	inline bool automatic_update = false;
	inline bool team_esp = false;
	inline float render_distance = -1.f;
	inline int flag_render_distance = 200;

	inline bool show_box_esp = false;
	inline bool show_skeleton_esp = false;
	inline bool show_head_tracker = false;
	inline bool show_extra_flags = false;
	inline bool show_console = false;
	inline bool streamproof = false;
	inline bool show_health_bar = false;
	inline bool show_armor_bar = false;

	// ✅ NOVAS FLAGS INDIVIDUAIS
	inline bool show_flag_name = true;
	inline bool show_flag_health = true;
	inline bool show_flag_armor = true;
	inline bool show_flag_weapon = true;
	inline bool show_flag_distance = true;
	inline bool show_flag_money = true;
	inline bool show_flag_flashed = true;
	inline bool show_flag_defusing = true;

	inline int cache_refresh_rate = 5;

	// Use COLORREF direto
	inline COLORREF esp_box_color_team = RGB(75, 175, 75);
	inline COLORREF esp_box_color_enemy = RGB(225, 75, 75);
	inline COLORREF esp_skeleton_color_team = RGB(75, 175, 75);
	inline COLORREF esp_skeleton_color_enemy = RGB(225, 75, 75);
	inline COLORREF esp_name_color = RGB(250, 250, 250);
	inline COLORREF esp_distance_color = RGB(75, 75, 175);
}