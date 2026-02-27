#include <thread>
#include <cmath>
#include "reader.hpp"
#include "../classes/render.hpp"
#include "../classes/config.hpp"
#include "../classes/globals.hpp"

namespace hack {
	std::vector<std::pair<std::string, std::string>> boneConnections = {
						{"neck_0", "spine_1"},
						{"spine_1", "spine_2"},
						{"spine_2", "pelvis"},
						{"spine_1", "arm_upper_L"},
						{"arm_upper_L", "arm_lower_L"},
						{"arm_lower_L", "hand_L"},
						{"spine_1", "arm_upper_R"},
						{"arm_upper_R", "arm_lower_R"},
						{"arm_lower_R", "hand_R"},
						{"pelvis", "leg_upper_L"},
						{"leg_upper_L", "leg_lower_L"},
						{"leg_lower_L", "ankle_L"},
						{"pelvis", "leg_upper_R"},
						{"leg_upper_R", "leg_lower_R"},
						{"leg_lower_R", "ankle_R"}
	};

	void loop() {

		std::lock_guard<std::mutex> lock(reader_mutex);

		if (g_game.isC4Planted)
		{
			Vector3 c4Origin = g_game.c4.get_origin();
			const Vector3 c4ScreenPos = g_game.world_to_screen(&c4Origin);

			if (c4ScreenPos.z >= 0.01f) {
				float c4Distance = g_game.localOrigin.calculate_distance(c4Origin);
				float c4RoundedDistance = std::round(c4Distance / 500.f);

				float height = 10 - c4RoundedDistance;
				float width = height * 1.4f;

				render::DrawFilledBox(
					g::hdcBuffer,
					c4ScreenPos.x - (width / 2),
					c4ScreenPos.y - (height / 2),
					width,
					height,
					config::esp_box_color_enemy
				);

				render::RenderText(
					g::hdcBuffer,
					c4ScreenPos.x + (width / 2 + 5),
					c4ScreenPos.y,
					"C4",
					config::esp_name_color,
					10
				);
			}
		}

		int playerIndex = 0;
		uintptr_t list_entry;

		for (auto player = g_game.players.begin(); player < g_game.players.end(); player++) {
			const Vector3 screenPos = g_game.world_to_screen(&player->origin);
			const Vector3 screenHead = g_game.world_to_screen(&player->head);

			if (
				screenPos.z < 0.01f ||
				!utils.is_in_bounds(screenPos, g_game.game_bounds.right, g_game.game_bounds.bottom)
				)
				continue;

			const float height = screenPos.y - screenHead.y;
			const float width = height / 2.4f;

			float distance = g_game.localOrigin.calculate_distance(player->origin);
			int roundedDistance = std::round(distance / 10.f);

			// ✅ SKELETON - DESENHA PONTOS EM CADA OSSO
			if (config::show_skeleton_esp) {
				// ✅ PRIMEIRO: DESENHA CÍRCULOS NOS OSSOS
				for (const auto& bone : player->bones.bonePositions) {
					Vector3 bonePos = bone.second;  // ✅ CÓPIA NÃO-CONST
					Vector3 boneScreen = g_game.world_to_screen(&bonePos);

					if (boneScreen.z >= 0.01f) {
						// Desenha um círculo vermelho em cada osso
						render::DrawCircle(
							g::hdcBuffer,
							(int)boneScreen.x,
							(int)boneScreen.y,
							3,
							RGB(255, 0, 0)  // Vermelho brilhante
						);
					}
				}

				// ✅ DEPOIS: DESENHA LINHAS ENTRE OSSOS
				for (const auto& connection : boneConnections) {
					const std::string& boneFrom = connection.first;
					const std::string& boneTo = connection.second;

					auto it_from = player->bones.bonePositions.find(boneFrom);
					auto it_to = player->bones.bonePositions.find(boneTo);

					if (it_from != player->bones.bonePositions.end() && it_to != player->bones.bonePositions.end()) {
						Vector3 boneFromPos = it_from->second;  // ✅ CÓPIA NÃO-CONST
						Vector3 boneToPos = it_to->second;      // ✅ CÓPIA NÃO-CONST

						Vector3 boneFromScreen = g_game.world_to_screen(&boneFromPos);
						Vector3 boneToScreen = g_game.world_to_screen(&boneToPos);

						if (boneFromScreen.z >= 0.01f && boneToScreen.z >= 0.01f) {
							render::DrawLine(
								g::hdcBuffer,
								(int)boneFromScreen.x, (int)boneFromScreen.y,
								(int)boneToScreen.x, (int)boneToScreen.y,
								RGB(0, 255, 0)  // Verde brilhante
							);
						}
					}
				}
			}

			// ✅ HEAD TRACKER
			if (config::show_head_tracker) {
				auto it = player->bones.bonePositions.find("head");
				if (it != player->bones.bonePositions.end()) {
					Vector3 headPos = it->second;  // ✅ CÓPIA NÃO-CONST
					Vector3 headScreen = g_game.world_to_screen(&headPos);

					if (headScreen.z >= 0.01f) {
						render::DrawCircle(
							g::hdcBuffer,
							(int)headScreen.x,
							(int)headScreen.y - (int)(width / 12),
							(int)(width / 5),
							(g_game.localTeam == player->team ? config::esp_skeleton_color_team : config::esp_skeleton_color_enemy)
						);
					}
				}
			}

			// ✅ BOX ESP
			if (config::show_box_esp)
			{
				render::DrawBorderBox(
					g::hdcBuffer,
					screenHead.x - width / 2,
					screenHead.y,
					width,
					height,
					(g_game.localTeam == player->team ? config::esp_box_color_team : config::esp_box_color_enemy)
				);
			}

			// ✅ ARMOR BAR
			if (config::show_armor_bar) {
				render::DrawBorderBox(
					g::hdcBuffer,
					screenHead.x - (width / 2 + 10),
					screenHead.y + (height * (100 - player->armor) / 100),
					2,
					height - (height * (100 - player->armor) / 100),
					RGB(0, 185, 255)
				);
			}

			// ✅ HEALTH BAR
			if (config::show_health_bar) {
				render::DrawBorderBox(
					g::hdcBuffer,
					screenHead.x - (width / 2 + 5),
					screenHead.y + (height * (100 - player->health) / 100),
					2,
					height - (height * (100 - player->health) / 100),
					RGB(
						(255 - player->health),
						(55 + player->health * 2),
						75
					)
				);
			}

			// ✅ EXTRA FLAGS
			if (config::show_extra_flags)
			{
				if (roundedDistance > config::flag_render_distance)
					continue;

				int flagYOffset = 0;

				if (config::show_flag_name) {
					render::RenderText(
						g::hdcBuffer,
						screenHead.x + (width / 2 + 5),
						screenHead.y + flagYOffset,
						player->name.c_str(),
						config::esp_name_color,
						10
					);
					flagYOffset += 12;
				}

				if (config::show_flag_health) {
					render::RenderText(
						g::hdcBuffer,
						screenHead.x + (width / 2 + 5),
						screenHead.y + flagYOffset,
						(std::to_string(player->health) + "hp").c_str(),
						RGB(
							(255 - player->health),
							(55 + player->health * 2),
							75
						),
						10
					);
					flagYOffset += 12;
				}

				if (config::show_flag_armor) {
					render::RenderText(
						g::hdcBuffer,
						screenHead.x + (width / 2 + 5),
						screenHead.y + flagYOffset,
						(std::to_string(player->armor) + "armor").c_str(),
						RGB(
							(255 - player->armor),
							(55 + player->armor * 2),
							75
						),
						10
					);
					flagYOffset += 12;
				}

				if (config::show_flag_weapon) {
					render::RenderText(
						g::hdcBuffer,
						screenHead.x + (width / 2 + 5),
						screenHead.y + flagYOffset,
						player->weapon.c_str(),
						config::esp_distance_color,
						10
					);
					flagYOffset += 12;
				}

				if (config::show_flag_distance) {
					render::RenderText(
						g::hdcBuffer,
						screenHead.x + (width / 2 + 5),
						screenHead.y + flagYOffset,
						(std::to_string(roundedDistance) + "m away").c_str(),
						config::esp_distance_color,
						10
					);
					flagYOffset += 12;
				}

				if (config::show_flag_money) {
					render::RenderText(
						g::hdcBuffer,
						screenHead.x + (width / 2 + 5),
						screenHead.y + flagYOffset,
						("$" + std::to_string(player->money)).c_str(),
						RGB(0, 125, 0),
						10
					);
					flagYOffset += 12;
				}

				if (config::show_flag_flashed && player->flashAlpha > 100) {
					render::RenderText(
						g::hdcBuffer,
						screenHead.x + (width / 2 + 5),
						screenHead.y + flagYOffset,
						"Player is flashed",
						config::esp_distance_color,
						10
					);
					flagYOffset += 12;
				}

				if (config::show_flag_defusing && player->is_defusing) {
					const std::string defuText = "Player is defusing";
					render::RenderText(
						g::hdcBuffer,
						screenHead.x + (width / 2 + 5),
						screenHead.y + flagYOffset,
						defuText.c_str(),
						config::esp_distance_color,
						10
					);
					flagYOffset += 12;
				}
			}
		}
	}
}