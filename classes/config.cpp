#include "config.hpp"

namespace config {
	bool read() {
		if (!updater::file_good(file_path)) {
			save();
			return false;
		}

		std::ifstream f(file_path);

		json data;
		try {
			data = json::parse(f);
		}
		catch (const std::exception& e) {
			save();
		}

		if (data.empty())
			return false;

		if (data["show_box_esp"].is_boolean())
			show_box_esp = data["show_box_esp"];
		if (data["show_skeleton_esp"].is_boolean())
			show_skeleton_esp = data["show_skeleton_esp"];
		if (data["show_head_tracker"].is_boolean())
			show_head_tracker = data["show_head_tracker"];
		if (data["team_esp"].is_boolean())
			team_esp = data["team_esp"];
		if (data["automatic_update"].is_boolean())
			automatic_update = data["automatic_update"];
		if (data["render_distance"].is_number())
			render_distance = data["render_distance"];
		if (data["flag_render_distance"].is_number())
			flag_render_distance = data["flag_render_distance"];
		if (data["show_extra_flags"].is_boolean())
			show_extra_flags = data["show_extra_flags"];
		if (data["show_console"].is_boolean())
			show_console = data["show_console"];
		if (data["streamproof"].is_boolean())
			streamproof = data["streamproof"];
		if (data["show_health_bar"].is_boolean())
			show_health_bar = data["show_health_bar"];
		if (data["show_armor_bar"].is_boolean())
			show_armor_bar = data["show_armor_bar"];
		if (data["cache_refresh_rate"].is_number())
			cache_refresh_rate = data["cache_refresh_rate"];

		if (data.find("esp_box_color_team") != data.end()) {
			esp_box_color_team = RGB(
				data["esp_box_color_team"][0].get<int>(),
				data["esp_box_color_team"][1].get<int>(),
				data["esp_box_color_team"][2].get<int>()
			);
		}

		if (data.find("esp_box_color_enemy") != data.end()) {
			esp_box_color_enemy = RGB(
				data["esp_box_color_enemy"][0].get<int>(),
				data["esp_box_color_enemy"][1].get<int>(),
				data["esp_box_color_enemy"][2].get<int>()
			);
		}

		if (data.find("esp_skeleton_color_team") != data.end()) {
			esp_skeleton_color_team = RGB(
				data["esp_skeleton_color_team"][0].get<int>(),
				data["esp_skeleton_color_team"][1].get<int>(),
				data["esp_skeleton_color_team"][2].get<int>()
			);
		}

		if (data.find("esp_skeleton_color_enemy") != data.end()) {
			esp_skeleton_color_enemy = RGB(
				data["esp_skeleton_color_enemy"][0].get<int>(),
				data["esp_skeleton_color_enemy"][1].get<int>(),
				data["esp_skeleton_color_enemy"][2].get<int>()
			);
		}

		if (data.find("esp_name_color") != data.end()) {
			esp_name_color = RGB(
				data["esp_name_color"][0].get<int>(),
				data["esp_name_color"][1].get<int>(),
				data["esp_name_color"][2].get<int>()
			);
		}

		if (data.find("esp_distance_color") != data.end()) {
			esp_distance_color = RGB(
				data["esp_distance_color"][0].get<int>(),
				data["esp_distance_color"][1].get<int>(),
				data["esp_distance_color"][2].get<int>()
			);
		}

		return true;
	}

	void save() {
		json data;

		data["show_box_esp"] = show_box_esp;
		data["show_skeleton_esp"] = show_skeleton_esp;
		data["show_head_tracker"] = show_head_tracker;
		data["team_esp"] = team_esp;
		data["automatic_update"] = automatic_update;
		data["render_distance"] = render_distance;
		data["flag_render_distance"] = flag_render_distance;
		data["show_extra_flags"] = show_extra_flags;
		data["show_console"] = show_console;
		data["streamproof"] = streamproof;
		data["show_health_bar"] = show_health_bar;
		data["show_armor_bar"] = show_armor_bar;
		data["cache_refresh_rate"] = cache_refresh_rate;

		data["esp_box_color_team"] = json::array({
			GetRValue(esp_box_color_team),
			GetGValue(esp_box_color_team),
			GetBValue(esp_box_color_team)
			});

		data["esp_box_color_enemy"] = json::array({
			GetRValue(esp_box_color_enemy),
			GetGValue(esp_box_color_enemy),
			GetBValue(esp_box_color_enemy)
			});

		data["esp_skeleton_color_team"] = json::array({
			GetRValue(esp_skeleton_color_team),
			GetGValue(esp_skeleton_color_team),
			GetBValue(esp_skeleton_color_team)
			});

		data["esp_skeleton_color_enemy"] = json::array({
			GetRValue(esp_skeleton_color_enemy),
			GetGValue(esp_skeleton_color_enemy),
			GetBValue(esp_skeleton_color_enemy)
			});

		data["esp_name_color"] = json::array({
			GetRValue(esp_name_color),
			GetGValue(esp_name_color),
			GetBValue(esp_name_color)
			});

		data["esp_distance_color"] = json::array({
			GetRValue(esp_distance_color),
			GetGValue(esp_distance_color),
			GetBValue(esp_distance_color)
			});

		std::ofstream file(file_path);
		file << data.dump(4);
		file.close();
	}
}