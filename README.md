# WzGui

A lightweight immediate-mode GUI library written in pure C. Built for real-time applications and game tooling — no heap allocations per frame, no retained widget trees, no C++ required.

```c
WzWidget panel = wz_vbox(root);
wz_widget_set_pad(panel, 8);

static WzInputState name_input;
wz_label(panel, wz_str("Name"));
wz_text_box(panel, &name_input, WZ_INPUT_NONE, NULL, NULL, "Enter name...");

bool save = false;
wz_command_button(wz_str("Save"), &save, panel);
if (save) write_to_disk(name_input.buffer);
```

---

## Why WzGui?

Most immediate-mode GUIs in C are either toy projects or require C++. WzGui is built for production use inside a game editor, with a full layout engine, multi-font text rendering, and a platform-agnostic draw command system that works with any renderer.

---

## Features

- **Immediate-mode** — rebuild the entire UI every frame, no retained state to sync
- **Flex layout** — horizontal/vertical boxes, stretch factors, constraints, padding, margins, gaps
- **Rich text editing** — cursor, selection, undo/redo, word navigation, horizontal scroll, placeholder text
- **Multi-font** — assign any font and size per widget, measured via a user-provided callback
- **Input filters** — built-in filters for decimal, float, hex, alpha, alphanumeric input
- **Widget states** — `ACTIVATING` / `ACTIVE` / `DEACTIVATING` / `INACTIVE` for precise interaction control
- **Keyboard state** — query any ASCII key's state (pressed, held, released) each frame
- **Clipping** — clip widget content to its bounds; children inherit the clip region
- **Draw command buffer** — rendering is fully decoupled; supply your own SDL/OpenGL/etc. backend
- **No C++ required** — pure C99

---

## Widgets

| Widget | Description |
|---|---|
| `wz_label` | Static text |
| `wz_command_button` | Clickable button, returns released state |
| `wz_text_box` | Full-featured single-line text editor |
| `wz_command_toggle` | Two-state toggle button |
| `wz_button_icon` | Icon button with texture |
| `wz_hbox` / `wz_vbox` | Layout containers |
| `wz_panel` | Generic container |
| `wz_dialog` | Draggable floating window |
| `wz_scroll_box` | Scrollable container |
| `wz_tabs` | Tab bar with panel switching |
| `wz_texture` | Image widget |
| `wz_spacer` | Empty space for layout gaps |

---

## Layout

WzGui uses a flex-style layout system. Widgets stack along a main axis, with optional stretching, alignment, and spacing.

```c
WzWidget toolbar = wz_hbox(root);
wz_widget_set_child_gap(toolbar, 4);
wz_widget_set_pad(toolbar, 6);

WzWidget search = wz_text_box(toolbar, &state, WZ_INPUT_NONE, NULL, NULL, "Search...");
wz_widget_set_flex(search);         // stretch to fill available width

WzWidget btn = wz_command_button(wz_str("Go"), &go, toolbar);
wz_widget_set_size(btn, 60, 24);    // fixed size
```

Key layout API:

```c
wz_widget_set_size(w, width, height);
wz_widget_set_flex(w);                              // expand to fill
wz_widget_set_flex_factor(w, n);                    // relative weight among siblings
wz_widget_set_constraints(w, min_w, min_h, max_w, max_h);
wz_widget_set_pad(w, pad);                          // inner padding (all sides)
wz_widget_set_margins(w, margin);                   // outer margin (all sides)
wz_widget_set_child_gap(w, gap);                    // space between children
wz_widget_set_cross_axis_alignment(w, alignment);
```

---

## Text Boxes

```c
static WzInputState state;
bool committed = false;

WzWidget input = wz_text_box(parent, &state, WZ_INPUT_GOTO_END, wz_filter_decimal, &committed, "0");

if (committed) {
    int value = atoi(state.buffer);
}
```

**Flags:**

| Flag | Effect |
|---|---|
| `WZ_INPUT_NONE` | Default |
| `WZ_INPUT_AUTO_SELECT` | Select all on focus |
| `WZ_INPUT_GOTO_END` | Move cursor to end on focus |
| `WZ_INPUT_READ_ONLY` | Disable editing |
| `WZ_INPUT_NO_CURSOR` | Hide blinking caret |

**Built-in filters:**

```c
wz_filter_decimal       // digits only
wz_filter_float         // decimal + . - + e E
wz_filter_hex           // hex digits
wz_filter_alpha         // letters only
wz_filter_alphanumeric  // letters and digits
```

**Keyboard shortcuts (always active):**

| Shortcut | Action |
|---|---|
| `Ctrl+A` | Select all |
| `Ctrl+C / X / V` | Copy / Cut / Paste |
| `Ctrl+Z / Y` | Undo / Redo |
| `Ctrl+Left/Right` | Word navigation |
| `Ctrl+Backspace` | Delete word left |
| `Ctrl+Delete` | Delete word right |
| `Double-click` | Select word |
| `Triple-click` | Deselect |

---

## Widget States

```c
if (wz_widget_is_activating(widget))   { /* pressed this frame */ }
if (wz_widget_is_active(widget))       { /* held down */ }
if (wz_widget_is_deactivating(widget)) { /* released this frame */ }
if (wz_widget_is_interacting(widget))  { /* activating or active */ }
if (wz_widget_is_focused(widget))      { /* has keyboard focus */ }
```

---

## Keyboard State

Query any ASCII key's state directly each frame:

```c
if (wz->keyboard.keys['z'] == WZ_ACTIVATING)  { undo(); }
if (wz->keyboard.keys[' '] == WZ_ACTIVE)       { hold_action(); }
if (wz->keyboard.keys['\n'] == WZ_DEACTIVATING){ confirm(); }
```

States: `WZ_INACTIVE`, `WZ_ACTIVATING`, `WZ_ACTIVE`, `WZ_DEACTIVATING`

---

## Multi-Font

Provide a measurement callback and assign fonts per widget:

```c
void my_measure(char* str, unsigned start, unsigned end, unsigned font_id, float* w, float* h) {
    // measure str[start..end] using your font array
}

wz_set_string_size_callback(my_measure);
wz_widget_set_font(heading_label, FONT_LARGE);
wz_widget_set_font(body_label, FONT_REGULAR);
```

Font IDs are plain integers — WzGui does not manage font loading, giving you full control.

---

## Rendering

WzGui produces a `WzDrawCommandBuffer` each frame. Feed it to any renderer:

```c
wz_end();
wz_do_layout_refactor_me();

WzDrawCommandBuffer* cmds = &wz->commands_buffer;
for (int i = 0; i < cmds->count; i++) {
    WzDrawCommand* cmd = &cmds->commands[i];
    switch (cmd->type) {
        case WZ_DRAW_COMMAND_TYPE_TEXT: render_text(...); break;
        case DrawCommandType_Rect:      render_rect(...); break;
        case DrawCommandType_Clip:      set_scissor(...); break;
        case DrawCommandType_StopClip:  clear_scissor(); break;
        // ...
    }
}
```

---

## Dependencies

- [stb_truetype](https://github.com/nothings/stb) — font rasterization
- [stb_textedit](https://github.com/nothings/stb) — text editing backend
- A rendering backend of your choice (reference implementation uses SDL3)

---

## License

MIT
