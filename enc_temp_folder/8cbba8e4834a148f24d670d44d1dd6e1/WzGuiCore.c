#define wz_assert(x) assert(x)
//#define wz_assert(x) (void)(x)
#define WZRD_UNUSED(x) (void)x

#include "WzGuiCore.h"
#include "Strings.h"
#include "WzLayout.h"
#include "WzLayoutScalar.h"

#define STB_TEXTEDIT_IMPLEMENTATION
#include "stb_textedit.h"

#define STACK_MAX_DEPTH 64

static WzGui* wz;

bool wz_widget_is_equal(WzWidget a, WzWidget b)
{
	if (a.handle == b.handle)
	{
		return true;
	}

	return false;
}

void wz_widget_add_source(WzWidget handle, const char* file, unsigned int line)
{
	if (!file) return;
	wz_assert(file);
	const char* f;
	for (f = file + strlen(file); *(f - 1) != '\\'; --f) {}

	WzWidgetData* w = wz_widget_get(handle);

	char temp_buffer[128];

	sprintf(temp_buffer, "%s %d ", f, line);
	strcat(w->source, temp_buffer);
}

void wz_widget_set_top_pad(WzWidget widget, unsigned pad)
{
	WzWidgetData* data = wz_widget_get(widget);

	data->constraint_max_h += pad;
	data->pad_top = pad;
}

void wz_widget_set_pad(WzWidget widget, unsigned pad)
{
	WzWidgetData* data = wz_widget_get(widget);

	data->pad_left = data->pad_right = data->pad_top = data->pad_bottom = pad;

	data->constraint_max_w += pad;
	data->constraint_max_h += pad;
}

void wz_widget_set_margin_left(WzWidget widget, unsigned int pad)
{
	wz_widget_get(widget)->margin_left = pad;
}

void wz_widget_set_margin_right(WzWidget widget, unsigned int pad)
{

	wz_widget_get(widget)->margin_right = pad;
}

void wz_widget_set_margin_top(WzWidget widget, unsigned int pad)
{
	WzWidgetData* d = wz_widget_get(widget);

	wz_widget_get(widget)->margin_top = pad;
}

void wz_widget_set_margin_bottom(WzWidget widget, unsigned int pad)
{

	wz_widget_get(widget)->margin_bottom = pad;
}

void wz_widget_set_child_gap(WzWidget widget, unsigned int child_gap)
{
	wz_widget_get(widget)->child_gap = child_gap;
}

bool wz_handle_is_valid(WzWidget handle)
{
	return (bool)handle.handle;
}

WzWidgetData* wz_widget_get(WzWidget handle)
{
	WzWidgetData* result = &wz->widgets[0];
	for (int i = 0; i < MAX_NUM_WIDGETS; ++i)
	{
		if (wz->widgets[i].handle.handle == handle.handle)
		{
			result = wz->widgets + i;
			break;
		}
	}

	return result;
}

bool wz_widget_is_valid(WzWidget widget)
{
	if (widget.handle)
	{
		return true;
	}

	return false;
}

WzWidget wz_create_handle()
{
	assert(wz);
	WzWidget handle = { 0 };
	for (unsigned i = 1; i < MAX_NUM_WIDGETS; ++i)
	{
		if (!wz->occupied[i])
		{
			handle.handle = i;
			wz->occupied[i] = true;
			return handle;
		}
	}

	wz_assert(0);

	return handle;
}

bool wzrd_is_rect_inside_rect(WzRect a, WzRect b) {
	bool res = a.x >= b.x;
	res &= a.x >= b.x;
	res &= a.x + a.w <= b.x + b.w;
	res &= a.y + a.h <= b.y + b.h;

	return res;
}

void wz_widget_set_max_h(WzWidget w, int height)
{
	wz_assert(height);
	WzWidgetData* b = wz_widget_get(w);
	b->constraint_max_h = height;
}

void wz_widget_set_max_w(WzWidget w, int width)
{
	wz_assert(width);
	WzWidgetData* b = wz_widget_get(w);
	b->constraint_max_w = width;
}

void wz_widget_set_min_w(WzWidget w, int width)
{
	wz_assert(width);
	WzWidgetData* b = wz_widget_get(w);
	b->min_w = width;
}

void wz_widget_data_set_x(WzWidgetData* widget, int x)
{
	widget->x = x;
}

void wz_widget_data_set_y(WzWidgetData* widget, int y)
{
	widget->y = y;
}

void wz_widget_set_x(WzWidget w, int x)
{
	WzWidgetData* widget = wz_widget_get(w);
	wz_widget_data_set_x(widget, x);

	wz->chunks[widget->chunk].relative_x[widget->slot] = x;

}

void wz_widget_set_y(WzWidget w, int y)
{
	WzWidgetData* widget = wz_widget_get(w);
	wz_widget_data_set_y(widget, y);

	wz->chunks[widget->chunk].relative_y[widget->slot] = y;
}

void wz_widget_set_margins(WzWidget w, unsigned int pad)
{
	wz_widget_set_margin_top(w, pad);
	wz_widget_set_margin_bottom(w, pad);
	wz_widget_set_margin_left(w, pad);
	wz_widget_set_margin_right(w, pad);
}

void wz_widget_data_set_border(WzWidgetData* d, WzBorderType border_type)
{
	d->border_type = border_type;
}

// Forward declare your library functions
void wz_input_key_event(int wz_keycode, bool is_down, bool is_repeat) {}
void wz_input_mouse_button_event(int button, bool is_down, float x, float y) {}
void wz_input_mouse_motion_event(float x, float y, float dx, float dy) {}
void wz_input_mouse_wheel_event(float dx, float dy) {}

void wz_widget_set_border(WzWidget w, WzBorderType border_type)
{
	WzWidgetData* d = wz_widget_get(w);
	wz_widget_data_set_border(d, border_type);
}

void wz_widget_set_pos(WzWidget handle, int x, int y)
{
	wz_widget_set_x(handle, x);
	wz_widget_set_y(handle, y);
}

void wz_widget_data_set_pos(WzWidgetData* handle, int x, int y)
{
	handle->x = x;
	handle->y = y;
}

void wz_widget_set_color_old(WzWidget widget, unsigned int color)
{
	WzWidgetData* d = wz_widget_get(widget);
	d->color = color;
}


void wz_inherit(WzWidget parent)
{
	WzWidget stack[STACK_MAX_DEPTH] = { 0 };
	unsigned count = 0, ptr = 0;

	stack[count++] = wz_widget_get(parent)->handle;

	WzWidgetData* parent_data = wz_widget_get(parent);
	parent_data->world_scale[0] = parent_data->scale[0];
	parent_data->world_scale[1] = parent_data->scale[1];
	parent_data->world_pos[0] = parent_data->pos[0];
	parent_data->world_pos[1] = parent_data->pos[1];
	parent_data->world_rotation = parent_data->rotation;

	while (ptr != count)
	{
		WzWidgetData* widget = wz_widget_get(stack[ptr]);

		for (int i = 0; i < widget->children_count; ++i)
		{
			wz_assert(count < MAX_NUM_WIDGETS);
			stack[count++] = widget->children[i];

			WzWidgetData* child = wz_widget_get(widget->children[i]);
			child->world_pos[0] = child->pos[0] + widget->world_pos[0];
			child->world_pos[1] += child->pos[1] + widget->world_pos[1];

			child->world_rotation += child->rotation + widget->world_rotation;

			if (!child->absolute_scale)
			{
				child->world_scale[0] = child->scale[0] * widget->world_scale[0];
				child->world_scale[1] = child->scale[1] * widget->world_scale[1];
			}
			else
			{
				child->world_scale[0] = child->scale[0];
				child->world_scale[1] = child->scale[1];
			}
		}

		ptr++;
	}
}

bool wzrd_handle_is_child_of_handle(WzWidget a, WzWidget b)
{
	if (a.handle == 0) return false;
	if (b.handle == 0) return false;

	int stack[MAX_NUM_WIDGETS] = { 0 };
	int count = 0;
	int ptr = 0;

	stack[count++] = wz_widget_get(a)->handle.handle;

	while (ptr != count)
	{
		WzWidgetData* widget = &wz->widgets[stack[ptr]];

		for (int i = 0; i < widget->children_count; ++i)
		{
			wz_assert(count < MAX_NUM_WIDGETS);
			stack[count++] = widget->children[i].handle;
		}

		if (wz_widget_is_equal(widget->handle, b))
			return true;

		ptr++;
	}

	return false;
}


int wzrd_box_get_current_index() {
	wz_assert(wz->current_crate_index >= 0);
	int current_box_index = wz->crates_stack[wz->current_crate_index].box_stack_count - 1;
	if (current_box_index < 0) return 0;
	int final_index = wz->crates_stack[wz->current_crate_index].box_stack[current_box_index];
	int result = final_index;

	return result;
}

void wz_widget_add_text_new(WzWidget parent, WzStr str, float x, float y)
{
	WzWidgetItem item;

	item.type = WZ_WIDGET_ITEM_TYPE_STRING;
	item.val.str = str;
	//item.color = wz_widget_get(parent)->font_color;
	item.margin_bottom = item.margin_top = item.margin_right = item.margin_left = 0;
	item.x = x;
	item.y = y;
	item.font_id = wz_widget_get(parent)->font_id;
	wz_widget_add_item(parent, item);
}


void wz_widget_add_text(WzWidget parent, WzStr str)
{
	WzWidgetItem item = { 0 };

	item.type = WZ_WIDGET_ITEM_TYPE_STRING;
	item.val.str = str;
	//item.color = wz_widget_get(parent)->font_color;
	item.margin_bottom = item.margin_top = item.margin_right = item.margin_left = 0;
	item.font_id = wz_widget_get(parent)->font_id;
	wz_widget_add_item(parent, item);
}

void goo(WzWidgetData* box, void* data)
{
	(void)data;
	(void)box;
	//box->color = EGUI_BEIGE;
}

void wz_widget_set_z(WzWidget handle, unsigned int z)
{
	wz_widget_get(handle)->z = z;
}

void wzrd_box_tree_apply(int index, void* data, void (*goo)(WzWidgetData* box, void* data))
{
	int stack[1024];
	int count = 0;
	int ptr = 0;

	stack[count++] = index;

	while (ptr != count)
	{
		WzWidgetData* box = &wz->widgets[stack[ptr]];

		for (int i = 0; i < box->children_count; ++i)
		{
			stack[count++] = box->children[i].handle;
		}

		goo(box, data);
		ptr++;
	}
}

bool wzrd_handle_is_active_tree(WzWidget handle)
{
	if (!handle.handle) return false;

	int stack[1024];
	int count = 0;
	int ptr = 0;

	stack[count++] = wz_widget_get(handle)->handle.handle;

	while (ptr != count)
	{
		WzWidgetData* box = &wz->widgets[stack[ptr]];

		for (int i = 0; i < box->children_count; ++i)
		{
			wz_assert(count < 1023);
			stack[count++] = box->children[i].handle;
		}

		if (wzrd_box_is_active(box))
		{
			return true;
		}

		ptr++;
	}

	return false;
}

bool wz_widget_is_interacting_tree(WzWidget handle)
{
	if (!handle.handle) return false;

	int stack[1024];
	int count = 0;
	int ptr = 0;

	stack[count++] = wz_widget_get(handle)->handle.handle;

	while (ptr != count)
	{
		WzWidgetData* box = &wz->widgets[stack[ptr]];

		for (int i = 0; i < box->children_count; ++i)
		{
			wz_assert(count < 1023);
			stack[count++] = box->children[i].handle;
		}

		if (wz_widget_is_interacting(box->handle))
		{
			return true;
		}

		ptr++;
	}

	return false;
}

void wz_widget_add_offset(WzWidget handle, int x, int y)
{
	int stack[1024];
	int count = 0;
	int ptr = 0;

	stack[count++] = wz_widget_get(handle)->handle.handle;

	while (ptr != count)
	{
		WzWidgetData* box = &wz->widgets[stack[ptr]];

		for (int i = 0; i < box->children_count; ++i)
		{
			stack[count++] = box->children[i].handle;
		}

		box->actual_x += x;
		box->actual_y += y;

		ptr++;
	}
}

bool wz_widget_is_deactivating(WzWidget handle)
{
	if (wz_widget_is_equal(handle, wz->deactivating_item))
	{
		return true;
	}

	return false;
}

bool wzrd_box_is_released(WzWidgetData* box) {
	if (wz_widget_is_equal(box->handle, wz->deactivating_item)) {
		return true;
	}

	return false;
}


void wz_widget_set_text_alignment(WzWidget widget, unsigned text_alignment)
{
	wz_widget_get(widget)->text_alignment = text_alignment;
}

bool wzrd_handle_is_released_tree(WzWidget handle)
{
	int stack[1024];
	int count = 0;
	int ptr = 0;

	stack[count++] = wz_widget_get(handle)->handle.handle;

	while (ptr != count)
	{
		WzWidgetData* box = &wz->widgets[stack[ptr]];

		for (int i = 0; i < box->children_count; ++i)
		{
			stack[count++] = box->children[i].handle;
		}

		if (wzrd_box_is_released(box))
		{
			return true;
		}

		ptr++;
	}

	return false;
}

int wz_compare_widgets(const void* element1, const void* element2) {
	WzWidget index1 = *(WzWidget*)element1;
	WzWidget index2 = *(WzWidget*)element2;
	WzWidgetData* c1 = wz_widget_get(index1);
	WzWidgetData* c2 = wz_widget_get(index2);

	if (c1->z > c2->z)
	{
		return 1;
	}

	if (c1->z < c2->z)
	{
		return -1;
	}

	if (c1->z == c2->z)
	{
#if 0
		if (c1->bring_to_front && !c2->bring_to_front)
		{
			return 1;
		}
		else if (!c1->bring_to_front && c2->bring_to_front)
		{
			return -1;
		}
#endif
#if 0
		if (wzrd_handle_is_child_of_handle(c2->handle, c1->handle))
		{
			return 1;
		}
		else if (wzrd_handle_is_child_of_handle(c1->handle, c2->handle))
		{
			return -1;
		}
#endif

		if (index1.handle > index2.handle)
		{
			return 1;
		}
		else if (index1.handle < index2.handle)
		{
			return -1;
		}
	}

	return 0;
}

int wzrd_float_compare(float a, float b)
{
	if (fabs(a - b) < FLT_EPSILON)
	{
		return 0;
	}
	if (a - b > 0)
	{
		return 1;
	}
	else if (a - b < 0)
	{
		return -1;
	}

	return 0;
}

void wz_widget_set_flex_factor(WzWidget widget, unsigned int flex_factor)
{
	wz_widget_get(widget)->flex_factor = flex_factor;
}

void wz_widget_set_expanded(WzWidget widget)
{
	WzWidgetData* widg = wz_widget_get(widget);
	widg->flex_factor = 1;
	widg->flex_fit = WZ_FLEX_FIT_TIGHT;
}

void wz_widget_set_flex(WzWidget widget)
{
	WzWidgetData* widg = wz_widget_get(widget);
	widg->flex_factor = 1;
	widg->flex_fit = WZ_FLEX_FIT_LOOSE;
}

void wz_widget_set_free_from_parent(WzWidget w)
{
	wz_widget_get(w)->free_from_parent = true;
}

void wz_widget_add_child(WzWidget parent, WzWidget child)
{
	if (!parent.handle) return;

	WzWidgetData* p = wz_widget_get(parent);
	WzWidgetData* c = wz_widget_get(child);
	wz_assert(p->children_count < MAX_NUM_CHILDREN - 1);
	for (unsigned i = 0; i < p->children_count; ++i)
	{
		wz_assert(c->handle.handle != p->children[i].handle);
	}

	p->children[p->children_count++] = c->handle;
	c->z = p->z;
	c->clip_widget = p->clip_widget;
	c->parent = parent;
}

WzWidgetData wz_widget_create(WzWidget parent)
{
	WzWidgetData box = { 0 };
	box.children_count = 0;
	box.free_children_count = 0;
	box.items_count = 0;
	box.color = 0xc0c0c0ff;
	box.font_color = WZ_BLACK;
	box.percentage_h = 0;
	box.percentage_w = 0;
	box.cross_axis_alignment = 0;
	box.best_fit = false;
	box.bring_to_front = false;
	box.child_gap = 0;
	box.fit_h = false;
	box.fit_w = false;
	box.margin_left = box.margin_right = box.margin_top = box.margin_bottom = 0;
	box.clip_widget.handle = 0;
	box.w_offset = 0;
	box.h_offset = 0;
	box.disable_hover = false;
	box.z = 0;
	box.layout = WZ_LAYOUT_NONE;
	box.flex_factor = 0;
	box.min_w = box.min_h = 0;
	box.constraint_max_w = box.constraint_max_h = UINT_MAX;
	box.x = box.y = 0;
	box.free_from_parent = 0;
	box.cull = false;
	box.source[0] = 0;
	box.tag = 0;
	//box.z = EguiGetCurrentWindow()->z;
	box.scale[0] = 1;
	box.scale[1] = 1;
	box.border_type = WZ_BORDER_TYPE_NONE;
	box.min_h = 50;
	box.min_w = 50;

	// TODO: Paddin for border. Remove!
	box.margin_top = 2;
	box.margin_bottom = 2;
	box.margin_left = 2;
	box.margin_right = 2;

	return box;
}

WzWidget wz_widget_add_to_frame(WzWidget parent, WzWidgetData widget)
{
	widget.handle = wz_create_handle();
	wz->widgets[widget.handle.handle] = widget;

	wz_widget_add_child(parent, widget.handle);

	return widget.handle;
}

WzWidget wz_widget_persistent(WzWidget parent, WzWidgetData widget_data)
{
	WzWidget widget = wz_widget_add_to_frame(parent, widget_data);

	return widget;
}

void wz_widget_add_to_layout_chunk(unsigned layout_chunk)
{

}

WzWidget wz_widget_raw(WzWidget parent, const char* file, unsigned int line)
{
	WzWidgetData box = wz_widget_create(parent);

	// New for layouting
		// TODO: Handle the case where we are increasing the stride but the next chunk is occupied
	WzWidgetData* parent_data = wz_widget_get(parent);
	WzChunkLayout* layout = &wz->layouts[parent_data->layout_chunk];
	WzChunk* chunk = &wz->chunks[layout->chunk + layout->chunk_stride];
	box.slot = chunk->count++;
	box.chunk = layout->chunk + layout->chunk_stride;
	layout->child_count++;

	if (chunk->count == WZ_CHUNK_SIZE)
	{
		layout->chunk_stride++;
		wz->chunks_count++;
	}

	WzWidget widget = wz_widget_add_to_frame(parent, box);
	wz_widget_add_source(widget, file, line);
	wz_widget_set_color(widget, 0xc0c0c0ff);
	wz->widgets_count++;

	chunk->widget_index[box.slot] = widget.handle;

	return widget;
}

WzWidget wz_vbox_raw(WzWidget parent, const char* file, unsigned int line)
{
	WzWidget p = wz_widget_raw(parent, file, line);
	wz_widget_set_layout(p, WZ_LAYOUT_VERTICAL);

	return p;
}

WzWidget wz_hbox_raw(WzWidget parent, const char* file, unsigned int line)
{
	WzWidget p = wz_widget_raw(parent, file, line);
	wz_widget_set_layout(p, WZ_LAYOUT_HORIZONTAL);

	return p;
}

void wz_widget_add_rect(WzWidget widget, unsigned int w, unsigned int h, unsigned int color)
{
	WzWidgetItem item = { 0 };
	item.w = w;
	item.h = h;
	item.type = WZ_ITEM_TYPE_RECT;
	item.color = color;
	item.margin_left = item.margin_right = item.margin_bottom = item.margin_top = 0;
	item.center_h = item.center_w = true;

	wz_widget_add_item(widget, item);
}

// No longer needed - colors are already unsigned int
// Keeping for backwards compatibility
unsigned int wz_convert_color(unsigned int color)
{
	return color;
}

void wz_widget_add_rect_new(WzWidget widget, float x, float y, float w, float h, unsigned color)
{
	WzWidgetItem item = { 0 };
	item.x = x;
	item.y = y;
	item.w = w;
	item.h = h;
	item.type = WZ_ITEM_TYPE_RECT;
	item.color = wz_convert_color(color);
	item.margin_left = item.margin_right = item.margin_bottom = item.margin_top = 0;

	wz_widget_add_item(widget, item);
}

void wz_widget_add_rect_absolute(WzWidget widget, int x, int y, unsigned w, unsigned h, unsigned int color)
{
	WzWidgetItem item;
	item.w = w;
	item.h = h;
	item.x = x;
	item.y = y;
	item.type = ItemType_RectAbsolute;
	item.color = color;
	item.margin_left = item.margin_right = item.margin_bottom = item.margin_top = 0;
	item.center_h = item.center_w = true;

	wz_widget_add_item(widget, item);
}

void wz_widget_add_texture(WzWidget parent, WzTexture texture, unsigned w, unsigned h) {

	WzWidgetItem item;
	item.type = ItemType_Texture;
	item.size = (wzrd_v2){ w, h };
	item.val.texture = texture;
	item.margin_left = item.margin_right = item.margin_bottom = item.margin_top = 0;
	item.center_h = item.center_w = true;
	item.w = w;
	item.h = h;

	wz_widget_add_item(parent, item);
}


#if 0
void wz_log_error(WzLogMessage* arr, unsigned int* count, const char* fmt, ...)
{
	WzLogMessage message;
	message.str[0] = 0;
	va_list args;
	va_start(args, fmt);
	//vsprintf_s(message.str, WZ_LOG_MESSAGE_MAX_SIZE, fmt, args);
	va_end(args);

	arr[*count] = message;
	*count = *count + 1;
}

#endif

wz_set_gui(WzGui* gui_in)
{
	wz = gui_in;
}

void wz_widget_set_type(WzWidget widget, unsigned type)
{
	wz_widget_get(widget)->type = type;
}

void wz_set_string_size_callback(void (*get_string_size)(char*, unsigned, unsigned, unsigned, float*, float*))
{
	wz->get_string_size = get_string_size;
}

void wz_widget_set_font(WzWidget widget, unsigned font_id)
{
	wz_widget_get(widget)->font_id = font_id;
}

WzWidget wz_begin(
	unsigned window_w, unsigned  window_h,
	//float mouse_x, float mouse_y,
	//WzState left_mouse_state,
	WzEvent* events,
	unsigned events_count,
	bool enable_input)
{
	memset(wz->occupied, 0, sizeof(*wz->occupied) * MAX_NUM_WIDGETS);
	for (unsigned i = 0; i < wz->persistent_widgets_count; ++i)
	{
		//wz->occupied[wz->persistent_widgets[i].handle.handle] = false;
	}

	memcpy(wz->events, events, events_count * sizeof(*wz->events));
	wz->events_count = events_count;


	//memset(wz->keyboard.keys, 0, sizeof(WzState) * 512);


	// Keyboard state advancement
	for (unsigned i = 0; i < 128; i++)
	{
		if (wz->keyboard.keys[i] == WZ_ACTIVATING)
			wz->keyboard.keys[i] = WZ_ACTIVE;
		else if (wz->keyboard.keys[i] == WZ_DEACTIVATING)
			wz->keyboard.keys[i] = WZ_INACTIVE;
	}

	// Mouse Input
	if (1)
	{
		if (wz->mouse_left == WZ_ACTIVATING)
		{
			wz->mouse_left = WZ_ACTIVE;
		}
		else if (wz->mouse_left == WZ_DEACTIVATING)
		{
			wz->mouse_left = WZ_INACTIVE;
		}

#if 0
		//if (mflags & SDL_BUTTON_LMASK)
		{
			if (wz->mouse_left == WZ_INACTIVE)
			{
				wz->mouse_left = WZ_ACTIVATING;
			}
		}
		//else
		{
			if (wz->mouse_left == WZ_ACTIVE)
			{
				wz->mouse_left = WZ_DEACTIVATING;
			}
		}
#endif
	}

	for (unsigned i = 0; i < events_count; ++i)
	{
		if (events[i].type == WZ_EVENT_TYPE_KEYBOARD)
		{
			WzKeyboardEvent key = events[i].key;
			if (key.down)
			{
				wz->keyboard.keys[key.key] = WZ_ACTIVATING;
			}
		}
		else if (events[i].type == WZ_EVENT_TYPE_MOUSE)
		{
			if (events[i].button.button == 1)
			{
				if (events[i].button.down)
				{
					wz->mouse_left = WZ_ACTIVATING;
				}
				else
				{
					wz->mouse_left = WZ_DEACTIVATING;
				}
			}

		}
	}

	//wz->mouse_left = left_mouse_state;
	//wz->mouse_pos = (wzrd_v2f){ mouse_x, mouse_y };
	wz->mouse_delta.x = wz->mouse_pos.x - wz->previous_mouse_pos.x;
	wz->mouse_delta.y = wz->mouse_pos.y - wz->previous_mouse_pos.y;

	wz->window = (WzRect){ 0, 0, window_w, window_h };
	wz->enable_input = enable_input;
	wz->styles_count = 0;
	wz->current_crate_index = 0;

	WZRD_UNUSED(wz);

	// Empty box
	//gui->widgets_count = 0;
	//gui->widgets[gui->widgets_count++] = (WzWidgetData){ 0 };

	// Window

	wz->scrollbars_count = 0;
	wz->scenes_count = 0;

	wz->layouts[0] = (WzChunkLayout){ .child_gap = 5 };
	wz->chunks_count = 1;
	wz->widgets_count = 1;

	{
		wz->layouts_count = 1;
		wz->layouts[0].available_width = wz->window.w;
		wz->layouts[0].available_height = wz->window.h;
	}
}

void wz_draw_rect(WzDrawCommand* out_command,
	WzRect rect, unsigned int color,
	int z, char* source)
{
	*out_command = (WzDrawCommand){
		.type = DrawCommandType_Rect,
		.color = color,
		.z = z,
		.x = rect.x,
		.y = rect.y,
		.w = rect.w,
		.h = rect.h,
		.source = source
	};

	wz_layout_log("Rect command: (id: %s) (rect: %u %u %u %u)  (color: %u)\n",
		source,
		rect.x, rect.y, rect.w, rect.h,
		color);
}

WzWidgetData* wzrd_box_get_previous() {
	//WzWidgetData* result = &gui->widgets[gui->widgets_count - 1];

	//return result;
}

void wzrd_handle_cursor()
{
#if 0
	canvas->cursor = wzrd_cursor_default;

	WzWidget* hot_box = wz_widget_get(canvas->hovered_item);
	WzWidget* active_box = wz_widget_get(canvas->active_item);

	if (canvas->hovered_item.handle) {
		if (hot_box->type == wzrd_box_type_button) {
			canvas->cursor = wzrd_cursor_hand;
		}
	}
	if (canvas->active_item.handle) {
		if (active_box->type == wzrd_box_type_button) {
			canvas->cursor = wzrd_cursor_hand;
		}
	}
#endif
}

void wzrd_handle_border_resize()
{
	wz->left_resized_item = (WzWidget){ 0 };
	wz->right_resized_item = (WzWidget){ 0 };
	wz->top_resized_item = (WzWidget){ 0 };
	wz->bottom_resized_item = (WzWidget){ 0 };

	for (int i = 0; i < MAX_NUM_WIDGETS; ++i) {
		WzWidgetData* owner = wz->widgets + i;
		for (int j = 0; j < owner->children_count; ++j) {
			WzWidgetData* child = &wz->widgets[owner->children[j].handle];

			int border_size = 10;

			bool is_inside_left_border =
				wz->mouse_pos.x >= child->actual_x &&
				wz->mouse_pos.y >= child->actual_y &&
				wz->mouse_pos.x < child->actual_x + border_size &&
				wz->mouse_pos.y < child->actual_y + child->actual_h;
			bool is_inside_right_border =
				wz->mouse_pos.x >= child->actual_x + child->actual_w - border_size &&
				wz->mouse_pos.y >= child->actual_y &&
				wz->mouse_pos.x < child->actual_x + child->actual_w &&
				wz->mouse_pos.y < child->actual_y + child->actual_h;
			bool is_inside_top_border =
				wz->mouse_pos.x >= child->actual_x &&
				wz->mouse_pos.y >= child->actual_y &&
				wz->mouse_pos.x < child->actual_x + child->actual_w &&
				wz->mouse_pos.y < child->actual_y + border_size;
			bool is_inside_bottom_border =
				wz->mouse_pos.x >= child->actual_x &&
				wz->mouse_pos.y >= child->actual_y + child->actual_h - border_size &&
				wz->mouse_pos.x < child->actual_x + child->actual_w &&
				wz->mouse_pos.y < child->actual_y + child->actual_h;

			if (wz_widget_is_equal(wz->hovered_item, child->handle) || wz_widget_is_equal(wz->active_item, child->handle))
			{
				if (is_inside_top_border || is_inside_bottom_border)
				{
					//child->color = WZ_BLUE;
					//*cursor = wzrd_cursor_vertical_arrow;
				}
				else if (is_inside_left_border || is_inside_right_border)
				{
					//child->color = WZ_BLUE;
					//*cursor = wzrd_cursor_horizontal_arrow;
				}
			}

			if (wz_widget_is_equal(wz->active_item, child->handle)) {
				if (is_inside_bottom_border) {
					//child->color = EGUI_PURPLE;
					wz->bottom_resized_item = child->handle;
				}
				else if (is_inside_top_border) {
					//child->color = EGUI_PURPLE;
					wz->top_resized_item = child->handle;
				}
				else if (is_inside_left_border) {
					//child->color = EGUI_PURPLE;
					wz->left_resized_item = child->handle;
				}
				else if (is_inside_right_border) {
					//child->color = EGUI_PURPLE;
					wz->right_resized_item = child->handle;
				}
			}
		}
	}
}

bool wzrd_handle_is_active(WzWidget handle) {
	if (wz_widget_is_equal(handle, wz->active_item)) {
		return true;
	}

	return false;
}

bool wzrd_handle_is_released(WzWidget handle) {
	if (wz_widget_is_equal(handle, wz->deactivating_item)) {
		return true;
	}

	return false;
}

bool wzrd_handle_is_hovered(WzWidget handle) {
	if (wz_widget_is_equal(handle, wz->hovered_item)) {
		return true;
	}

	return false;
}


void wz_draw_rect_new(float x, float y, float w, float h, unsigned color)
{
	wz->commands_buffer.commands[wz->commands_buffer.count++] = (WzDrawCommand){
			.type = DrawCommandType_Rect,
			.x = x,
			.y = y,
			.w = w,
			.h = h,
			.color = color,
	};
}



bool wzrd_handle_is_hovered_from_list(WzWidget handle)
{

	for (int i = 0; i < wz->hovered_items_list_count; ++i)
	{
		if (wz_widget_is_equal(handle, wz->hovered_items_list[i]))
		{
			return true;
		}
	}

	return false;
}

bool wzrd_handle_is_interacting(WzWidget handle) {
	if (wz_widget_is_equal(handle, wz->activating_item) ||
		wz_widget_is_equal(handle, wz->active_item) ||
		wz_widget_is_equal(handle, wz->deactivating_item))
	{
		return true;
	}

	return false;
}



void wz_input(int* indices, int count)
{
	WzWidgetData* hovered_box = wz->widgets;
	unsigned int max_z = 0;
	wz->hovered_items_list_count = 0;
	wz->hovered_boxes_count = 0;
	for (int i = 0; i < count; ++i) {
		WzWidgetData* box = wz->widgets + indices[i];

		if (!wz->occupied[indices[i]])
		{
			continue;
		}

		WzRect scaled_rect = { box->actual_x, box->actual_y, box->actual_w, box->actual_h };

#if 0
		// Input Clipping: Block mouse from reaching clipped widgets
		if (wz_handle_is_valid(box->clip_widget))
		{
			WzWidgetData* clip_widget = wz_widget_get(box->clip_widget);

			if (box->actual_x < clip_widget->actual_w)
			{
				y			scaled_rect.x = clip_widget->actual_w;
			}

			if (box->actual_y < clip_widget->actual_y)
			{
				scaled_rect.y = clip_widget->actual_y;
			}

			if (box->actual_x + box->actual_w > clip_widget->actual_w + clip_widget->actual_w)
			{
				scaled_rect.x = clip_widget->actual_w;
			}

			if (box->actual_y + box->actual_h > clip_widget->actual_y + clip_widget->actual_h)
			{
				scaled_rect.y = clip_widget->actual_h;
			}
		}
#endif

		bool is_hover = false;
		{
			// Clamp rect size to some arbitrary number in case it's uint_max
			float w, h;
			if (scaled_rect.w < 8000)
			{
				w = scaled_rect.w;
			}
			else
			{
				w = 8000;
			}

			if (scaled_rect.h < 8000)
			{
				h = scaled_rect.h;
			}
			else
			{
				h = 8000;
			}

			if (wz->mouse_pos.x >= scaled_rect.x &&
				wz->mouse_pos.x <= scaled_rect.x + w &&
				wz->mouse_pos.y >= scaled_rect.y &&
				wz->mouse_pos.y <= scaled_rect.y + h)
			{
				is_hover = true;
			}
		}

		if (is_hover && !box->disable_hover)
		{
			wz_assert(wz->hovered_items_list_count < MAX_NUM_WIDGETS);
			wz->hovered_items_list[wz->hovered_items_list_count++] = box->handle;
			wz->hovered_boxes[wz->hovered_boxes_count++] = *box;

			if (box->z >= max_z)
			{
				hovered_box = box;
				max_z = box->z;
			}
			else if (box->z == max_z)
			{
				if (box->bring_to_front)
				{
					hovered_box = box;
				}
			}

		}
	}

	// ...
	WzWidgetData* half_clicked_box = wz_widget_get(wz->activating_item);
	if (half_clicked_box && (wz->mouse_left == WZ_ACTIVE || wz->mouse_left == WZ_DEACTIVATING))
	{
		wz->activating_item = (WzWidget){ 0 };
	}

	if (wz->mouse_left == WZ_DEACTIVATING)
	{
		wz->dragged_box = (WzWidgetData){ 0 };
		wz->dragged_item = (WzWidget){ 0 };

		wz->clean = false;
	}

	if (wz->mouse_left == WZ_INACTIVE)
	{
		wz->deactivating_item = (WzWidget){ 0 };
	}

	if (wz_handle_is_valid(hovered_box->handle))
	{
		wz->hovered_item = hovered_box->handle;
	}
	else {
		wz->hovered_item = (WzWidget){ 0 };
	}

	WzWidgetData* hot_box = wz_widget_get(wz->hovered_item);
	WzWidgetData* active_box = wz_widget_get(wz->active_item);

	if (wz_handle_is_valid(active_box->handle))
	{
		if (wz->mouse_left == WZ_DEACTIVATING)
		{
			if (wz_handle_is_valid(wz->active_item))
			{
				if (hot_box == active_box)
				{
					wz->clicked_item = active_box->handle;
					//canvas->selected_item = hot_box->handle;
				}

				wz->deactivating_item = wz->active_item;
				wz->active_item = (WzWidget){ 0 };
			}
		}
	}

	if (wz_handle_is_valid(hot_box->handle)) {

		/*	if (hot_box->type == wzrd_box_type_flat_button)
			{
				hot_box->color = (wzrd_color){ 0, 255, 255, 255 };
			}*/
		if (wz->mouse_left == WZ_ACTIVATING) {
			wz->active_item = hot_box->handle;

			wz->activating_item = hot_box->handle;

			// Dragging
			wz_assert(half_clicked_box);
			if (half_clicked_box->is_draggable) {
				wz->dragged_item = half_clicked_box->handle;

				wz->dragged_box = *half_clicked_box;
				//g_gui->dragged_box.x = g_gui->dragged_box.x;
				//g_gui->dragged_box.y = g_gui->dragged_box.y;
				//g_gui->dragged_box.w = g_gui->dragged_box.w;
				//g_gui->dragged_box.h = g_gui->dragged_box.h;
				//g_gui->dragged_box.name = wzrd_str_create("drag");
				//g_gui->dragged_box.absolute_rect = (wzrd_rect){ 0 };
				wz->dragged_box.disable_hover = true;
			}
		}
	}

	// Clicked item
	if (wz_handle_is_valid(wz->clicked_item) && wz->mouse_left == WZ_INACTIVE) {
		wz->clicked_item = (WzWidget){ 0 };

		wz->clean = false;
	}

	// Input box
	if (wz_handle_is_valid(wz->clicked_item)) {
		WzWidgetData* clicked_box = wz_widget_get(wz->clicked_item);
		wz_assert(clicked_box);
		/*	if (clicked_box->type == wzrd_box_type_input_box) {
				canvas->active_input_box = clicked_box->handle;
			}
			else {
				canvas->active_input_box = (wzrd_handle){ 0 };
			}*/
	}
}

bool wz_widget_is_activating(WzWidget handle) {
	if (wz_widget_is_equal(handle, wz->activating_item)) {
		return true;
	}

	return false;
}

bool wz_is_any_widget_activating()
{
	if (wz->activating_item.handle != 0)
	{
		return true;
	}

	return false;
}

bool wz_widget_is_active(WzWidget handle) {
	if (wz_widget_is_equal(handle, wz->active_item)) {
		return true;
	}

	return false;
}

double clamp(double d, double min, double max) {
	const double t = d < min ? min : d;
	double result = t > max ? max : t;

	return result;
}

void wz_slider_run(WzWidget parent)
{
	WzWidget slider = wz_widget_get(parent)->slider;
	float* slider_pos_relative = wz_widget_get(parent)->slider_pos;
	WzWidgetData* slider_parent = wz_widget_get(parent);
	WzWidgetData* slider_data = wz_widget_get(slider);

	if (wz_widget_is_active(slider))
	{

		float pos = (float)slider_parent->actual_x + *slider_pos_relative + wz->mouse_delta.x;

		pos = clamp(pos, slider_parent->actual_x,
			slider_parent->actual_x + slider_parent->actual_w - slider_data->actual_w);

		if ((wz->mouse_pos.x < slider_parent->actual_x && wz->mouse_delta.x < 0) ||
			(wz->mouse_pos.x >= slider_parent->actual_x + slider_parent->actual_w && wz->mouse_delta.x > 0) ||
			(wz->mouse_pos.x >= slider_parent->actual_x && wz->mouse_pos.x <
				slider_parent->actual_x + slider_parent->actual_w))
		{
			*slider_pos_relative = pos - (float)slider_parent->actual_x;
		}

		printf("%f\n", wz->mouse_delta.x);

		//if (pos > slider_parent->actual_x &&
		//	pos + slider_data->actual_w <= slider_parent->actual_x + slider_parent->actual_w)
		//{
		//	*slider_pos_relative = *slider_pos_relative + wz->mouse_delta.x;
		//}

	}

	float slider_pos = slider_parent->actual_x + *slider_pos_relative;
}

void wz_slider(WzWidget parent, unsigned width, float* pos)
{
	WzWidget widget = wz_hbox(parent);
	wz_widget_set_size(widget, width, 20);
	WzWidgetData* d = wz_widget_get(widget);
	d->type = WZ_WIDGET_TYPE_SLIDER;
	d->slider_pos = pos;


	//wz_widget_add_rect_new(widget, width, 1, WZ_BLACK);

#if 1
	WzWidget handle = wz_widget(widget);
	wz_widget_set_size(handle, 15, 15);
	//wz_widget_set_pos(handle, (*pos) * width, 0);
	wz_widget_set_pos(handle, (int)*pos, 0);
	wz_widget_set_border(handle, WZ_BORDER_TYPE_DEFAULT);

	d->slider = handle;

#endif
}

bool wz_widget_is_interacting(WzWidget handle)
{
	bool result = false;
	if (wz_widget_is_activating(handle) || wz_widget_is_active(handle))
	{
		result = true;
	}

	return result;
}

void wzrd_box_bring_to_front(WzWidgetData* box, void* data)
{
	(void)data;
	box->bring_to_front = true;
}

void wz_widget_set_stretch_factor(WzWidget handle, unsigned int flex_factor)
{
	WzWidgetData* w = wz_widget_get(handle);
	w->flex_factor = flex_factor;
}

void wz_widget_set_main_axis_size_min(WzWidget w)
{
	WzWidgetData* d = wz_widget_get(w);
	d->main_axis_size_type = MAIN_AXIS_SIZE_TYPE_MIN;
}

void wz_widget_set_layout(WzWidget handle, unsigned int layout_type)
{
	WzWidgetData* d = wz_widget_get(handle);
	d->layout = layout_type;

	//d->flex_fit = WZ_FLEX_FIT_TIGHT;
	//d->main_axis_size_type = MAIN_AXIS_SIZE_TYPE_MAX;

	unsigned chunk_idx = wz->chunks_count++;
	unsigned layout_idx = wz->layouts_count++;
	WzChunkLayout layout =
	{
		.parent_chunk = d->chunk,
		.parent_slot = d->slot,
		.chunk = chunk_idx,
		.child_gap = 5,
		.layout_type = layout_type,
		.pad_bottom = 5, .pad_left = 5, .pad_top = 5, .pad_right = 5
	};

	wz->layouts[layout_idx] = layout;
	d->layout_chunk = layout_idx;

}

void wz_draw_text(WzDrawCommand* out_command, const char* string, WzRect rect)
{
	*out_command = (WzDrawCommand){
		.type = WZ_DRAW_COMMAND_TYPE_TEXT,
		.str = wz_str_create(string),
		.x = rect.x,
		.y = rect.y,
		.w = rect.w,
		.h = rect.h,
		.color = WZ_BLACK,
		.z = 10
	};
}

void wz_widget_set_max_constraints(WzWidget widget, unsigned int w, unsigned int h)
{
	WzWidgetData* d = wz_widget_get(widget);
	d->constraint_max_w = w;
	d->constraint_max_h = h;
}

void wz_widget_set_constraints(WzWidget widget,
	unsigned int min_w, unsigned int min_h, unsigned int max_w, unsigned int max_h)
{
	WzWidgetData* data = wz_widget_get(widget);
	data->min_w = min_w;
	data->min_h = min_h;
	data->constraint_max_w = max_w;
	data->constraint_max_h = max_h;
}

void wz_draw(WzWidget* boxes_indices)
{
	WzWidgetData* widget;
	WzDrawCommandBuffer* buffer = &wz->commands_buffer;
	buffer->count = 0;
	unsigned int line_size;
	WzWidget current_clip_widget;
	WzWidgetItem item;

	current_clip_widget.handle = 0;

	for (int i = 0; i < wz->widgets_count; ++i)
	{
		widget = wz_widget_get(boxes_indices[i]);

		// Clip widget
		if (0)
		{
			if (wz_handle_is_valid(widget->clip_widget))
			{
				if (!wz_widget_is_equal(current_clip_widget, widget->clip_widget))
				{
					WzWidgetData* clip_box = wz_widget_get(widget->clip_widget);

					WzRect clip_rect;
					clip_rect.x = clip_box->actual_x + clip_box->pad_left;
					clip_rect.y = clip_box->actual_y + clip_box->pad_top;
					clip_rect.w = clip_box->actual_w -
						clip_box->pad_left - clip_box->pad_right;
					clip_rect.h = clip_box->actual_h -
						clip_box->pad_top - clip_box->pad_bottom;

					if (!clip_rect.w ||
						!clip_rect.h ||
						!(clip_rect.w > (widget->margin_left + widget->margin_right)) ||
						!(clip_rect.h > (widget->margin_top + widget->margin_bottom)))
					{
						//printf("clip space too small for widget (%s %u)\n", widget->file, widget->line);
						printf("erorr!\n");
						continue;
					}

					// Append clip item
					wz->commands_buffer.commands[wz->commands_buffer.count++] = (WzDrawCommand){
						.type = DrawCommandType_Clip,
						.x = clip_rect.x,
						.y = clip_rect.y,
						.w = clip_rect.w,
						.h = clip_rect.h,
					};
				}
			}
			else
			{
				wz->commands_buffer.commands[wz->commands_buffer.count++] =
					(WzDrawCommand){ .type = DrawCommandType_StopClip };
			}

			current_clip_widget = widget->clip_widget;
		}

		if (widget->cull)
		{
			//continue;
		}
#if 1

		if (widget->actual_w <= 2 || widget->actual_h <= 2)
		{
			continue;
		}

		// Draw Widget background
		if (wz->focused_widget_unique_id && widget->unique_id == wz->focused_widget_unique_id)
		{
			wz_draw_rect_new(widget->actual_x, widget->actual_y,
				widget->actual_w, widget->actual_h, WZ_RED);
		}
		else
		{
			wz_assert(buffer->count < MAX_NUM_DRAW_COMMANDS - 1);
			wz_draw_rect(&buffer->commands[buffer->count],
				(WzRect) {
				.x = widget->actual_x,
					.y = widget->actual_y,
					.w = widget->actual_w,
					.h = widget->actual_h,
			},
				widget->color,
				widget->z,
				widget->source);
			buffer->commands[buffer->count].widget_index = widget->handle.handle;
			buffer->count++;
		}

		// Borders
		if (!(widget->actual_w <= 2 || widget->actual_h <= 2)) {
			if (widget->border_type != WZ_BORDER_TYPE_NONE) {
				//WZ_ASSERT(widget->actual_w >= 4);
				//WZ_ASSERT(widget->actual_h >= 4);
			}

			line_size = WZ_BORDER_SIZE;

			WzRect top0 = (WzRect){ widget->actual_x, widget->actual_y, widget->actual_w - line_size, line_size };
			WzRect left0 = (WzRect){ widget->actual_x, widget->actual_y, line_size, widget->actual_h };
			WzRect top1 = (WzRect){ widget->actual_x + line_size, widget->actual_y + line_size, widget->actual_w - 3 * line_size, line_size };
			WzRect left1 = (WzRect){ widget->actual_x + line_size, widget->actual_y + line_size, line_size, widget->actual_h - line_size };
			WzRect bottom0 = (WzRect){ widget->actual_x, widget->actual_y + widget->actual_h - line_size, widget->actual_w, line_size };
			WzRect right0 = (WzRect){ widget->actual_x + widget->actual_w - line_size, widget->actual_y, line_size, widget->actual_h };
			WzRect bottom1 = (WzRect){ widget->actual_x + 1 * line_size, widget->actual_y + widget->actual_h - 2 * line_size, widget->actual_w - 3 * line_size, line_size };
			WzRect right1 = (WzRect){ widget->actual_x + widget->actual_w - 2 * line_size, widget->actual_y + 1 * line_size, line_size, widget->actual_h - 2 * line_size };

			wzrd_is_rect_inside_rect(top0, (WzRect) { widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h });
			wzrd_is_rect_inside_rect(left0, (WzRect) { widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h });
			wzrd_is_rect_inside_rect(top1, (WzRect) { widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h });
			wzrd_is_rect_inside_rect(left1, (WzRect) { widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h });
			wzrd_is_rect_inside_rect(bottom0, (WzRect) { widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h });
			wzrd_is_rect_inside_rect(right0, (WzRect) { widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h });
			wzrd_is_rect_inside_rect(bottom1, (WzRect) { widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h });
			wzrd_is_rect_inside_rect(right1, (WzRect) { widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h });

			if (widget->border_type == WZ_BORDER_TYPE_DEFAULT) {
				// Draw top and left lines
				wz_draw_rect_new(top0.x, top0.y, top0.w, top0.h, EGUI_WHITE2);
				wz_draw_rect_new(left0.x, left0.y, left0.w, left0.h, EGUI_WHITE2);
				wz_draw_rect_new(top1.x, top1.y, top1.w, top1.h, WZ_LIGHTGRAY);
				wz_draw_rect_new(left1.x, left1.y, left1.w, left1.h, WZ_LIGHTGRAY);

				// Draw bottom and right lines
				wz_draw_rect_new(bottom0.x, bottom0.y, bottom0.w, bottom0.h, WZ_BLACK);
				wz_draw_rect_new(right0.x, right0.y, right0.w, right0.h, WZ_BLACK);
				wz_draw_rect_new(bottom1.x, bottom1.y, bottom1.w, bottom1.h, EGUI_GRAY);
				wz_draw_rect_new(right1.x, right1.y, right1.w, right1.h, EGUI_GRAY);
			}
			else if (widget->border_type == WZ_BORDER_TYPE_RED) {
				// Draw top and left lines
				wz_draw_rect_new(top0.x, top0.y, top0.w, top0.h, WZ_RED);
				wz_draw_rect_new(left0.x, left0.y, left0.w, left0.h, WZ_RED);
				wz_draw_rect_new(top1.x, top1.y, top1.w, top1.h, WZ_RED);
				wz_draw_rect_new(left1.x, left1.y, left1.w, left1.h, WZ_RED);

				// Draw bottom and right lines
				wz_draw_rect_new(bottom0.x, bottom0.y, bottom0.w, bottom0.h, WZ_RED);
				wz_draw_rect_new(right0.x, right0.y, right0.w, right0.h, WZ_RED);
				wz_draw_rect_new(bottom1.x, bottom1.y, bottom1.w, bottom1.h, WZ_RED);
				wz_draw_rect_new(right1.x, right1.y, right1.w, right1.h, WZ_RED);
			}
			else if (widget->border_type == WZ_BORDER_TYPE_TAB) {
				right0.y += 2;
				right0.h -= 2;
				left0.y += 2;
				left0.h -= 2;
				top0.x += 2;
				top0.w -= 3;

				// Draw top and left lines
				wz_draw_rect_new(top0.x, top0.y, top0.w, top0.h, EGUI_WHITE2);
				wz_draw_rect_new(left0.x, left0.y, left0.w, left0.h, EGUI_WHITE2);
				wz_draw_rect_new(top1.x, top1.y, top1.w, top1.h, WZ_LIGHTGRAY);
				wz_draw_rect_new(left1.x, left1.y, left1.w, left1.h, WZ_LIGHTGRAY);

				// Draw bottom and right lines
				wz_draw_rect_new(right0.x, right0.y, right0.w, right0.h, WZ_BLACK);
				wz_draw_rect_new(right1.x, right1.y, right1.w, right1.h, EGUI_GRAY);
				wz_draw_rect_new(bottom1.x, bottom1.y, bottom1.w, bottom1.h, 0);
				wz_draw_rect_new(bottom0.x, bottom0.y, bottom0.w, bottom0.h, 0);

				// ...
				wz_draw_rect_new(top1.x + top1.w, top1.y, 1, 1, 0X000000FF | (unsigned)(0.5 * WZ_BLACK + 0.5 * WZ_LIGHTGRAY));
				wz_draw_rect_new(top1.x, top1.y, 1, 1, EGUI_WHITE2);
			}
			else if (widget->border_type == WZ_BORDER_TYPE_CLICKED || widget->border_type == WZ_BORDER_TYPE_TEXT_BOX) {
				// Draw top and left lines
				wz_draw_rect_new(top0.x, top0.y, top0.w, top0.h, WZ_BLACK);
				wz_draw_rect_new(left0.x, left0.y, left0.w, left0.h, WZ_BLACK);
				wz_draw_rect_new(top1.x, top1.y, top1.w, top1.h, EGUI_GRAY);
				wz_draw_rect_new(left1.x, left1.y, left1.w, left1.h, EGUI_GRAY);

				// Draw bottom and right lines
				wz_draw_rect_new(bottom0.x, bottom0.y, bottom0.w, bottom0.h, EGUI_WHITE2);
				wz_draw_rect_new(right0.x, right0.y, right0.w, right0.h, EGUI_WHITE2);
				wz_draw_rect_new(bottom1.x, bottom1.y, bottom1.w, bottom1.h, EGUI_LIGHTESTGRAY);
				wz_draw_rect_new(right1.x, right1.y, right1.w, right1.h, EGUI_LIGHTESTGRAY);
			}
#if 0
			else if (widget->border_type == WZ_BORDER_TYPE_TEXT_BOX) {
				// Draw top and left lines
				wz_draw_rect_new(top0.x, top0.y, top0.w, top0.h, EGUI_GRAY);
				wz_draw_rect_new(left0.x, left0.y, left0.w, left0.h, EGUI_GRAY);
				wz_draw_rect_new(top1.x, top1.y, top1.w, top1.h, WZ_BLACK);
				wz_draw_rect_new(left1.x, left1.y, left1.w, left1.h, WZ_BLACK);

				// Draw bottom and right lines
				wz_draw_rect_new(bottom0.x, bottom0.y, bottom0.w, bottom0.h, EGUI_WHITE2);
				wz_draw_rect_new(right0.x, right0.y, right0.w, right0.h, EGUI_WHITE2);
				wz_draw_rect_new(bottom1.x, bottom1.y, bottom1.w, bottom1.h, EGUI_LIGHTESTGRAY);
				wz_draw_rect_new(right1.x, right1.y, right1.w, right1.h, EGUI_LIGHTESTGRAY);
			}
#endif
			else if (widget->border_type == WZ_BORDER_TYPE_BLACK) {
#if 1
				// Draw top and left lines
				(widget->handle, left0.x, left0.y, left0.w, left0.h, WZ_BLACK);
				wz_draw_rect_new(left0.x, left0.y, left0.w, left0.h, WZ_BLACK);
				wz_draw_rect_new(top0.x, top0.y, top0.w, top0.h, WZ_BLACK);
				wz_draw_rect_new(top1.x, top1.y, top1.w, top1.h, WZ_BLACK);
				wz_draw_rect_new(left1.x, left1.y, left1.w, left1.h, WZ_BLACK);

				// Draw bottom and right lines
				wz_draw_rect_new(bottom0.x, bottom0.y, bottom0.w, bottom0.h, WZ_BLACK);
				wz_draw_rect_new(right0.x, right0.y, right0.w, right0.h, WZ_BLACK);
				wz_draw_rect_new(bottom1.x, bottom1.y, bottom1.w, bottom1.h, WZ_BLACK);
				wz_draw_rect_new(right1.x, right1.y, right1.w, right1.h, WZ_BLACK);
#endif
			}
			else if (widget->border_type == WZ_BORDER_TYPE_BOTTOM_LINE) {
				wz_draw_rect_new(bottom0.x, bottom0.y, bottom0.w, bottom0.h, EGUI_WHITE2);
				wz_draw_rect_new(bottom1.x, bottom1.y, bottom1.w, bottom1.h, EGUI_GRAY);
			}
			else if (widget->border_type == BorderType_LeftLine) {
				wz_draw_rect_new(left0.x, left0.y, left0.w, left0.h, EGUI_GRAY);
				wz_draw_rect_new(left1.x, left1.y, left1.w, left1.h, EGUI_WHITE2);
			}
		}

		// Draw content
		int content_start = buffer->count;

		// Clip content
		//if (widget->clip_content)
		if (0)
		{
			WzWidgetData* clip_box = widget;

			WzRect clip_rect;
			clip_rect.x = clip_box->actual_x + clip_box->pad_left;
			clip_rect.y = clip_box->actual_y + clip_box->pad_top;
			clip_rect.w = clip_box->actual_w -
				clip_box->pad_left - clip_box->pad_right;
			clip_rect.h = clip_box->actual_h -
				clip_box->pad_top - clip_box->pad_bottom;

			if (!clip_rect.w ||
				!clip_rect.h ||
				!(clip_rect.w > (widget->margin_left + widget->margin_right)) ||
				!(clip_rect.h > (widget->margin_top + widget->margin_bottom)))
			{
				//printf("clip space too small for widget (%s %u)\n", widget->file, widget->line);
				printf("erorr!\n");
				continue;
			}

			// Append clip item
			wz->commands_buffer.commands[wz->commands_buffer.count++] = (WzDrawCommand){
				.type = DrawCommandType_Clip,
				.x = clip_rect.x,
				.y = clip_rect.y,
				.w = clip_rect.w,
				.h = clip_rect.h,
			};

			current_clip_widget = widget->handle;
		}

		for (int j = 0; j < widget->items_count; ++j)
		{
			wz_assert(buffer->count < MAX_NUM_DRAW_COMMANDS - 1);
			item = widget->items[j];

			if (item.size.x == 0)
				item.size.x = widget->actual_w;
			if (item.size.y == 0)
				item.size.y = widget->actual_h;

			WzRect item_dest_rect = (WzRect)
			{
				widget->actual_x + item.x,
				widget->actual_y + item.y,
				item.w,
				item.h
			};

			if (item.center_w)
			{
				item_dest_rect.x = widget->actual_x + (widget->actual_w - item.w - widget->pad_left - widget->pad_right) / 2;
			}
			if (item.center_h)
			{
				item_dest_rect.y = widget->actual_y + (widget->actual_h - item.h - widget->pad_bottom - widget->pad_top) / 2;
			}

			// String item
			if (item.type == WZ_WIDGET_ITEM_TYPE_STRING)
			{
				float w, h;
				wz->get_string_size(item.val.str.str, 0, strlen(item.val.str.str), item.font_id, &w, &h);

				item_dest_rect.w = w;
				item_dest_rect.h = h;

				if (widget->text_alignment == WZ_TEXT_ALIGNMENT_LEFT)
				{
					item_dest_rect.x = widget->actual_x + widget->pad_left + item.x;
					item_dest_rect.y = widget->actual_y + widget->pad_right + item.y;
				}
				else if (widget->text_alignment == WZ_TEXT_ALIGNMENT_CENTER)
				{
					item_dest_rect.x = widget->actual_x + widget->actual_w / 2 - w / 2;
					item_dest_rect.y = widget->actual_y + widget->actual_h / 2 - h / 2;
				}

				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = WZ_DRAW_COMMAND_TYPE_TEXT,
					.str = item.val.str,
					.x = item_dest_rect.x,
					.y = item_dest_rect.y,
					.w = item_dest_rect.w,
					.h = item_dest_rect.h,
					.color = widget->font_color,
					.font_id = item.font_id,
					.z = widget->z
				};
			}

			// Texture item
			if (item.type == ItemType_Texture)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_Texture,
					.rotation_angle = widget->rotation,
					.x = widget->actual_x,
					.y = widget->actual_y,
					.w = widget->actual_w,
					.h = widget->actual_h,
					.src_rect = (WzRect) {0, 0, item.val.texture.w, item.val.texture.h},
					.texture = item.val.texture,
					.z = widget->z
				};
			}

			// Rect item
			if (item.type == WZ_ITEM_TYPE_RECT)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_Rect,
					.x = item_dest_rect.x,
					.y = item_dest_rect.y,
					.w = item_dest_rect.w,
					.h = item_dest_rect.h,
					.color = item.color,
					.z = widget->z
				};

			}
			else if (item.type == ItemType_RectAbsolute)
			{

				item_dest_rect.x = item.x;
				item_dest_rect.y = item.y;
				item_dest_rect.w = item.w;
				item_dest_rect.h = item.h;

				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_Rect,
					.x = item_dest_rect.x,
					.y = item_dest_rect.y,
					.w = item_dest_rect.w,
					.h = item_dest_rect.h,
					.color = item.color,
					.z = widget->z
				};

				wz_assert(buffer->commands[buffer->count - 1].w > 0);
				wz_assert(buffer->commands[buffer->count - 1].h > 0);
			}

			// Line items
			if (item.type == ItemType_Line) {
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_Line,
					.x = widget->actual_w + item.val.rect.x,
					.y = widget->actual_y + item.val.rect.y,
					.w = widget->actual_w + item.val.rect.w,
					.h = widget->actual_y + item.val.rect.h,
					.color = item.color,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_HorizontalDottedLine)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_HorizontalLine,
					.x = item_dest_rect.x,
					.y = item_dest_rect.y,
					.w = item_dest_rect.w,
					.h = item_dest_rect.h,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_LeftHorizontalDottedLine)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_HorizontalLine,
					.x = widget->actual_x,
					.y = widget->actual_y + widget->actual_h / 2,
					.w = widget->actual_x + widget->actual_w / 2,
					.h = widget->actual_y + widget->actual_h / 2,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_RightHorizontalDottedLine)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_HorizontalLine,
					.x = widget->actual_x + widget->actual_w / 2,
					.y = widget->actual_y + widget->actual_h / 2,
					.w = widget->actual_x + widget->actual_w,
					.h = widget->actual_y + widget->actual_h / 2,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_VerticalDottedLine)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_VerticalLine,
					.x = item_dest_rect.x,
					.y = item_dest_rect.y,
					.w = item_dest_rect.w,
					.h = item_dest_rect.h,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_VerticalLine)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_VerticalLine,
					.x = item_dest_rect.x,
					.y = item_dest_rect.y,
					.w = item_dest_rect.w,
					.h = item_dest_rect.h,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_LineAbsolute)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_Line,
					.line = item.val.line,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_DottedLineAbsolute)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_LineDotted,
					.line = item.val.line,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_HorizontalLine)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_HorizontalLine,
					.x = item_dest_rect.x,
					.y = item_dest_rect.y,
					.w = item_dest_rect.w,
					.h = item_dest_rect.h,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_TopVerticalDottedLine)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_VerticalLine,
					.x = widget->actual_w + widget->actual_w / 2,
					.y = widget->actual_y,
					.w = widget->actual_w + widget->actual_w / 2 + widget->actual_w,
					.h = widget->actual_y + widget->actual_h / 2,
					.z = widget->z
				};
			}
			else if (item.type == ItemType_BottomVerticalDottedLine)
			{
				buffer->commands[buffer->count++] = (WzDrawCommand){
					.type = DrawCommandType_VerticalLine,
					.x = widget->actual_w + widget->actual_w / 2,
					.y = widget->actual_y + widget->actual_h / 2,
					.w = widget->actual_w + widget->actual_w / 2,
					.h = widget->actual_y + widget->actual_h,
					.z = widget->z
				};
			}

			// Apply item margins
			buffer->commands[buffer->count - 1].x += item.margin_left;
			buffer->commands[buffer->count - 1].y += item.margin_top;

			wz_assert(buffer->commands[buffer->count - 1].w >= 0);
			wz_assert(buffer->commands[buffer->count - 1].h >= 0);
		}



		// Apply transformations to content commands in-place
		if (0) {
			WzWidgetData* current_widget = &widget;

			for (int cmd_idx = content_start; cmd_idx < buffer->count; ++cmd_idx)
			{
				WzDrawCommand* cmd = &buffer->commands[cmd_idx];

				cmd->source = widget->source;

				// Scale
				cmd->w *= current_widget->world_scale[0];
				cmd->h *= current_widget->world_scale[1];

				// Rotate position
				float x = cmd->x;
				float y = cmd->y;
				cmd->x = x * cos(current_widget->world_rotation) -
					y * sin(current_widget->world_rotation);
				cmd->y = x * sin(current_widget->world_rotation) +
					y * cos(current_widget->world_rotation);

				// Translate
				cmd->x += current_widget->world_pos[0];
				cmd->y += current_widget->world_pos[1];

#if 1
				// Apply camera offset (for all widgets)
				cmd->x -= wz->camera_x;
				cmd->y -= wz->camera_y;

				// Apply zoom (for all widgets)
				cmd->x *= (1 + wz->zoom_factor);
				cmd->y *= (1 + wz->zoom_factor);
				cmd->w *= (1 + wz->zoom_factor);
				cmd->h *= (1 + wz->zoom_factor);
#endif
			}
		}

#endif
	}
}

void wz_widget_clip(WzWidget handle)
{
	WzWidgetData* box = wz_widget_get(handle);
	box->clip_widget = handle;
}

WzWidgetData wzrd_widget_get_cached_box_with_secondary_tag(const char* tag, const char* secondary_tag)
{
	for (int i = 0; i < wz->cached_boxes_count; ++i)
	{
		if (wz->cached_boxes[i].tag == tag && wz->cached_boxes[i].secondary_tag == secondary_tag)
		{
			return wz->cached_boxes[i];
		}
	}

	return (WzWidgetData) { 0 };
}

WzWidgetData wzrd_widget_get_cached_box(const char* tag)
{
	for (int i = 0; i < wz->cached_boxes_count; ++i)
	{
		if (wz->cached_boxes[i].tag == tag && !wz->cached_boxes[i].secondary_tag)
		{
			return wz->cached_boxes[i];
		}
	}

	return (WzWidgetData) { 0 };
}

void wz_widget_add_tag(WzWidget parent, const void* tag)
{
	wz_widget_get(parent)->tag = tag;
}

WzWidgetData* wz_widget_get_parent(WzWidget widget)
{
	return wz_widget_get(wz_widget_get(widget)->parent);
}

void wz_widget_resize(WzWidget handle, int* w_offset, int* h_offset)
{
	WzWidgetData* w = wz_widget_get(handle);
	WzWidgetData* parent = wz_widget_get(w->parent);

	if (parent->layout == WZ_LAYOUT_HORIZONTAL)
	{
		if (wz_widget_is_equal(w->handle, wz->right_resized_item)) {
			*w_offset += wz->mouse_delta.x;
		}
	}
	else {
		if (wz_widget_is_equal(w->handle, wz->bottom_resized_item)) {
			*h_offset += wz->mouse_delta.y;
		}
	}

	w->w_offset = *w_offset;
	w->h_offset = *h_offset;
}

#define SCROLLBAR_SIZE 20

WzWidget wz_scroll_box(wzrd_v2 size, unsigned int* scroll, WzWidget parent, const void* tag)
{
	WzWidget panel = wz_hbox(parent);
	wz_widget_set_size(panel, size.x, size.y);
	wz_widget_set_color_old(panel, EGUI_BEIGE);
	wz_widget_set_main_axis_size_min(panel);
	wz_widget_set_cross_axis_alignment(panel, WZ_CROSS_AXIS_ALIGNMENT_STRETCH);

	WzWidget clip_panel = wz_widget(panel);
	wz_widget_clip(clip_panel);
	wz_widget_set_flex(clip_panel);

	WzWidget user_panel = wz_vbox(clip_panel);
	wz_widget_set_color(user_panel, 0x00ffffff);
	wz_widget_add_tag(user_panel, tag);
	wz_widget_set_flex(user_panel);
	wz_widget_set_cross_axis_alignment(user_panel, WZ_CROSS_AXIS_ALIGNMENT_STRETCH);

	WzWidget c = wz_widget(user_panel);
	wz_widget_set_max_h(c, 150);
	wz_widget_set_color(c, 0x0000ffff);

	WzWidget scrollbar_panel = wz_vbox(panel);
	wz_widget_set_max_w(scrollbar_panel, SCROLLBAR_SIZE + 4);
	wz_widget_set_color(scrollbar_panel, 0xFF00FFFF);
	wz_widget_set_cross_axis_alignment(scrollbar_panel, WZ_CROSS_AXIS_ALIGNMENT_STRETCH);

	WzWidget top_button = wz_widget(scrollbar_panel);
	wz_widget_set_size(top_button, SCROLLBAR_SIZE, SCROLLBAR_SIZE);
	wz_widget_set_border(top_button, WZ_BORDER_TYPE_DEFAULT);

	WzWidget middle_panel = wz_widget(scrollbar_panel);
	wz_widget_get(middle_panel)->color = EGUI_GRAY;
	wz_widget_get(middle_panel)->tag = tag;
	wz_widget_get(middle_panel)->secondary_tag = "scrollbar_middle";
	wz_widget_set_flex(middle_panel);
	wz_widget_set_border(middle_panel, WZ_BORDER_TYPE_DEFAULT);

	WzWidget bottom_button = wz_widget(scrollbar_panel);
	wz_widget_set_size(bottom_button, SCROLLBAR_SIZE, SCROLLBAR_SIZE);
	wz_widget_set_border(bottom_button, WZ_BORDER_TYPE_DEFAULT);

	WzScrollbar* wz_scrollbar = &wz->scrollbars[wz->scrollbars_count];
	wz_scrollbar->content = user_panel;
	wz_scrollbar->scrollbar = middle_panel;
	wz_scrollbar->panel = panel;
	wz_scrollbar->top_button = top_button;
	wz_scrollbar->bottom_button = bottom_button;
	wz_scrollbar->scroll = scroll;
	wz->scrollbars_count++;

#if 0

	//#if 0
	WzWidgetData b = wzrd_widget_get_cached_box(tag);
	WzWidget scrollbar = (WzWidget){ 0 };
	//if (wz_handle_is_valid(b.handle))
	{
		//WzWidgetData* box = &b;
	/*	float ratio = (float)b.actual_h / (float)b.content_h;
		if (ratio > 1)
			ratio = 1;*/

			//scrollbar = wz_hbox(middle_panel);
			//wz_widget_get(scrollbar)->percentage_h = ratio;
		wz_widget_get(scrollbar)->y = *scroll;

		//wz_widget_get(scrollbar)->tag = tag;
		//wz_widget_get(scrollbar)->secondary_tag = "scrollbar";
	}

	WzWidgetData scrollbar_middle_box = wzrd_widget_get_cached_box_with_secondary_tag(tag, "scrollbar_middle");
	WzWidgetData scrollbar_box = wzrd_widget_get_cached_box_with_secondary_tag(tag, "scrollbar");

	if (wzrd_widget_is_active(top_button))
	{
		if (*scroll > 0)
			*scroll -= 1;
	}

	if (wzrd_widget_is_active(bottom_button))
	{
		if (*scroll + scrollbar_box.actual_h < scrollbar_middle_box.actual_h - 2)
			*scroll += 1;
	}

	if (wzrd_widget_is_active(scrollbar))
	{
		int scroll_temp = *scroll + wzrd_canvas_get()->mouse_delta.y;

		if (scroll_temp + scrollbar_box.actual_h < scrollbar_middle_box.actual_h - 2 && scroll_temp > 0)
		{
			*scroll = scroll_temp;
		}
	}
#endif

	return user_panel;
}

void wz_handle_input()
{
	// Mouse interaction
	if (wz->enable_input)
	{
		wz_input(wz->boxes_indices, MAX_NUM_WIDGETS);
		wzrd_handle_cursor();
		wzrd_handle_border_resize();
	}
	else
	{
		if (wz->mouse_left == WZ_DEACTIVATING)
		{
			//canvas->released_item = canvas->dragged_item;
			//canvas->released_item = canvas->dragged_item;
			wz->dragged_box = (WzWidgetData){ 0 };
			wz->dragged_item = (WzWidget){ 0 };
			wz->active_item = (WzWidget){ 0 };
		}
	}

	wz->previous_mouse_pos = wz->mouse_pos;

	// Keyboard
	for (unsigned i = 0; i < wz->events_count; ++i)
	{
		WzEvent* event = &wz->events[i];
		bool handled = false;
		int stb_key = 0;

		if (event->key.type != WZ_EVENT_TYPE_KEYBOARD)
		{
			continue;
		}

		if ((unsigned)event->key.key < 128)
		{
			if (event->key.down)
				wz->keyboard.keys[(unsigned)event->key.key] = WZ_ACTIVATING;
			else
				wz->keyboard.keys[(unsigned)event->key.key] = WZ_DEACTIVATING;
		}

		if (!event->key.down)
		{
			continue;
		}

		switch (event->key.key)
		{
		case '\t':
		{
			break;
		}
		}
	}


}

#define WZ_TREE_ROW_HEIGHT 32
#define WZ_TREE_BOX_SIZE 32

void wz_widget_ignore_unique_id(WzWidget widget)
{
	wz_widget_get(widget)->ignore_unique_id = true;
}

void wz_widget_add_horizontal_dotted_line(WzWidget widget, unsigned w)
{
	WzWidgetItem item;
	item.type = ItemType_HorizontalDottedLine;
	item.size.x = 20;
	item.margin_bottom = item.margin_top = item.margin_left = item.margin_right = 0;
	item.center_h = item.center_w = true;
	item.w = w;
	item.h = 1;
	wz_widget_add_item(widget, item);
}

void wz_widget_add_vertical_dotted_line(WzWidget widget, unsigned h)
{
	WzWidgetItem item;
	item.type = ItemType_VerticalDottedLine;
	item.margin_bottom = item.margin_top = item.margin_left = item.margin_right = 0;
	item.center_h = item.center_w = true;
	item.w = 1;
	item.h = h;
	wz_widget_add_item(widget, item);
}

void wz_widget_add_horizontal_line(WzWidget widget, unsigned w)
{
	WzWidgetItem item;
	item.type = ItemType_HorizontalLine;
	item.size.x = 20;
	item.margin_bottom = item.margin_top = item.margin_left = item.margin_right = 0;
	item.center_h = item.center_w = true;
	item.w = w;
	item.h = 1;
	wz_widget_add_item(widget, item);
}

void wz_widget_add_vertical_line(WzWidget widget, unsigned h)
{
	WzWidgetItem item;
	item.type = ItemType_VerticalLine;
	item.margin_bottom = item.margin_top = item.margin_left = item.margin_right = 0;
	item.center_h = item.center_w = true;
	item.w = 1;
	item.h = h;
	wz_widget_add_item(widget, item);
}

void wz_widget_add_line_absolute(WzWidget widget, int x0, int y0, int x1, int y1)
{
	WzWidgetItem item;
	item.type = ItemType_LineAbsolute;
	item.margin_bottom = item.margin_top = item.margin_left = item.margin_right = 0;
	item.center_h = item.center_w = true;
	item.val.line.x0 = x0;
	item.val.line.y0 = y0;
	item.val.line.x1 = x1;
	item.val.line.y1 = y1;
	wz_widget_add_item(widget, item);
}

void wz_widget_add_dotted_line_absolute(WzWidget widget, int x0, int y0, int x1, int y1)
{
	WzWidgetItem item;
	item.type = ItemType_DottedLineAbsolute;
	item.margin_bottom = item.margin_top = item.margin_left = item.margin_right = 0;
	item.center_h = item.center_w = true;
	item.val.line.x0 = x0;
	item.val.line.y0 = y0;
	item.val.line.x1 = x1;
	item.val.line.y1 = y1;
	wz_widget_add_item(widget, item);
}

void wz_widget_add_horizontal_line_absolute(WzWidget widget, unsigned h)
{
	WzWidgetItem item;
	item.type = ItemType_HorizontalLineAbsolute;
	item.margin_bottom = item.margin_top = item.margin_left = item.margin_right = 0;
	item.center_h = item.center_w = true;
	item.w = 1;
	item.h = h;
	wz_widget_add_item(widget, item);
}

void wz_widget_add_content_margins(WzWidget widget, unsigned left, unsigned right, unsigned top, unsigned bottom)
{
	WzWidgetData* d = wz_widget_get(widget);
	d->constraint_max_h += top + bottom;
	d->constraint_max_w += left + right;
}

void wz_widget_set_cross_axis_alignment(WzWidget widget, unsigned int cross_axis_alignment)
{
	wz_widget_get(widget)->cross_axis_alignment = cross_axis_alignment;
}

void wz_widget_set_main_axis_alignment(WzWidget widget, unsigned int main_axis_alignment)
{
	wz_widget_get(widget)->main_axis_alignment = main_axis_alignment;
}

void wz_widget_add_content_margin_left(WzWidget widget, unsigned left)
{
	WzWidgetData* d = wz_widget_get(widget);
	d->constraint_max_w += left;
}

void wz_widget_add_content_margin_right(WzWidget widget, unsigned right)
{
	WzWidgetData* d = wz_widget_get(widget);
	d->constraint_max_w += right;
}

void wz_widget_get_all_children(WzWidget widget, WzWidget* children, unsigned* children_count)
{
	WzWidget stack[1024];
	int stack_count = 0;
	*children_count = 0;
	stack[stack_count++] = widget;

	while (stack_count > 0)
	{
		WzWidgetData* box = wz_widget_get(stack[stack_count - 1]);
		stack_count--;

		for (int i = 0; i < box->children_count; ++i)
		{
			WzWidget child = wz->widgets[box->children[i].handle].handle;
			stack[stack_count++] = child;
			children[*children_count] = child;
			*children_count = *children_count + 1;
		}
	}
}
WzWidget wz_tree_add_row_raw(WzTree* tree, WzStr str, WzTexture texture, unsigned depth, bool* expand, bool* selected, WzTreeNodeData* node, const char* file, unsigned line)
{
	const toggle_size = 20;
	const icon_size = 20;

	WzWidget row = wz_hbox(tree->menu);
	wz_widget_add_source(row, file, line);
	wz_widget_set_cross_axis_alignment(row, WZ_CROSS_AXIS_ALIGNMENT_CENTER);
	wz_widget_set_margins(row, 5);

	WzWidget box = wz_widget(row);
	wz_widget_set_max_constraints(box, depth * (icon_size + 10), icon_size);
	wz_widget_ignore_unique_id(box);

	WzWidget toggle;
	if (node->children_count)
	{
		toggle = wz_toggle(row, toggle_size, toggle_size, 0x00000000, expand);
		wz_widget_add_source(toggle, file, line);
		wz_widget_add_rect(toggle, toggle_size - 2, toggle_size - 2, WZ_WHITE);
		wz_widget_set_border(toggle, WZ_BORDER_TYPE_BLACK);
	}
	else
	{
		toggle = wz_widget(row);
		wz_widget_set_size(toggle, toggle_size, toggle_size);
		wz_widget_add_source(toggle, file, line);
	}

	WzWidget hline = wz_widget(row);
	wz_widget_set_max_constraints(hline, 10, 10);
	wz_widget_add_source(hline, file, line);

	WzWidget icon = wz_texture(row, texture, icon_size, icon_size);
	wz_widget_add_source(icon, file, line);

	WzWidget label = wz_label(row, str);
	wz_widget_add_source(label, file, line);
	wz_widget_add_content_margin_left(label, 5);
	wz_widget_add_content_margin_right(label, 5);

	WzWidget children[16];
	unsigned children_count;
	wz_widget_get_all_children(row, children, &children_count);

	*selected = false;
	*selected |= wz_widget_is_deactivating(row);

	for (unsigned i = 0; i < children_count; ++i)
	{
		if (wz_widget_is_deactivating(children[i]))
		{
			*selected |= true;
		}
	}

	node->row_widget = row;
	node->icon_widget = icon;
	node->expand_widget = toggle;

	return row;
}

WzTreeNodeData* wz_tree_get_node(WzTree* tree, WzTreeNode node)
{
	return &tree->nodes[node.index];
}

WzStr wz_str_create_slice(const char* str, unsigned begin, unsigned end)
{
	WzStr result = { 0 };
	memcpy(result.str, str + begin, end);
	result.len = end - begin;

	return result;
}

static void wz_update_scroll_window(WzWidget widget, WzInputState* input_state)
{
	float pad_x = (float)wz->widgets[widget.handle].pad_left;
	float box_w = wz->widgets[widget.handle].actual_w - pad_x * 2.0f;
	float text_up_to_cursor_w, text_h;
	wz->get_string_size(input_state->buffer, 0,
		input_state->textedit_state.cursor, wz->widgets[widget.handle].font_id, &text_up_to_cursor_w, &text_h);
	float window_start = (float)input_state->offset_x;
	float window_end = window_start + box_w;
	if (text_up_to_cursor_w <= window_start)
	{
		input_state->offset_x = (text_up_to_cursor_w > 0 ? text_up_to_cursor_w : 0);
	}
	else if (text_up_to_cursor_w >= window_end)
	{
		input_state->offset_x = (text_up_to_cursor_w - box_w + 1);
	}
}

void wz_text_box_run(WzWidget widget, WzInputState* input_state)
{
	WzWidgetData* wdata = &wz->widgets[widget.handle];
	unsigned      flags = wdata->input_flags;
	input_state->font_id = wdata->font_id;
	bool moving_cursor = false;

	bool focused = wz_widget_is_equal(wz->active_input, widget);

	static unsigned   click_count = 0;
	static int   anchor_word_start = 0;
	static int   anchor_word_end = 0;

	float pad_x = (float)wz->widgets[widget.handle].pad_left;
	float pad_y = (float)wz->widgets[widget.handle].pad_top;

	float y = 0;

	float origin_x = wz->widgets[widget.handle].actual_x + pad_x - (float)input_state->offset_x;
	float origin_y = wz->widgets[widget.handle].actual_y + y + pad_y;

	// Mouse coordinates relative to text area, accounting for scroll
	float rel_x = wz->mouse_pos.x - origin_x;
	float rel_y = wz->mouse_pos.y - origin_y;
	float text_x = rel_x; // scroll-adjusted x for STB

	if (wz_widget_is_activating(widget))
	{
		// Init stb state
		if (!input_state->textedit_state.initialized)
		{
			stb_textedit_initialize_state(&input_state->textedit_state, 1);
		}

		// Count clicks
		// TODO: SDL supports double clicks
		{
			unsigned long t = wz->ticks_in_ms();
			unsigned long delta_time = t - input_state->time_since_click;

			if (delta_time < DOUBLE_CLICK_TIME_MS)
			{
				click_count++;
			}
			else
			{
				click_count = 1;
			}

			input_state->time_since_click = wz->ticks_in_ms();
		}

		// ...
		{
			wz->active_input = widget;
			focused = true;
		}

		if (!(flags & WZ_INPUT_READ_ONLY))
		{
			int cursor = input_state->textedit_state.cursor;
			stb_textedit_click(input_state, &input_state->textedit_state,
				text_x, rel_y);

			if (cursor != input_state->textedit_state.cursor)
			{
				moving_cursor = true;
				input_state->time_since_blink = wz->ticks_in_ms();
			}

			if (click_count == 1)
			{
#if 1
				if (flags & WZ_INPUT_AUTO_SELECT)
				{
					stb_textedit_key(input_state, &input_state->textedit_state, STB_TEXTEDIT_K_TEXTSTART);
					stb_textedit_key(input_state, &input_state->textedit_state, STB_TEXTEDIT_K_TEXTEND | STB_TEXTEDIT_K_SHIFT);
				}
				else if (flags & WZ_INPUT_GOTO_END)
				{
					stb_textedit_key(input_state, &input_state->textedit_state, STB_TEXTEDIT_K_TEXTEND);
				}
#endif
			}
			if (click_count == 2)
			{
#if 1
				int cur = input_state->textedit_state.cursor;
				anchor_word_start = wz_input_word_start(input_state->buffer, cur);
				anchor_word_end = wz_input_word_end(input_state->buffer,
					input_state->length, cur);
				stb_textedit_key(input_state,
					&input_state->textedit_state, STB_TEXTEDIT_K_WORDLEFT);
				stb_textedit_key(input_state,
					&input_state->textedit_state,
					STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
#endif
			}
			else if (click_count >= 3)
			{
				click_count = 1;
				// stb_textedit_click (above) already placed the cursor and cleared the selection
			}

		}

		wz_update_scroll_window(widget, input_state);
	}
	else if (wz_widget_is_active(widget) && !(flags & WZ_INPUT_READ_ONLY))
	{
		if (click_count == 1)
		{
			stb_textedit_drag(input_state, &input_state->textedit_state,
				text_x, rel_y);
		}
		else if (click_count == 2)
		{
#if 0
			int drag_char = wz_input_char_at_x(wz, input_state->buffer, input_state->length, text_x, wz->widgets[widget.handle].font_id);
			STB_TexteditState* ts = &input_state->textedit_state;
			if (drag_char <= anchor_word_start)
			{
				int ws = wz_input_word_start(input_state->buffer, drag_char);
				ts->select_start = anchor_word_end;
				ts->select_end = ws;
				ts->cursor = ws;
			}
			else
			{
				int we = wz_input_word_end(input_state->buffer, input_state->length, drag_char);
				ts->select_start = anchor_word_start;
				ts->select_end = we;
				ts->cursor = we;
			}
#endif
		}

		// click_count >= 3: no drag, all-text selection stays
		wz_update_scroll_window(widget, input_state);
	}
	else if (wz->mouse_left == WZ_ACTIVATING)
	{
		if (focused)
		{
			focused = false;
			wz->active_input = (WzWidget){ 0 };
		}
	}

	//if (focused)
	{
		wz_widget_set_color(widget, WZ_RAYWHITE);
	}

	if (focused && !(flags & WZ_INPUT_READ_ONLY))
	{
		for (unsigned i = 0; i < wz->events_count; ++i)
		{
			WzEvent* event = &wz->events[i];
			bool handled = false;
			int stb_key = 0;

			if (event->key.type != WZ_EVENT_TYPE_KEYBOARD || !event->key.down)
			{
				continue;
			}

			switch (event->key.key)
			{
			case '\b': // Backspace / Ctrl+Backspace (delete word left)
				if (event->key.mod & WZ_KMOD_CTRL)
					stb_textedit_key(input_state, &input_state->textedit_state,
						STB_TEXTEDIT_K_WORDLEFT | STB_TEXTEDIT_K_SHIFT);
				stb_key = STB_TEXTEDIT_K_BACKSPACE;
				handled = true;
				moving_cursor = true;
				break;
			case WZ_KEY_DELETE: // Delete / Ctrl+Delete / Shift+Delete (cut)
				if ((event->key.mod & WZ_KMOD_SHIFT) && !(flags & WZ_INPUT_PASSWORD))
				{
					int sel_start = input_state->textedit_state.select_start;
					int sel_end = input_state->textedit_state.select_end;
					if (sel_start > sel_end) { int tmp = sel_start; sel_start = sel_end; sel_end = tmp; }
					int len = sel_end - sel_start;
					if (len > 0 && len < 128)
					{
						memcpy(wz->copied_text, input_state->buffer + sel_start, len);
						wz->copied_text[len] = '\0';
						stb_textedit_key(input_state, &input_state->textedit_state, STB_TEXTEDIT_K_DELETE);
					}
					handled = true;
					break;
				}
				if (event->key.mod & WZ_KMOD_CTRL)
					stb_textedit_key(input_state, &input_state->textedit_state, STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
				stb_key = STB_TEXTEDIT_K_DELETE;
				handled = true;
				moving_cursor = true;
				break;
			case WZ_KEY_INSERT: // Shift+Insert = paste, otherwise toggle insert/replace mode
				if ((event->key.mod & WZ_KMOD_SHIFT) && wz->pasted_text)
				{
					stb_textedit_paste(input_state, &input_state->textedit_state,
						wz->pasted_text, strlen(wz->pasted_text));
					SDL_free(wz->pasted_text);
					wz->pasted_text = NULL;
					handled = true;
					break;
				}
				stb_key = STB_TEXTEDIT_K_INSERT;
				handled = true;
				break;
			case WZ_KEY_LEFT:
				stb_key = (event->key.mod & WZ_KMOD_CTRL) ? STB_TEXTEDIT_K_WORDLEFT : STB_TEXTEDIT_K_LEFT;
				if (event->key.mod & WZ_KMOD_SHIFT) stb_key |= STB_TEXTEDIT_K_SHIFT;
				handled = true;
				moving_cursor = true;
				break;
			case WZ_KEY_RIGHT:
				stb_key = (event->key.mod & WZ_KMOD_CTRL) ? STB_TEXTEDIT_K_WORDRIGHT : STB_TEXTEDIT_K_RIGHT;
				if (event->key.mod & WZ_KMOD_SHIFT) stb_key |= STB_TEXTEDIT_K_SHIFT;
				handled = true;
				moving_cursor = true;
				break;
			case WZ_KEY_UP:
				stb_key = STB_TEXTEDIT_K_UP;
				if (event->key.mod & WZ_KMOD_SHIFT) stb_key |= STB_TEXTEDIT_K_SHIFT;
				handled = true;
				break;
			case WZ_KEY_DOWN:
				stb_key = STB_TEXTEDIT_K_DOWN;
				if (event->key.mod & WZ_KMOD_SHIFT) stb_key |= STB_TEXTEDIT_K_SHIFT;
				handled = true;
				break;
			case WZ_KEY_HOME:
				stb_key = (event->key.mod & WZ_KMOD_CTRL) ? STB_TEXTEDIT_K_TEXTSTART : STB_TEXTEDIT_K_LINESTART;
				if (event->key.mod & WZ_KMOD_SHIFT) stb_key |= STB_TEXTEDIT_K_SHIFT;
				handled = true;
				moving_cursor = true;
				break;
			case WZ_KEY_END:
				stb_key = (event->key.mod & WZ_KMOD_CTRL) ? STB_TEXTEDIT_K_TEXTEND : STB_TEXTEDIT_K_LINEEND;
				if (event->key.mod & WZ_KMOD_SHIFT) stb_key |= STB_TEXTEDIT_K_SHIFT;
				handled = true;
				moving_cursor = true;
				break;
			case 27: // Escape — lose focus
				wz->active_input = (WzWidget){ 0 };
				focused = false;
				handled = true;
				break;
			case '\r': // Enter — commit
			case '\n':
				if (wdata->input_committed) *wdata->input_committed = true;
				handled = true;
				break;
			case 'a':
				if (event->key.mod & WZ_KMOD_CTRL)
				{
					stb_textedit_key(input_state, &input_state->textedit_state, STB_TEXTEDIT_K_TEXTSTART);
					stb_textedit_key(input_state, &input_state->textedit_state, STB_TEXTEDIT_K_TEXTEND | STB_TEXTEDIT_K_SHIFT);
					handled = true;
					break;
				}
				goto insert_char;
			case '\t': // Tab
				if (flags & WZ_INPUT_ALLOW_TAB)
				{
					char tab = '\t';
					stb_textedit_paste(input_state, &input_state->textedit_state, &tab, 1);
				}
				handled = true; // always consume tab
				break;
			case 'c':
				if ((event->key.mod & WZ_KMOD_CTRL) && !(flags & WZ_INPUT_PASSWORD))
				{
					int sel_start = input_state->textedit_state.select_start;
					int sel_end = input_state->textedit_state.select_end;
					if (sel_start > sel_end) { int tmp = sel_start; sel_start = sel_end; sel_end = tmp; }
					int len = sel_end - sel_start;
					if (len > 0 && len < 128)
					{
						memcpy(wz->copied_text, input_state->buffer + sel_start, len);
						wz->copied_text[len] = '\0';
					}
					handled = true;
					break;
				}
				goto insert_char;
			case 'x':
				if ((event->key.mod & WZ_KMOD_CTRL) && !(flags & WZ_INPUT_PASSWORD))
				{
					int sel_start = input_state->textedit_state.select_start;
					int sel_end = input_state->textedit_state.select_end;
					if (sel_start > sel_end) { int tmp = sel_start; sel_start = sel_end; sel_end = tmp; }
					int len = sel_end - sel_start;
					if (len > 0 && len < 128)
					{
						memcpy(wz->copied_text, input_state->buffer + sel_start, len);
						wz->copied_text[len] = '\0';
						stb_textedit_key(input_state, &input_state->textedit_state, STB_TEXTEDIT_K_DELETE);
					}
					handled = true;
					break;
				}
				goto insert_char;
			case 'v':
				if (event->key.mod & WZ_KMOD_CTRL)
				{
					if (wz->pasted_text)
					{
						stb_textedit_paste(input_state, &input_state->textedit_state,
							wz->pasted_text, strlen(wz->pasted_text));
						SDL_free(wz->pasted_text);
						wz->pasted_text = NULL;
					}
					handled = true;
					break;
				}
				goto insert_char;
			case 'z':
				if (event->key.mod & WZ_KMOD_CTRL)
				{
					stb_key = STB_TEXTEDIT_K_UNDO;
					handled = true;
					moving_cursor = true;
					break;
				}
				goto insert_char;
			case 'Z': // Ctrl+Shift+Z = redo
				if (event->key.mod & WZ_KMOD_CTRL)
				{
					stb_key = STB_TEXTEDIT_K_REDO;
					handled = true;
					moving_cursor = true;
					break;
				}
				goto insert_char;
			case 'y':
				if (event->key.mod & WZ_KMOD_CTRL)
				{
					stb_key = STB_TEXTEDIT_K_REDO;
					handled = true;
					moving_cursor = true;
					break;
				}
				goto insert_char;
			insert_char:
			default:
				if (!(event->key.mod & WZ_KMOD_CTRL) &&
					(isprint((int)event->key.key) || event->key.key == ' '))
				{
					char c = (char)event->key.key;

					stb_textedit_paste(input_state, &input_state->textedit_state, &c, 1);

					handled = true;
				}
				break;
			}

			if (handled)
			{
				if (stb_key)
				{
					stb_textedit_key(input_state, &input_state->textedit_state, stb_key);
				}

				moving_cursor = true;

			}
		}
	}

	wz_update_scroll_window(widget, input_state);

	// Build display buffer: password mode masks every char with '*'
	char display_buf[128];
	const char* disp = input_state->buffer;
	if (flags & WZ_INPUT_PASSWORD)
	{
		memset(display_buf, '*', input_state->length);
		display_buf[input_state->length] = '\0';
		disp = display_buf;
	}

	if (focused)
	{
		int first_char = 0;

		// Cursor blinking
		{
			unsigned long time = wz->ticks_in_ms();
			unsigned long delta = time - input_state->time_since_blink;

			if (!moving_cursor)
			{
				if (delta > 800000000)
				{
					input_state->dont_show_cursor = !input_state->dont_show_cursor;
					input_state->time_since_blink = time;
				}
			}
			else
			{
				input_state->dont_show_cursor = false;
				input_state->time_since_blink = time;
			}
		}

		// Selection highlight
		{
			int selection_start = input_state->textedit_state.select_start;
			int selection_end = input_state->textedit_state.select_end;

			if (selection_start > selection_end)
			{
				int tmp = selection_start;
				selection_start = selection_end;
				selection_end = tmp;
			}
			if (selection_end > selection_start)
			{
				float selection_w, h, up_to_sel_w;
				wz->get_string_size((char*)disp, selection_start,
					selection_end, wz->widgets[widget.handle].font_id, &selection_w, &h);
				wz->get_string_size((char*)disp, 0,
					selection_start, wz->widgets[widget.handle].font_id, &up_to_sel_w, &h);
				wz_widget_add_rect_new(widget, pad_x + up_to_sel_w - (float)input_state->offset_x,
					pad_y, selection_w, h, 0x6699FFAA);
			}
			else if (!(flags & WZ_INPUT_NO_CURSOR) &&
				!input_state->dont_show_cursor)
			{
				// Draw cursor
				float w, h;
				wz->get_string_size((char*)disp, 0,
					input_state->textedit_state.cursor, wz->widgets[widget.handle].font_id, &w, &h);
				float x = pad_x + w - (float)input_state->offset_x;
				float y = pad_y + 2;
				wz_widget_add_rect_new(widget, x, y,
					1, 16, WZ_BLACK);
			}
		}

		if (input_state->length == 0 && input_state->input_placeholder)
		{
			wz_widget_set_font_color(widget, 0x999999FF);
			wz_widget_add_text_new(widget,
				wz_str_create(input_state->input_placeholder), 0, y);
		}
		else
		{
			wz_widget_set_font_color(widget, WZ_BLACK);
			wz_widget_add_text_new(widget, wz_str_create_slice((char*)disp, first_char, input_state->length), -(float)input_state->offset_x, y);
		}
	}
	else
	{
#if 0
		// Unfocused: show placeholder when empty, otherwise show (masked) text
		if (input_state->length == 0 && input_state->input_placeholder)
		{
			wz_widget_set_font_color(widget, 0x999999FF);
			wz_widget_add_text(widget, wz_str_create(input_state->input_placeholder));
		}
		else
#endif
		{
			wz_widget_set_font_color(widget, WZ_BLACK);
			wz_widget_add_text_new(widget, wz_str_create_slice((char*)disp,
				0, input_state->length), -(float)input_state->offset_x, y);

		}
	}
}

void wz_tree_node_get_children(WzTree* tree, WzTreeNode node, WzTreeNodeData** children,
	unsigned* children_count)
{
	WzTreeNodeData* n = wz_tree_get_node(tree, node);
	*children = &tree->children_indices[n->children_index];
	*children_count = tree->children_count;
}

#if 0

void wz_do_layout_refactor_me(int from, int to)
{
	static struct WzChunk chunks[MAX_NUM_WIDGETS];
	static struct WzSlot  slots[MAX_NUM_WIDGETS];
	static int widget_to_chunk[MAX_NUM_WIDGETS];
	static int chunk_parent_chunk[MAX_NUM_WIDGETS];
	static int chunk_parent_lane[MAX_NUM_WIDGETS];
	static int widget_parent_chunk[MAX_NUM_WIDGETS];
	static int widget_lane[MAX_NUM_WIDGETS];
	static int overflow_prev[MAX_NUM_WIDGETS];

	memset(chunks, 0, sizeof(chunks));
	memset(slots, 0, sizeof(slots));

	for (int i = 0; i < MAX_NUM_WIDGETS; ++i)
	{
		widget_to_chunk[i] = -1;
		overflow_prev[i] = -1;
	}

	int stack[MAX_NUM_WIDGETS];
	int stack_head = 0, stack_tail = 0;
	int num_chunks = 0;

	widget_to_chunk[from] = 0;
	chunk_parent_chunk[0] = 0;
	chunk_parent_lane[0] = 0;
	slots[0].avail_width[0] = wz->window.w;
	slots[0].avail_height[0] = wz->window.h;
	num_chunks = 1;
	stack[stack_tail++] = from;

	while (stack_head < stack_tail)
	{
		unsigned widget = stack[stack_head++];
		WzWidgetData* widget_data = &wz->widgets[widget];
		int k = widget_to_chunk[widget];
		int head_k = k;

		chunks[k].pad_left = (float)widget_data->pad_left;
		chunks[k].pad_right = (float)widget_data->pad_right;
		chunks[k].pad_top = (float)widget_data->pad_top;
		chunks[k].pad_bottom = (float)widget_data->pad_bottom;
		chunks[k].child_gap = (float)widget_data->child_gap;
		chunks[k].is_horizontal = widget_data->is_horizontal;
		chunks[k].cursor_x = (float)widget_data->pad_left;
		chunks[k].cursor_y = (float)widget_data->pad_top;
		chunks[k].parent_chunk = chunk_parent_chunk[k];
		chunks[k].parent_slot = chunk_parent_lane[k];
		chunks[k].shrink_width = widget_data->fit_w;
		chunks[k].shrink_height = widget_data->fit_h;

		int lane = 0;
		int non_free_count = 0;
		float total_min_w = 0.f, total_min_h = 0.f;

		for (int ci = 0; ci < widget_data->children_count; ++ci)
		{
			int chandle = widget_data->children[ci].handle;
			WzWidgetData* child = &wz->widgets[chandle];
			if (child->free_from_parent) continue;

			total_min_w += (float)(child->min_w + child->margin_left + child->margin_right);
			total_min_h += (float)(child->min_h + child->margin_top + child->margin_bottom);
			++non_free_count;

			if (lane == 8)
			{
				int prev_k = k;
				k = num_chunks++;
				overflow_prev[k] = prev_k;
				chunks[k].is_continuation = 1;
				chunks[k].overflow_group_head = head_k;
				chunks[k].parent_chunk = chunk_parent_chunk[head_k];
				chunks[k].parent_slot = chunk_parent_lane[head_k];
				chunks[k].child_gap = chunks[head_k].child_gap;
				chunks[k].is_horizontal = chunks[head_k].is_horizontal;
				chunks[k].cursor_x = 0;
				chunks[k].cursor_y = 0;
				lane = 0;
			}

			slots[k].min_width[lane] = (float)child->min_w;
			slots[k].min_height[lane] = (float)child->min_h;
			slots[k].max_width[lane] = child->constraint_max_w ? (float)child->constraint_max_w : 1e9f;
			slots[k].max_height[lane] = child->constraint_max_h ? (float)child->constraint_max_h : 1e9f;
			slots[k].flex[lane] = (float)child->flex_factor;
			slots[k].margin_left[lane] = (float)child->margin_left;
			slots[k].margin_right[lane] = (float)child->margin_right;
			slots[k].margin_top[lane] = (float)child->margin_top;
			slots[k].margin_bottom[lane] = (float)child->margin_bottom;
			slots[k].cross_align[lane] = (float)child->cross_axis_alignment;

			widget_parent_chunk[chandle] = k;
			widget_lane[chandle] = lane;

			//if (child->layout != 0)g
			{
				int k_child = num_chunks++;
				widget_to_chunk[chandle] = k_child;
				chunk_parent_chunk[k_child] = k;
				chunk_parent_lane[k_child] = lane;
				stack[stack_tail++] = chandle;
			}
			++lane;
		}
		chunks[k].child_count = lane;

		if (k != head_k)
		{
			chunks[head_k].total_child_count = non_free_count;
			chunks[head_k].total_children_min_width = total_min_w;
			chunks[head_k].total_children_min_height = total_min_h;
		}
	}

	wz_layout_chunks_c(chunks, slots, num_chunks);

	// Apply position corrections for overflow continuation chunks.
	// Each continuation chunk's abs_x/abs_y was computed with cursor starting at 0;
	// add the cumulative cursor advance from all preceding chunks in the same group.
	float correction_x[MAX_NUM_WIDGETS];
	float correction_y[MAX_NUM_WIDGETS];
	memset(correction_x, 0, num_chunks * sizeof(float));
	memset(correction_y, 0, num_chunks * sizeof(float));
	for (int i = 0; i < num_chunks; ++i)
	{
		int prev = overflow_prev[i];
		if (prev < 0) continue;
		correction_x[i] = correction_x[prev] + chunks[prev].cursor_x;
		correction_y[i] = correction_y[prev] + chunks[prev].cursor_y;
		int horiz = chunks[i].is_horizontal;
		for (int j = 0; j < (int)chunks[i].child_count; ++j)
		{
			if (horiz) slots[i].abs_x[j] += correction_x[i];
			else       slots[i].abs_y[j] += correction_y[i];
		}
	}

	// Apply actual sizes
	// Iterate all slots; widget_to_chunk[i] != -1 means this widget was visited
	// by the BFS in this call. Widget handles are not guaranteed to fall in
	// [from, to], so we cannot restrict by index here.
	for (int i = 1; i < MAX_NUM_WIDGETS; ++i)
	{
		if (!wz->occupied[i]) continue;

		WzWidgetData* widget = &wz->widgets[i];

		if (widget->free_from_parent)
		{
			widget->actual_x = widget->x;
			widget->actual_y = widget->y;
			widget->actual_w = widget->min_w;
			widget->actual_h = widget->min_h;
			continue;
		}

		if (widget_to_chunk[i] == -1) continue;

		int parent_chunk = widget_parent_chunk[i];
		int lane = widget_lane[i];

		widget->actual_x = (int)slots[parent_chunk].abs_x[lane];
		widget->actual_y = (int)slots[parent_chunk].abs_y[lane];
		widget->actual_w = (unsigned)slots[parent_chunk].abs_width[lane];
		widget->actual_h = (unsigned)slots[parent_chunk].abs_height[lane];

		// Not sure it should be here
		widget->actual_x += widget->x;
		widget->actual_y += widget->y;

	}
}
#endif

void wz_widget_set_width(WzWidget widget, unsigned w)
{
	WzWidgetData* data = wz_widget_get(widget);
	data->min_w = data->constraint_max_w = w;
}

void wz_widget_set_height(WzWidget widget, unsigned h)
{
	WzWidgetData* data = wz_widget_get(widget);
	data->min_h = data->constraint_max_h = h;
}

WzWidget wz_sized_box(WzWidget parent, unsigned w, unsigned h)
{
	WzWidget result = wz_widget(parent);
	wz_widget_set_size(result, w, h);

	return result;
}


WzStr wz_str_create(const char* str)
{
	WzStr result =
	{
		.len = strlen(str)
	};

	strcpy(result.str, str);

	return result;
}

void wz_tabs(WzWidget parent, WzStr* tab_names, unsigned tabs_count,
	WzWidget* panels, unsigned* current_tab)
{
	WzWidget big_panel = wz_vbox(parent);
	wz_widget_set_border(big_panel, WZ_BORDER_TYPE_DEFAULT);
	wz_widget_set_main_axis_size_min(big_panel);

	WzWidget tabs_panel = wz_hbox(big_panel);
	wz_widget_set_cross_axis_alignment(tabs_panel, WZ_CROSS_AXIS_ALIGNMENT_END);
	wz_widget_set_main_axis_size_min(tabs_panel);

	WzWidget panel = wz_vbox(big_panel);
	wz_widget_set_size(panel, 100, 100);
	wz_widget_set_border(panel, WZ_BORDER_TYPE_DEFAULT);
	wz_widget_set_margins(panel, 10);

#define MAX_NUM_TABS 32
	WzWidget tabs[MAX_NUM_TABS];

	for (unsigned i = 0; i < tabs_count; ++i)
	{
		bool b = false;
		WzWidget tab = wz_command_button(tabs_panel, tab_names[i], &b);
		wz_widget_set_width(tab, 30);
		wz_widget_set_border(tab, WZ_BORDER_TYPE_TAB);
		tabs[i] = tab;

		if (b)
		{
			*current_tab = i;
		}
	}

	for (unsigned i = 0; i < tabs_count; ++i)
	{
		if (*current_tab == i)
		{
			wz_widget_add_child(panel, panels[i]);
			wz_widget_set_pad(tabs[i], 5);
			wz_widget_set_pos(tabs[i], 0, 2);
		}
	}
}


void wz_dropdown_run(WzWidgetData* data)
{
	//if (*data->active)
	if (1)
	{
#if 0
		bool is_selected;
		unsigned items[] = { 0, 1, 2 };
		unsigned unique_ids[] = { 100, 101, 102 };
		wzrd_label_list(data->handle, texts, texts_count, items, unique_ids, w, 20, WZ_WHITE,
			0, selected_text, &is_selected);
#endif
		WzWidget w = wz_widget(data->handle);
		wz_widget_set_size(w, 50, 50);
	}
}

void wz_clean()
{
	// Zero out everything
	wz->widgets_count = 0;

	memset(wz->chunks, 0, MAX_NUM_WIDGETS * sizeof(*wz->chunks));
	memset(wz->layouts, 0, MAX_NUM_WIDGETS * sizeof(*wz->layouts));
}

void wz_layout_newest()
{
	// Bottom sizing up pass
	// TODO: The algorithm fails when traversing ALL widgets since they will write 0 to (0, 0)
	for (int i = wz->layouts_count - 1; i >= 0; --i)
	{
		WzChunkLayout* layout = &wz->layouts[i];

		unsigned max_min_width = 0, max_min_height = 0;

		uint32_t total_children_min_width = 0, total_children_min_height = 0, flex_total = 0;

		for (unsigned k = 0; k <= layout->chunk_stride; ++k)
		{
			WzChunk* chunk = &wz->chunks[layout->chunk + k];

			for (unsigned j = 0; j < WZ_CHUNK_SIZE; ++j)
			{
				total_children_min_width += chunk->min_width[j];
				total_children_min_height += chunk->min_height[j];
				flex_total += chunk->flex[j];
				max_min_width = chunk->min_width[j] > max_min_width ? chunk->min_width[j] : max_min_width;
				max_min_height = chunk->min_height[j] > max_min_height ? chunk->min_height[j] : max_min_height;
			}
		}

		layout->total_children_min_width = total_children_min_width;
		layout->total_children_min_height = total_children_min_height;
		layout->flex_total = flex_total;

		// TODO: Remove branch
		if (i > 0 && !(layout->layout_type & WZ_LAYOUT_NONE))
		{
			unsigned new_min_width = (layout->layout_type & WZ_LAYOUT_HORIZONTAL) ?
				layout->total_children_min_width + (layout->child_count - 1) * layout->child_gap + layout->pad_left + layout->pad_right :
				max_min_width + layout->pad_left + layout->pad_right;
			unsigned new_min_height = (layout->layout_type & WZ_LAYOUT_VERTICAL) ?
				layout->total_children_min_height + (layout->child_count - 1) * layout->child_gap + layout->pad_top + layout->pad_bottom :
				max_min_height + layout->pad_top + layout->pad_bottom;

			wz->chunks[layout->parent_chunk].min_width[layout->parent_slot] =
				new_min_width > wz->chunks[layout->parent_chunk].min_width[layout->parent_slot] ?
				new_min_width :
				wz->chunks[layout->parent_chunk].min_width[layout->parent_slot];
			wz->chunks[layout->parent_chunk].min_height[layout->parent_slot] =
				new_min_height > wz->chunks[layout->parent_chunk].min_height[layout->parent_slot] ?
				new_min_height :
				wz->chunks[layout->parent_chunk].min_height[layout->parent_slot];
		}
	}

	// Top down phase
	for (int i = 0; i < wz->layouts_count; ++i)
	{
		WzChunkLayout* layout = &wz->layouts[i];

		for (unsigned k = 0; k <= layout->chunk_stride; ++k)
		{
			WzChunk* chunk = &wz->chunks[layout->chunk + k];

			uint32_t children_width = 0, children_height = 0;

			for (unsigned j = 0; j < WZ_CHUNK_SIZE; ++j)
			{
				children_width += chunk->min_width[j] & (layout->layout_type & WZ_LAYOUT_HORIZONTAL);
				children_height += chunk->min_height[j] & (layout->layout_type & WZ_LAYOUT_VERTICAL);
			}

			layout->available_width -= children_width;
			layout->available_height -= children_height;
		}

		// TODO: Handle there is no availble space for min widgets
		uint16_t flex_w = layout->flex_total && layout->available_width > 0 && (layout->layout_type & WZ_LAYOUT_HORIZONTAL) ?
			layout->available_width / layout->flex_total : 0;
		uint16_t flex_h = layout->flex_total && layout->available_height > 0 && (layout->layout_type & WZ_LAYOUT_VERTICAL) ?
			layout->available_height / layout->flex_total : 0;

		// TODO: This loop can be merged with the one above it 
		for (unsigned k = 0; k <= layout->chunk_stride; ++k)
		{
		 	WzChunk* chunk = &wz->chunks[layout->chunk + k];

			for (unsigned j = 0; j < WZ_CHUNK_SIZE; ++j)
			{
				chunk->absolute_w[j] += flex_w;
				chunk->absolute_h[j] += flex_h;
			}
		}
	}

	// Layouting 
	for (unsigned i = 0; i < wz->layouts_count; ++i)
	{
		WzChunkLayout* layout = &wz->layouts[i];
		unsigned parent_x = wz->chunks[layout->parent_chunk].absolute_x[layout->parent_slot];
		unsigned parent_y = wz->chunks[layout->parent_chunk].absolute_y[layout->parent_slot];
		unsigned cursor_x = parent_x + layout->pad_left;
		unsigned cursor_y = parent_y + layout->pad_top;

		for (unsigned k = 0; k <= layout->chunk_stride; ++k)
		{
			WzChunk* chunk = &wz->chunks[layout->chunk + k];

			for (unsigned j = 0; j < WZ_CHUNK_SIZE; ++j)
			{
				chunk->absolute_x[j] = (layout->layout_type & WZ_LAYOUT_NONE) ? parent_x + chunk->relative_x[j] : cursor_x;
				chunk->absolute_y[j] = (layout->layout_type & WZ_LAYOUT_NONE) ? parent_y + chunk->relative_y[j] : cursor_y;

				chunk->absolute_w[j] = chunk->min_width[j];
				chunk->absolute_h[j] = chunk->min_height[j];

				cursor_x += (bool)(layout->layout_type & WZ_LAYOUT_HORIZONTAL) * (chunk->absolute_w[j] + layout->child_gap);
				cursor_y += (bool)(layout->layout_type & WZ_LAYOUT_VERTICAL) * (chunk->absolute_h[j] + layout->child_gap);
			}
		}
	}

	// Slots to widgets
	for (unsigned i = 0; i < wz->chunks_count; ++i)
	{
		WzChunk* chunk = &wz->chunks[i];

		for (unsigned j = 0; j < WZ_CHUNK_SIZE; ++j)
		{
			WzWidgetData* widget = &wz->widgets[chunk->widget_index[j]];

			widget->actual_x = chunk->absolute_x[j];
			widget->actual_y = chunk->absolute_y[j];

			widget->actual_w = chunk->absolute_w[j];
			widget->actual_h = chunk->absolute_h[j];

		}
	}

}

void wz_end()
{
	// TODO: Enable this
	//wz_inherit(wz->widgets[1].handle);

	// First layout
	wz_layout_newest();
	//wz_do_layout_refactor_me(1, wz->widgets_count);

	// Check all ids are unique
	// WARNING O(N^2)
	for (unsigned int i = 0; i < MAX_NUM_WIDGETS; ++i)
	{
		for (unsigned int j = 0; j < MAX_NUM_WIDGETS; ++j)
		{
			if (i != j)
			{
				if (!wz->widgets[j].ignore_unique_id)
				{
					//wz_assert(!wz_widget_is_equal(gui->widgets[i].handle, gui->widgets[j].handle));
				}
			}
		}
	}

	// Second pass: handle scrollbars
	for (int i = 0; i < wz->scrollbars_count; ++i)
	{
		WzScrollbar scrollbar = wz->scrollbars[i];
		WzWidgetData* content_panel = wz_widget_get(scrollbar.content);
		unsigned int content_panel_h = content_panel->actual_h;
		unsigned int content_h = 0;

		for (int i = 0; i < content_panel->children_count; ++i)
		{
			content_h += wz->widgets[content_panel->children[i].handle].actual_h;
		}

		if (!content_h)
		{
			continue;
		}

		wz_assert(content_panel_h);
		if (content_panel_h < 2 * SCROLLBAR_SIZE)
		{
			continue;
		}

		float ratio = (float)content_panel_h / (float)content_h;
		if (ratio > 1)
		{
			ratio = 1;
		}
		unsigned int scrollbar_h = (unsigned int)(ratio * (float)(content_panel_h - 2 * SCROLLBAR_SIZE));
		WzWidgetData* scrollbar_data = wz_widget_get(wz->scrollbars[i].scrollbar);
		scrollbar_data->actual_h = scrollbar_h;

		if (wz_widget_is_active(scrollbar.top_button))
		{
			if (scrollbar_data->actual_y + *scrollbar.scroll + scrollbar_data->actual_h <
				content_panel->actual_y + content_panel->actual_h - SCROLLBAR_SIZE - 1)
			{
				*scrollbar.scroll += 1;
			}
		}

		scrollbar_data->actual_y += *scrollbar.scroll;
		wz_widget_add_offset(content_panel->handle, 0, -1 * *scrollbar.scroll);
	}

	// Run widgets
	for (unsigned i = 0; i < MAX_NUM_WIDGETS; ++i)
	{
		WzWidgetData* widget = &wz->widgets[i];
		switch (widget->type)
		{
		case WZ_WIDGET_TYPE_COMMAND_BUTTON:
		{
			wz_command_button_run(widget->handle, widget->released);
			break;
		}
		case WZ_WIDGET_TYPE_INPUT_BOX:
		{
			wz_text_box_run(widget->handle, widget->input_state);
			break;
		}
		case WZ_WIDGET_TYPE_SLIDER:
		{
			wz_slider_run(widget->handle);
			break;
		}
		case WZ_WIDGET_TYPE_DROPDOWN:
		{
			wz_dropdown_run(widget);
			break;
		}
		}
	}

	// Second layout
	//wz_layout_new(wz->widgets_count, MAX_NUM_WIDGETS - 1);

	// Cull
	for (unsigned int i = 1; i < MAX_NUM_WIDGETS; ++i)
	{
		WzWidgetData* widget = &wz->widgets[i];
		WzWidgetData* parent = wz_widget_get_parent(wz->widgets[i].handle);
		if (parent->actual_h < parent->margin_bottom + parent->margin_top || parent->actual_w < parent->margin_left + parent->margin_right)
		{
			widget->cull = true;
		}
		if (!widget->actual_w || !widget->actual_h)
		{
			widget->cull = true;
		}
		if (widget->actual_x > parent->actual_x + parent->actual_w - parent->margin_right - parent->margin_left)
		{
			widget->cull = true;
		}
		if (widget->actual_y > parent->actual_y + parent->actual_h - parent->margin_bottom - parent->margin_top)
		{
			widget->cull = true;
		}
		if (parent->cull)
		{
			widget->cull = true;
		}
	}

	// Cache tagged elements
	wz->cached_boxes_count = 0;
	for (int i = 1; i < MAX_NUM_WIDGETS; ++i)
	{
		assert(wz->cached_boxes_count < MAX_NUM_CACHED_BOXES - 1);
		WzWidgetData* box = &wz->widgets[i];
		if (box->tag)
		{
			wz->cached_boxes[wz->cached_boxes_count++] = *box;
		}
	}

	// Sort widgets
	WzWidget indices_stack[STACK_MAX_DEPTH] = { 0 };
	unsigned stack_size = 0;
	indices_stack[stack_size++] = wz->widgets[1].handle;
	wz->boxes_indices_count = 0;
	{
		for (unsigned i = 0; i < wz->widgets_count; ++i)
		{
			wz->boxes_indices[wz->boxes_indices_count++] = wz->widgets[i].handle;
		}

		qsort(wz->boxes_indices, wz->widgets_count, sizeof(WzWidget), wz_compare_widgets);
	}

	wz_handle_input();

	// Tabs
	for (unsigned i = 0; i < wz->events_count; ++i)
	{
		WzEvent* event = &wz->events[i];
		bool handled = false;
		int stb_key = 0;

		if (event->key.type != WZ_EVENT_TYPE_KEYBOARD || !event->key.down)
		{
			continue;
		}

		switch (event->key.key)
		{
		case '\t':
		{
			unsigned end = wz->focused_widget_index;
			for (unsigned i = wz->focused_widget_index + 1; i != end; )
			{
				if (wz->widgets[i].unique_id)
				{
					wz->focused_widget_index = i;
					wz->focused_widget_unique_id = wz->widgets[i].unique_id;
					break;
				}

				++i;
				i = i % MAX_NUM_WIDGETS;
			}
			break;
		}
		}
	}


	// Handle trees
	WzTree* tree = &wz->trees[0];

	for (unsigned i = 0; i < tree->nodes_count; ++i)
	{
		WzTreeNodeData* node = &tree->nodes[i];

		if (wz_widget_is_equal(node->row_widget, tree->selected_row))
		{
			WzWidgetData* data = wz_widget_get(node->row_widget);
			wz_widget_add_rect_absolute(tree->menu, data->actual_x, data->actual_y,
				data->actual_w, data->actual_h, EGUI_DARKGREEN);
		}
	}

	for (unsigned i = 0; i < tree->nodes_count; ++i)
	{
		WzTreeNodeData* node = &tree->nodes[i];

		if (node->visible)
		{
			WzWidgetData* icon_widget = wz_widget_get(node->icon_widget);
			WzWidgetData* expand_widget = wz_widget_get(node->expand_widget);

			int x0 = icon_widget->actual_x + icon_widget->actual_w / 2;
			int y0 = icon_widget->actual_y + icon_widget->actual_h / 2;
			int x1 = expand_widget->actual_x + expand_widget->actual_w / 2;
			int y1 = expand_widget->actual_y + expand_widget->actual_h / 2;

			wz_widget_add_dotted_line_absolute(tree->menu, x0, y0, x1, y1);

			unsigned children_count = node->children_count;

			if (children_count)
			{
				if (node->expand)
				{
					WzTreeNode* node_children = &tree->children_indices[tree->nodes[i].children_index];

					WzTreeNodeData* child_first = &tree->nodes[tree->children_indices[0].index];
					WzTreeNodeData* child_last = &tree->nodes[tree->children_indices[children_count - 1].index];
					WzWidgetData* child_expand_widget = wz_widget_get(child_last->expand_widget);

					x1 = child_expand_widget->actual_x + child_expand_widget->actual_w / 2;
					y1 = child_expand_widget->actual_y + child_expand_widget->actual_h / 2;

					wz_widget_add_dotted_line_absolute(tree->menu, x0, y0, x1, y1);
				}

				unsigned size = expand_widget->actual_w / 2;
				wz_widget_add_line_absolute(expand_widget->handle,
					expand_widget->actual_x + (expand_widget->actual_w - size) / 2,
					expand_widget->actual_y + expand_widget->actual_h / 2,
					expand_widget->actual_x + (expand_widget->actual_w - size) / 2 + size,
					expand_widget->actual_y + expand_widget->actual_h / 2);

				if (node->expand)
				{
					wz_widget_add_line_absolute(expand_widget->handle,
						expand_widget->actual_x + expand_widget->actual_w / 2,
						expand_widget->actual_y + (expand_widget->actual_h - size) / 2,
						expand_widget->actual_x + expand_widget->actual_w / 2,
						expand_widget->actual_y + (expand_widget->actual_h - size) / 2 + size);
				}
			}
		}
	}

	wz_draw(wz->boxes_indices);

	// Debug string
	{
#if 0

		WzWidgetData* widget = wz_widget_get(wz->hovered_item);
		char line_str[128];
		sprintf(line_str, "%s %u (x:%d y:%d w:%u h:%u)", widget->source, widget->handle.handle,
			widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h);
		int w = 0, h = 0;
		if (wz->get_string_size)
		{
			wz->get_string_size(line_str, 0, strlen(line_str), 0, (float*)&w, (float*)&h);
		}

		WzRect rect = {
			.x = wz->mouse_pos.x + 20,
				.y = wz->mouse_pos.y + 20,
				.w = (unsigned int)w,
				.h = (unsigned int)h,
		};

		wz_draw_rect(&wz->commands_buffer, rect,
			0xff000064,
			3, widget->source);

		wz_draw_string_to_buffer(&temp_buffer[temp_buffer_count++], line_str, rect);
#endif
	}

	wz_clean();
}

WzGui* wzrd_canvas_get()
{
	return wz;
}

WzWidget wz_toggle_raw(WzWidget parent, unsigned w, unsigned h, unsigned int color,
	bool* active, const char* file_name, unsigned int line)
{
	bool b = false;
	WzWidget handle = wz_widget_raw(parent, file_name, line);

	wz_widget_add_source(handle, file_name, line);
	wz_widget_set_max_constraints(handle, w, h);
	wz_widget_set_color_old(handle, color);

	if (wz_widget_is_deactivating(handle))
	{
		*active = !*active;
	}

	if (*active)
	{
		//wz_widget_get(widget)->border_type = WZ_BORDER_TYPE_CLICKED;
	}

	return handle;
}

WzWidget wz_icon_toggle_raw(WzWidget parent, WzTexture texture, unsigned w, unsigned h, bool* active,
	const char* file_name, unsigned int line) {
	WzWidget handle = wz_widget(parent, file_name, line);
	wz_widget_add_source(handle, file_name, line);
	wz_widget_set_max_constraints(handle, w, h);
	wz_widget_add_texture(handle, texture, w, h);

	if (wz_widget_is_deactivating(handle))
	{
		*active = !*active;
	}

	if (*active)
	{
		//wz_widget_get(widget)->border_type = WZ_BORDER_TYPE_CLICKED;
	}

	return handle;
}

WzWidget wz_texture_raw(WzWidget parent, WzTexture texture,
	unsigned w, unsigned h, const char* file_name, unsigned int line)
{
	WzWidget handle = wz_widget(parent, file_name, line);
	wz_widget_add_source(handle, file_name, line);
	wz_widget_set_size(handle, w, h);
	wz_widget_add_texture(handle, texture, w, h);

	return handle;
}

WzWidget wz_button_icon_raw(WzWidget parent, bool* result, WzTexture texture,
	const char* file, unsigned int line) {

	const int icon_size = 32;
	WzWidget icon = wz_texture(parent, texture, 16, 16);
	wz_widget_add_source(icon, file, line);
	wz_widget_set_border(icon, WZ_BORDER_TYPE_DEFAULT);

	*result = wz_widget_is_deactivating(icon);

	if (wz_widget_is_active(icon) || wz_widget_is_activating(icon))
	{
		wz_widget_set_border(icon, WZ_BORDER_TYPE_CLICKED);
	}

	return icon;
}

WzWidget wz_toggle_icon_raw(WzWidget parent, bool* result, WzTexture texture,
	const char* file, unsigned int line) {

	const int icon_size = 32;
	WzWidget icon = wz_texture(parent, texture, 16, 16);
	wz_widget_add_source(icon, file, line);
	wz_widget_set_border(icon, WZ_BORDER_TYPE_DEFAULT);

	if (wz_widget_is_deactivating(icon))
	{
		*result = !*result;
	}

	//if (wz_widget_is_active(icon) || wz_widget_is_activating(icon))
	if (*result)
	{
		wz_widget_set_border(icon, WZ_BORDER_TYPE_CLICKED);
	}

	return icon;
}

WzWidget wz_command_toggle_raw(WzWidget parent, WzStr str, bool* active,
	const char* file_name, unsigned int line)
{
	WzWidget widget = wz_label_raw(parent, str, file_name, line);
	wz_widget_set_border(widget, WZ_BORDER_TYPE_DEFAULT);
	wz_widget_set_text_alignment(widget, WZ_TEXT_ALIGNMENT_CENTER);

	if (wz_widget_is_deactivating(widget))
	{
		*active = !*active;
	}

	if (*active)
	{
		wz_widget_get(widget)->border_type = WZ_BORDER_TYPE_CLICKED;
	}

	return widget;
}

WzWidget egui_button_raw_begin_on_half_click(bool* b, WzWidget parent, const char* file, unsigned int line) {
	WzWidget h = wz_widget_raw(parent, file, line);

	//if (wzrd_handle_is_valid(h))
	if (wz_widget_is_equal(h, wz->activating_item)) {
		*b = true;
	}
	else
	{
		*b = false;
	}

	return h;
}

WzWidget wzrd_label_button_activating_raw(WzStr str, bool* active, WzWidget parent, const char* file, unsigned int line)
{
	WzWidget h = wz_label_raw(parent, str, file, line);

	//if (wzrd_handle_is_valid(h))
	if (wz_widget_is_equal(h, wz->activating_item)) {
		*active = true;
	}
	else
	{
		*active = false;
	}

	return h;
}

#if 0
bool EguiButton2(wzrd_box box, wzrd_str str, wzrd_color color) {
	(void)color;
	bool flag = false;
	bool result = egui_button_raw_begin(box);
	{
		if (IsHovered()) {
			flag = true;
		}

		result |= egui_button_raw_begin((wzrd_box) {
			.style = wzrd_style_create((wzrd_style) {
				.border_type = BorderType_None,
					.color = EGUI_WHITE,
			})
		});
		{
			if (IsHovered()) {
				flag = true;
			}

			wzrd_text_add(str);
			wzrd_item_add((Item) { .type = wzrd_item_type_str, .val = { .str = str } });
		}
		egui_button_raw_end();
	}
	egui_button_raw_end();

	if (flag) {
		//canvas->widgets[canvas->widgets_count - 1].color = EGUI_DARKBLUE;
		//canvas->widgets[canvas->widgets_count - 2].color = EGUI_DARKBLUE;
	}

	return result;
}
#endif

void wz_widget_add_item(WzWidget widget, WzWidgetItem item)
{
	WzWidgetData* b = wz_widget_get(widget);
	wz_assert(b->items_count < MAX_NUM_ITEMS - 1);
	b->items[b->items_count++] = item;
}

WzWidget wz_label_raw(WzWidget handle, WzStr str, const char* file, unsigned int line)
{
	float w = 0, h = 0;
	wz->get_string_size(str.str, 0, strlen(str.str), 0, &w, &h);

	WzWidget parent = wz_widget_raw(handle, file, line);
	wz_widget_set_size(parent, (int)w, (int)h);
	wz_widget_add_text(parent, str);
	wz_widget_set_border(parent, WZ_BORDER_TYPE_NONE);

	return parent;
}

// Implement the required callbacks
static void wz_textedit_layout_row(StbTexteditRow* r, WzInputState* obj, int n) {
	r->x0 = 0;
	r->x1 = obj->length * 8; // Assuming fixed-width font
	r->baseline_y_delta = 1.25f;
	r->ymin = 0;
	r->ymax = 1;
	r->num_chars = obj->length - n;
}

float wz_get_font_width(WzInputState* state, int line_start, int char_index)
{
	char c[2] = { 0 };
	c[0] = state->buffer[char_index];

	float w, h;
	wz->get_string_size(c, 0, 1, state->font_id, &w, &h);

	return w;
}

static int wz_input_char_at_x(WzGui* wz_, const char* buf, int len, float x, unsigned font_id)
{
	float w, h;
	for (int i = 0; i < len; i++)
	{
		wz_->get_string_size((char*)buf, 0, i + 1, font_id, &w, &h);
		if (w > x) return i;
	}
	return len;
}

static int wz_input_word_start(const char* buf, int pos)
{
	while (pos > 0 && (isalnum((unsigned char)buf[pos - 1]) || buf[pos - 1] == '_'))
		pos--;
	return pos;
}

static int wz_input_word_end(const char* buf, int len, int pos)
{
	while (pos < len && (isalnum((unsigned char)buf[pos]) || buf[pos] == '_'))
		pos++;
	return pos;
}

int wz_filter_decimal(unsigned int c) { return c >= '0' && c <= '9'; }
int wz_filter_float(unsigned int c) { return (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E'; }
int wz_filter_hex(unsigned int c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
int wz_filter_alpha(unsigned int c) { return isalpha((unsigned char)c); }
int wz_filter_alphanumeric(unsigned int c) { return isalnum((unsigned char)c); }

WzWidget wz_text_box_raw(
	WzWidget parent, WzInputState* state,
	unsigned flags, WzInputFilter filter,
	bool* committed, const char* placeholder,
	const char* file, unsigned int line)
{
	WzWidget widget = wz_widget_raw(parent, file, line);
	wz_widget_set_size(widget, 101, 18 + 4 * 2);
	wz_widget_set_pad(widget, 4);
	wz_widget_set_border(widget, WZ_BORDER_TYPE_TEXT_BOX);

	WzWidgetData* data = wz_widget_get(widget);
	data->type = WZ_WIDGET_TYPE_INPUT_BOX;
	data->input_flags = flags;
	data->input_committed = committed;
	data->input_state = state;
	data->clip_content = true;
	return widget;
}

WzWidget wzrd_label_button_raw(WzStr str, bool* result, WzWidget handle,
	const char* file, unsigned  int line) {
	float w = 0, h = 0;
	wz->get_string_size(str.str, 0, strlen(str.str), 0, &w, &h);

	WzWidget parent = wz_label_raw(handle, str, file, line);

	wz_widget_set_max_w(parent, w);
	wz_widget_set_max_h(parent, h);

	if (wz_widget_is_activating(parent))
	{
		*result = true;
	}

	return parent;
}

void wz_spacer(WzWidget parent)
{
	WzWidget spacer = wz_widget(parent);
	wz_widget_set_color(spacer, 0);
	wz_widget_set_border(spacer, WZ_BORDER_TYPE_NONE);
	wz_widget_set_flex(spacer);
	wz_widget_set_size(spacer, 0, 0);
}

void wz_widget_set_font_color(WzWidget widget, unsigned color)
{
	// Color is already in the right format (RGBA as unsigned int)
	wz_widget_get(widget)->font_color = color;
}

WzWidget wz_dialog_raw(int* x, int* y, unsigned* w, unsigned* h,
	bool* active, WzStr name_str, int z,
	WzWidget parent, const char* file, unsigned int line)
{

	*active = true;

	WzWidget window = wz_vbox(parent);
	wz_widget_set_free_from_parent(window);
	wz_widget_set_size(window, *w, *h);
	wz_widget_set_z(window, z);
	wz_widget_add_source(window, file, line);
	wz_widget_set_cross_axis_alignment(window, WZ_CROSS_AXIS_ALIGNMENT_STRETCH);

#if 1
	WzWidget bar = wz_hbox(window);
	wz_widget_set_color(bar, 0x000080FF);
	wz_widget_add_source(bar, file, line);
	wz_widget_set_border(bar, WZ_BORDER_TYPE_NONE);
	wz_widget_set_margin_left(bar, 5);
	wz_widget_set_cross_axis_alignment(bar, WZ_CROSS_AXIS_ALIGNMENT_CENTER);

	WzWidget name = wz_label(bar, name_str);
	wz_widget_set_font_color(name, 0xffffffff);
	wz_widget_set_color(name, 0);

	wz_spacer(bar);
	WzWidget quit_button = wz_button_icon(bar, &active, wz->x_icon);

	if (wz_widget_is_interacting_tree(bar) && !wz_widget_is_interacting(quit_button))
	{
		*x += wz->mouse_pos.x - wz->previous_mouse_pos.x;
		*y += wz->mouse_pos.y - wz->previous_mouse_pos.y;
	}

	wz_widget_set_pos(window, *x, *y);

	WzWidget user_section = wz_vbox(window);
	wz_widget_set_border(user_section, WZ_BORDER_TYPE_NONE);
	wz_widget_set_flex(user_section);

	return user_section;
#else
	return window;
#endif

}


void wzrd_label_list_raw(WzWidget parent, WzStr* item_names,
	unsigned int count, unsigned* items, unsigned* unique_ids,
	unsigned int width, unsigned int height, unsigned int color,
	WzWidget* widgets, unsigned int* selected, bool* is_selected,
	const char* file, unsigned int line)
{
	wz_assert(width);
	wz_assert(height);

	WzWidget panel = wz_vbox(parent);
	wz_widget_set_main_axis_size_min(panel);
	wz_widget_add_source(panel, file, line);
	wz_widget_set_margins(panel, 0);

	if (wz_widget_is_activating(panel))
	{
		*selected = 0;
		*is_selected = false;
	}

	WzWidget selected_label = { 0 };

	for (unsigned int i = 0; i < count; ++i)
	{
		bool is_label_clicked = false;
		WzWidget wdg;

		unsigned idx = items[i];
		wdg = wz_command_button(panel, item_names[idx], &is_label_clicked);
		wz_widget_set_color(wdg, color);
		wz_widget_get(wdg)->unique_id = unique_ids[idx];
		wz_widget_set_border(wdg, WZ_BORDER_TYPE_NONE);
		wz_widget_set_size(wdg, width, height);
		wz_widget_set_margins(wdg, 0);
		wz_widget_get(wdg)->dont_show_special_border_on_click = true;


		if (widgets)
		{
			widgets[i] = wdg;
		}

		if (is_label_clicked) {
			*selected = i;
			*is_selected = true;
			selected_label = wdg;
		}

		if (*is_selected && *selected == i)
		{
			//wz_widget_get(wdg)->color = wz->stylesheet.label_item_selected_color;
			//wz_widget_get(wdg)->color = color
		}
	}
}

WzWidget wz_dropdown(WzWidget parent, 
	const WzStr* texts, int texts_count, unsigned* selected_text, bool* active)
{
	WzWidget widget = wz_widget(parent);
	wz_widget_set_layout(widget, WZ_LAYOUT_NONE);

	const int w = 200;
	const int h = 20;
	wz_widget_set_size(widget, w, h);
	wz_widget_set_color(widget, WZ_WHITE);
	wz_widget_set_border(widget, WZ_BORDER_TYPE_CLICKED);
	wz_widget_add_text(widget, wz_str_create("wowow"));
	wz_widget_set_type(widget, WZ_WIDGET_TYPE_DROPDOWN);

	WzWidget button = wz_command_toggle(widget, wz_str_create("www"), active);
	wz_widget_set_x(button, w - 10);

	if (*active)
	{
		WzWidget list = wz_widget(widget);
		wz_widget_set_layout(list, WZ_LAYOUT_VERTICAL);
		wz_widget_set_y(list, h);
		wz_widget_set_z(list, 1);

		wz_label(list, wz_str_create("wowy"));
	}

	return widget;
}

void wz_widget_set_color(WzWidget widget, unsigned color)
{
	WzWidgetData* data = wz_widget_get(widget);
	WzChunk* chunk = &wz->chunks[data->chunk];
	chunk->color[data->slot] = color;

	wz_widget_set_color_old(widget, color);
}

WzWidget wz_vpanel_raw(WzWidget parent, const char* file, unsigned int line)
{
	WzWidget widget = wz_vbox_raw(parent, file, line);
	wz_widget_set_margins(widget, 5);
	wz_widget_set_child_gap(widget, 5);
	wz_widget_set_border(widget, WZ_BORDER_TYPE_DEFAULT);
	wz_widget_set_cross_axis_alignment(widget, WZ_CROSS_AXIS_ALIGNMENT_CENTER);
	wz_widget_set_color_old(widget, WZ_LIGHTGRAY);

	return widget;
}

WzWidget wz_hpanel_raw(WzWidget parent, const char* file, unsigned int line)
{
	WzWidget widget = wz_hbox_raw(parent, file, line);
	wz_widget_set_margins(widget, 5);
	wz_widget_set_child_gap(widget, 5);
	wz_widget_set_border(widget, WZ_BORDER_TYPE_DEFAULT);
	wz_widget_set_cross_axis_alignment(widget, WZ_CROSS_AXIS_ALIGNMENT_CENTER);
	wz_widget_set_color_old(widget, WZ_LIGHTGRAY);

	return widget;
}

WzWidget wz_panel_raw(WzWidget parent, const char* file, unsigned int line)
{
	WzWidget widget = wz_widget_raw(parent, file, line);
	wz_widget_set_border(widget, WZ_BORDER_TYPE_DEFAULT);
	wz_widget_set_cross_axis_alignment(widget, WZ_CROSS_AXIS_ALIGNMENT_CENTER);
	wz_widget_set_color_old(widget, WZ_LIGHTGRAY);

	return widget;
}

void wz_label_list_sorted_raw(WzStr* item_names, unsigned int count, unsigned* items, unsigned* unique_ids,
	unsigned int width, unsigned int height, unsigned int color, unsigned int* selected,
	bool* is_selected, WzWidget parent, const char* file_name, unsigned int line)
{
	// WHAT IS THIS
	WzWidget widgets[MAX_NUM_LABELS] = { 0 };


	wzrd_label_list_raw(parent, item_names, count, items, unique_ids, width, height,
		color, widgets, selected, is_selected, file_name, line);

	// Ordering
	{
		assert(items);
		WzWidget active_label = { 0 }, hovered_label = { 0 }, released_label = { 0 };
		int hovered_label_index = -1, released_label_index = -1;

		// Set variables
		for (unsigned int i = 0; i < count; ++i)
		{
			if (wzrd_handle_is_active(widgets[i]) || wzrd_handle_is_active_tree(widgets[i]))
			{
				active_label = widgets[i];
			}
			else if (wzrd_handle_is_hovered_from_list(widgets[i]))
			{
				hovered_label = widgets[i];
				hovered_label_index = i;
			}
			else if (wzrd_handle_is_hovered(widgets[i]))
			{
				// ...
			}
			else if (wzrd_handle_is_released(widgets[i]) || wzrd_handle_is_released_tree(widgets[i]))
			{
				released_label = widgets[i];
				released_label_index = i;
			}
		}

		// Get hovered widget
		bool is_bottom = false;
		WzWidgetData* hovered_parent = 0;
		for (int i = 0; i < wz->hovered_boxes_count; ++i)
		{
			if (wz_widget_is_equal(wz->hovered_boxes[i].handle, hovered_label))
			{
				hovered_parent = wz->hovered_boxes + i;
				break;
			}
		}

		// Set hover position
		if (hovered_parent)
		{
			if (wz->mouse_pos.y > hovered_parent->y + hovered_parent->actual_h / 2)
			{
				is_bottom = true;
			}

			// Label grabbed and hovering over another one
			if (wz_handle_is_valid(active_label) && wz_handle_is_valid(hovered_label) && !wz_widget_is_equal(hovered_label, active_label))
			{
				wz_widget_set_color(hovered_label, 0xFF00FFFF);
				WzWidgetData* p = wz_widget_get(hovered_label);
				WzWidgetData* c = 0;
				if (is_bottom)
				{
					WzWidget cc = wz_widget(p->handle);
					c = wz_widget_get(cc);
					wz_widget_add_source(c->handle, __FILE__, __LINE__);
					wz_widget_set_y(c->handle, hovered_parent->actual_h - 2);
					wz_widget_set_max_h(c->handle, 2);
					wz_widget_set_free_from_parent(c->handle);
				}
				else
				{
					/*	 c = wzrd_box_create((wzrd_box) {
							.h = 10, .border_type = BorderType_None, .color = EGUI_BROWN, .free = true
						});*/
				}
			}
		}

		// Label released over another label
		if (wz_handle_is_valid(released_label) && wz_handle_is_valid(hovered_label) && !wz_widget_is_equal(hovered_label, released_label))
		{
			int val = items[released_label_index];

			if (is_bottom)
			{
				if (released_label_index < hovered_label_index)
				{
					for (int i = released_label_index + 1; i <= hovered_label_index; ++i)
					{
						items[i - 1] = items[i];
					}

					items[hovered_label_index] = val;
				}
				else if (released_label_index > hovered_label_index)
				{
					for (int i = released_label_index; i > hovered_label_index; --i)
					{
						items[i] = items[i - 1];
					}

					items[hovered_label_index + 1] = val;
				}
			}

			*selected = hovered_label_index;
		}
	}

}

typedef struct
{
	WzWidgetData widgets[MAX_NUM_WIDGETS];
} WzBinary;

void save_widgets(WzGui* wz)
{
	FILE* file = fopen("C:\\Users\\Elior\\source\\repos\\AdventureWizard\\Gui", "wb");
	wz_assert(file);

	for (unsigned i = 0; i < MAX_NUM_PERSISTENT_WIDGETS; ++i)
	{
		WzWidgetData* widget = &wz->persistent_widgets[i];
		fwrite(widget, sizeof(WzWidgetData), 1, file);
	}
}

void load_widgets(WzGui* wz)
{
	FILE* file = fopen("C:\\Users\\Elior\\source\\repos\\AdventureWizard\\Gui", "rb");
	if (!file) return;

	for (unsigned i = 0; i < MAX_NUM_WIDGETS; ++i)
	{
		WzWidgetData widget;
		fread(&widget, sizeof(WzWidgetData), 1, file);
		if (widget.handle.handle)
		{
			wz->widgets[widget.handle.handle] = widget;
			wz->persistent_widgets[wz->persistent_widgets_count++] = widget;
		}
	}
}

WzWidget wzrd_handle_button_raw(bool* active, WzRect rect,
	unsigned int color, WzStr name, WzWidget handle, const char* file_name, unsigned int line)
{
	WzWidget parent = wz_widget(handle);
	wz_widget_set_free_from_parent(parent);
	wz_widget_add_source(parent, file_name, line);
	wz_widget_set_pos(parent, rect.x, rect.y);
	wz_widget_set_size(parent, rect.w, rect.h);
	wz_widget_set_color_old(parent, color);
	*active = wzrd_box_is_active(wz_widget_get(parent));

	return parent;
}

void wzrd_drag(bool* drag) {

	if (wz->mouse_left == WZ_INACTIVE)
	{
		*drag = false;
	}

	if (!(*drag)) return;

	wz->dragged_box.x += wz->mouse_delta.x;
	wz->dragged_box.y += wz->mouse_delta.y;

	//wzrd_crate(1, wz->dragged_box);
}

bool wzrd_box_is_active(WzWidgetData* box) {
	if (wz_widget_is_equal(box->handle, wz->active_item)) {
		return true;
	}

	return false;
}

bool wzrd_box_is_dragged(WzWidgetData* box) {
	if (wz_widget_is_equal(box->handle, wz->dragged_item)) {
		return true;
	}

	return false;
}

bool wzrd_box_is_hot_using_canvas(WzGui* c, WzWidgetData* box) {
	if (wz_widget_is_equal(box->handle, c->hovered_item)) {
		return true;
	}

	return false;
}

bool wzrd_box_is_hot(WzWidgetData* box) {
	if (wz_widget_is_equal(box->handle, wz->hovered_item)) {
		return true;
	}

	return false;
}

WzWidgetData* wzrd_box_get_released()
{
	WzWidgetData* result = 0;

	if (wz_handle_is_valid(wz->deactivating_item))
	{
		result = wz_widget_get(wz->deactivating_item);
	}

	return result;
}

wzrd_v2 wzrd_lerp(wzrd_v2 pos, wzrd_v2 end_pos) {
	(void)pos;
	(void)end_pos;
	/*float lerp_amount = 0.2f;
	int delta = 0.1f;
	if (fabs((float)end_pos.x - pos.x) > delta) {
		pos.x = pos.x + (int)(lerp_amount * (end_pos.x - pos.x));
	}

	if (fabs(end_pos.y - pos.y) > delta) {
		pos.y = pos.y + (int)(lerp_amount * (end_pos.y - pos.y));
	}*/

	return pos;
}

bool wzrd_v2_is_inside_polygon(wzrd_v2 point, wzrd_polygon polygon) {

	bool inside = false;
	int i, j;

	for (i = 0, j = polygon.count - 1; i < polygon.count; j = i++) {
		if (((polygon.vertices[i].y > point.y) != (polygon.vertices[j].y > point.y)) &&
			(point.x < (polygon.vertices[j].x - polygon.vertices[i].x) * (point.y - polygon.vertices[i].y) / (polygon.vertices[j].y - polygon.vertices[i].y) + polygon.vertices[i].x))
			inside = !inside;
	}

	return inside;
}

bool wz_widget_is_focused(WzWidget widget)
{
	bool result = false;

	if (wz_widget_get(widget)->unique_id == wz->focused_widget_unique_id)
	{
		result = true;
	}

	return result;
}

void wz_command_button_run(WzWidget button, bool* released)
{
	if (wz_widget_is_deactivating(button))
	{
		*released = true;
	}

	if (wz_widget_is_interacting(button) ||
		((wz->keyboard.keys['\n'] == WZ_ACTIVATING || wz->keyboard.keys['\n'] == WZ_ACTIVE) && wz_widget_is_focused(button)))
	{
		*released = true;

		if (!wz_widget_get(button)->dont_show_special_border_on_click)
		{
			wz_widget_set_border(button, WZ_BORDER_TYPE_CLICKED);
		}
	}
}

void wz_widget_disable(WzWidget widget, bool disable)
{
	wz_widget_get(widget)->disable = disable;
}

#define DEFAULT_PADDING 11

WzWidget wz_command_button_raw(WzWidget parent, WzStr str, bool* released,
	const char* file_name, unsigned int line)
{
	float w = 0, h = 0;
	wz->get_string_size(str.str, 0, strlen(str.str), 0, &w, &h);

	WzWidget widget = wz_widget_raw(parent, file_name, line);
	wz_widget_set_size(widget, (int)w + DEFAULT_PADDING, (int)h + DEFAULT_PADDING);
	wz_widget_add_text(widget, str);
	wz_widget_set_border(widget, WZ_BORDER_TYPE_DEFAULT);
	wz_widget_set_text_alignment(widget, WZ_TEXT_ALIGNMENT_CENTER);
	wz_widget_set_type(widget, WZ_WIDGET_TYPE_COMMAND_BUTTON);
	wz_widget_get(widget)->released = released;

	return widget;
}




#include "WzLayout.h"

//#define wz_log(x) (void)x;

unsigned int wz_layout_failed;

void wz_log(WzLogMessage* arr, unsigned int* count, const char* fmt, ...)
{
	WzLogMessage message = { 0 };
	message.str[0] = 0;
	va_list args;
	va_start(args, fmt);
	vsprintf_s(message.str, WZ_LOG_MESSAGE_MAX_SIZE, fmt, args);
	va_end(args);

	arr[*count] = message;
	*count = *count + 1;

}

void wz_layout(unsigned int index,
	WzWidgetData* widgets, WzLayoutRect* rects,
	unsigned int count, unsigned int* failed)
{
	wz_assert(count);

	wz_layout_failed = 0;

	unsigned int widgets_stack_count = 0;

	unsigned int size_per_flex_factor;
	WzWidgetData* widget;
	WzWidgetData* child;

	unsigned int constraint_max_w, constraint_max_h;

	unsigned int w;
	unsigned int h;
	unsigned int available_size_main_axis, available_size_cross_axis;
	int i;
	unsigned int children_flex_factor;
	unsigned int children_h, max_child_w;
	unsigned int parent_index;

	unsigned int* widgets_visits = calloc(sizeof(*widgets_visits), count);
	unsigned int* widgets_stack = calloc(sizeof(*widgets_stack), STACK_MAX_DEPTH);

	WzLogMessage* log_messages = malloc(sizeof(*log_messages) * count * 20);
	unsigned int log_messages_count = 0;

	widgets_stack[widgets_stack_count++] = index;

	unsigned int* constraint_min_main_axis, * constraint_max_main_axis,
		* constraint_min_cross_axis = 0, * constraint_max_cross_axis = 0,
		* actual_size_main_axis, * actual_size_cross_axis;

	unsigned int* child_constraint_min_main_axis, * child_constraint_max_main_axis,
		* child_constraint_min_cross_axis, * child_constraint_max_cross_axis,
		* child_actual_size_main_axis, * child_cross_axis_actual_size;

	unsigned int screen_size_main_axis, screen_size_cross_axis;

	unsigned int parent_cross_axis_size;

	unsigned int margins_cross_axis;
	unsigned int margins_main_axis;
	//WzLayoutRect* child_rect, * widget_rect;
	unsigned int root_w, root_h;

	root_w = widgets[index].constraint_max_w;
	root_h = widgets[index].constraint_max_h;

	// Constraints pass
	while (widgets_stack_count)
	{
		unsigned int children_size, max_child_h;

		parent_index = widgets_stack[widgets_stack_count - 1];
		widget = &widgets[parent_index];

		if (widget->disable)
		{
			widgets_stack_count--;
			continue;
		}

		//wz_assert(widget->constraint_max_w >= widget->min_w);
		//wz_assert(widget->constraint_max_h >= widget->min_h);

		if (!widget->children_count)
		{
			// Size leaf widgets, and pop immediately
			// For now all leaf widgets must have a finite constraint
			// Later on we'll let them decide their own size based on their content
			if (widget->constraint_max_w == WZ_UINT_MAX)
			{
				wz_log(log_messages, &log_messages_count, "(%s) ERROR: Widget width has unbounded constraints\n",
					widget->source);
			}

			if (widget->constraint_max_h == WZ_UINT_MAX)
			{
				wz_log(log_messages, &log_messages_count, "(%s) ERROR: Widget height has unbounded constraints\n",
					widget->source);
			}

			widget->actual_x = 0;
			widget->actual_y = 0;
			widget->actual_w = widget->constraint_max_w;
			widget->actual_h = widget->constraint_max_h;

			if (!widget->actual_w)
			{
				wz_log(log_messages, &log_messages_count,
					"(%s) ERROR: Widget width has no constraints\n",
					widget->source);
			}
			if (!widget->actual_h)
			{
				wz_log(log_messages, &log_messages_count,
					"(%s) ERROR: Widget height has no constraints\n",
					widget->source);
			}

			wz_assert(widget->actual_w <= widget->constraint_max_w);
			wz_assert(widget->actual_h <= widget->constraint_max_h);

			wz_log(log_messages, &log_messages_count,
				"(%s) LOG: Leaf widget with constraints (%u %u %u, %u) determined its size (%u, %u)\n",
				widget->source,
				widget->min_w, widget->min_h,
				widget->constraint_max_w, widget->constraint_max_h,
				widget->actual_w, widget->actual_h);

			widgets_stack_count--;
		}
		else
		{
			// Handle widgets with children
			if (widget->layout == WZ_LAYOUT_HORIZONTAL || widget->layout == WZ_LAYOUT_VERTICAL)
			{
				if (widget->layout == WZ_LAYOUT_HORIZONTAL)
				{
					constraint_min_main_axis = &widget->min_w;
					constraint_max_main_axis = &widget->constraint_max_w;
					constraint_min_cross_axis = &widget->min_h;
					constraint_max_cross_axis = &widget->constraint_max_h;
					actual_size_main_axis = &widget->actual_w;
					actual_size_cross_axis = &widget->actual_h;
					screen_size_main_axis = root_w;
					screen_size_cross_axis = root_h;
					margins_cross_axis = widget->margin_top + widget->margin_bottom;
					margins_main_axis = widget->margin_left + widget->margin_right;
				}
				else if (widget->layout == WZ_LAYOUT_VERTICAL)
				{
					constraint_min_main_axis = &widget->min_h;
					constraint_max_main_axis = &widget->constraint_max_h;
					constraint_min_cross_axis = &widget->min_w;
					constraint_max_cross_axis = &widget->constraint_max_w;
					actual_size_main_axis = &widget->actual_h;
					actual_size_cross_axis = &widget->actual_w;
					screen_size_main_axis = root_h;
					screen_size_cross_axis = root_w;
					margins_main_axis = widget->margin_top + widget->margin_bottom;
					margins_cross_axis = widget->margin_left + widget->margin_right;
				}
				else
				{
					child_constraint_min_main_axis = child_constraint_max_main_axis =
						child_constraint_max_cross_axis = child_constraint_min_cross_axis = 0;
					constraint_max_main_axis = 0;
					actual_size_main_axis = actual_size_cross_axis = 0;

					wz_assert(0);
				}

				// You got 3 visits for layout widget.
				// 1. Non Flex children get fixed constraints
				// 2. Above children determine their desired size, and now we allocate available space to flex children
				// 3. It's the turn of the flex children to determine their size, and then we can finally assess the 
				// Layout widget's size using it's children's 
				if (widgets_visits[parent_index] == WZ_LAYOUT_STAGE_NON_FLEX_CHILDREN)
				{
					// In the first visit give cross axis constraints to all children,
					// clamping them by their parents constraints
					const char* widget_type = 0;
					if (widget->layout == WZ_LAYOUT_HORIZONTAL)
					{
						widget_type = "Row";
					}
					else if (widget->layout == WZ_LAYOUT_VERTICAL)
					{
						widget_type = "Column";
					}

					wz_log(log_messages, &log_messages_count,
						"(%s) LOG: %s with constraints (main %u, cross %u) begins allocating cross axis constraints to children\n",
						widget->source, widget_type,
						*constraint_max_main_axis, *constraint_max_cross_axis);

					for (int i = 0; i < widget->children_count; ++i)
					{
						child = &widgets[widget->children[i].handle];

						if (child->free_from_parent)
						{
							continue;
						}

						if (widget->layout == WZ_LAYOUT_HORIZONTAL)
						{
							child_constraint_min_cross_axis = &child->min_h;
							child_constraint_max_cross_axis = &child->constraint_max_h;
						}
						else if (widget->layout == WZ_LAYOUT_VERTICAL)
						{
							child_constraint_min_cross_axis = &child->min_w;
							child_constraint_max_cross_axis = &child->constraint_max_w;
						}
						else
						{
							wz_assert(0);
							child_constraint_min_cross_axis = 0;
							child_constraint_max_cross_axis = 0;
						}

						// Cross axis constraints
						if (*constraint_max_cross_axis > margins_cross_axis)
						{
							// Clamp cross axis constraints
							unsigned int parent_max_cross_axis_constraints = *constraint_max_cross_axis;
							unsigned int parent_min_cross_axis_constraints = *constraint_min_cross_axis;

#if 0
							if (parent_max_cross_axis_constraints > margins_cross_axis)
							{
								parent_max_cross_axis_constraints = parent_max_cross_axis_constraints - margins_cross_axis;
							}
							if (parent_min_cross_axis_constraints > margins_cross_axis)
							{
								parent_min_cross_axis_constraints = parent_min_cross_axis_constraints - margins_cross_axis;
							}

							if (parent_max_cross_axis_constraints < *child_constraint_max_cross_axis)
							{
								*child_constraint_max_cross_axis = parent_max_cross_axis_constraints;
							}

							if (parent_min_cross_axis_constraints > *child_constraint_min_cross_axis)
							{
								*child_constraint_min_cross_axis = parent_min_cross_axis_constraints;
							}
#endif

							if (widget->cross_axis_alignment == WZ_CROSS_AXIS_ALIGNMENT_STRETCH)
							{
								if (widget->layout == WZ_LAYOUT_VERTICAL)
								{
									wz_assert(parent_max_cross_axis_constraints <= wz->window.w);
								}
								else if (widget->layout == WZ_LAYOUT_HORIZONTAL)
								{
									wz_assert(parent_max_cross_axis_constraints <= wz->window.h);
								}

								*child_constraint_min_cross_axis =
									*child_constraint_max_cross_axis =
									parent_max_cross_axis_constraints;
							}

							wz_log(log_messages, &log_messages_count,
								"(%s) LOG: Widget recieved cross %u\n",
								child->source, *child_constraint_max_cross_axis);
						}
						else
						{
							*child_constraint_max_cross_axis = 0;
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Flex widget had no space available to it \n",
								child->source);
						}
					}

					wz_log(log_messages, &log_messages_count,
						"(%s) LOG: %s with constraints (main %u, cross %u) ends allocating cross axis constraints to children\n",
						widget->source, widget_type,
						*constraint_max_main_axis, *constraint_max_cross_axis);

					// Give constraints to non flex children
					// A child with flex factor 0 recieves unbounded constraints in the main axis
					wz_log(log_messages, &log_messages_count,
						"(%s) LOG: %s  with constraints (main %u, cross %u) begins constrains non-flex children\n",
						widget->source, widget_type,
						*constraint_max_main_axis, *constraint_max_cross_axis);

					// We leave  non-flex constraints unchanged
					// ie unbounded
					for (int i = 0; i < widget->children_count; ++i)
					{
						child = &widgets[widget->children[i].handle];

						if (child->flex_factor == 0)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) LOG: Non-flex widget with constraints (%u %u %u %u)\n",
								child->source,
								child->min_w, child->min_h,
								child->constraint_max_w, child->constraint_max_h);

							widgets_stack[widgets_stack_count] = widget->children[i].handle;
							widgets_stack_count++;
						}
					}

					wz_log(log_messages, &log_messages_count,
						"(%s) LOG: %s ends constrains non-flex children\n",
						widget->source, widget_type);

					widgets_visits[parent_index] = 1;
				}
				else if (widgets_visits[parent_index] == WZ_LAYOUT_STAGE_FLEX_CHILDREN)
				{
					// Give constraints to flex children, allocating from the availble space
					if (widget->layout == WZ_LAYOUT_HORIZONTAL)
					{
						available_size_main_axis = widget->constraint_max_w;
						available_size_cross_axis = widget->constraint_max_h;
					}
					else if (widget->layout == WZ_LAYOUT_VERTICAL)
					{
						available_size_main_axis = widget->constraint_max_h;
						available_size_cross_axis = widget->constraint_max_w;
					}

					children_flex_factor = 0;

					for (i = 0; i < widget->children_count; ++i)
					{
						child = &widgets[widget->children[i].handle];
						//child_rect = &rects[widget->children[i]];

						if (child->free_from_parent)
						{
							continue;
						}

						if (widget->layout == WZ_LAYOUT_HORIZONTAL)
						{
							child_actual_size_main_axis = &child->actual_w;
						}
						else if (widget->layout == WZ_LAYOUT_VERTICAL)
						{
							child_actual_size_main_axis = &child->actual_h;
						}
						else
						{
							child_actual_size_main_axis = 0;
						}

						if (!child->flex_factor)
						{
							//wz_assert(*child_actual_size_main_axis);
							if (available_size_main_axis >= *child_actual_size_main_axis)
							{
								available_size_main_axis -= *child_actual_size_main_axis;
							}
							else
							{
								available_size_main_axis = 0;
							}
						}
						else
						{
							children_flex_factor += child->flex_factor;
						}
					}

					// Widget allocating space to flex children cannot be unbounded in the main axis
					if (children_flex_factor && constraint_max_main_axis == UINT_MAX)
					{
						wz_log(log_messages, &log_messages_count,
							"(%s) ERROR: Widget allocating space to flex \
							children cannot be unbounded in the main axis\n",
							widget->source);
					}

					// Substract margins and child gap 
					if (widget->layout == WZ_LAYOUT_HORIZONTAL)
					{
						if (available_size_main_axis >= widget->margin_left + widget->margin_right)
						{
							available_size_main_axis -= widget->margin_left + widget->margin_right;
						}
						else
						{
							available_size_main_axis = 0;
						}
					}
					else if (widget->layout == WZ_LAYOUT_VERTICAL)
					{
						if (available_size_main_axis >= widget->margin_top + widget->margin_bottom)
						{
							available_size_main_axis -= widget->margin_top + widget->margin_bottom;
						}
						else
						{
							available_size_main_axis = 0;
						}
					}

					if (available_size_main_axis > widget->child_gap * (widget->children_count - 1))
					{
						available_size_main_axis -= widget->child_gap * (widget->children_count - 1);
					}
					else
					{
						available_size_main_axis = 0;
					}

					if (children_flex_factor)
					{
						size_per_flex_factor = available_size_main_axis / children_flex_factor;
					}

					const char* widget_type = 0;
					if (widget->layout == WZ_LAYOUT_HORIZONTAL)
					{
						widget_type = "Row";
					}
					else if (widget->layout == WZ_LAYOUT_VERTICAL)
					{
						widget_type = "Column";
					}

					wz_log(log_messages, &log_messages_count,
						"(%s) LOG: %s begins constrains flex children\n",
						widget->source, widget_type);

					for (int i = 0; i < widget->children_count; ++i)
					{
						child = &widgets[widget->children[i].handle];

						if (widget->layout == WZ_LAYOUT_HORIZONTAL)
						{
							child_constraint_min_main_axis = &child->min_w;
							child_constraint_max_main_axis = &child->constraint_max_w;
							child_constraint_min_cross_axis = &child->min_h;
							child_constraint_max_cross_axis = &child->constraint_max_h;
						}
						else if (widget->layout == WZ_LAYOUT_VERTICAL)
						{
							child_constraint_min_main_axis = &child->min_h;
							child_constraint_max_main_axis = &child->constraint_max_h;
							child_constraint_min_cross_axis = &child->min_w;
							child_constraint_max_cross_axis = &child->constraint_max_w;
						}
						else
						{
							child_constraint_min_main_axis = child_constraint_max_main_axis =
								child_constraint_max_cross_axis = child_constraint_min_cross_axis = 0;
							wz_assert(0);
						}

						if (child->flex_factor)
						{
							unsigned int main_axis_size = size_per_flex_factor * child->flex_factor;

							if (widget->flex_fit == WZ_FLEX_FIT_TIGHT)
							{
								*child_constraint_min_main_axis = main_axis_size;
							}

							*child_constraint_max_main_axis = main_axis_size;

							if (child_constraint_max_main_axis == 0)
							{
								wz_log(log_messages, &log_messages_count,
									"(%s) ERROR: Flex widget had no space available to it \n",
									child->source);
							}

							wz_log(log_messages, &log_messages_count,
								"(%s) LOG: Flex widget recieved constraints (%u, %u) \n",
								child->source,
								child->constraint_max_w, child->constraint_max_h);

							widgets_stack[widgets_stack_count] = widget->children[i].handle;
							widgets_stack_count++;
						}
					}

					wz_log(log_messages, &log_messages_count,
						"(%s) LOG: %s  ends constrains flex children\n",
						widget->source, widget_type);

					widgets_visits[parent_index] = 2;
				}
				else if (widgets_visits[parent_index] == WZ_LAYOUT_STAGE_PARENT)
				{
					// We finally determined the size of all the children of a widget with a layout
					// Now we determine it's size

					wz_assert(widget->children_count);

					// Main axis size
					if (widget->main_axis_size_type == MAIN_AXIS_SIZE_TYPE_MIN)
					{
						if (widget->constraint_max_h == widget->min_h &&
							widget->layout == WZ_LAYOUT_HORIZONTAL)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Row has tight constraints in it's horizontal axis, but it's supposed to shrink-wrap.\n",
								widget->source);
						}

						if (widget->constraint_max_h == widget->min_h &&
							widget->layout == WZ_LAYOUT_VERTICAL)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Column has tight constraints in it's vertical axis, but it's supposed to shrink-wrap.\n",
								widget->source);
						}

						if (!widget->children_count)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Shrink-wrapping in the main axis, but no children found.\n",
								widget->source);
						}

						children_size = 0;
						for (i = 0; i < widget->children_count; ++i)
						{
							child = &widgets[widget->children[i].handle];

							if (widget->layout == WZ_LAYOUT_HORIZONTAL)
							{
								child_actual_size_main_axis = &child->actual_w;
							}
							else if (widget->layout == WZ_LAYOUT_VERTICAL)
							{
								child_actual_size_main_axis = &child->actual_h;
							}
							else
							{
								child_actual_size_main_axis = 0;
								wz_assert(0);
							}

							children_size += *child_actual_size_main_axis;
						}

						*actual_size_main_axis = children_size + widget->child_gap * (widget->children_count - 1);
					}
					else
					{
						// Layout widget is unconstrained in the main axis
						// It must shrink-wrap
						if (widget->layout == WZ_LAYOUT_HORIZONTAL &&
							widget->constraint_max_w == WZ_UINT_MAX)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Row recieved unbounded constraints in the horizontal axis\n",
								widget->source);
						}

						if (widget->layout == WZ_LAYOUT_VERTICAL &&
							widget->constraint_max_h == WZ_UINT_MAX)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Column recieved unbounded constraints in the vertical axis\n",
								widget->source);
						}

						// Determine size of widget that is constrained in the main axis
						*actual_size_main_axis = *constraint_max_main_axis;
					}

					// Cross axis size (use tallest child)
					parent_cross_axis_size = 0;
					for (int i = 0; i < widget->children_count; ++i)
					{
						child = &widgets[widget->children[i].handle];

						if (widget->layout == WZ_LAYOUT_HORIZONTAL)
						{
							if (child->actual_h > parent_cross_axis_size)
							{
								parent_cross_axis_size = child->actual_h;
							}
						}
						else if (widget->layout == WZ_LAYOUT_VERTICAL)
						{
							if (child->actual_w > parent_cross_axis_size)
							{
								parent_cross_axis_size = child->actual_w;
							}
						}
					}

					if (!parent_cross_axis_size)
					{
						wz_log(log_messages, &log_messages_count,
							"(%s) LOG: widget has not room for children in the cross axis\n",
							widget->source);
					}

					*actual_size_cross_axis = parent_cross_axis_size;

					widget->actual_h += widget->margin_top + widget->margin_bottom;
					widget->actual_w += widget->margin_left + widget->margin_right;

					// Clamp
#if 1
					if (widget->actual_h < widget->min_h)
					{
						widget->actual_h = widget->min_h;
					}

					if (widget->actual_h > widget->constraint_max_h)
					{
						widget->actual_h = widget->constraint_max_h;
					}

					if (widget->actual_w < widget->min_w)
					{
						widget->actual_w = widget->min_w;
					}

					if (widget->actual_w > widget->constraint_max_w)
					{
						widget->actual_w = widget->constraint_max_w;
					}
#endif

					if (widget->layout == WZ_LAYOUT_HORIZONTAL)
					{
						if (widget->actual_w >= wz->window.w)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Row widget with constraints (%u, %u) \
								 determined its size (%u, %u). Width is too big. \n",
								widget->source,
								widget->constraint_max_w, widget->constraint_max_h, widget->actual_w, widget->actual_h);
						}
						else if (widget->actual_h >= wz->window.h)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Row widget with constraints (%u, %u) \
								 determined its size (%u, %u). Height is too big. \n",
								widget->source,
								widget->constraint_max_w, widget->constraint_max_h, widget->actual_w, widget->actual_h);
						}
						else
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) LOG: Row widget  with constraints (%u, %u) \
									determined its size (%u, %u)\n",
								widget->source,
								widget->constraint_max_w, widget->constraint_max_h, widget->actual_w, widget->actual_h);
						}
					}
					else if (widget->layout == WZ_LAYOUT_VERTICAL)
					{
						if (widget->actual_w >= wz->window.w)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Column widget with constraints (%u, %u) \
								 determined its size (%u, %u). Width is too big. \n",
								widget->source,
								widget->constraint_max_w, widget->constraint_max_h, widget->actual_w, widget->actual_h);
						}
						else if (widget->actual_h >= wz->window.h)
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) ERROR: Column widget with constraints (%u, %u) \
								 determined its size (%u, %u). Height is too big. \n",
								widget->source,
								widget->constraint_max_w, widget->constraint_max_h, widget->actual_w, widget->actual_h);
						}
						else
						{
							wz_log(log_messages, &log_messages_count,
								"(%s) LOG: Column widget  with constraints \
								(%u, %u) determined its size (%u, %u)\n",
								widget->source,
								widget->constraint_max_w, widget->constraint_max_h, widget->actual_w, widget->actual_h);
						}
					}

					wz_assert(*actual_size_main_axis <= *constraint_max_main_axis);
					wz_assert(*actual_size_cross_axis <= *constraint_max_cross_axis);

					// Give positions to children
					unsigned int offset = 0;
					widget->actual_x = 0;
					widget->actual_y = 0;
					for (int i = 0; i < widget->children_count; ++i)
					{
						child = &widgets[widget->children[i].handle];

						if (child->free_from_parent)
						{
							continue;
						}

						if (widget->layout == WZ_LAYOUT_HORIZONTAL)
						{
							child_actual_size_main_axis = &child->actual_w;
							child->actual_x = offset + widget->margin_left;
							child->actual_y = widget->margin_top;
						}
						else if (widget->layout == WZ_LAYOUT_VERTICAL)
						{
							child_actual_size_main_axis = &child->actual_h;
							child->actual_y = offset + widget->margin_top;
							child->actual_x = widget->margin_left;
						}
						else
						{
							child_actual_size_main_axis = 0;
							wz_assert(0);
						}

#if 0

						wz_assert(child->actual_x + child->actual_w <=
							widget->actual_x + widget->actual_w);
						wz_assert(child->actual_y + child->actual_h <=
							widget->actual_y + widget->actual_h);
#endif
						offset += *child_actual_size_main_axis;
						offset += widget->child_gap;

						wz_log(log_messages, &log_messages_count,
							"(%s) LOG: Child widget (%u) will have the raltive position %u %u\n",
							child->source, widget->children[i],
							child->actual_x, child->actual_y);
					}

					widgets_stack_count--;
				}
				else
				{
					wz_assert(0);
				}
			}
			else if (widget->layout == WZ_LAYOUT_NONE)
			{
				if (widgets_visits[parent_index] == 0)
				{
					for (int i = 0; i < widget->children_count; ++i)
					{
						child = &widgets[widget->children[i].handle];

						constraint_max_w = widget->constraint_max_w - (widget->margin_left + widget->margin_right);
						constraint_max_h = widget->constraint_max_h - (widget->margin_top + widget->margin_bottom);

						if (constraint_max_w < child->constraint_max_w)
						{
							child->constraint_max_w = constraint_max_h;
						}
						if (constraint_max_h < child->constraint_max_h)
						{
							child->constraint_max_h = constraint_max_h;
						}

						child->actual_x = widget->margin_left;
						child->actual_y = widget->margin_right;

						wz_assert(i < MAX_NUM_CHILDREN);
						wz_assert(widgets_stack_count < STACK_MAX_DEPTH);
						widgets_stack[widgets_stack_count] = widget->children[i].handle;
						widgets_stack_count++;
						widgets_visits[parent_index] = 1;
						wz_log(log_messages, &log_messages_count, "(%s) LOG: Non-layout widget passes to child constraints (%u, %u) and position (%u %u)\n",
							child->source,
							child->constraint_max_w, child->constraint_max_h, child->actual_x, child->actual_y);
					}
				}
				else if (widgets_visits[parent_index] == 1)
				{
					widget->actual_x = 0;
					widget->actual_y = 0;
					widget->actual_w = widget->constraint_max_w;
					widget->actual_h = widget->constraint_max_h;

					wz_log(log_messages, &log_messages_count,
						"(%s) LOG: Non-layout widget  with constraints (%u, %u) determined its size (%u, %u)\n",
						widget->source,
						widget->constraint_max_w, widget->constraint_max_h, widget->actual_w, widget->actual_h);

					//wz_assert(widget->actual_w <= widget->constraint_max_w);
					//wz_assert(widget->actual_h <= widget->constraint_max_h);
					//wz_assert(widget->actual_w <= root_w);
					//wz_assert(widget->actual_h <= root_h);

					widgets_stack_count--;
				}
			}
			else
			{
				wz_assert(0);
			}

		}
	}

	// Final stage. Calculate the widgets non-relative final position and cull 
	for (int i = 1; i < count; ++i)
	{
		widget = &widgets[i];
		unsigned int offset = 0;
		unsigned int children_size_main_axis = 0;

		// Add to absolute position the ralative position
		widget->actual_x += widget->x;
		widget->actual_y += widget->y;

		// Get children's total size
		{
			for (int j = 0; j < widget->children_count; ++j)
			{
				child = &widgets[widget->children[j].handle];

				if (widget->layout == WZ_LAYOUT_HORIZONTAL)
				{
					children_size_main_axis += child->actual_w;
				}
				else if (widget->layout == WZ_LAYOUT_VERTICAL)
				{
					children_size_main_axis += child->actual_h;
				}
			}

			children_size_main_axis += widget->child_gap * (widget->children_count - 1);
		}

		// Set position for each child of the widget
		for (int j = 0; j < widget->children_count; ++j)
		{
			child = &widgets[widget->children[j].handle];

			// ...
			child->actual_x += widget->actual_x;
			child->actual_y += widget->actual_y;

			if (!widget->actual_w || !widget->actual_h)
			{
				child->actual_w = 0;
				child->actual_h = 0;
				continue;
			}

			unsigned int parent_size_h = widget->actual_h
				- widget->margin_top - widget->margin_bottom;
			unsigned int parent_size_w = widget->actual_w
				- widget->margin_left - widget->margin_right;

			if (widget->main_axis_alignment == WZ_MAIN_AXIS_ALIGNMENT_END)
			{
				if (widget->layout == WZ_LAYOUT_HORIZONTAL)
				{
					child->actual_x += parent_size_w - children_size_main_axis;
				}
				else if (widget->layout == WZ_LAYOUT_VERTICAL)
				{
					child->actual_y += parent_size_h - children_size_main_axis;
				}
			}
			else if (widget->main_axis_alignment == WZ_MAIN_AXIS_ALIGNMENT_CENTER)
			{
				if (widget->layout == WZ_LAYOUT_HORIZONTAL && parent_size_w > child->actual_w)
				{
					child->actual_x += (parent_size_w - children_size_main_axis) / 2;
				}
				else if (widget->layout == WZ_LAYOUT_VERTICAL && parent_size_h > child->actual_h)
				{
					child->actual_y += (parent_size_h - children_size_main_axis) / 2;
				}
			}

			if (widget->cross_axis_alignment == WZ_CROSS_AXIS_ALIGNMENT_CENTER)
			{
				if (widget->layout == WZ_LAYOUT_HORIZONTAL && parent_size_h > child->actual_h)
				{
					child->actual_y += (parent_size_h - child->actual_h) / 2;
				}
				else if (widget->layout == WZ_LAYOUT_VERTICAL && parent_size_w > child->actual_w)
				{
					child->actual_x += (parent_size_w - child->actual_w) / 2;
				}
			}
			else if (widget->cross_axis_alignment == WZ_CROSS_AXIS_ALIGNMENT_END)
			{
				if (widget->layout == WZ_LAYOUT_HORIZONTAL)
				{
					child->actual_y = widget->actual_y + widget->actual_h
						// - widget->margin_bottom
						- child->actual_h;
				}
				else if (widget->layout == WZ_LAYOUT_VERTICAL)
				{
					child->actual_x = widget->actual_x + widget->actual_w -
						widget->margin_right - child->actual_w;
				}
			}
			else if (widget->cross_axis_alignment == CROSS_AXIS_ALIGNMENT_START)
			{
				// Do nothing
			}

			// Check the widgets size doesnt exceeds its parents
			if (child->actual_x + child->actual_w >= widget->actual_x + widget->actual_w)
			{
				wz_log(log_messages, &log_messages_count, "(%s) ERROR: Widget exceeds it's parents horizontally\n",
					child->source);
			}
			if (child->actual_y + child->actual_h >= widget->actual_y + widget->actual_h)
			{
				wz_log(log_messages, &log_messages_count, "(%s) ERROR: Widget exceeds it's parents vertically\n",
					child->source);
			}
		}
	}


	wz_log(log_messages, &log_messages_count, "---------------------------\n");
	wz_log(log_messages, &log_messages_count, "Final Layout:\n");

	for (unsigned int i = 0; i < count; ++i)
	{
		widget = &widgets[i];

		wz_log(log_messages, &log_messages_count, "(%u %s %u) : (%d %d %u %u)\n",
			i, widget->source, widget->actual_x, widget->actual_y, widget->actual_w, widget->actual_h);
	}
	wz_log(log_messages, &log_messages_count, "---------------------------\n");

	free(widgets_visits);
	free(widgets_stack);
	free(log_messages);

	*failed = wz_layout_failed;
}

void wz_gui_deinit(WzGui* wz)
{
	save_widgets(wz);
}

#if 0
void wz_gui_init(WzGui* wz)
{
	wz->scrollbars = malloc(sizeof(*wz->scrollbars) * MAX_NUM_SCROLLBARS);
	wz->widgets = calloc(MAX_NUM_WIDGETS, sizeof(*wz->widgets));
	wz->rects = calloc(MAX_NUM_WIDGETS, sizeof(*wz->rects));
	wz->boxes_indices = calloc(MAX_NUM_WIDGETS, sizeof(*wz->boxes_indices));
	wz->hovered_items_list = calloc(MAX_NUM_WIDGETS, sizeof(*wz->hovered_items_list));
	wz->hovered_boxes = calloc(MAX_NUM_WIDGETS, sizeof(*wz->hovered_boxes));
	wz->cached_boxes = calloc(MAX_NUM_WIDGETS, sizeof(*wz->cached_boxes));
	wz->free_widgets = calloc(MAX_NUM_WIDGETS, sizeof(*wz->free_widgets));
	wz->zoom_factor = 1;
}
#endif


void wz_widget_scale(WzWidget widget, float w, float h)
{
	wz_widget_get(widget)->scale[0] = w;
	wz_widget_get(widget)->scale[1] = h;
}

void wz_widget_transform(WzWidget widget, float x, float y)
{
	wz_widget_get(widget)->pos[0] = x;
	wz_widget_get(widget)->pos[1] = y;
}

void wz_zoom(float zoom_factor)
{
	wz->zoom_factor += zoom_factor;
}

void wz_frame(WzWidget parent, unsigned w, unsigned h, WzStr str)
{
	WzWidget widget = wz_widget(parent);
	const int offset = 12;
	wz_widget_set_size(widget, w, h);
	wz_widget_add_rect_new(widget, offset, offset, w - 2 * offset, 1, 0xa0a0a0ff);
	wz_widget_add_rect_new(widget, w - offset, offset, 1, h - 2 * offset, 0xa0a0a0ff);
	wz_widget_add_rect_new(widget, offset, h - offset, w - 2 * offset, 1, 0xa0a0a0ff);
	wz_widget_add_rect_new(widget, offset, offset, 1, h - 2 * offset, 0xa0a0a0ff);
	wz_widget_set_border(widget, WZ_BORDER_TYPE_NONE);
	WzWidget label = wz_label(widget, str);
	wz_widget_set_x(label, 20);
	wz_widget_set_y(label, 2);
}

WzWidget wz_scene(WzScene scene, WzWidget parent, WzTexture texture, int x, int y, unsigned w, unsigned h)
{
	WzWidget widget = wz_widget(parent);
	wz_widget_set_size(widget, 100, 100);

	scene.root = widget;
	wz->scenes[wz->scenes_count++] = scene;

	return widget;
}

void wz_add_resize_widgets_maintain_aspect_ratio(WzWidget parent, float* scale_x,
	float* scale_y, float* transform_x, float* transform_y)
{
	WzWidget blue_widget = wz_widget(parent);
	const int blue_widget_size = 20;
	wz_widget_set_constraints(blue_widget, 0, 0, blue_widget_size, blue_widget_size);
	wz_widget_set_pos(blue_widget, -blue_widget_size, -blue_widget_size);
	wz_widget_set_color(blue_widget, 0x0000FFFF);
	wz_widget_get(blue_widget)->absolute_scale = true;

	WzWidgetData* widget_data = wz_widget_get(parent);

	if (wz_widget_is_active(blue_widget))
	{
		float mouse_delta_x = (wz->mouse_delta.x) / wz->zoom_factor;
		float mouse_delta_y = (wz->mouse_delta.y) / wz->zoom_factor;

		// Two version. Second one maintains aspect ratio.
#if 0
		* scale_x -= mouse_delta_x / (float)widget_data->constraint_max_w;
		*scale_y -= mouse_delta_y / (float)widget_data->constraint_max_h;

		*transform_x += mouse_delta_x;
		*transform_y += mouse_delta_y;
#else 

		* scale_x -= mouse_delta_x / (float)widget_data->constraint_max_w;
		*scale_y -= mouse_delta_x / (float)widget_data->constraint_max_w;

		*transform_x += mouse_delta_x;
		*transform_y += mouse_delta_x;
#endif
	}
}

void wz_add_resize_widgets_maintain_aspect_ratio2(WzWidget parent, float* angle)
{
	WzWidget blue_widget = wz_widget(parent);
	const int blue_widget_size = 20;
	wz_widget_set_constraints(blue_widget, 0, 0, blue_widget_size, blue_widget_size);
	wz_widget_set_pos(blue_widget, -blue_widget_size, -blue_widget_size);
	wz_widget_set_color(blue_widget, 0x0000FFFF);
	wz_widget_get(blue_widget)->absolute_scale = true;

	WzWidgetData* widget_data = wz_widget_get(parent);

	if (wz_widget_is_active(blue_widget))
	{
		float mouse_delta_x = (wz->mouse_delta.x) / wz->zoom_factor;
		float mouse_delta_y = (wz->mouse_delta.y) / wz->zoom_factor;

		*angle = atan2f(mouse_delta_x, mouse_delta_y) * (180 * M_PI);
	}
}

void wz_widget_rotate(WzWidget widget, float w)
{
	wz_widget_get(widget)->rotation = w;
}

void wz_widget_set_size(WzWidget c, unsigned int w, unsigned int h)
{
	WzWidgetData* data = wz_widget_get(c);
	data->min_h = data->constraint_max_h = h;
	data->min_w = data->constraint_max_w = w;

	// TODO: Handle max size too
	WzChunk* chunk = &wz->chunks[data->chunk];
	chunk->min_width[data->slot] = w;
	chunk->min_height[data->slot] = h;
}

// define the functions we need
void layout_func(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i)
{
	float w, h;
	wz->get_string_size(str->buffer, 0, str->length, str->font_id, &w, &h);
	int remaining_chars = str->length - start_i;
	row->num_chars = remaining_chars; // should do real word wrap here
	row->x0 = 0;
	row->x1 = w; // need to account for actual size of characters
	row->baseline_y_delta = 1.25;
	row->ymin = -1;
	row->ymax = 0;
}

int delete_chars(STB_TEXTEDIT_STRING* str, int pos, int num)
{
	memmove(&str->buffer[pos], &str->buffer[pos + num], str->length - (pos + num));
	str->length -= num;
	return 1; // always succeeds
}

int insert_chars(STB_TEXTEDIT_STRING* str, int pos, STB_TEXTEDIT_CHARTYPE* newtext, int num)
{
	//str->buffer = realloc(str->buffer, str->length + num);
	memmove(&str->buffer[pos + num], &str->buffer[pos], str->length - pos);
	memcpy(&str->buffer[pos], newtext, num);
	str->length += num;
	return 1; // always succeeds
}
