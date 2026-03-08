#include "Game.h"
#include "Editor.h"

typedef enum game_editor_layer
{
	game_editor_layer_default,
	game_editor_layer_handle,
} game_editor_layer;

Game g_game;

wzrd_v2 TextGetSize(const char* str) {
	PlatformV2i result = PlatformTextGetSize(str);
	return *(wzrd_v2*)&result;
}

wzrd_v2 TextGetSizeFromLength(int len) {
	assert(len < 128);
	str128 str = { 0 };
	for (int i = 0; i < len; ++i)
		str.val[i] = 'a';
	PlatformV2i result = PlatformTextGetSize(str.val);
	return *(wzrd_v2*)&result;
}

WzRect EguiRectScale(WzRect rect, int scale) {
	int new_w = rect.w * scale;
	int new_h = rect.h * scale;

	WzRect result = {
		rect.x + (rect.w / 2 - new_w / 2),
		rect.y + (rect.h / 2 - new_h / 2),
		new_w,
		new_h
	};

	return result;
}

bool f;


#if 0
void nodes() {
	if (nodes[i].depth < minimized_depth)
		disable_next_depth = false;

	if (disable_next_depth)
		continue;

	EguiBoxBegin((Box) {
		.row_mode = true,
			.h = 20,

			.color = EGUI_WHITE,
			.border_type = BorderType_None,
	});
	{
		if (nodes[i].depth > 0) {
			// Space
			if (nodes[i].depth > 1)
				EguiBox((Box) { .border_type = BorderType_None, .w = EDITOR_ICON_SIZE * (nodes[i].depth - 1), .h = EDITOR_ICON_SIZE });

			// Lines and button
			EguiBoxBegin((Box) {
				.border_type = BorderType_None, .w = EDITOR_ICON_SIZE, .h = EDITOR_ICON_SIZE + 2
			});
			{
				EguiItemAdd((Item) { .type = ItemType_TopVerticalDottedLine });

				EguiItemAdd((Item) { .type = ItemType_RightHorizontalDottedLine });

				// Button
				if (nodes[i].expandable) {
					EguiRect new_rect = EguiRectScale((EguiRect) { 0, 0, EguiBoxGetCurrent()->w, EguiBoxGetCurrent()->h }, 0.5);
					bool* is_toggled = EguiToggleBegin((Box) {
						.border_type = BorderType_None, .color = EGUI_WHITE,
							.x = new_rect.x, .y = new_rect.y, .w = new_rect.w, .h = new_rect.h
					});
					{
						EguiItemAdd((Item) {
							.type = ItemType_Rect,
								.rect = (EguiRect){ 0, 0, new_rect.w, 1 },
								.color = EGUI_GRAY
						});
						EguiItemAdd((Item) {
							.type = ItemType_Rect,
								.rect = (EguiRect){ 0 + new_rect.w - 1, 0, 1, new_rect.h },
								.color = EGUI_GRAY
						});
						EguiItemAdd((Item) {
							.type = ItemType_Rect,
								.rect = (EguiRect){ 0, 0 + new_rect.h - 1, new_rect.w, 1
							},
								.color = EGUI_GRAY
						});
						EguiItemAdd((Item) {
							.type = ItemType_Rect,
								.rect = (EguiRect){ 0, 0, 1, new_rect.h },
								.color = EGUI_GRAY
						});
						EguiItemAdd((Item) {
							.type = ItemType_Rect,
								.rect = (EguiRect){ 0 + new_rect.w / 2, 0 + 3, 1, new_rect.h - 6 },
								.color = EGUI_BLACK
						});
						EguiItemAdd((Item) {
							.type = ItemType_Rect,
								.rect = (EguiRect){ 0 + 3, 0 + new_rect.w / 2, new_rect.w - 6, 1 },
								.color = EGUI_BLACK
						});
					}
					EguiToggleEnd();

					if (*is_toggled) {
						disable_next_depth = true;
					}
				}
			}
			EguiBoxEnd();
		}

		// Icon
		char buff[64] = { 0 };
		sprintf(buff, "icon %d", i);
		EguiBoxBegin((Box) {
			.border_type = BorderType_None,
				.color = EGUI_WHITE,
				.w = 15,
				.h = 15,
				.name = Str128Create(buff)
		});
		{
			if (nodes[i].depth)
				EguiItemAdd((Item) { .type = ItemType_LeftHorizontalDottedLine });

			EguiItemAdd((Item) {
				.type = ItemType_Texture,
					.size = (EguiV2){ 30, 30 },
					.texture = *(EguiTexture*)&g_texture
			});
		}
		EguiBoxEnd();

		// Name
		if (EguiLabelButton(nodes[i].str)) {
			result = nodes[i].str;
		}
	}
	EguiBoxEnd();

	int f = 5;
	f++;
}
#endif

wzrd_v2 EguiV2iAdd(wzrd_v2 a, wzrd_v2 b) {
	wzrd_v2 result = (wzrd_v2){ a.x + b.x, a.y + b.y };

	return result;
}




Texture_handle game_texture_add(Texture texture) {
	Handle handle = handle_create(&g_game.textures_handle_map);
	int index = handle_get(&g_game.textures_handle_map, handle);
	g_game.textures_widget_a[index] = texture;

	Texture_handle result = (Texture_handle){ .val = handle };

	return result;
}

void game_texture_remove_by_index(int index) {
	handle_remove_by_index(&g_game.textures_handle_map, index);
}

void game_texture_remove(Texture_handle handle) {
	handle_remove(&g_game.textures_handle_map, handle.val);
}

Texture* game_texture_get_next(int* iterator_index) {
	Handle handle = { 0 };
	Texture* result = 0;
	if (handle_get_next(&g_game.textures_handle_map, iterator_index, &handle)) {
		result = &g_game.textures_widget_a[handle.index];
	}

	return result;
}

bool game_texture_get_next_handle(int* iterator_index, Texture_handle* handle) {
	bool result = handle_get_next(&g_game.textures_handle_map, iterator_index, (Handle*)handle);

	return result;
}

Texture_handle game_texture_get_handle_by_name(str128 str)
{
	int it = 1;
	Texture_handle texture_handle = { 0 };
	bool found = true;
	while ((game_texture_get_next_handle(&it, &texture_handle)))
	{
		str128 name = game_texture_get(texture_handle)->name;
		if (str128_equal(name, str))
		{
			found = true;
			break;
		}
	}

	assert(found);

	return texture_handle;
}

Texture* game_texture_get_by_name(str128 str)
{
	int it = 1;
	Texture* texture = 0;
	while ((texture = game_texture_get_next(&it)))
	{
		if (str128_equal(texture->name, str))
		{
			break;
		}
	}

	assert(texture);
	return texture;
}

Texture* game_texture_get(Texture_handle handle) {
	int index = handle_get(&g_game.textures_handle_map, handle.val);
	Texture* result = 0;

	if (index)
	{
		result = g_game.textures_widget_a + index;
	}

	return result;
}


int compare_entities_by_rendering_order(const void* a, const void* b)
{
	Entity* entity_a = &g_game.entities[*(int*)a];
	Entity* entity_b = &g_game.entities[*(int*)b];

	if (entity_a->rendering_order > entity_b->rendering_order)
	{
		return 1;
	}
	else if (entity_a->rendering_order < entity_b->rendering_order)
	{
		return -1;
	}

	return 0;
}

void game_sort_entities_by_rendering_order()
{
	int it = 1;
	Entity* entity = 0;
	g_game.sorted_entities_count = 0;
	memset(g_game.sorted_entities, 0, sizeof(*g_game.sorted_entities) * g_game.sorted_entities_count);
	while ((entity = entity_get_next(&it)))
	{
		g_game.sorted_entities[g_game.sorted_entities_count++] = entity->handle;
	}

	qsort(g_game.sorted_entities, g_game.sorted_entities_count, sizeof(int), compare_entities_by_rendering_order);
}

Entity_handle game_entity_create(Entity entity) {
	entity.rendering_order = g_game.entities_handle_map.count;
	Handle handle = handle_create(&g_game.entities_handle_map);
	int index = handle_get(&g_game.entities_handle_map, handle);

	Entity_handle result = (Entity_handle){ .val = handle };
	entity.handle = result;
	g_game.entities[index] = entity;
	game_sort_entities_by_rendering_order();

	return result;
}

void game_entity_remove_by_index(int index) {
	handle_remove_by_index(&g_game.entities_handle_map, index);
	game_sort_entities_by_rendering_order();
}

void game_entity_remove(Entity_handle handle) {
	handle_remove(&g_game.entities_handle_map, handle.val);
	game_sort_entities_by_rendering_order();
}

Entity* game_entity_get(Entity_handle handle) {
	int index = handle_get(&g_game.entities_handle_map, handle.val);

	Entity* result = g_game.entities + index;

	return result;
}

/*
 * ===============================================================
 *
 *                          S_MISC
 *
 * ===============================================================
 */



v2i v2i_sub(v2i v0, v2i v1) {
	v2i result = { v0.x - v1.x, v0.y - v1.y };

	return result;
}

v2i v2i_add(v2i v0, v2i v1) {
	v2i result = { v0.x + v1.x, v0.y + v1.y };

	return result;
}

v2i v2_sub(v2i v0, v2i v1) {
	v2i result = { v0.x - v1.x, v0.y - v1.y };

	return result;
}

v2i v2_add(v2i v0, v2i v1) {
	v2i result = { v0.x + v1.x, v0.y + v1.y };

	return result;
}

v2i v2_normalize(v2i v) {
	float size = (float)sqrt(pow((v.x), 2) + pow((v.y), 2));
	v2i result = { v.x / size, v.y / size };

	return result;
}

v2i v2_lerp(v2i pos, v2i end_pos) {
	float lerp_amount = 0.05f;
	float delta = 5;
	if (fabs(end_pos.x - pos.x) > delta) {
		pos.x = pos.x + lerp_amount * (end_pos.x - pos.x);
	}

	if (fabs(end_pos.y - pos.y) > delta) {
		pos.y = pos.y + lerp_amount * (end_pos.y - pos.y);
	}

	return pos;
}

int GetSign(int n) {
	if (n < 0) return -1;
	else if (n > 0) return 1;
	return 0;
}

#define MAX_NUM_VERTICES_IN_POLYGON 32

typedef struct Polygon {
	v2i vertices[MAX_NUM_VERTICES_IN_POLYGON];
	int count;
} Polygon;

bool v2_is_inside_polygon(v2i point, Polygon polygon) {

	bool inside = false;
	int i, j;

	for (i = 0, j = polygon.count - 1; i < polygon.count; j = i++) {
		if (((polygon.vertices[i].y > point.y) != (polygon.vertices[j].y > point.y)) &&
			(point.x < (polygon.vertices[j].x - polygon.vertices[i].x) * (point.y - polygon.vertices[i].y) / (polygon.vertices[j].y - polygon.vertices[i].y) + polygon.vertices[i].x))
			inside = !inside;
	}

	return inside;
}


bool v2_is_inside_rect(v2i point, Rect rect) {
	bool result = false;

	if (point.x >= rect.x && point.y >= rect.y && point.x < rect.x + rect.w && point.y < rect.y + rect.h) {
		result = true;
	}

	return result;
}

/*
 * ===============================================================
 *
 *                          Section_Entities
 *
 * ===============================================================
 */


EntityId GetAvailableId() {
#if 0
	if (!entity_system.num_available_handles) {
		Log(__LINE__, "No available entity handles");

		return (EntityId) { 0 };
	}

	EntityId result = entity_system.available_handles[entity_system.num_available_handles - 1];
	entity_system.num_available_handles--;

	return result;
#else
	return (EntityId) { 0 };
#endif
}

void DestroyId(EntityId id) {
	assert(g_entity_system.num_available_handles < MAX_NUM_ENTITIES);
	g_entity_system.available_handles[g_entity_system.num_available_handles++] = (EntityId){ .index = id.index, .generation = id.generation + 1 };
}

#define MAX_NUM_STACK 64

typedef struct UndoSystem {
	EntitySystem entity_system_stack[MAX_NUM_STACK];
	size_t count;
	size_t index;
} UndoSystem;

UndoSystem undo_system;

void UndoSystemDo() {
	undo_system.entity_system_stack[undo_system.count] = g_entity_system;
	undo_system.index = undo_system.count;
	undo_system.count = (undo_system.count + 1) % MAX_NUM_STACK;
}

EntitySystem UndoSystemRedo() {
	if (undo_system.index == undo_system.count) return undo_system.entity_system_stack[undo_system.count];
	undo_system.index = (undo_system.index) % MAX_NUM_STACK;
	return undo_system.entity_system_stack[undo_system.index];
}

EntitySystem UndoSystemUndo() {
	//if (undo_system.index == undo_system.num_stack) return undo_system.entity_system_stack[undo_system.num_stack];
	EntitySystem result = undo_system.entity_system_stack[undo_system.index];

	if (undo_system.index) undo_system.index--;

	return result;
}

Entity* GetEntity(EntityId id) {

	Entity* entity = &g_entity_system.entities[id.index];

	return entity;
}


int CompareEntities(const void* element1, const void* element2) {
	EntityId* id1 = (EntityId*)element1;
	EntityId* id2 = (EntityId*)element2;
	Entity* e1 = GetEntity(*id1);
	Entity* e2 = GetEntity(*id2);

	if (e1->type > e2->type) return 1;
	if (e1->type < e2->type) return -1;
	return 0;
}

EntityId* GetOrderedEntities() {
	EntityId* ordered_entities = 0;
#if 0
	for (int i = 1; i < MAX_NUM_ENTITIES; ++i) {
		if (entity_system.entities[i].type != EntityType_None)
			arrput(ordered_entities, entity_system.entities[i].id);
	}

	int _x = arrlen(ordered_entities);
	qsort(ordered_entities, arrlen(ordered_entities), sizeof(EntityId), CompareEntities);
#endif
	return ordered_entities;
}

void DestroyEntityByName(const char* str) {
	//TODO: delete from entity map too!
	for (int i = 0; i < MAX_NUM_ENTITIES; ++i) {
		if (strcmp(str, g_entity_system.entities[i].name.val) == 0) {
			//DestroyEntity(g_entity_system.entities[i].id);
		}
	}
}

Entity* GetEntityByName(char* name) {
	(void)name;
	//int index = GetHashTableElement(entity_system.entities_map, name);

	//if (!index) {
		//OldLog("LOG: Could not find entity \"%s\"\n", name);
	//}

	//Entity* result = &entity_system.entities[index];

	//return result;

	return 0;
}

void ResetEntitySystem() {
	//entity_system.num_entities = 0;
	g_entity_system.entities_map.num = 0;
	//Entity nop = { 0 };
	g_grab_system.num_hovered_entities = 0;
}

bool IsRectHovered(Rect r) {
	v2i mouse_pos = (v2i){ g_platform.mouse_x, g_platform.mouse_y };

	if (mouse_pos.x < r.x + r.w &&
		mouse_pos.x > r.x &&
		mouse_pos.y < r.y + r.h &&
		mouse_pos.y > r.y) {
		return true;
	}

	return false;
}

bool IsRectClicked(Rect r) {
	return IsRectHovered(r) && g_platform.mouse_left == Deactivating;
}

PixelPos GetEntityPixelPos(EntityId id) {

	return  GetEntity(id)->pixel_pos;
}

PixelPos MovePos(PixelPos current_pos, PixelPos dest) {
	//TODO: warning! broken code!
	// Multiply by delta_time
	float speed = 1;

	v2i distance = v2_sub((v2i) { (float)dest.pos.x, (float)dest.pos.y }, (v2i) { (float)current_pos.pos.x, (float)current_pos.pos.y });
	v2i direction = v2_normalize(distance);

	current_pos.pos.x += (int)(direction.x * speed);
	current_pos.pos.y += (int)(direction.y * speed);

	//assert(current_pos.pos.x < GAME_SCREEN_WIDTH);
	//assert(current_pos.pos.y < GAME_SCREEN_HEIGHT);

	return current_pos;
}

bool IsEntityNone(EntityId id) {
	Entity* e = GetEntity(id);
	return e->type == EntityType_None;
}
#pragma optimize( "", off )

EntityId CreateEntityOriginal(Entity entity) {
	if (!entity.name.len) return (EntityId) { 0 };
#if 0
	entity.id = (EntityId){ .index = entity_system.num_entities };
	strcpy(entity.id.name, entity.name);

	assert(entity_system.num_entities < MAX_NUM_ENTITIES - 1);
	entity_system.entities[entity_system.num_entities++] = entity;

	Entity* e = entity_system.entities + (entity_system.num_entities - 1);
	AddToHashTable(&entity_system.entities_map, e->name, entity_system.num_entities - 1);

	Log("LOG: Added the entity \"%s\"\n", entity.name);
#else

#endif
	//return entity.id;
	return (EntityId) { 0 };
}

Entity CreateEntityRaw(char* name, EntityType type, PixelPos pixel_pos, v2i size, Texture sprite) {
	(void)sprite;
	//TODO: input santiizing
	Entity entity = {
		.type = type,
		.pixel_pos = pixel_pos,
		.size = size,
		//.sprite = sprite,
		.name = str128_create(name)
	};

	return entity;
}

#define WZRD_UNUSED(x) (void)(x)

EntityId CreateEntity(const char* name, EntityType type, v2i pixel_pos, v2i size, Texture sprite) {
	WZRD_UNUSED(name);
	WZRD_UNUSED(type);
	WZRD_UNUSED(pixel_pos);
	WZRD_UNUSED(size);
	WZRD_UNUSED(sprite);

#if 0
	Entity entity = CreateEntityRaw(name, type, pixel_pos, size, sprite);

	entity.id = GetAvailableId();

	if (!entity.id.index) {
		Log(__LINE__, "Failed to add the entity \"%s\"", entity.name);
		return entity.id;
	}

	//strcpy(entity.id.name, name);

	entity_system.entities[entity.id.index] = entity;

	AddToHashTable(&entity_system.entities_map, name, entity.id.index);

	Log(__LINE__, "Added the entity \"%s\"", entity.name);

	return entity.id;
#else
	return (EntityId) { 0 };
#endif
}

v2i ScreenPosToCenterPos(v2i pos, v2i size) {
	v2i result = { pos.x + size.x / 2, pos.y + size.y / 2 };

	return result;
}

bool IsIdEqual(EntityId id0, EntityId id1) {
	return id0.index == id1.index;
}

EntityId CreateCharacter(const char* name, v2i pos, v2i size) {

	EntityId id = CreateEntity(name, EntityType_Character, (v2i) { (int)pos.x, (int)pos.y }, (v2i) { size.x, size.y }, texture_get_by_name(str128_create(name)));

	return id;
}

Entity* entity_get_next(int* iterator_index) {
	Handle handle = { 0 };
	Entity* result = 0;
	if (handle_get_next(&g_game.entities_handle_map, iterator_index, &handle)) {
		result = &g_game.entities[handle.index];
	}

	return result;
}

bool entity_get_next_handle(int* iterator_index, Entity_handle* handle) {
	bool result = handle_get_next(&g_game.entities_handle_map, iterator_index, (Handle*)handle);

	return result;
}

platform_color wzrd_color_to_platform_color(unsigned color) {

	platform_color result = *(platform_color*)&color;

	return result;
}

typedef struct array_list_node
{
	int index;
	int next_node;
} array_list_node;

void game_polygon_gui_do(wzrd_v2 mouse_pos, WzRect window,
	unsigned int scale_w, unsigned int scale_h, WzWidget parent)
{
	static v2i nodes[32];
	static ArrayList32 list;

	if (g_game.polygon_adding_active)
	{
		if (g_platform.mouse_left == Deactivating)
		{
			if (mouse_pos.x >= 0 && mouse_pos.y >= 0 && mouse_pos.x < window.w && mouse_pos.y < window.h)
			{
				int index = array_list_32_add(&list);
				nodes[index] = (v2i){ (float)mouse_pos.x, (float)mouse_pos.y };
			}
		}
	}

	int it = 1;
	int index = 0;
	while ((index = array_list_32_get_next(&list, &it)))
	{
		if (list.count > 1)
		{
			int next_index = list.nodes[index].next_node;
			float x0 = nodes[index].x;
			float y0 = nodes[index].y;
			float x1 = nodes[next_index].x;
			float y1 = nodes[next_index].y;

			PlatformLineDraw(x0, y0, x1, y1, 255, 0, 0);
		}

		str128 name = str128_create("wow %d", index);
		name.len = (int)strlen(name.val);
		WzRect rect = { (int)nodes[index].x, (int)nodes[index].y, 8, 8 };
		bool active = false;
		WzStr str = (WzStr){
			.str = name.val, .len = name.len
		};

		WzWidget w = wzrd_handle_button(&active, rect, WZ_RED, str, parent);
		wz_widget_set_layer(w, game_editor_layer_handle);
		wz_widget_add_tag(w, &nodes[index]);

		if (active)
		{
			nodes[index].x += g_platform.mouse_delta_x / scale_w;
			nodes[index].y += g_platform.mouse_delta_y / scale_h;
		}

#if 0
		if (1)
		{
			//wzrd_box_get_last()->color = (wzrd_color){ 0, 0, 255, 255 };

			if (g_game.delete)
			{
				array_list_32_delete(&list, index);
			}
		}
		else if (wzrd_box_is_active(wzrd_box_get_last()))
		{
			//wzrd_box_get_last()->color = (wzrd_color){ 0, 0, 255, 255 };
		}
		else {
			//wzrd_box_get_last()->color = (wzrd_color){ 255, 0, 0, 255 };
		}
#endif
	}
}

bool game_entity_is_equal(Entity_handle a, Entity_handle b)
{
	if (handle_is_equal(a.val, b.val))
	{
		return true;
	}

	return false;
}

void game_draw_screen_dots()
{
	int size = 16;
	int rects_in_row_count = g_game.target_texture.data->w / size;
	int rects_in_column_count = g_game.target_texture.data->h / size;
	float x = 0, y = 0;
	bool flip = false;
	for (int i = 1; i < rects_in_column_count; ++i) {
		for (int j = 1; j < rects_in_row_count; ++j) {

			platform_rect_draw((PlatformRect) { x, y, 2, 2 }, PLATFORM_GRAY);

			x += size;
		}

		x = 0;
		y += size;
		flip = !flip;
	}
}

void game_entity_gui_do(unsigned int scale_w, unsigned int scale_h, WzGui* wz, WzWidgetData* background_box)
{
	(void)wz;
	WzWidget selected_entity_handle = { 0 };
#if 1

	for (unsigned int i = 0; i < g_game.sorted_entities_count; ++i)
	{
		Entity* entity = game_entity_get(g_game.sorted_entities[i]);
		// Entity gui rect
		WzWidget handle = { 0 };
		{
#if		1
			handle = wz_widget(background_box->handle);
			wz_widget_set_max_w(handle, entity->rect.w);
			wz_widget_set_max_h(handle, entity->rect.h);
			wz_widget_set_x(handle, entity->rect.x);
			wz_widget_set_y(handle, entity->rect.y);
			wz_widget_set_color(handle, 0);
			wz_widget_add_tag(handle, entity);
#endif
			if (wz_widget_is_active(handle))
			{
				selected_entity_handle = handle;
				g_game.selected_entity_index_to_sorted_entities = i;
				g_game.is_entity_selected = true;
			}
			else if (g_game.is_entity_selected && g_game.selected_entity_index_to_sorted_entities == i)
			{
				selected_entity_handle = handle;
			}
		}
		// Entity dragging and scaling gui
		if (wz_widget_is_equal(selected_entity_handle, handle))
		{
			str128 button_name = str128_create("blue button %s", entity->name);
			WzRect rect = (WzRect){ (int)entity->rect.x + (int)entity->rect.w / 2 - 5, (int)entity->rect.y + (int)entity->rect.h / 2 - 5, 10, 10 };
			bool blue_button = false;
			WzStr str = (WzStr){
				.str = button_name.val, .len = button_name.len,
			};
			WzWidget blue_handle = wzrd_handle_button(&blue_button, rect, WZ_BLUE, str, background_box->handle);

			wz_widget_set_layer(blue_handle, game_editor_layer_handle);

			// Late logic
			static float offset_x, offset_y;
			if (blue_button)
			{
				if (entity) {
					offset_x += g_game.mouse_delta.x / (float)scale_h;
					offset_y += g_game.mouse_delta.y / (float)scale_h;

					printf("%f %f\n", offset_x, offset_y);

					if (g_game.mouse_delta.x)
					{
						entity->rect.x += g_game.mouse_delta.x / (float)scale_h;
					}

					if (g_game.mouse_delta.y)
					{
						entity->rect.y += g_game.mouse_delta.y / (float)scale_h;
					}
				}
			}

			// Resize
			rect.x = (int)entity->rect.x - 5;
			rect.y = (int)entity->rect.y - 5;
			button_name = str128_create("green button %s", entity->name);
			bool green_button;
			unsigned color = 0X00FF00FF;
			WzStr name = (WzStr){
				.str = button_name.val, .len = button_name.len
			};
			WzWidget green_handle = wzrd_handle_button(&green_button,
				rect,
				color,
				name,
				background_box->handle);
			wz_widget_set_layer(green_handle, game_editor_layer_handle);

			if (green_button)
			{
				if (g_game.mouse_delta.x < 0)
				{
					entity->rect.x += g_game.mouse_delta.x / (float)scale_h;
					entity->rect.w += (float)fabs(g_game.mouse_delta.x) / (float)scale_h;
				}
				else {
					entity->rect.x += g_game.mouse_delta.x / (float)scale_h;
					entity->rect.w += -(float)fabs(g_game.mouse_delta.x) / (float)scale_h;
				}

				if (g_game.mouse_delta.y < 0)
				{
					entity->rect.y += g_game.mouse_delta.y / (float)scale_h;
					entity->rect.h += (float)fabs(g_game.mouse_delta.y) / (float)scale_h;
				}
				else
				{
					entity->rect.y += g_game.mouse_delta.y / (float)scale_h;
					entity->rect.h += -(float)fabs(g_game.mouse_delta.y) / (float)scale_h;
				}
			}
		}
	}

	// Deselect entity
	if (wzrd_box_is_active(background_box))
	{
		g_game.selected_entity_index_to_sorted_entities = 0;
		g_game.is_entity_selected = false;
	}
#endif

}

void game_gui_do(WzGui* wz, WzRect window, bool enable_input, unsigned int scale_w, unsigned int scale_h, WzStr* debug_str)
{
	assert(scale_w && scale_h);

	wzrd_v2 mouse_pos = (wzrd_v2){ g_game.mouse_pos.x / (int)scale_w,
		g_game.mouse_pos.y / (int)scale_h };

	WZRD_UNUSED(enable_input);

	WzKeyboard keyboard = { 0 };
	WzWidget window_widget = (WzWidget){ 0 };
	#if 0
		wz_begin(window.w, window.h, mouse_pos.x, mouse_pos.y,
		(WzState)g_platform.mouse_left,
		//*(WzKeyboardKeys*)&g_platform.keys_pressed,
		keyboard,
		enable_input);
#endif
	{
		wz_widget_set_color(window_widget, 0);
#if 0
		WzWidget t = wz_widget(window_widget);
		wz_widget_set_tight_constraints(t, 50, 50);
		wz_widget_set_pos(t, mouse_pos.x, mouse_pos.y);
		wz_widget_set_free_from_parent(t);
#endif	
		PlatformTextureBeginTarget(g_game.target_texture);
		{
			game_polygon_gui_do(mouse_pos, window, scale_w, scale_h, window_widget);
		}
		PlatformTextureEndTarget();

		game_entity_gui_do(scale_w, scale_h, wz, wz_widget_get(window_widget));
	}

	wz_end(debug_str);
}

void game_run(v2i window_size, bool enable, unsigned int scale) {

	// entity mouse interaction 
	(void)scale;
	if (enable)
	{
		if (g_game.mouse_pos.x >= 0 && g_game.mouse_pos.y >= 0 &&
			g_game.mouse_pos.x <= window_size.x && g_game.mouse_pos.y <= window_size.y) {
			Entity_handle hovered_entity = { 0 };

			int iterator_index = 1;
			Entity_handle entity_handle = { 0 };
			while (entity_get_next_handle(&iterator_index, &entity_handle)) {
				Entity* entity = game_entity_get(entity_handle);

				Rect scaled_rect = (Rect){ entity->rect.x * scale, entity->rect.y * scale, entity->rect.w * scale, entity->rect.h * scale };

				bool is_hover = v2_is_inside_rect((v2i) { g_game.mouse_pos.x, g_game.mouse_pos.y }, scaled_rect);
				if (is_hover) {
					hovered_entity = entity_handle;
				}
			}

			if (hovered_entity.val.index) {
				if (g_game.active_entity.val.index) {
					if (g_platform.mouse_left == Deactivating) {
						//g_game.selected_entity_index_to_sorted_entities = g_game.active_entity;
						g_game.active_entity = (Entity_handle){ 0 };
					}
				}

				if (g_game.hot_entity.val.index) {
					if (g_platform.mouse_left == Activating) {
						g_game.active_entity = g_game.hot_entity;
					}
				}

				if (hovered_entity.val.index) {
					g_game.hot_entity = hovered_entity;
				}
				else {
					g_game.hot_entity = (Entity_handle){ 0 };
				}
			}
			else {
				if (g_platform.mouse_left == Deactivating) {
					//g_game.selected_entity = (Entity_handle){ 0 };
				}
			}
		}
	}
}

PlatformTargetTexture game_target_texture_get() {
	return g_game.target_texture;
}

void game_draw_entities()
{
	PlatformTextureBeginTarget(g_game.target_texture);
	{
		for (unsigned int i = 0; i < g_game.sorted_entities_count; ++i)
		{
			Entity* entity = game_entity_get(g_game.sorted_entities[i]);
			Texture* texture = game_texture_get(entity->texture);
			if (texture)
			{
				PlatformTextureDrawFromSource(game_texture_get(entity->texture)->val, *(PlatformRect*)&entity->rect,
					(PlatformRect) {
					0, 0, texture->val.w, texture->val.h
				}, * (platform_color*)&entity->color);
			}
			else
			{
				platform_rect_draw(*(PlatformRect*)&entity->rect, (platform_color) { 255, 0, 0, 255 });
			}
		}
	}
	PlatformTextureEndTarget();
}

void game_init() {

	textures_load();

#if 0
	g_texture = PlatformTextureLoad("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\Textures\\ass.png");
	g_checkmark_texture = PlatformTextureLoad("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\Textures\\checkmark.png");
#endif
	g_game.target_texture = PlatformTargetTextureCreate();

	PlatformTextureBeginTarget(g_game.target_texture);
	{
		SDL_SetRenderDrawColor(g_sdl.renderer, 255, 255, 255, 255);
	}
	PlatformTextureEndTarget();

	PlatformTexture icon_delete = PlatformTextureLoad("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\Textures\\icon_delete.png");
	PlatformTexture icon_entity = PlatformTextureLoad("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\Textures\\entity.png");
	PlatformTexture icon_play = PlatformTextureLoad("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\Textures\\play.png");
	PlatformTexture icon_pause = PlatformTextureLoad("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\Textures\\pause.png");
	PlatformTexture icon_stop = PlatformTextureLoad("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\Textures\\stop.png");
	PlatformTexture icon_dropdown = PlatformTextureLoad("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\Textures\\icon_dropdown.png");
	PlatformTexture close_icon = PlatformTextureLoad("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\Textures\\x.png");

	g_game.icons = (wzrd_icons){
		.close = *(WzTexture*)&close_icon,
		.delete = *(WzTexture*)&icon_delete,
		.entity = *(WzTexture*)&icon_entity,
		.play = *(WzTexture*)&icon_play,
		.pause = *(WzTexture*)&icon_pause,
		.stop = *(WzTexture*)&icon_stop,
		.dropdown = *(WzTexture*)&icon_dropdown,
	};

	// Empty texture
	game_texture_add((Texture) { 0 });

	// Empty entity
	game_entity_create((Entity) { 0 });

	// ...
	game_texture_add(texture_get_by_name(str128_create("player")));

	game_entity_create((Entity) {
		.rect = { .x = 50, .y = 50, .w = 100, .h = 100 }, .name = str128_create("e1"),
			.texture = game_texture_add(texture_get_by_name(str128_create("clouds"))),
			.color = 0XFFFFFFFF
	});
	game_entity_create((Entity) {
		.rect = { .x = 50, .y = 50, .w = 100, .h = 100 }, .name = str128_create("e2"),
			.texture = game_texture_add(texture_get_by_name(str128_create("clouds"))),
			.color = 0XFFFFFFFF
	});
	game_entity_create((Entity) {
		.rect = { .x = 50, .y = 50, .w = 100, .h = 100 }, .name = str128_create("e3"),
			.texture = game_texture_add(texture_get_by_name(str128_create("clouds"))),
			.color = 0XFFFFFFFF
	});
	game_entity_create((Entity) {
		.rect = { .x = 50, .y = 50, .w = 100, .h = 100 }, .name = str128_create("e4"),
			.texture = game_texture_add(texture_get_by_name(str128_create("clouds"))),
			.color = 0XFFFFFFFF
	});


}

#if 0
void DoAnimations() {
	for (int i = 1; i < MAX_NUM_ENTITIES; ++i) {
		if (entity_system.entities[i].type == EntityType_None) {
			continue;
		}

		if (entity_system.entities[i].current_animation) {
			assert(entity_system.entities[i].animations_map);

			int ii = shgeti(entity_system.entities[i].animations_map, entity_system.entities[i].current_animation);
			Animation* animation = &entity_system.entities[i].animations_map[ii].value;
			if (animation->status == Status_Running) {
				Animate(animation);
			}
		}
	}
}

void DrawCurrentPath() {

	for (int i = 0; i < arrlen(pathfinding_system.current_path_buff); ++i) {
		PixelPos pos = GetPixelPosFromNodePos(pathfinding_system.current_path_buff[i]);
		PlatformRectDraw((Rect) { pos.pos.x - 5, pos.pos.y - 5, 10, 10 }, PLATFORM_BLUE);
	}

	for (int j = 0; j < hmlen(pathfinding_system.obstacle_map); ++j) {
		PixelPos pos = GetPixelPosFromNodePos(pathfinding_system.obstacle_map[j].key);

		PlatformRectDraw((Rect) { pos.pos.x - 5, pos.pos.y - 5, 10, 10 }, PLATFORM_YELLOW);
	}
}

void RunGameLoop(PlatformRenderTexture target, bool enable_input, bool is_playing) {

	CountTime();

	if (enable_input) {

		HandleInput();

		HandleEntityGrabbing(is_playing);

		// Handle entity movement during edit mode
		if (!is_playing) {
			if (grab_system.grabbed_entity.index) {
				V2 mouse_pos = input_system.mouse_pos;

				Vector2 size = GetEntity(grab_system.grabbed_entity)->size;
				Vector2 left_corner = { mouse_pos.x - size.x / 2, mouse_pos.y - size.y / 2 };
				Vector2 right_corner = { left_corner.x + size.x, left_corner.y + size.y };

				if (mouse_pos.x < GAME_SCREEN_WIDTH && mouse_pos.y < GAME_SCREEN_HEIGHT && mouse_pos.y > 0 && mouse_pos.x > 0) {
					GetEntity(grab_system.grabbed_entity)->pixel_pos.pos.x += mouse_pos.x - input_system.previous_mouse_pos.x;
					GetEntity(grab_system.grabbed_entity)->pixel_pos.pos.y += mouse_pos.y - input_system.previous_mouse_pos.y;
				}
			}
		}

		// Handle entity grabbing during play mode
		// NOTE: Same code as HandleInventoryGrabbing?
#if 0
		if (is_playing) {
			if (entity_system.grabbed_entity.index) {
				if (GetEntity(entity_system.grabbed_entity)->type == EntityType_InventoryItem) {
					GetEntity(entity_system.grabbed_entity)->pixel_pos.pos = Vector2iAdd(GetEntity(entity_system.grabbed_entity)->pixel_pos.pos,
						Vector2iSubtract(input_system.mouse_pos, input_system.previous_mouse_pos));
				}
			}
		}
#endif

	}

	// Game rendering?
#if 0
	BeginTextureMode(target);
	{
		ClearBackground(GRAY);

		if (is_playing) {
			HandleInventoryItemGrabbing();
#if 0
			HandlePlayerMovement();
			MoveEntity();
			DoActions2();
#endif
		}
		DoAnimations();
		DrawEntities();

	}
	EndTextureMode();
#endif
}

void PlatformSetRenderTarget(PlatformRenderTexture texture) {

}

void InitGame() {
	Log(__LINE__, "Init started");

	//GuiLoadIcons("C:\\Users\\elior\\OneDrive\\Desktop\\ConsoleApplication1\\Resoruces\\iconset.rgi", false);

	InitStringSystem();

	LoadTextures();

	bool load = false;
	bool load_parse_file = false;
	bool load_cold = true;

	if (load_cold) {
		InitEntitySystem();
	}
	else if (load_parse_file) {
		InitGameFromText();
		NodePos node_pos = (NodePos){ 20, 20 };
		PixelPos player_pixel_pos = GetPixelPosFromNodePos(node_pos);
		//game_state.player = AddEntityToScreen("player", CreateString("run"), (PlatformRect) { player_pixel_pos.pos.x, player_pixel_pos.pos.y, 100, 100 }, EntityType_Character);
		GetEntityByName("player")->node_pos = node_pos;
	}
	else if (load) {
		LoadGame("filey");
		//game_state.player = GetEntityByName("player")->id;
	}

	//LoadInventory();
	//LoadScreenItems2();

	LoadMatches();
	LoadScreenMatches();


	SpriteSheet sprite_sheet = ParseSpriteData("anim");
	Animation run_animation = CreateAnimation("run", sprite_sheet, 0);
	Animation idle_animation = CreateAnimation("idle", sprite_sheet, 1);

#if FIX_ME
	Entity* player = GetEntityByName("player");
	if (player->type) {
		shput(player->animations_map, "run", run_animation);
		shput(player->animations_map, "idle", idle_animation);
		player->current_animation = "idle";
	}
	LoadPathFindingSystem();
#endif

#if 0
	for (int i = 0; i < arrlen(pathfinding_system.limit_nodes_buff); ++i) {
		NodePos* nodes = GetLine2(pathfinding_system.limit_nodes_buff[i], pathfinding_system.limit_nodes_buff[(i + 1) % arrlen(pathfinding_system.limit_nodes_buff)]);
		for (int j = 0; j < arrlen(nodes); ++j) {
			hmput(pathfinding_system.obstacle_map, nodes[j], 0);
		}
	}
#endif

	//StartEntityAnimation(GetEntity);

	target = (PlatformRenderTexture){ PlatformLoadRenderTexture(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT) };

	//CreateBackground();

	drop_down_panel_target = (PlatformRenderTexture){ PlatformLoadRenderTexture(100, 300) };

	// main loop


	//run_game_loop = false;
	//run_slice_spritesheet = true;

	CreateEntity("background", EntityType_Background,
		(Vector2i) {
		GAME_SCREEN_WIDTH / 2, GAME_SCREEN_HEIGHT / 2
	},
		(Vector2) {
		GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT
	}, GetSprite("background"));

	Log(__LINE__, "Init ended");
}

Vector2 GetScreenCenter() {
	Vector2 result = (Vector2){ GAME_SCREEN_WIDTH / 2, GAME_SCREEN_HEIGHT / 2 };

	return result;
}

#endif

wzrd_icons game_icons_get() {
	return g_game.icons;
}