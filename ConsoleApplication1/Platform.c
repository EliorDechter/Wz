#include "Platform.h"
//#include "Editor.h"
#include <SDL3/SDL_main.h>
#include "Game.h"
#include "Editor.h"

// Color component extraction macros (for RGBA format: 0xRRGGBBAA)
#define WZ_COLOR_R(c) (((c) >> 24) & 0xFF)
#define WZ_COLOR_G(c) (((c) >> 16) & 0xFF)
#define WZ_COLOR_B(c) (((c) >> 8) & 0xFF)
#define WZ_COLOR_A(c) ((c) & 0xFF)

typedef struct Test
{
	unsigned x[32];
	unsigned y[32];
} Test;

WzTree wz_tree_create()
{
	WzTree result;
	result.children_indices = malloc(1024);
	result.children_count = 0;
	result.nodes = malloc(1024);
	result.nodes_count = 0;
	result.selected_row = (WzWidget){ 0 };

	return result;
}

WzTreeNode wz_tree_node_create(WzTree* tree, const char* name, WzTexture texture, bool expanded)
{
	WzTreeNodeData node;
	node.name = name;
	node.texture = texture;
	node.expand = expanded;
	node.depth = 0;
	node.children_count = 0;
	node.visible = false;

	tree->nodes[tree->nodes_count] = node;
	tree->nodes_count++;
	WzTreeNode result;
	result.index = tree->nodes_count - 1;

	return result;
}

void wz_tree_node_add_children(WzTree* tree, WzTreeNode node,
	WzTreeNode* children, unsigned children_count)
{
	unsigned child_depth = tree->nodes[node.index].depth + 1;
	for (unsigned i = 0; i < children_count; ++i)
	{
		tree->children_indices[i + tree->children_count] = children[i];
		tree->nodes[children[i].index].depth = child_depth;
	}
	tree->children_count += children_count;
	tree->nodes[node.index].children_index = tree->children_count - children_count;
	tree->nodes[node.index].children_count = children_count;
}

WzTree g_tree;

void load_tree()
{
	g_tree = wz_tree_create();
	PlatformTexture icon = PlatformTextureLoad("C:\\Users\\Elior\\Desktop\\Folder.png");
	WzTexture texture = *((WzTexture*)&icon);

	WzTreeNode node_a = wz_tree_node_create(&g_tree, "a", texture, false);
	g_tree.nodes[node_a.index].visible = true;
	WzTreeNode node_b = wz_tree_node_create(&g_tree, "b", texture, false);
	WzTreeNode node_c = wz_tree_node_create(&g_tree, "c", texture, false);
	WzTreeNode children[] = { node_b, node_c };
	wz_tree_node_add_children(&g_tree, node_a, children, 2);
}

#define WZRD_UNUSED(x) (void)x;

PlatformSystem g_platform;

// TODO: make it not global!
WzGui editor_gui, game_gui;

void PlatformTextDraw(const char* str, float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	TTF_Text* text = TTF_CreateText(g_sdl.text_engine, g_sdl.font, str, 0);
	TTF_SetTextColor(text, r, g, b, a);
	TTF_DrawRendererText(text, x, y);
	TTF_DestroyText(text);
}

PlatformV2i PlatformTextGetSize(const char* str) {
	PlatformV2i result = { 0 };
	TTF_Text* text = TTF_CreateText(g_sdl.text_engine, g_sdl.font, str, 0);
	TTF_GetTextSize(text, &result.x, &result.y);
	TTF_DestroyText(text);

	return result;
}

void get_string_size(char* str, int* w, int* h)
{
	int w_int = 0, h_int = 0;
	str128 line = { 0 };
	size_t index = 0;
	size_t str_len = strlen(str);
	*w = 0;
	*h = 0;
	while (str[index])
	{
		line = (str128){ 0 };
		for (; index < str_len; ++index)
		{
			if (str[index] == '\n')
			{
				++index;
				line.val[line.len] = 0;
				break;
			}

			line.val[line.len++] = str[index];
		}

		bool result = TTF_GetStringSize(g_sdl.font, line.val, 0, &w_int, &h_int);
		assert(result);

		*w += (int)w_int;
		*h += (int)h_int;
	}
}

void platform_rect_draw(PlatformRect rect, platform_color color) {
	// WARNING: 0.01 ms to run this function
	//SDL_FRect sdl_rect = { rect.x), (float)floor(rect.y), (float)floor(rect.w), (float)floor(rect.h) };
	SDL_FRect sdl_rect = { rect.x, rect.y, rect.w, rect.h };
	SDL_SetRenderDrawBlendMode(g_sdl.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(g_sdl.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(g_sdl.renderer, &sdl_rect);
}

void PlatformTextureDraw(PlatformTexture texture, PlatformRect rect) {
	SDL_RenderTexture(g_sdl.renderer, texture.data, 0, (SDL_FRect*)&rect);
}

void PlatformTextureDrawFromSource(PlatformTexture texture, PlatformRect dest, PlatformRect src, platform_color color) {
	if (!(color.r == 0 && color.g == 0 && color.b == 0))
		SDL_SetTextureColorMod(texture.data, color.r, color.g, color.b);
	SDL_RenderTexture(g_sdl.renderer, texture.data, (SDL_FRect*)&src, (SDL_FRect*)&dest);
}

PlatformTargetTexture PlatformTargetTextureCreate() {
	SDL_Texture* texture = SDL_CreateTexture(g_sdl.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GAME_WIDTH, GAME_HEIGHT);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	PlatformTargetTexture result = { .data = texture, .w = GAME_WIDTH , .h = GAME_HEIGHT };

	return result;
}

void platform_clear_target_to_white()
{
	SDL_SetRenderDrawColor(g_sdl.renderer, 255, 255, 255, 255);
	SDL_RenderClear(g_sdl.renderer);
}

void PlatformTextureBeginTarget(PlatformTargetTexture texture) {
	SDL_SetRenderTarget(g_sdl.renderer, texture.data);
}

void PlatformTextureEndTarget() {
	SDL_SetRenderTarget(g_sdl.renderer, 0);
}

void PlatformLineDraw(float x0, float y0, float x1, float y1, unsigned char r, unsigned char g, unsigned char b) {
	SDL_SetRenderDrawColor(g_sdl.renderer, r, g, b, 255);
	SDL_RenderLine(g_sdl.renderer, x0, y0, x1, y1);
}

void platform_draw_points(float* x, float* y, unsigned count)
{
	SDL_FPoint* points = malloc(sizeof(*points) * count);
	for (unsigned i = 0; i < count; ++i)
	{
		SDL_FPoint point;
		point.x = x[i];
		point.y = y[i];
		points[i] = point;
	}

	SDL_RenderPoints(g_sdl.renderer, points, (int)count);
	free(points);
}

void platform_draw_horizontal_line(int x, int y, unsigned w) {
	SDL_SetRenderDrawColor(g_sdl.renderer, 125, 125, 125, 255);

	for (int i = x; i < x + w; ++i) {
		if (i % 2 == 1) continue;
		SDL_RenderPoint(g_sdl.renderer, (float)i, (float)y);
	}
}

void platform_draw_vertical_line(int x, int y, unsigned h) {
	SDL_SetRenderDrawColor(g_sdl.renderer, 125, 125, 125, 255);

	for (int i = y; i < y + h; ++i) {
		if (i % 2 == 1) continue;
		SDL_RenderPoint(g_sdl.renderer, (float)x, (float)i);
	}
}

PlatformTexture PlatformTextureLoad(const char* path) {
	float w, h;
	SDL_Surface* surface = IMG_Load(path);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(g_sdl.renderer, surface);

	const char* error = SDL_GetError();
	printf("%s\n", error);

	SDL_GetTextureSize(texture, &w, &h);
	PlatformTexture result = (PlatformTexture){ .data = texture, .w = w, .h = h };

	return result;
}

#define DELETE_ME 0

SDL_Texture* g_target;
SDL_Texture* g_texture;


void WSDL_WzEnd(WzGui* canvas)
{
	SDL_Renderer* renderer = g_sdl.renderer;
	WzDrawList* dl = &canvas->draw_list;

	if (dl->vtx_count == 0) return;

	// WzVertex layout matches SDL_Vertex exactly: {float x,y}, {float r,g,b,a}, {float u,v}
	SDL_Vertex* sdl_verts = (SDL_Vertex*)dl->vertices;

	for (unsigned i = 0; i < dl->draw_call_count; ++i)
	{
		WzDrawCall* dc = &dl->draw_calls[i];

		if (dc->has_clip) {
			SDL_Rect clip = { (int)dc->clip_rect.x, (int)dc->clip_rect.y,
				(int)dc->clip_rect.w, (int)dc->clip_rect.h };
			SDL_SetRenderClipRect(renderer, &clip);
		} else {
			SDL_SetRenderClipRect(renderer, NULL);
		}

		SDL_RenderGeometry(renderer, (SDL_Texture*)dc->texture,
			sdl_verts, (int)dl->vtx_count,
			dl->indices + dc->idx_offset, (int)dc->idx_count);
	}

	SDL_SetRenderClipRect(renderer, NULL);
}

// Upload a single-channel alpha bitmap to an RGBA SDL_Texture for font atlas rendering
static SDL_Texture* wz_upload_font_atlas(SDL_Renderer* renderer, const unsigned char* bitmap, int w, int h)
{
	SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, w, h);
	assert(tex);

	// Convert 8-bit alpha to RGBA (white + alpha)
	unsigned char* rgba = (unsigned char*)malloc(w * h * 4);
	for (int i = 0; i < w * h; i++) {
		rgba[i * 4 + 0] = 255;
		rgba[i * 4 + 1] = 255;
		rgba[i * 4 + 2] = 255;
		rgba[i * 4 + 3] = bitmap[i];
	}

	SDL_UpdateTexture(tex, NULL, rgba, w * 4);
	free(rgba);

	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
	return tex;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
	(void)appstate;
	(void)argc;
	(void)argv;

	//wz_gui_init(&game_gui);
	//wz_gui_init(&editor_gui);

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d");

	/* Create the window */
	if (!SDL_CreateWindowAndRenderer("Hello World", EDITOR_WIDTH, EDITOR_HEIGHT, SDL_WINDOW_RESIZABLE, &g_sdl.window, &g_sdl.renderer)) {
		SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	PlatformTexture icon = PlatformTextureLoad("C:\\Users\\Elior\\Desktop\\Folder.png");
	editor_textures[EditorTextureButton] = *((WzTexture*)&icon);

	load_tree();

	// Initialize GUI and load font atlas
	wz_set_gui(&editor_gui);

	{
		// Read TTF file
		SDL_IOStream* io = SDL_IOFromFile("C:\\Windows\\Fonts\\Arial.ttf", "rb");
		assert(io);
		Sint64 ttf_size = SDL_GetIOSize(io);
		unsigned char* ttf_data = (unsigned char*)malloc((size_t)ttf_size);
		SDL_ReadIO(io, ttf_data, (size_t)ttf_size);
		SDL_CloseIO(io);

		// Bake font atlas
		wz_font_load(0, ttf_data, (unsigned)ttf_size, 16.0f);
		free(ttf_data);

		// Upload atlas bitmap to GPU texture
		WzFont* font = &editor_gui.fonts[0];
		SDL_Texture* atlas_tex = wz_upload_font_atlas(g_sdl.renderer, font->atlas_bitmap, font->atlas_w, font->atlas_h);
		free(font->atlas_bitmap);
		font->atlas_bitmap = NULL;

		WzTexture wz_atlas = { .data = atlas_tex, .w = (float)font->atlas_w, .h = (float)font->atlas_h };
		wz_font_set_atlas_texture(0, wz_atlas);
	}

	// Dropdown icon (oversampled triangle)
	{
		wz_create_dropdown_icon();
		SDL_Texture* tex = wz_upload_font_atlas(g_sdl.renderer,
			editor_gui.dropdown_icon_bitmap,
			editor_gui.dropdown_icon_w,
			editor_gui.dropdown_icon_h);
		free(editor_gui.dropdown_icon_bitmap);
		editor_gui.dropdown_icon_bitmap = NULL;
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
		wz_set_dropdown_icon_texture((WzTexture){
			.data = tex,
			.w = (float)editor_gui.dropdown_icon_w,
			.h = (float)editor_gui.dropdown_icon_h });
	}

	game_init();

	g_sdl.mutex = SDL_CreateMutex();

	g_sdl.cursor_hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
	g_sdl.cursor_default = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	g_sdl.cursor_horizontal_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
	g_sdl.cursor_vertical_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);

	return SDL_APP_CONTINUE;
}

SDL_AppResult handle_events(SDL_Event* event) {

	for (int i = 0; i < 128; ++i) {
		if (g_platform.keyboard_states[i] == Deactivating) {
			g_platform.keyboard_states[i] = Inactive;
		}
		if (g_platform.keyboard_states[i] == Activating) {
			g_platform.keyboard_states[i] = Active;
		}
	}

	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
	}

	if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
	{
		if (event->button.button == SDL_BUTTON_LEFT)
		{
			if (g_platform.mouse_left == Inactive)
			{
				g_platform.mouse_left = Activating;
			}
		}
	}
	else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
	{
		if (event->button.button == SDL_BUTTON_LEFT)
		{
			g_platform.mouse_left = Deactivating;
		}
	}

	// Keyboard input
	SDL_Keycode keycode = 0;

	if (event->type == SDL_EVENT_KEY_DOWN) {
		keycode = SDL_GetKeyFromScancode(event->key.scancode, event->key.mod, false);
		if (keycode < 128) {
			if (g_platform.keyboard_states[keycode] == Inactive)
			{
				g_platform.keyboard_states[keycode] = Activating;
			}
			else if (g_platform.keyboard_states[keycode] == Activating)
			{
				g_platform.keyboard_states[keycode] = Active;
			}
		}
	}
	else if (event->type == SDL_EVENT_KEY_UP) {
		keycode = SDL_GetKeyFromScancode(event->key.scancode, event->key.mod, false);
		if (keycode < 128) {
			if (g_platform.keyboard_states[keycode] == Active || g_platform.keyboard_states[keycode] == Activating) {
				g_platform.keyboard_states[keycode] = Deactivating;
			}
			else if (g_platform.keyboard_states[keycode] == Deactivating) g_platform.keyboard_states[keycode] = Inactive;
		}
	}
	else if (event->type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
		g_platform.focus = false;
	}
	else if (event->type == SDL_EVENT_WINDOW_MOUSE_ENTER) {
		g_platform.focus = true;
	}

	if (keycode < 128 && keycode > 0) {
		g_platform.keys_pressed.keys[g_platform.keys_pressed.count++] = (PlatformKey){ .val = (unsigned int)keycode, .state = g_platform.keyboard_states[keycode] };
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	(void)appstate;
	
	SDL_AppResult result = handle_events(event);

	return result;
}

void platform_cursor_set(PlatformCursor cursor) {
	switch (cursor) {
	case PlatformCursorDefault:
		SDL_SetCursor(g_sdl.cursor_default);
		break;
	case PlatformCursorHand:
		SDL_SetCursor(g_sdl.cursor_hand);
		break;
	case PlatformCursorHorizontalArrow:
		SDL_SetCursor(g_sdl.cursor_horizontal_arrow);
		break;
	case PlatformCursorVerticalArrow:
		SDL_SetCursor(g_sdl.cursor_vertical_arrow);
		break;
	}
}

str1024 g_debug_text;

typedef struct Second
{
	uint64_t val;
} Second;

Second platform_get_time()
{
	Second result = { SDL_GetTicks() };

	return result;
}

void platform_end()
{
	SDL_RenderPresent(g_sdl.renderer);

	// Input
	g_platform.keys_pressed.count = 0;

	if (g_platform.mouse_left == Activating) {
		g_platform.mouse_left = Active;
	}
	else if (g_platform.mouse_left == Deactivating) {
		g_platform.mouse_left = Inactive;
	}

	float time = SDL_GetTicks() / 1000.0f;
	g_platform.last_frame_time_in_seconds = time - g_platform.time;

	g_platform.time_samples[g_platform.time_samples_count] = g_platform.last_frame_time_in_seconds;
	g_platform.time_samples_count = (g_platform.time_samples_count + 1) % 32;
	g_platform.average_spf = 0;
	for (int i = 0; i < 32; ++i)
	{
		g_platform.average_spf += g_platform.time_samples[i];
	}
	g_platform.average_spf = g_platform.average_spf / 32;

	const float spf = 1.0f / 60.0f;
	if (g_platform.last_frame_time_in_seconds < spf)
	{
		SDL_Delay((int)(spf * 1000.0f - g_platform.last_frame_time_in_seconds * 1000.0f));
		//SDL_Delay(2000);
	}
}

void platform_begin()
{

	g_platform.time = SDL_GetTicks() / 1000.0f;

	float sdl_mouse_x, sdl_mouse_y;
	SDL_GetMouseState(&sdl_mouse_x, &sdl_mouse_y);

	g_platform.mouse_x = (int)sdl_mouse_x;
	g_platform.mouse_y = (int)sdl_mouse_y;

	g_platform.mouse_delta_x = g_platform.mouse_x - g_platform.previous_mouse_x;
	g_platform.mouse_delta_y = g_platform.mouse_y - g_platform.previous_mouse_y;

	g_platform.previous_mouse_x = g_platform.mouse_x;
	g_platform.previous_mouse_y = g_platform.mouse_y;

	if (!g_platform.focus) {
		g_platform.mouse_x = -1;
		g_platform.mouse_y = -1;
	}

	platform_color color = PLATFORM_LIGHTGRAY;
	SDL_SetRenderDrawColor(g_sdl.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(g_sdl.renderer);
	SDL_GetWindowSize(g_sdl.window, &g_platform.window_width, &g_platform.window_height);
}

uint64_t g_time;
uint64_t g_time_a;
uint64_t g_time_b;

void platform_time_begin()
{
	g_time_a = SDL_GetTicksNS();
}

void platform_time_end()
{
	g_time_b = SDL_GetTicksNS();
	g_time = g_time_b - g_time_a;
}

void test_gui(WzGui* wz)
{
	WzKeyboard keyboard = { 0 };
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

	WzWidget window = wz_vbox(window0);
	WzWidget menu = wz_vbox(window);
	wz_widget_set_border(menu, WZ_BORDER_TYPE_DEFAULT);
	wz_widget_set_size(menu, 200, 800);
	wz_widget_set_color(menu, 0xffffffff);

	bool expand = false, selected = false;
	WzTreeNodeData* nodes_stack[1024];
	unsigned nodes_stack_count = 0;
	g_tree.menu = menu;

	// Root Node
	WzTreeNodeData* current_node = &g_tree.nodes[0];

	WzWidget row = wz_tree_add_row(&g_tree, wz_str_create(current_node->name), current_node->texture,
		current_node->depth, &current_node->expand, &selected, current_node);

	if (selected)
	{
		g_tree.selected_row = row;
	}

	nodes_stack_count++;

	while (1)
	{
		for (unsigned i = 0; i < current_node->children_count; ++i)
		{
			unsigned* children = g_tree.children_indices + current_node->children_index;
			WzTreeNodeData* child = &g_tree.nodes[children[i]];
			if (current_node->expand)
			{
				selected = false;
				row = wz_tree_add_row(&g_tree, wz_str_create(child->name),
					child->texture,
					child->depth, &child->expand, &selected, child);

				if (selected)
				{
					g_tree.selected_row = row;
				}

				child->visible = true;
				nodes_stack[nodes_stack_count++] = child;
			}
			else
			{
				child->visible = false;
			}
		}

		current_node = nodes_stack[nodes_stack_count - 1];
		nodes_stack_count--;

		if (!nodes_stack_count)
		{
			break;
		}

	}
	wz->trees[0] = g_tree;

	wz_end();

	WSDL_WzEnd(wz);
}

WzGui test_gui_obj;

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{

	platform_begin();
	{

		static WzRect game_screen_rect;
		WzWidget target_panel;
		// Editor
		const bool enable_editor = true;

		WzStr debug_str =
		{ g_debug_text.val, g_debug_text.len };

		//test_gui(&editor_gui);

		if (enable_editor) {
			wzrd_icons icons = game_icons_get();
			unsigned int time_a = SDL_GetTicksNS();
			editor_run(&editor_gui, game_target_texture_get(), icons, &debug_str, &target_panel);
			unsigned int time_b = SDL_GetTicksNS();

			WSDL_WzEnd(&editor_gui);
			g_debug_text = (str1024){ 0 };

			static uint64_t samples[32];
			static size_t samples_count;

			samples[samples_count] = (time_b - time_a);
			samples_count = (samples_count + 1) % 32;
			float average_time = 0;
			for (int i = 0; i < 32; ++i)
			{
				average_time += (float)samples[i] / 1000000;
			}
			average_time /= 32;
			str1024_concat(&g_debug_text, str1024_create("editor time: %f\n", average_time));
			//printf("editor time: %f\n", (float)(time_b - time_a) / 1000000);
		}

		// Game ui and stuff
		bool enable_game_input = false;

#if 0 
		WzWidgetData* target_panel_data = wz_widget_get(target_panel);
		game_screen_rect = (WzRect){ target_panel_data->actual_x, target_panel_data->actual_y,
			target_panel_data->actual_w, target_panel_data->actual_h };

		if (wzrd_box_is_hot_using_canvas(&editor_gui, target_panel_data))
		{
			enable_game_input = true;
		}
#endif
		{
			// Clear target
			PlatformTextureBeginTarget(game_target_texture_get());
			{
				SDL_SetRenderDrawColor(g_sdl.renderer, 0xc3, 0xc3, 0xc3, 255);
				SDL_RenderClear(g_sdl.renderer);
			}
			PlatformTextureEndTarget();

			float mouse_x = g_platform.mouse_x - game_screen_rect.x;
			float mouse_y = g_platform.mouse_y - game_screen_rect.y;
			g_game.mouse_delta = (v2i){ mouse_x - g_game.mouse_pos.x, mouse_y - g_game.mouse_pos.y };
			g_game.mouse_pos.x = mouse_x;
			g_game.mouse_pos.y = mouse_y;

			// Run game
			PlatformTextureBeginTarget(g_game.target_texture);
			game_draw_screen_dots();
			PlatformTextureEndTarget();
			game_draw_entities();

			// Inside game editing gui
			if (!g_game.run)
			{
				unsigned int game_texture_width = game_target_texture_get().w;
				unsigned int game_texture_height = game_target_texture_get().h;
				unsigned int scale_w = (unsigned int)(game_screen_rect.w / game_texture_width);
				unsigned int scale_h = (unsigned int)(game_screen_rect.h / game_texture_height);
#if 0
				game_gui_do(&game_gui,
					(WzRect) {
					0, 0, (int)game_target_texture_get().w, (int)game_target_texture_get().h
				},
					enable_game_input, scale_w, scale_h, & debug_str);
#endif
				PlatformTextureBeginTarget(g_game.target_texture);
				WSDL_WzEnd(&game_gui);
				PlatformTextureEndTarget();
			}
		}

		// Debug view
		//str1024_concat(&g_debug_text, str1024_create(debug_str.str));

		str1024_concat(&g_debug_text, str1024_create("frame time: %f ms", g_platform.average_spf * 1000.0f));


		// Cursor
		if (enable_game_input) {
			platform_cursor_set(*(PlatformCursor*)&game_gui.cursor);
		}
		else {
			platform_cursor_set(*(PlatformCursor*)&editor_gui.cursor);
		}

	}
	platform_end();

	return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	WZRD_UNUSED(appstate);
	WZRD_UNUSED(result);

	wz_gui_deinit(&editor_gui);
}