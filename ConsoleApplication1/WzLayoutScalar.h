#pragma once
#include <stdint.h>

#define WZ_CHUNK_SIZE 8

struct WzChunk {
    float    pad_left, pad_right, pad_top, pad_bottom;
    float    border_left, border_right, border_top, border_bottom;
    float    child_gap;
    float    min_width, min_height;
    float    flex_total;
    float    inner_width, inner_height;
    float    cursor_x, cursor_y;
    int32_t  shrink_width, shrink_height;
    uint32_t child_count;
    uint32_t parent_chunk, parent_slot;
    bool  is_horizontal;
    float    total_children_min_width, total_children_min_height;
    float    w_per_flex_cache, h_per_flex_cache;
    int32_t  total_child_count;
    int32_t  is_continuation;
    int32_t  overflow_group_head;
};

struct WzSlot {
    float min_width[WZ_CHUNK_SIZE];
    float min_height[WZ_CHUNK_SIZE];
    float flex[WZ_CHUNK_SIZE];
    float margin_left[WZ_CHUNK_SIZE];
    float margin_right[WZ_CHUNK_SIZE];
    float margin_top[WZ_CHUNK_SIZE];
    float margin_bottom[WZ_CHUNK_SIZE];
    float max_width[WZ_CHUNK_SIZE];
    float max_height[WZ_CHUNK_SIZE];
    float cross_align[WZ_CHUNK_SIZE];
    float avail_width[WZ_CHUNK_SIZE];
    float avail_height[WZ_CHUNK_SIZE];
    float abs_width[WZ_CHUNK_SIZE];
    float abs_height[WZ_CHUNK_SIZE];
    float abs_x[WZ_CHUNK_SIZE];
    float abs_y[WZ_CHUNK_SIZE];
};

void wz_layout_chunks_c(struct WzChunk *chunks, struct WzSlot *slots, int num_chunks);
