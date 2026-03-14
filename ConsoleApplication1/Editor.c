#include "Editor.h"
#include "Platform.h"
#include "Textures.h"
#include "Game.h"

WzStr wzrd_str_from_str128(str128* str)
{
	WzStr result = { .str = str->val, .len = str->len };

	return result;
}

void editor_seperator_horizontal(WzWidget parent)
{
	WzWidget w = wz_widget(parent);
	wz_widget_set_max_h(w, 2);
	wz_widget_set_border(w, WZ_BORDER_TYPE_BOTTOM_LINE);
}

void editor_seperator_vertical(WzWidget parent)
{
	WzWidget w = wz_widget(parent);
	wz_widget_set_max_w(w, 2);
}

WzWidget editor_horizontal_panel_bordered(wzrd_v2 size, WzWidget parent)
{
	WzWidget p = wz_hbox(parent);

	//wz_widget_set_size(p, size.x, size.y);

	return p;
}

Editor g_editor;

float g_width = 20, g_target_width = 255;

void editor_init()
{

}

void editor_run(WzGui* wz, PlatformTargetTexture target_texture,
	wzrd_icons icons, WzStr* debug_str, WzWidget* target_panel_out)
{
	(void)icons;
	static bool finish;
	static int selected_category;
	static str128 name;
	static unsigned int selected;
	static bool is_selected = false;

	WzWidget draw_panel;

	WzKeyboard keyboard = { 0 };

#
	WzWidget window0 = { 0 };
#if 0
		wz_begin(
		g_platform.window_width, g_platform.window_height,
		g_platform.mouse_x, g_platform.mouse_y,
		(WzState)g_platform.mouse_left,
		//*(WzKeyboardKeys*)&g_platform.keys_pressed,
		keyboard,
		true);
#endif
	WzWidget window = wz_vpanel(window0);
	wz_widget_set_max_constraints(window, g_platform.window_width, g_platform.window_height);

	wz_widget_set_cross_axis_alignment(window, WZ_CROSS_AXIS_ALIGNMENT_STRETCH);

	static bool draw_command_button;

	{
		WzWidget files_panel = wz_hpanel(window);

		bool b1 = false, b2 = false;
		wz_label_button(wz_str_create("View"), &b1, files_panel);
		wz_label_button(wz_str_create("Help"), &b2, files_panel);

		editor_seperator_horizontal(window);
		{
			WzWidget panel = wz_hpanel(window);
			wz_widget_set_margins(panel, 10);
			wz_widget_set_child_gap(panel, 15);

			bool b = false;
			WzWidget button = wz_command_button(panel, wz_str_create("Add Object"), &b);

			if (b)
			{
				g_editor.create_object_dialog_active = true;
			}

			wz_command_toggle(panel, wz_str_create("Add Polygon"), &g_game.polygon_adding_active);

			wz_toggle_icon(panel, &draw_command_button, editor_textures[EditorTextureButton]);

			//bool delete_toggle;
			//wzrd_button_icon(icons.delete, &delete_toggle, buttons_panel);
			//g_game.delete = delete_toggle;

			// seperator

			bool is_dragged = wzrd_box_is_dragged(&(WzWidgetData) { 0 });
			if (!is_dragged && !finish)
			{
				/*	wzrd_widget((wzrd_style) {
						.style = wzrd_style_create((wzrd_style_template) {
							.w = 25, .h = 25
						}), .is_draggable = true,
					}, panel);*/
			}

			//static unsigned int color = 0x646464ff;
			//wzrd_widget((wzrd_style) {
			//	.is_slot = true,
			//		.style = wzrd_style_create((wzrd_style_template) {
			//		.color = color, .w = 25, .h = 25,
			//	})
			//}, panel);

			//if (wzrd_box_is_hot(wzrd_box_get_last()) && wzrd_is_releasing())
			//{
			//	color = EGUI_LIME;
			//	finish = true;
			//}

			//editor_seperator_vertical(panel);

			//wzrd_toggle_icon(icons.play, &g_game.run, &panel);

			//editor_seperator_vertical(panel);
		}

		WzWidget bottom_panel = wz_hpanel(window);
		wz_widget_set_expanded(bottom_panel);
		wz_widget_set_cross_axis_alignment(bottom_panel, WZ_CROSS_AXIS_ALIGNMENT_STRETCH);

		//editor_left_panel(bottom_panel);
		{
			WzWidget outer_panel = wz_vpanel(bottom_panel);
			//wz_widget_set_expanded(outer_panel);

			//wzrd_str texts[] = { wzrd_str_create("Entities"), wzrd_str_create("Textures"), wzrd_str_create("Actions") };
			//static bool active;
			//wzrd_dropdown(&selected_category, texts, 3, 100, &active);

			WzWidget inner_panel = wz_vpanel(outer_panel);
			wz_widget_set_expanded(inner_panel);

			if (selected_category == 0) {

				static WzStr labels[MAX_NUM_ENTITIES] = { 0 };

				for (unsigned int i = 0; i < g_game.sorted_entities_count; ++i)
				{
					Entity_handle handle = g_game.sorted_entities[i];
					Entity* entity = game_entity_get(handle);
					labels[i] = wz_str_create(entity->name.val);
				}

				if (g_game.selected_entity_index_to_sorted_entities >= 0)
				{
					// TODO: if I remove the if code crashes
					if (g_game.delete)
					{
						game_entity_remove(g_game.sorted_entities[g_game.selected_entity_index_to_sorted_entities]);
					}
				}

				unsigned ids[3];

				wz_label_list_sorted(labels, g_game.sorted_entities_count,
					(int*)g_game.sorted_entities, ids,
					50, 70, 0XC8C8C8FF,
					&g_game.selected_entity_index_to_sorted_entities,
					&g_game.is_entity_selected, inner_panel);
			}
		}


		WzWidget panel = wz_hpanel(bottom_panel);
		wz_widget_set_expanded(panel);
		wz_widget_set_cross_axis_alignment(panel, WZ_CROSS_AXIS_ALIGNMENT_STRETCH);

		draw_panel = wz_panel(panel);
		wz_widget_set_expanded(draw_panel);
		wz_widget_set_cross_axis_alignment(draw_panel, WZ_CROSS_AXIS_ALIGNMENT_STRETCH);
		wz_widget_clip(draw_panel);

		wz_frame(draw_panel, 100, 100, wz_str_create("wow"));

#if 0
		// Game panel
		WzWidget target_panel = editor_hpanel(panel);
		//wz_widget_set_expanded(target_panel);
		wz_widget_set_size(target_panel, GAME_WIDTH, GAME_HEIGHT);
		wz_widget_get(target_panel)->disable_input = true;
		*target_panel_out = target_panel;

		wz_widget_add_item(target_panel, (WzWidgetItem) {
			.type = ItemType_Texture,
				.val = { .texture = *(WzTexture*)&target_texture },
				.scissor = true
		});
#endif


		if (g_editor.create_object_dialog_active)
		{
			{
				wzrd_v2 size = { 500, 500 };
				bool ok = false, cancel = false;

				WzWidget dialog = wz_dialog(&g_editor.create_object_dialog_pos.x, &g_editor.create_object_dialog_pos.y,
					&size.x, &size.y, &g_editor.create_object_dialog_active, wz_str_create("add object"), 4, window);

				bool* active = &g_editor.create_object_dialog_active;
				if (1)
				{
					WzWidget panel = wz_hpanel(dialog);
					wz_widget_set_expanded(panel);

					WzStr labels[] = { wz_str_create("Entity"), wz_str_create("Texture") };
					unsigned items[2];
					unsigned ids[2];
#if 0
					wzrd_label_list(labels, 2, items, ids,
						100, 100,
						0xFFFFFFFF,
						0, &selected, &is_selected, panel);
#endif
					WzWidget form = wz_vpanel(panel);
					wz_widget_set_expanded(form);
					wz_widget_set_cross_axis_alignment(form, WZ_CROSS_AXIS_ALIGNMENT_STRETCH);
					WzWidget row = wz_hpanel(form);
					wz_label(row, wz_str_create("Name:"));
					//WzWidget wdg = wz_input_box(name.val, &name.len, 10, row);

					wz_widget_set_border(row, WZ_BORDER_TYPE_DEFAULT);
					{
						WzWidget buttons_panel = wz_hpanel(dialog);
						{
							bool b1;

							wz_command_button(buttons_panel, wz_str_create("OK"), &b1);
							if (b1) {
								ok = true;
							}

							bool b2;
							wz_command_button(buttons_panel, wz_str_create("Cancel"), &b2);
							if (b2) {
								cancel = true;
							}
						}
					}
				}

				if (ok) {
					if (selected == 1) {
						Texture texture = texture_get_by_name(name);
						game_texture_add(texture);
					}
					else if (selected == 0)
					{
						game_entity_create((Entity) {
							.name = str128_create(name.val),
								.texture = game_texture_get_handle_by_name(str128_create("player")),
								.rect = { .x = 0, .y = 0, .w = 100, .h = 100 },
								.color = 0xffffffff
						});
					}

					*active = false;
				}

				if (cancel) {
					*active = false;
				}
			}

		}
	}

	wz_do_layout_refactor_me(1, MAX_NUM_WIDGETS - 1);

	if (wz_widget_is_activating(draw_panel))
	{
		WzWidgetData* draw_panel_data = wz_widget_get(draw_panel);
		g_editor.drawing_widget_x = wz->mouse_pos.x;
		g_editor.drawing_widget_y = wz->mouse_pos.y;
	}

	unsigned w = wz->mouse_pos.x - g_editor.drawing_widget_x;
	unsigned h = wz->mouse_pos.y - g_editor.drawing_widget_y;

	// Drawing persistent widgets
	if (wz->persistent_widgets_count)
	{
		static WzWidget selected_widget;

		for (unsigned i = 0; i < wz->persistent_widgets_count; ++i)
		{
			WzWidget widget = wz->persistent_widgets[i].handle;
			wz->widgets[widget.handle] = wz->persistent_widgets[i];
			wz_widget_add_child(draw_panel, widget);

			WzWidgetData* widget_data = wz_widget_get(widget);

			if (wz_widget_is_activating(widget))
			{
				selected_widget = widget;
			}

			if (wz_widget_is_interacting(widget))
			{
				wz_widget_data_set_pos(&wz->persistent_widgets[i],
					widget_data->x + wz->mouse_delta.x,
					widget_data->y + wz->mouse_delta.y);
			}

			// Add blue buttons
			if (wz_widget_is_equal(widget, selected_widget))
			{
				WzWidget b = wz_widget(widget);
				wz_widget_set_constraints(b, 0, 0, 50, 50);
				wz_widget_set_pos(b, -50, -50);
				wz_widget_set_color(b, 0x0000FFFF);

				if (wz_widget_is_active(b))
				{
#if 0 
					wz_widget_data_set_tight_constraints(&wz->persistent_widgets[i],
						widget_data->constraint_max_w - wz->mouse_delta.x,
						widget_data->constraint_max_h - wz->mouse_delta.y);
					wz_widget_data_set_pos(&wz->persistent_widgets[i],
						widget_data->x + wz->mouse_delta.x,
						widget_data->y + wz->mouse_delta.y);
#endif
				}
			}

			if (widget_data->type == WZ_WIDGET_TYPE_COMMAND_BUTTON)
			{
				bool released = false;
				wz_command_button_run(widget, &released);
			}

		}
	}

	// User creating widgets
	{
		if (wz_widget_is_interacting(draw_panel) || wz_widget_is_deactivating(draw_panel))
		{
			if (draw_command_button)
			{
				WzWidgetData* draw_panel_data = wz_widget_get(draw_panel);
				bool b;
				WzWidget widg = wz_command_button(draw_panel, wz_str_create("wow"), &b);
				wz_widget_set_size(widg, w, h);
				wz_widget_set_x(widg, g_editor.drawing_widget_x - draw_panel_data->actual_x);
				wz_widget_set_y(widg, g_editor.drawing_widget_y - draw_panel_data->actual_y);

				if (wz_widget_is_deactivating(draw_panel))
				{
					wz->persistent_widgets[wz->persistent_widgets_count++] = *wz_widget_get(widg);
					draw_command_button = false;
				}
			}
		}
	}

	wz_do_layout_refactor_me(1, MAX_NUM_WIDGETS - 1);

	if (0)
	{
		// Draw panel dots
		WzWidgetData* draw_panel_data = wz_widget_get(draw_panel);
		unsigned stride_x = draw_panel_data->actual_w / 5;
		unsigned stride_y = draw_panel_data->actual_h / 5;

		for (unsigned i = 0; i < draw_panel_data->actual_w / 2; ++i)
		{
			for (unsigned j = 0; j < draw_panel_data->actual_h / 2; ++j)
			{
				wz_widget_add_rect_absolute(draw_panel_data->handle,
					draw_panel_data->actual_x + i * stride_x,
					draw_panel_data->actual_y + j * stride_y,
					10, 10, 0x000000ff);
			}
		}
	}

	wz_end(debug_str);
}

char multi_line_text[1024 * 10];

bool IsNumber(const char* str) {
	if (!str || str[0] == '\0')
		return false;
	size_t i = 0;
	if (str[0] == '-') i = 1;
	for (; i < strlen(str); ++i) {
		if (str[i] > '9' || str[i] < '0') return false;
	}

	return true;
}