#ifndef WZRD_GUI_H
#define WZRD_GUI_H

//==============================================================================
// INCLUDES
//==============================================================================
#include <float.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "WzLayout.h"
#include "Strings.h"
#include <stdlib.h>
#include <string.h> // memmove
#include <ctype.h>  // isspace

//==============================================================================
// DEFINES
//==============================================================================
//#define WZ_LOG(...) printf(__VA_ARGS__)
#define wz_layout_log(...) (void)0;

#define WZ_BORDER_SIZE 1

// BUG: FONT SIZE CLASHES WITH THAT DEFINED IN PLATFORM.C
#define WZRD_FONT_HEIGHT 22
#define FONT_WIDTH (WZRD_FONT_HEIGHT / 2)

#define BUTTON_WIDTH 110
#define BUTTON_HEIGHT 24

#define ICON_BUTTON_WIDTH 24
#define ICON_BUTTON_HEIGHT 24

#define TEXT_BOX_WIDTH 110
#define TEXT_BOX_HEIGHT 24

#define GAP_WIDTH 24
#define GAP_HEIGHT 24

#define MAX_NUM_WIDGETS 32

#define MAX_NUM_ITEMS 128
#define MAX_NUM_CHILDREN 256

#define DEBUG_PANELS 0

#define MAX_NUM_DRAW_COMMANDS 1024

#define MAX_NUM_SOURCES 8

#define MAX_SOURCE_SIZE 128

#define MAX_NUM_CACHED_BOXES 128

#define MAX_NUM_PERSISTENT_WIDGETS 1024

#define MAX_NUM_SCENES 8

#define MAX_NUM_HASHTABLE_ELEMENTS 32

#define EDITOR_LINE_GAP 10
#define EDITOR_ICON_SIZE 24
#define EDITOR_BUTTON_SIZE_X 72
#define EDITOR_BUTTON_SIZE_Y 24

#define MAX_NUM_LABELS 32

#define MAX_NUM_VERTICES_IN_POLYGON 32

#define MAX_NUM_SCROLLBARS 32

#define WZL_LOG(...) (void)0;

#define WZ_FLEX_FIT_LOOSE 0
#define WZ_FLEX_FIT_TIGHT 1

enum
{
	WZ_LAYOUT_NONE = 1 << 0,
	WZ_LAYOUT_HORIZONTAL = 1 << 1,
	WZ_LAYOUT_VERTICAL = 1 << 2,
};

#define	MAIN_AXIS_SIZE_TYPE_MIN  0
#define	MAIN_AXIS_SIZE_TYPE_MAX  1

#define WZ_UINT_MAX 4294967295

#define WZ_LOG_MESSAGE_MAX_SIZE 256

#define WZ_LAYOUT_MAX_NUM_SOURCES 8



// Color Defines (RGBA format: 0xRRGGBBAA)
#define WZ_DEFAULT_COLOR 0xc0c0c0ff
#define WZ_LIGHTGRAY 0xc8c8c8ff      // Light Gray
#define EGUI_LIGHTESTGRAY 0xe1e1e1ff   // Lightest Gray
#define EGUI_GRAY 0xb4b4b4ff           // Gray
#define WZ_DARKGRAY 0x505050ff         // Dark Gray
#define WZ_YELLOW 0xffff00ff           // Yellow
#define EGUI_GOLD 0xffcb00ff           // Gold
#define EGUI_ORANGE 0xffa100ff         // Orange
#define EGUI_PINK 0xff6dc2ff           // Pink
#define WZ_RED 0xff0000ff              // Red
#define EGUI_MAROON 0xbe2137ff         // Maroon
#define WZ_GREEN 0x00e430ff            // Green
#define EGUI_LIME 0x009e2fff           // Lime
#define EGUI_DARKGREEN 0x00752cff      // Dark Green
#define EGUI_SKYBLUE 0x66bfffff        // Sky Blue
#define WZ_BLUE 0x0000ffff             // Blue
#define EGUI_DARKBLUE 0x0052acff       // Dark Blue
#define EGUI_PURPLE 0xc87affff         // Purple
#define EGUI_VIOLET 0x873cbeff         // Violet
#define EGUI_DARKPURPLE 0x701f7eff     // Dark Purple
#define EGUI_BEIGE 0xd3b083ff          // Beige
#define EGUI_BROWN 0x7f6a4fff          // Brown
#define EGUI_DARKBROWN 0x4c3f2fff      // Dark Brown
#define WZ_WHITE 0xffffffff            // White
#define EGUI_WHITE2 0xe6e6e6ff         // White2
#define WZ_BLACK 0x000000ff            // Black
#define EGUI_BLANK 0x00000000          // Blank (Transparent)
#define EGUI_MAGENTA 0xff00ffff        // Magenta
#define WZ_RAYWHITE 0xf5f5f5ff       // My own White (raylib logo)


#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_STRING WzInputState

#include "stb_textedit.h"

typedef struct WzInputState
{
	char buffer[128];
	int length;
	int max_length;
	STB_TexteditState textedit_state;
	unsigned long time_since_click;
	const char* input_placeholder;

	bool dont_show_cursor;
	unsigned time_since_blink;
	float offset_x;
	unsigned font_id;
} WzInputState;

// define all the #defines needed 
int insert_chars(STB_TEXTEDIT_STRING* str, int pos, STB_TEXTEDIT_CHARTYPE* newtext, int num);
int delete_chars(STB_TEXTEDIT_STRING* str, int pos, int num);
void layout_func(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i);

#define KEYDOWN_BIT                    0x80000000

#define STB_TEXTEDIT_STRINGLEN(tc)     ((tc)->length)
#define STB_TEXTEDIT_LAYOUTROW         layout_func
#define STB_TEXTEDIT_GETWIDTH(tc,n,i)  wz_get_font_width(tc, n, i)
#define STB_TEXTEDIT_KEYTOTEXT(key)    (((key) & KEYDOWN_BIT) ? 0 : (key))
#define STB_TEXTEDIT_GETCHAR(tc,i)     ((tc)->buffer[i])
#define STB_TEXTEDIT_NEWLINE           '\n'
#define STB_TEXTEDIT_IS_SPACE(ch)      isspace(ch)
#define STB_TEXTEDIT_DELETECHARS       delete_chars
#define STB_TEXTEDIT_INSERTCHARS       insert_chars

#define STB_TEXTEDIT_K_SHIFT           0x40000000
#define STB_TEXTEDIT_K_CONTROL         0x20000000
#define STB_TEXTEDIT_K_LEFT            (KEYDOWN_BIT | 1) // actually use VK_LEFT, SDLK_LEFT, etc
#define STB_TEXTEDIT_K_RIGHT           (KEYDOWN_BIT | 2) // VK_RIGHT
#define STB_TEXTEDIT_K_UP              (KEYDOWN_BIT | 3) // VK_UP
#define STB_TEXTEDIT_K_DOWN            (KEYDOWN_BIT | 4) // VK_DOWN
#define STB_TEXTEDIT_K_LINESTART       (KEYDOWN_BIT | 5) // VK_HOME
#define STB_TEXTEDIT_K_LINEEND         (KEYDOWN_BIT | 6) // VK_END
#define STB_TEXTEDIT_K_TEXTSTART       (STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_TEXTEND         (STB_TEXTEDIT_K_LINEEND   | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_DELETE          (KEYDOWN_BIT | 7) // VK_DELETE
#define STB_TEXTEDIT_K_BACKSPACE       (KEYDOWN_BIT | 8) // VK_BACKSPACE
#define STB_TEXTEDIT_K_UNDO            (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | 'z')
#define STB_TEXTEDIT_K_REDO            (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | 'y')
#define STB_TEXTEDIT_K_INSERT          (KEYDOWN_BIT | 9) // VK_INSERT
#define STB_TEXTEDIT_K_WORDLEFT        (STB_TEXTEDIT_K_LEFT  | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_WORDRIGHT       (STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_PGUP            (KEYDOWN_BIT | 10) // VK_PGUP -- not implemented
#define STB_TEXTEDIT_K_PGDOWN          (KEYDOWN_BIT | 11) // VK_PGDOWN -- not implemented

//==============================================================================
// STRUCTS AND ENUMS
//==============================================================================


typedef enum EditorWindow {
	EditorWindow_Lists,
	EditorWindow_Slice_Spritesheet
} EditorWindow;

typedef enum EditorTab {
	EditorTab_Entities,
	EditorTab_Events,
	EditorTab_Areas,
	EditorTab_Inventory
} EditorTab;

typedef enum PanelId {
	PanelId_None,
	PanelId_Window,
	PanelId_TopPanel,
	PanelId_RightPanel,
	PanelId_GameScreen,
	PanelId_ModifyEntityDropDown
} PanelId;

typedef enum EditorWindowId {
	EditorWindowId_ModifyEntity,
	EditorWindowId_DrawTool
} EditorWindowId;

// WzColor is now unsigned int in RGBA format (0xRRGGBBAA)

typedef enum WzSizePolicyFlag
{
	WzSizePolicyFlagGrow = 1 << 0,
	WzSizePolicyFlagExpand = 1 << 1,
	WzSizePolicyFlagShrink = 1 << 2,
} WzSizePolicyFlag;

typedef enum WzState {
	WZ_INACTIVE, WZ_ACTIVATING, WZ_ACTIVE, WZ_DEACTIVATING
} WzState;

typedef struct WzRect {
	float x, y;
	float w, h;
} WzRect;

typedef struct wzrd_v2 {
	int x, y;
} wzrd_v2;

typedef struct wzrd_v2f {
	float x, y;
} wzrd_v2f;

typedef enum WzBorderType
{
	WZ_BORDER_TYPE_NONE, WZ_BORDER_TYPE_TAB, WZ_BORDER_TYPE_DEFAULT, WZ_BORDER_TYPE_RED,
	WZ_BORDER_TYPE_BLACK, WZ_BORDER_TYPE_CLICKED,
	WZ_BORDER_TYPE_TEXT_BOX, WZ_BORDER_TYPE_BOTTOM_LINE, BorderType_LeftLine, BorderType_Custom
} WzBorderType;

typedef struct wzrd_texture {
	void* data;
	float w, h;
} WzTexture;

typedef enum ItemType {
	ItemType_None,
	WZ_WIDGET_ITEM_TYPE_STRING,
	WZ_ITEM_TYPE_RECT,
	ItemType_RectAbsolute,
	ItemType_Texture,
	ItemType_DropdownIcon,
	ItemType_CloseIcon,
	ItemType_VerticalLine,
	ItemType_LineAbsolute,
	ItemType_DottedLineAbsolute,
	ItemType_HorizontalLineAbsolute,
	ItemType_HorizontalLine,
	ItemType_VerticalDottedLine,
	ItemType_BottomVerticalDottedLine,
	ItemType_TopVerticalDottedLine,
	ItemType_HorizontalDottedLine,
	ItemType_LeftHorizontalDottedLine,
	ItemType_RightHorizontalDottedLine,
	ItemType_Line,
	ItemType_IconClose,
} ItemType;

typedef struct Line {
	int x0, y0, x1, y1;
} Line;

typedef struct wzrd_str
{
	char str[128];
	size_t len;
} WzStr;

typedef struct Item {
	ItemType type;
	wzrd_v2 size;
	unsigned int color;
	int margin_left, margin_right, margin_top, margin_bottom;
	union {
		WzStr str;
		WzTexture texture;
		WzRect rect;
		Line line;
	} val;
	bool scissor;
	bool center_w, center_h;
	float x, y, w, h;
	unsigned font_id;
	
} WzWidgetItem;

typedef enum ButtonType {
	ButtonType_None,
	ButtonType_Flat,
	ButtonType_ThreeDimensional
} ButtonType;

typedef struct wzrd_widget_id {
	WzStr val;
} wzrd_widget_id;

enum
{
	WZ_WIDGET_TYPE_COMMAND_NONE,
	WZ_WIDGET_TYPE_COMMAND_BUTTON,
	WZ_WIDGET_TYPE_TOGGLE,
	WZ_WIDGET_TYPE_INPUT_BOX,
	WZ_WIDGET_TYPE_SLIDER,
	WZ_WIDGET_TYPE_DROPDOWN,
};

typedef struct wzrd_handle
{
	unsigned handle;
	unsigned type;
} WzWidget;

typedef struct Crate {
	int layer;
	int index;

	int box_stack[32];
	int box_stack_count;

	bool current_column_mode;
	int current_child_gap;
} Crate;

typedef enum EguiDrawCommandType {
	DrawCommandType_Default,
	DrawCommandType_Rect,
	DrawCommandType_RectAbsolute,
	DrawCommandType_Line,
	DrawCommandType_LineDotted,
	DrawCommandType_VerticalLine,
	DrawCommandType_HorizontalLine,
	WZ_DRAW_COMMAND_TYPE_TEXT,
	DrawCommandType_Texture,
	DrawCommandType_Clip,
	DrawCommandType_StopClip,
	WZ_DRAW_COMMAND_TYPE_D
} EguiDrawCommandType;

typedef struct EguiDrawCommand {
	EguiDrawCommandType type;
	char* source;
	WzStr str;
	float x, y, w, h;
	union {
		struct { WzRect src_rect; };
		Line line;
	};
	unsigned int color;
	WzTexture texture;
	int z;
	float rotation_angle;
	unsigned widget_index;
	unsigned font_id;
} WzDrawCommand;

typedef struct WzDrawCommandBuffer {
	WzDrawCommand commands[MAX_NUM_DRAW_COMMANDS];
	int count;
} WzDrawCommandBuffer;

typedef struct WzSource
{
	char* file;
	unsigned int line;
} WzSource;

typedef enum
{
	WZ_TEXT_ALIGNMENT_LEFT,
	WZ_TEXT_ALIGNMENT_CENTER,
};

typedef struct
{
	WzWidget handle;
	char source[MAX_SOURCE_SIZE];
	
	unsigned unique_id;

	// New layout stuff
	unsigned int min_w, min_h, constraint_max_h, constraint_max_w;
	WzWidget parent;
	unsigned char main_axis_size_type, size_type_vertical;
	unsigned int flex_fit; // Should the widget take all the space given to it 
	int w_offset, h_offset;

	// ...
	int actual_x, actual_y;
	unsigned int actual_w, actual_h;
	unsigned int layout;

	// Old
	bool disable_hover;
	WzWidget clip_widget;
	bool disable_input;
	bool is_draggable, is_slot;
	bool free;

	WzWidget children[MAX_NUM_CHILDREN];
	unsigned char children_count;

	unsigned int free_children[MAX_NUM_CHILDREN];
	unsigned char free_children_count;

	WzWidgetItem items[MAX_NUM_ITEMS];
	unsigned int items_count;
	bool bring_to_front;
	bool is_selected;

	float percentage_w, percentage_h;
	unsigned int flex_factor;

	unsigned int margin_right, margin_bottom, margin_left, margin_top;
	unsigned int pad_right, pad_bottom, pad_left, pad_top;

	int child_gap;
	bool fit_h, fit_w;
	bool best_fit;
	unsigned int cross_axis_alignment;
	unsigned int main_axis_alignment;

	unsigned int font_color;
	unsigned font_id;
	unsigned int color;
	unsigned int b0, b1, b2, b3;
	WzBorderType border_type;
	WzBorderType window_border_type;

	const char* tag;
	const char* secondary_tag;

	unsigned x, y;
	int z;

	bool free_from_parent;
	bool cull;

	bool ignore_unique_id;

	unsigned type;

	int pos[2];
	int	rotation;
	float scale[2];

	int world_pos[2];
	int	world_rotation;
	float world_scale[2];

	bool absolute_scale;

	unsigned text_alignment;
	bool disable;

	bool clip;
	float clip_x, clip_y, clip_w, clip_h;

	// Input box configuration (set by wz_text_box_raw, read by wz_text_box_run)
	unsigned input_flags;
	bool* input_committed;
	WzInputState* input_state;

	bool clip_content;

	// State
	bool* released_or_active;
	WzWidget slider;
	float* slider_pos;
	bool* active;

	// Options
	bool dont_show_special_border_on_click;

	// New code for layouting
	unsigned chunk, slot, layout_chunk;


} WzWidgetData;

typedef struct WzScene
{
	WzWidget root;
	int center_x, center_y;
	unsigned w, h;
} WzScene;

typedef struct CachedBox
{
	const char* tag;
	WzWidgetData box;
} CachedBox;

typedef struct wzrd_keyboard_key {
	char val;
	WzState state;
} wzrd_keyboard_key;

typedef struct WzKeyboard
{
	WzState keys[512];
} WzKeyboard;

typedef struct wzrd_keyboard_keys {
	wzrd_keyboard_key keys[32];
	int count;
} WzKeyboardKeys;

typedef struct wzrd_icons {
	WzTexture close, delete, entity, play, pause, stop, dropdown;
} wzrd_icons;

typedef enum wzrd_cursor {
	wzrd_cursor_default,
	wzrd_cursor_hand,
	wzrd_cursor_vertical_arrow,
	wzrd_cursor_horizontal_arrow
} wzrd_cursor;

typedef void (*ScrollbarCallback)(WzWidget, WzWidget, WzWidget);

typedef struct WzScrollbar
{
	WzWidget panel, scrollbar, content, top_button, bottom_button;
	unsigned int* scroll;
} WzScrollbar;

typedef struct WzTreeNodeData
{
	const char* name;
	WzTexture texture;
	bool expand;
	unsigned children_index;
	unsigned children_count;
	unsigned depth;
	WzWidget row_widget, icon_widget, expand_widget;
	bool visible;
} WzTreeNodeData;

typedef struct WzTreeNode
{
	unsigned index;
} WzTreeNode;

typedef struct WzTree
{
	WzTreeNodeData* nodes;
	unsigned nodes_count;
	WzTreeNode* children_indices;
	unsigned children_count;
	WzWidget menu;
	WzWidget selected_row;
} WzTree;

enum
{
	WZ_EVENT_TYPE_NONE,
	WZ_EVENT_TYPE_MOUSE,
	WZ_EVENT_TYPE_KEYBOARD,
};

typedef struct WzKeyboardEvent
{
	unsigned type;
	bool down;
	bool repeat;
	unsigned key;
	unsigned short mod;
} WzKeyboardEvent;

typedef struct WzButtonEvent
{
	unsigned type;
	unsigned char button;
	bool down;
	float x, y;
} WzButtonEvent;

typedef union WzEevent
{
	unsigned type;
	WzKeyboardEvent key;
	WzButtonEvent button;
} WzEvent;

enum
{
	WZ_KMOD_NONE = 0x0000,

	WZ_KMOD_LSHIFT = 0x0001,
	WZ_KMOD_RSHIFT = 0x0002,
	WZ_KMOD_LCTRL = 0x0040,
	WZ_KMOD_RCTRL = 0x0080,
	WZ_KMOD_LALT = 0x0100,
	WZ_KMOD_RALT = 0x0200,
	WZ_KMOD_LGUI = 0x0400,
	WZ_KMOD_RGUI = 0x0800,
	
	// Combined flags (convenience)
	WZ_KMOD_CTRL = WZ_KMOD_LCTRL | WZ_KMOD_RCTRL,
	WZ_KMOD_SHIFT = WZ_KMOD_LSHIFT | WZ_KMOD_RSHIFT,
	WZ_KMOD_ALT = WZ_KMOD_LALT | WZ_KMOD_RALT,
	WZ_KMOD_GUI = WZ_KMOD_LGUI | WZ_KMOD_RGUI,
};

#define DOUBLE_CLICK_TIME_MS 300

#define WZ_CHUNK_SIZE 32

typedef struct WzChunk
{
	// Chunk data
	uint16_t min_width[WZ_CHUNK_SIZE];
	uint16_t min_height[WZ_CHUNK_SIZE];
	uint8_t flex[WZ_CHUNK_SIZE];
	uint16_t margin_left[WZ_CHUNK_SIZE];
	uint16_t margin_right[WZ_CHUNK_SIZE];
	uint16_t margin_top[WZ_CHUNK_SIZE];
	uint16_t margin_bottom[WZ_CHUNK_SIZE];
	uint16_t max_width[WZ_CHUNK_SIZE];
	uint16_t max_height[WZ_CHUNK_SIZE];
	uint16_t cross_align[WZ_CHUNK_SIZE];
	uint16_t available_width[WZ_CHUNK_SIZE];
	uint16_t available_height[WZ_CHUNK_SIZE];
	uint32_t color[WZ_CHUNK_SIZE];

	uint16_t absolute_x[WZ_CHUNK_SIZE];
	uint16_t absolute_y[WZ_CHUNK_SIZE];
	uint16_t absolute_w[WZ_CHUNK_SIZE];
	uint16_t absolute_h[WZ_CHUNK_SIZE];

	uint16_t relative_x[WZ_CHUNK_SIZE];
	uint16_t relative_y[WZ_CHUNK_SIZE];

	uint16_t widget_index[WZ_CHUNK_SIZE];

	uint16_t count;
} WzChunk;

typedef struct WzChunkLayout
{
	// Chunk layout data
	uint16_t pad_left, pad_right, pad_top, pad_bottom;
	uint16_t border_left, border_right, border_top, border_bottom;
	uint16_t child_gap;
	uint16_t min_width, min_height;
	uint16_t flex_total;
	uint16_t inner_width, inner_height;
	uint16_t cursor_x, cursor_y;
	int32_t  shrink_width, shrink_height;
	uint32_t child_count;
	uint32_t parent_chunk, parent_slot;
	//bool  is_horizontal;
	uint8_t layout_type;
	uint16_t total_children_min_width, total_children_min_height;
	uint16_t w_per_flex_cache, h_per_flex_cache;
	uint16_t total_child_count;

	//int32_t  is_continuation;
	//int32_t  overflow_group_head;
	unsigned chunk;
	unsigned chunk_stride;

	int16_t available_width, available_height;

} WzChunkLayout;

typedef struct WzGui
{
	WzWidgetData persistent_widgets[MAX_NUM_PERSISTENT_WIDGETS];
	unsigned persistent_widgets_count;

	// Persistent
	WzWidget hovered_item, hot_item_previous, active_item, clicked_item, activating_item, deactivating_item, dragged_item;
	WzWidget right_resized_item, left_resized_item, bottom_resized_item, top_resized_item;
	//WzWidget active_input_box;
	WzWidget hovered_items_list[MAX_NUM_WIDGETS];
	int hovered_items_list_count;
	WzWidgetData hovered_boxes[MAX_NUM_WIDGETS];
	int hovered_boxes_count;

	WzRect window;
	double tooltip_time;
	WzWidgetData dragged_box;
	bool clean;
	void (*get_string_size)(char*, unsigned, unsigned, unsigned, float*, float*);

	wzrd_v2f mouse_pos, previous_mouse_pos, mouse_delta, screen_mouse_pos;
	WzKeyboard keyboard;
	WzState mouse_left, mouse_right;

	// Frame ?
	WzWidgetData widgets[MAX_NUM_WIDGETS * 8];
	bool occupied[MAX_NUM_WIDGETS];
	unsigned widgets_count;

	int boxes_in_stack_count, total_num_windows;

	Crate crates_stack[32];
	int current_crate_index;

	bool enable_input;

	int styles_count;
	wzrd_cursor cursor;
	WzDrawCommandBuffer commands_buffer;

	WzWidgetData cached_boxes[MAX_NUM_WIDGETS];
	int cached_boxes_count;

	WzRect* rects; // aka Final Layout

	WzScrollbar scrollbars[MAX_NUM_WIDGETS];
	unsigned scrollbars_count;

	WzWidget boxes_indices[MAX_NUM_WIDGETS];
	unsigned  boxes_indices_count;

	WzTree trees[8];

	WzTexture x_icon;

	float zoom_factor;
	int camera_x, camera_y;

	WzScene scenes[MAX_NUM_SCENES];
	unsigned scenes_count;

	// NEW 14.02 from stb
	WzWidget active_input;

#define MAX_NUM_EVENTS 128
	WzEvent events[MAX_NUM_EVENTS];
	unsigned events_count;

	char* pasted_text;
	char copied_text[128];

	unsigned long (*ticks_in_ns)();
	unsigned long (*ticks_in_ms)();

	unsigned focused_widget_unique_id;
	unsigned focused_widget_index;


	// New 3.15 for layouting
	WzChunk chunks[MAX_NUM_WIDGETS];
	unsigned chunks_count;

	WzChunkLayout layouts[MAX_NUM_WIDGETS];
	unsigned layouts_count;

} WzGui;

typedef struct HashTableElement {
	char key[32];
	int value;
} HashTableElement;

typedef struct HashTable {
	HashTableElement elements[MAX_NUM_HASHTABLE_ELEMENTS];
	int num;
} HashTable;

typedef struct WidgetData {
	int active; // ie selected box
	bool flag;
	char text[32];
} Widget;

typedef struct Label_list {
	WzStr val[MAX_NUM_LABELS];
	int count;
} Label_list;

typedef struct WzLayoutWidget
{
	unsigned int index;
} WzLayoutWidget;

typedef struct wzrd_polygon {
	wzrd_v2 vertices[MAX_NUM_VERTICES_IN_POLYGON];
	int count;
} wzrd_polygon;

enum
{
	WZ_EXPANSION_NONE,
	WZ_EXPANSION_SHRUNK,
	WZ_EXPANSION_EXPANDED,
};

typedef struct wzl_str
{
	char* str;
	unsigned int len;
} wzl_str;

typedef struct WzlRect
{
	int x, y;
	unsigned int w, h;
} WzLayoutRect;

enum
{
	CROSS_AXIS_ALIGNMENT_START,
	WZ_CROSS_AXIS_ALIGNMENT_END,
	WZ_CROSS_AXIS_ALIGNMENT_CENTER,
	WZ_CROSS_AXIS_ALIGNMENT_STRETCH,
	CROSS_AXIS_ALIGNMENT_BASELINE,
	CROSS_AXIS_ALIGNMENT_TOTAL,
};

enum
{
	WZ_MAIN_AXIS_ALIGNMENT_START,
	WZ_MAIN_AXIS_ALIGNMENT_END,
	WZ_MAIN_AXIS_ALIGNMENT_CENTER,
};

typedef struct WzLayoutSource
{
	char* file;
	unsigned int line;
} WzLayoutSource;

#if 0
typedef struct WzWidgetDescriptor
{
	char* source;
	unsigned handle;
	unsigned int constraint_min_w, constraint_min_h, constraint_max_w, constraint_max_h;
	unsigned int layout;
	unsigned int pad_left, pad_right, pad_top, pad_bottom;
	unsigned int gap;
	unsigned int* children;
	unsigned int children_count;
	unsigned int flex_factor;
	unsigned char free_from_parent;
	unsigned char flex_fit;
	unsigned char main_axis_size_type;
	unsigned int alignment;
	unsigned int cross_axis_alignment;
	int x, y;
	unsigned margin_left, margin_right, margin_top, margin_bottom;
} WzWidgetDescriptor;
#endif

enum
{
	WZ_LAYOUT_STAGE_NON_FLEX_CHILDREN,
	WZ_LAYOUT_STAGE_FLEX_CHILDREN,
	WZ_LAYOUT_STAGE_PARENT,
};

typedef struct WzDebugInfo
{
	unsigned int stage;
	unsigned int index;
	unsigned int constraint_min_w, constraint_min_h, constraint_max_w, constraint_max_h;
	unsigned int x, y, w, h;
} WzDebugInfo;

typedef struct WzLogMessage
{
	char str[WZ_LOG_MESSAGE_MAX_SIZE];
} WzLogMessage;

//==============================================================================
// FUNCTION DECLARATIONS
//==============================================================================


// Core API
void wz_set_string_size_callback(void (*get_string_size)(char*, unsigned, unsigned, unsigned, float*, float*));
void wz_widget_set_font(WzWidget widget, unsigned font_id);
wz_set_gui(WzGui* gui_in);
WzWidget wz_create_handle();
void wz_gui_init(WzGui* wz);
void wz_gui_deinit(WzGui* wz);
WzWidget wz_begin(
	unsigned window_w, unsigned  window_h,
	//float mouse_x, float mouse_y,
	//WzState left_mouse_state,
	WzEvent* events,
	unsigned events_count,
	bool enable_input);
void wz_spacer(WzWidget parent);
void wz_slider(WzWidget parent, unsigned width, float* pos);
void wz_widget_scale(WzWidget widget, float w, float h);
void wz_widget_rotate(WzWidget widget, float rotation);
void wz_end();
void wz_tabs(WzWidget parent, WzStr* tab_names, unsigned tabs_count, WzWidget* panels, unsigned* current_tab);
void wz_widget_set_margins(WzWidget w, unsigned int pad);
void wz_widget_set_pad(WzWidget w, unsigned  pad);
void wz_widget_set_child_gap(WzWidget widget, unsigned int child_gap);
void wz_widget_set_constraints(WzWidget widget, unsigned int min_w, unsigned int min_h, unsigned int max_w, unsigned int max_h);
WzWidget wz_widget_raw(WzWidget parent, const char* file, unsigned int line);
void wz_widget_add_rect(WzWidget widget, unsigned int w, unsigned int h, unsigned int color);
void wz_widget_set_margin_bottom(WzWidget widget, unsigned int pad);
void wz_widget_set_margin_top(WzWidget widget, unsigned int pad);
void wz_widget_set_margin_left(WzWidget widget, unsigned int pad);
void wz_widget_set_margin_right(WzWidget widget, unsigned int pad);
void wz_widget_set_max_constraints(WzWidget widget, unsigned int w, unsigned int h);
void wz_widget_set_main_axis_size_min(WzWidget w);
void wz_widget_set_z(WzWidget handle, unsigned int layer);
void wz_widget_set_fixed_size(WzWidget widget, unsigned int w, unsigned int h);
void wz_widget_set_layout(WzWidget handle, unsigned int layout);
void wz_widget_set_stretch_factor(WzWidget handle, unsigned int strech_factor);
WzWidget wzrd_widget_free(WzWidget parent);
void wz_widget_add_child(WzWidget parent, WzWidget child);
WzWidgetData* wz_widget_get(WzWidget handle);
WzStr wz_str_create(char* str);
int wzrd_box_get_current_index();
bool wzrd_box_is_active(WzWidgetData* box);
bool wzrd_box_is_activating(WzWidgetData* box);
bool wzrd_is_releasing();
bool wzrd_box_is_dragged(WzWidgetData* box);
void wzrd_box_resize(wzrd_v2* size);
void wz_widget_add_item(WzWidget box, WzWidgetItem item);
bool wzrd_box_is_hot(WzWidgetData* box);
bool wzrd_box_is_hot_using_canvas(WzGui* canvas, WzWidgetData* box);
bool wz_widget_is_equal(WzWidget a, WzWidget b);
WzGui* wzrd_canvas_get();
WzWidget wz_widget_persistent(WzWidget parent, WzWidgetData widget_data);
WzWidgetData wzrd_widget_get_cached_box(const char* tag);
void wz_widget_add_tag(WzWidget widget, const void* str);
void wz_widget_clip(WzWidget handle);
bool wz_widget_is_deactivating(WzWidget handle);
bool wz_widget_is_active(WzWidget handle);
bool wz_widget_is_interacting(WzWidget handle);
bool wz_widget_is_activating(WzWidget handle);
bool wz_is_any_widget_activating();
WzWidgetData wzrd_widget_get_cached_box_with_secondary_tag(const char* tag, const char* secondary_tag);
void wz_widget_set_color_old(WzWidget widget, unsigned int color);
void wz_widget_set_max_constraint_w(WzWidget w, int width);
void wz_widget_set_max_constraint_h(WzWidget h, int height);
void wz_frame(WzWidget parent, unsigned w, unsigned h, WzStr str);
void wz_widget_set_x(WzWidget w, int x);
void wz_widget_set_type(WzWidget w, unsigned type);
void wz_widget_set_y(WzWidget h, int y);
void wz_widget_data_set_x(WzWidgetData* w, int x);
void wz_widget_data_set_y(WzWidgetData* h, int y);
void wz_widget_set_pos(WzWidget handle, int x, int y);
void wz_widget_data_set_pos(WzWidgetData* handle, int x, int y);
bool wz_handle_is_valid(WzWidget handle);
void wz_widget_set_tight_constraints(WzWidget handle, unsigned w, unsigned h);
void wz_widget_set_border(WzWidget w, WzBorderType border_type);
void wz_widget_data_set_tight_constraints(WzWidgetData* handle, unsigned w, unsigned h);
void wz_widget_data_set_border(WzWidgetData* w, WzBorderType border_type);
WzWidgetData wz_widget_create(WzWidget parent);
void wz_widget_resize(WzWidget widget, int* w, int* h);
void wz_widget_set_flex_factor(WzWidget widget, unsigned int flex_factor);
void wz_widget_set_expanded(WzWidget widget);
void wz_widget_set_flex(WzWidget widget);
void wz_widget_set_size(WzWidget c, unsigned int w, unsigned int h);
void wz_widget_set_free_from_parent(WzWidget w);
void wz_widget_set_free_from_parent_vertically(WzWidget w);
void wz_widget_set_color(WzWidget widget, unsigned int color);
void wz_widget_set_cross_axis_alignment(WzWidget widget, unsigned int cross_axis_alignment);
void wz_widget_set_main_axis_alignment(WzWidget widget, unsigned int cross_axis_alignment);
void wz_widget_ignore_unique_id(WzWidget widget);

// Widget Creation (Raw Functions)
WzWidget wzrd_label_button_raw(WzStr str, bool* result, WzWidget parent, const char* file, unsigned int line);
WzWidget wz_button_icon_raw(WzWidget parent, bool* result, WzTexture texture, const char* file, unsigned int line);
WzWidget wz_toggle_icon_raw(WzWidget parent, bool* result, WzTexture texture, const char* file, unsigned int line);
WzWidget wz_command_button_raw(WzWidget parent, WzStr str, bool* b, const char* file, unsigned int line);
WzWidget wz_dropdown(WzWidget parent, const WzStr* texts, int texts_count, unsigned* selected_text, bool* active);
WzWidget wz_dialog_raw(int* x, int* y, unsigned* w, unsigned* h, bool* active, WzStr name, int layer, WzWidget parent, const char* file, unsigned int line);
WzWidget wz_command_toggle_raw(WzWidget parent, WzStr str, bool* active, const char* file, unsigned int line);
WzWidget wz_icon_toggle_raw(WzWidget parent, WzTexture texture, unsigned w, unsigned h, bool* active, const char* file, unsigned int line);
void wz_label_list_sorted_raw(WzStr* item_names, unsigned int count, unsigned* items, unsigned *ids, unsigned int width, unsigned int height, unsigned int color, unsigned int* selected, bool* is_selected, WzWidget parent, const char* file, unsigned int line);
void wzrd_label_list_raw(WzWidget parent, WzStr* item_names, unsigned int count, unsigned *items,
	unsigned *unique_ids, unsigned int width, unsigned int height, unsigned int color,
	WzWidget* handles, unsigned int* selected, bool* is_selected, const char* file, unsigned int line);
// Flags for wz_input_box
#define WZ_INPUT_NONE        0
#define WZ_INPUT_AUTO_SELECT (1 << 0)   // select all text when focused
#define WZ_INPUT_GOTO_END    (1 << 1)   // move cursor to end when focused
#define WZ_INPUT_READ_ONLY   (1 << 2)   // display only, no editing
#define WZ_INPUT_NO_CURSOR   (1 << 3)   // hide blinking caret
#define WZ_INPUT_PASSWORD    (1 << 4)   // mask text with * (blocks copy/cut)
#define WZ_INPUT_ALLOW_TAB   (1 << 5)   // Tab key inserts a tab character

// Character filter callback: return nonzero to allow, zero to block
typedef int (*WzInputFilter)(unsigned int c);
int wz_filter_decimal(unsigned int c);
int wz_filter_float(unsigned int c);
int wz_filter_hex(unsigned int c);
int wz_filter_alpha(unsigned int c);
int wz_filter_alphanumeric(unsigned int c);

WzWidget wz_text_box_raw(WzWidget parent, WzInputState* state, unsigned flags, WzInputFilter filter, bool* committed, const char* placeholder, const char* file, unsigned int line);
WzWidget wzrd_handle_button_raw(bool* active, WzRect rect, unsigned int color, WzStr name, WzWidget parent, const char* file, unsigned int line);
WzWidget wz_label_raw(WzWidget parent, WzStr str, const char* file, unsigned int line);
WzWidget wzrd_vbox_border_raw(wzrd_v2 size, WzWidget parent, const char* file, unsigned int line);
WzWidget wz_hbox_raw(WzWidget parent, const char* file, unsigned int line);
WzWidget wz_vbox_raw(WzWidget parent, const char* file, unsigned int line);
WzWidget wzrd_label_button_activating_raw(WzStr str, bool* active, WzWidget parent, const char* file, unsigned int line);
WzWidget wz_tree_add_row_raw(WzTree* tree, WzStr str, WzTexture texture, unsigned depth,
	bool* expand, bool* selected, WzTreeNodeData* node, const char* file, unsigned line);
WzWidget wz_toggle_raw(WzWidget parent, unsigned w, unsigned h, unsigned int color,
	bool* active, const char* file_name, unsigned int line);
WzWidget wz_texture_raw(WzWidget parent, WzTexture texture, unsigned w, unsigned h, const char* file_name, unsigned int line);
void wz_widget_add_horizontal_line(WzWidget widget, unsigned w);
void wz_widget_add_vertical_line(WzWidget widget, unsigned h);
void wz_do_layout_refactor_me(int from, int to);
void wz_widget_add_rect_absolute(WzWidget widget, int x, int y, unsigned w, unsigned h, unsigned int color);
WzWidget wz_vpanel_raw(WzWidget parent, const char* file, unsigned int line);
WzWidget wz_hpanel_raw(WzWidget parent, const char* file, unsigned int line);
WzWidget wz_panel_raw(WzWidget parent, const char* file, unsigned int line);
bool wz_widget_is_valid(WzWidget handle);
void wz_widget_set_font_color(WzWidget widget, unsigned color);

// Additional Functions
WzWidget wz_widget_add_to_frame(WzWidget parent, WzWidgetData widget);
void wz_zoom(float x);
void wz_transform_x(unsigned x);
void wz_add_resize_widgets_maintain_aspect_ratio(WzWidget parent, float* scale_x, float* scale_y, float* transform_x, float* transform_y);
WzWidget wz_scroll_box(wzrd_v2 size, unsigned int* scroll, WzWidget parent, const void* tag);
void wz_add_persistent_widget(WzWidgetData widget);
void wz_widget_disable(WzWidget widget, bool disable);
void wz_add_resize_widgets_maintain_aspect_ratio2(WzWidget parent, float* angle);
void wz_widget_transform(WzWidget widget, float x, float y);

// Layout Functions
void wz_layout(unsigned int index,
	WzWidgetData* widgets, WzLayoutRect* rects,
	unsigned int count, unsigned int* failed);
WzWidget wz_scene(WzScene scene, WzWidget parent, WzTexture texture, int x, int y, unsigned w, unsigned h);

//==============================================================================
// MACROS
//==============================================================================

#define wz_texture(parent, texture, w, h, file_name, line) wz_texture_raw(parent, texture, w, h, __FILE__, __LINE__)
#define wz_toggle(parent, w, h, color, active, file_name, line) wz_toggle_raw(parent, w, h, color, active, __FILE__, __LINE__)
#define wz_tree_add_row(tree, str, texture, depth, expand, selected, node) wz_tree_add_row_raw(tree, str, texture, depth, expand, selected, node, __FILE__, __LINE__)
#define wz_label_button(str, result, parent) wzrd_label_button_raw(str, result, parent, __FILE__, __LINE__)
#define wz_button_icon(parent, result, texture) wz_button_icon_raw(parent, result, texture, __FILE__, __LINE__)
#define wz_toggle_icon(parent, result, texture) wz_toggle_icon_raw(parent, result, texture, __FILE__, __LINE__)
#define wzrd_dropdown(selected_text, texts, texts_count, w, active, parent) wzrd_dropdown_raw(selected_text, texts, texts_count, w, active, parent, __FILE__, __LINE__)
#define wz_dialog(x, y, w, h, active, name, layer, parent) wz_dialog_raw(x, y, w, h, active, name, layer, parent, __FILE__, __LINE__)
#define wz_command_toggle(parent, str, active) wz_command_toggle_raw(parent, str, active, __FILE__, __LINE__)
#define wz_icon_toggle(parent, texture, w, h, active) wz_icon_toggle_raw(parent, texture, w, h, active, __FILE__, __LINE__)
#define wz_label_list_sorted(item_names, count, items, ids, width, height, color, selected, is_selected, parent) wz_label_list_sorted_raw(item_names, count, items, ids, width, height, color, selected, is_selected, parent, __FILE__, __LINE__)
#define wzrd_label_list(parent, item_names, count, items, ids, width, height, color, handles, selected, is_selected) wzrd_label_list_raw(parent, item_names, count, items, ids, width, height, color, handles, selected, is_selected, __FILE__, __LINE__)
#define wzrd_handle_button(active, rect, color, name, parent) wzrd_handle_button_raw(active, rect, color, name, parent, __FILE__, __LINE__)
#define wzrd_vbox_border(size, parent) wzrd_vbox_border_raw(size, parent, __FILE__, __LINE__)
#define wz_hbox(parent) wz_hbox_raw(parent,  __FILE__, __LINE__)
#define wz_vbox(parent) wz_vbox_raw(parent,  __FILE__, __LINE__)
#define wz_widget(parent) wz_widget_raw(parent,  __FILE__, __LINE__)
#define wzrd_label_button_activating(str, active, parent) wzrd_label_button_activating_raw(str, active, parent, __FILE__, __LINE__)
#define wz_vpanel(parent) wz_vpanel_raw(parent, __FILE__, __LINE__)
#define wz_hpanel(parent) wz_hpanel_raw(parent, __FILE__, __LINE__)
#define wz_panel(parent) wz_panel_raw(parent, __FILE__, __LINE__)

// High importance widgets
// Missing checkbox, radiobox, combox
#define wz_label(parent, str) wz_label_raw(parent, str, __FILE__, __LINE__)
#define wz_command_button(parent, str, b) wz_command_button_raw(parent, str, b, __FILE__, __LINE__)
#define wz_text_box(parent, state, flags, filter, committed, placeholder) \
	wz_text_box_raw(parent, state, flags, filter, committed, placeholder, __FILE__, __LINE__)
	
float wz_get_font_width(WzInputState *, int, int);

// Forward declare your library functions
void wz_input_key_event(int wz_keycode, bool is_down, bool is_repeat);
void wz_input_mouse_button_event(int button, bool is_down, float x, float y);
void wz_input_mouse_motion_event(float x, float y, float dx, float dy);
void wz_input_mouse_wheel_event(float dx, float dy);

// Numerical
// Spinbox, ?DoubleSpinBox, Slider, ProgressBar, Dial

// Containers
// GroupBox, TabWidget, StackedWidget, Splitter, ScrollArea

// Advanced
// Table, Tree, List, TextEdit, Calendar

// Windows
// MainWindow, Dialog, MessageBox

// MenuBar, Menu, Toolbar

// GraphicsView

// Layout Managers?

// Frame?

//==============================================================================
// STATIC DATA
//==============================================================================

#if 1
static unsigned wz_x_icon[] =
{
	0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff,
	0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0,
	0, 0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0, 0,
	0, 0, 0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0, 0, 0,
	0, 0, 0, 0, 0xff, 0, 0, 0, 0, 0, 0, 0xff, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0xff, 0, 0, 0, 0, 0xff, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0xff, 0, 0, 0xff, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0xff, 0, 0, 0xff, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0xff, 0, 0, 0, 0, 0xff, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0xff, 0, 0, 0, 0, 0, 0, 0xff, 0, 0, 0, 0,
	0, 0, 0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0, 0, 0,
	0, 0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0, 0,
	0, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0,
	0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0xff,
	0xff, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0xff
};
#endif


// WZ Keycode system
// 0-127: ASCII characters (printable and control codes)
// 128+: Extended keys (arrows, function keys, etc.)

typedef int WZ_Keycode;

// Extended keycodes (128+)
enum 
{
	WZ_KEY_UNKNOWN = 0,

	// ASCII range (0-127) is reserved for direct character codes
	// 'a' = 97, 'A' = 65, ' ' = 32, '\n' = 10, etc.

	// Extended keys start at 128
	WZ_KEY_CAPSLOCK = 128,
	WZ_KEY_F1,
	WZ_KEY_F2,
	WZ_KEY_F3,
	WZ_KEY_F4,
	WZ_KEY_F5,
	WZ_KEY_F6,
	WZ_KEY_F7,
	WZ_KEY_F8,
	WZ_KEY_F9,
	WZ_KEY_F10,
	WZ_KEY_F11,
	WZ_KEY_F12,

	WZ_KEY_PRINTSCREEN,
	WZ_KEY_SCROLLLOCK,
	WZ_KEY_PAUSE,
	WZ_KEY_INSERT,
	WZ_KEY_HOME,
	WZ_KEY_PAGEUP,
	WZ_KEY_DELETE,
	WZ_KEY_END,
	WZ_KEY_PAGEDOWN,
	WZ_KEY_RIGHT,
	WZ_KEY_LEFT,
	WZ_KEY_DOWN,
	WZ_KEY_UP,

	WZ_KEY_NUMLOCKCLEAR,
	WZ_KEY_KP_DIVIDE,
	WZ_KEY_KP_MULTIPLY,
	WZ_KEY_KP_MINUS,
	WZ_KEY_KP_PLUS,
	WZ_KEY_KP_ENTER,
	WZ_KEY_KP_1,
	WZ_KEY_KP_2,
	WZ_KEY_KP_3,
	WZ_KEY_KP_4,
	WZ_KEY_KP_5,
	WZ_KEY_KP_6,
	WZ_KEY_KP_7,
	WZ_KEY_KP_8,
	WZ_KEY_KP_9,
	WZ_KEY_KP_0,
	WZ_KEY_KP_PERIOD,

	WZ_KEY_APPLICATION,
	WZ_KEY_POWER,
	WZ_KEY_KP_EQUALS,
	WZ_KEY_F13,
	WZ_KEY_F14,
	WZ_KEY_F15,
	WZ_KEY_F16,
	WZ_KEY_F17,
	WZ_KEY_F18,
	WZ_KEY_F19,
	WZ_KEY_F20,
	WZ_KEY_F21,
	WZ_KEY_F22,
	WZ_KEY_F23,
	WZ_KEY_F24,

	WZ_KEY_EXECUTE,
	WZ_KEY_HELP,
	WZ_KEY_MENU,
	WZ_KEY_SELECT,
	WZ_KEY_STOP,
	WZ_KEY_AGAIN,
	WZ_KEY_UNDO,
	WZ_KEY_CUT,
	WZ_KEY_COPY,
	WZ_KEY_PASTE,
	WZ_KEY_FIND,
	WZ_KEY_MUTE,
	WZ_KEY_VOLUMEUP,
	WZ_KEY_VOLUMEDOWN,

	WZ_KEY_LCTRL,
	WZ_KEY_LSHIFT,
	WZ_KEY_LALT,
	WZ_KEY_LGUI,
	WZ_KEY_RCTRL,
	WZ_KEY_RSHIFT,
	WZ_KEY_RALT,
	WZ_KEY_RGUI,

	WZ_KEY_MODE,

	WZ_KEY_COUNT
};

#endif