// WzLayout.ispc
// Vectorised chunk-based GUI layout.
//
// WzSlot has float[CHUNK_SIZE] arrays for every field — this IS SOA layout,
// just expressed as plain struct members instead of the soa<N> qualifier
// (which cannot be combined with uniform on a function parameter).
//
// C side sees the same struct directly; no conversion needed.
//
// Compile:
//   ispc WzLayout.ispc -o WzLayout.obj --target=avx2-i32x8 --header-outfile=WzLayout_ispc.h

#define CHUNK_SIZE 8

// ---------------------------------------------------------------------------
// Per-slot data.  Each field is float[CHUNK_SIZE] so all values for one
// field across all slots in a chunk are contiguous in memory -> vector load.
// ---------------------------------------------------------------------------
struct WzSlot {
    float min_width[CHUNK_SIZE];
    float min_height[CHUNK_SIZE];
    float flex[CHUNK_SIZE];
    float margin_left[CHUNK_SIZE];    // per-slot margin (outside the box)
    float margin_right[CHUNK_SIZE];
    float margin_top[CHUNK_SIZE];
    float margin_bottom[CHUNK_SIZE];
    float max_width[CHUNK_SIZE];     // 0 = unlimited
    float max_height[CHUNK_SIZE];     // 0 = unlimited
    float cross_align[CHUNK_SIZE]; // 0=start, 0.5=center, 1=end (cross axis only)
    float avail_width[CHUNK_SIZE];   // scratch: margin-inclusive allocated size (pass 2)
    float avail_height[CHUNK_SIZE];
    float abs_width[CHUNK_SIZE];     // output: final box size, margins excluded (pass 3)
    float abs_height[CHUNK_SIZE];
    float abs_x[CHUNK_SIZE];     // output: final box position, inside margin (pass 5)
    float abs_y[CHUNK_SIZE];
};

// ---------------------------------------------------------------------------
// Per-chunk scalar metadata.
// ---------------------------------------------------------------------------
struct WzChunk {
    float        pad_left, pad_right, pad_top, pad_bottom;
    float        border_left, border_right, border_top, border_bottom;
    float        child_gap;
    float        min_width, min_height;   // chunk's own inherent minimum size
    float        flex_total;              // scratch – zero before call
    float        inner_width;             // scratch – cross-axis inner size, written in pass 2
    float        inner_height;            // scratch – cross-axis inner size, written in pass 2
    float        cursor_x;               // init to pad_left + border_left before call
    float        cursor_y;               // init to pad_top  + border_top before call
    int          shrink_width;           // if 1: clamp avail_width to min_width at start of pass 2
    int          shrink_height;          // if 1: clamp avail_height to min_height at start of pass 2
    unsigned int child_count;
    unsigned int parent_chunk;
    unsigned int parent_slot;
    int          is_horizontal;
    // overflow support (> CHUNK_SIZE children per widget)
    float        total_children_min_width;  // head chunk: sum of min+margins across ALL overflow chunks (0 = no overflow)
    float        total_children_min_height;
    float        w_per_flex_cache;          // scratch: head writes in pass 2, continuations read
    float        h_per_flex_cache;
    int          total_child_count;         // head chunk: total children across all overflow chunks (0 = no overflow)
    int          is_continuation;           // 1 if this is not the first chunk for this widget
    int          overflow_group_head;       // continuation: index of the head chunk for this overflow group
};

// ---------------------------------------------------------------------------
// wz_layout  –  runs all 5 passes in one call
// ---------------------------------------------------------------------------
export void wz_layout_chunks(
    uniform WzChunk chunks[],   // [num_chunks]
    uniform WzSlot  slots[],    // [num_chunks]  – one slot-group per chunk
    uniform int     num_chunks)
{
    // -----------------------------------------------------------------------
    // Pass 1 – bottom-up minimum-size accumulation
    // -----------------------------------------------------------------------
    for (uniform int i = num_chunks - 1; i > 0; --i)
    {
        varying float sum_child_width  = 0.f, sum_child_height = 0.f;
        varying float max_child_width  = 0.f, max_child_height = 0.f;
        varying float sum_flex         = 0.f;

        foreach (j = 0 ... CHUNK_SIZE)
        {
            float w = slots[i].min_width[j] + slots[i].margin_left[j] + slots[i].margin_right[j];
            float h = slots[i].min_height[j] + slots[i].margin_top[j] + slots[i].margin_bottom[j];
            sum_child_width  += w;
            sum_child_height += h;
            max_child_width   = max(max_child_width,  w);
            max_child_height  = max(max_child_height, h);
            sum_flex         += slots[i].flex[j];
        }

        uniform unsigned int parent_chunk_idx   = chunks[i].parent_chunk;
        uniform unsigned int parent_slot_idx   = chunks[i].parent_slot;

        chunks[parent_chunk_idx].flex_total += reduce_add(sum_flex);

        uniform float pad_w   = chunks[i].pad_left + chunks[i].pad_right + chunks[i].border_left + chunks[i].border_right;
        uniform float pad_h   = chunks[i].pad_top  + chunks[i].pad_bottom + chunks[i].border_top + chunks[i].border_bottom;
        uniform float gaps    = chunks[i].child_gap * (float)(chunks[i].child_count - 1);

        if (chunks[i].is_horizontal)
        {
            slots[parent_chunk_idx].min_width[parent_slot_idx]  = max(reduce_add(sum_child_width)  +
            pad_w + gaps, max(chunks[i].min_width,  slots[parent_chunk_idx].min_width[parent_slot_idx]));
            slots[parent_chunk_idx].min_height[parent_slot_idx] = max(reduce_max(max_child_height) +
            pad_h, max(chunks[i].min_height, slots[parent_chunk_idx].min_height[parent_slot_idx]));
        }
        else
        {
            slots[parent_chunk_idx].min_width[parent_slot_idx]  = max(reduce_max(max_child_width)  + pad_w,        max(chunks[i].min_width,  slots[parent_chunk_idx].min_width[parent_slot_idx]));
            slots[parent_chunk_idx].min_height[parent_slot_idx] = max(reduce_add(sum_child_height) + pad_h + gaps, max(chunks[i].min_height, slots[parent_chunk_idx].min_height[parent_slot_idx]));
        }
    }

    // -----------------------------------------------------------------------
    // Pass 2 – top-down available-space distribution
    // -----------------------------------------------------------------------
    for (uniform int i = 0; i < num_chunks; ++i)
    {
        uniform unsigned int parent_chunk_idx = chunks[i].parent_chunk;
        uniform unsigned int parent_slot_idx  = chunks[i].parent_slot;
        uniform bool         horiz = chunks[i].is_horizontal;

        // ---------------------------------------------------------------
        // Continuation chunk: its head already computed the flex rate.
        // Copy cross-axis inner size from the head and apply the cached
        // per-flex value directly — no padding or gap re-subtraction.
        // ---------------------------------------------------------------
        if (chunks[i].is_continuation)
        {
            uniform int head = chunks[i].overflow_group_head;
            chunks[i].inner_width  = chunks[head].inner_width;
            chunks[i].inner_height = chunks[head].inner_height;

            uniform float wf = chunks[head].w_per_flex_cache;
            uniform float hf = chunks[head].h_per_flex_cache;

            foreach (j = 0 ... CHUNK_SIZE)
            {
                float mw = slots[i].min_width[j]  + slots[i].margin_left[j] + slots[i].margin_right[j];
                float mh = slots[i].min_height[j] + slots[i].margin_top[j]  + slots[i].margin_bottom[j];
                slots[i].avail_width[j]  = mw + (horiz ? slots[i].flex[j] * wf : 0.f);
                slots[i].avail_height[j] = mh + (horiz ? 0.f : slots[i].flex[j] * hf);
            }
            continue;
        }

        // shrink-to-content: clamp parent avail to its own min before distributing
        if (chunks[i].shrink_width)  slots[parent_chunk_idx].avail_width[parent_slot_idx]  = min(slots[parent_chunk_idx].avail_width[parent_slot_idx],  slots[parent_chunk_idx].min_width[parent_slot_idx]);
        if (chunks[i].shrink_height) slots[parent_chunk_idx].avail_height[parent_slot_idx] = min(slots[parent_chunk_idx].avail_height[parent_slot_idx], slots[parent_chunk_idx].min_height[parent_slot_idx]);

        uniform float space_w = slots[parent_chunk_idx].avail_width[parent_slot_idx]  - chunks[i].pad_left - chunks[i].pad_right - chunks[i].border_left - chunks[i].border_right;
        uniform float space_h = slots[parent_chunk_idx].avail_height[parent_slot_idx] - chunks[i].pad_top  - chunks[i].pad_bottom - chunks[i].border_top - chunks[i].border_bottom;
        uniform float gap     = chunks[i].child_gap;

        // store inner cross-axis size for pass 5 alignment
        if (horiz) chunks[i].inner_height = max(space_h, 0.f);
        else       chunks[i].inner_width  = max(space_w, 0.f);

        // Use total_child_count for gap subtraction when this is an overflow head chunk,
        // so all continuation chunks share the same correctly-sized flex pool.
        uniform int effective_cnt = chunks[i].total_child_count > 0
                                  ? chunks[i].total_child_count
                                  : (int)chunks[i].child_count;
        if (horiz) space_w -= gap * (float)(effective_cnt - 1);
        else       space_h -= gap * (float)(effective_cnt - 1);
        space_w = max(space_w, 0.f);
        space_h = max(space_h, 0.f);

        varying float v_mw = 0.f, v_mh = 0.f;

        foreach (j = 0 ... CHUNK_SIZE)
        {
            float mw = slots[i].min_width[j]  + slots[i].margin_left[j] + slots[i].margin_right[j];
            float mh = slots[i].min_height[j] + slots[i].margin_top[j]  + slots[i].margin_bottom[j];
            slots[i].avail_width[j]  = mw;
            slots[i].avail_height[j] = mh;
            v_mw += mw;
            v_mh += mh;
        }

        // Use precomputed total minimum when this is an overflow head chunk.
        if (chunks[i].total_children_min_width > 0.f) {
            if (horiz) space_w = max(space_w - chunks[i].total_children_min_width,  0.f);
            else       space_h = max(space_h - chunks[i].total_children_min_height, 0.f);
        } else {
            if (horiz) space_w = max(space_w - reduce_add(v_mw), 0.f);
            else       space_h = max(space_h - reduce_add(v_mh), 0.f);
        }

        uniform float tf         = chunks[parent_chunk_idx].flex_total;
        uniform float w_per_flex = (tf > 0.f) ? (space_w / tf) : 0.f;
        uniform float h_per_flex = (tf > 0.f) ? (space_h / tf) : 0.f;

        // Cache so continuation chunks can reuse the same rate.
        chunks[i].w_per_flex_cache = w_per_flex;
        chunks[i].h_per_flex_cache = h_per_flex;

        foreach (j = 0 ... CHUNK_SIZE)
        {
            float f = slots[i].flex[j];
            slots[i].avail_width[j]  += horiz ? f * w_per_flex : 0.f;
            slots[i].avail_height[j] += horiz ? 0.f            : f * h_per_flex;
        }
    }

    // -----------------------------------------------------------------------
    // Pass 3 – available -> absolute  (copy)
    // -----------------------------------------------------------------------
    for (uniform int i = 0; i < num_chunks; ++i)
    {
        foreach (j = 0 ... CHUNK_SIZE)
        {
            float bw = max(slots[i].avail_width[j]  - slots[i].margin_left[j] - slots[i].margin_right[j],  0.f);
            float bh = max(slots[i].avail_height[j] - slots[i].margin_top[j]  - slots[i].margin_bottom[j], 0.f);
            if (slots[i].max_width[j]  > 0.f) bw = min(bw, slots[i].max_width[j]);
            if (slots[i].max_height[j] > 0.f) bh = min(bh, slots[i].max_height[j]);
            slots[i].abs_width[j]  = bw;
            slots[i].abs_height[j] = bh;
        }
    }

    // -----------------------------------------------------------------------
    // Pass 4 – assert non-negative parent space  (debug)
    // -----------------------------------------------------------------------
    for (uniform int i = 1; i < num_chunks; ++i)
    {
        uniform unsigned int parent_chunk_idx = chunks[i].parent_chunk;
        uniform unsigned int parent_slot_idx = chunks[i].parent_slot;
        assert(slots[parent_chunk_idx].avail_width[parent_slot_idx]  >= 0.f);
        assert(slots[parent_chunk_idx].avail_height[parent_slot_idx] >= 0.f);
    }

    // -----------------------------------------------------------------------
    // Pass 5 – write absolute positions
    //
    // slots[i].abs_w[programIndex] loads 8 contiguous floats -> vector load.
    // exclusive_scan_add gives each slot its cumulative offset in one gang op.
    // Requires programCount == CHUNK_SIZE  (avx2-i32x8 -> CHUNK_SIZE 8).
    // -----------------------------------------------------------------------
    for (uniform int i = 0; i < num_chunks; ++i)
    {
        uniform unsigned int parent_chunk_idx       = chunks[i].parent_chunk;
        uniform unsigned int parent_slot_idx       = chunks[i].parent_slot;
        uniform bool         horiz    = chunks[i].is_horizontal;
        uniform float        gap      = chunks[i].child_gap;
        uniform float        parent_x = slots[parent_chunk_idx].abs_x[parent_slot_idx];
        uniform float        parent_y = slots[parent_chunk_idx].abs_y[parent_slot_idx];
        uniform float        cx       = chunks[i].cursor_x;
        uniform float        cy       = chunks[i].cursor_y;

        // Use avail (margin-inclusive) for scan so siblings are spaced correctly.
        // Actual box position is then offset inward by the slot's own margin.
        varying float aw = slots[i].avail_width[programIndex];
        varying float ah = slots[i].avail_height[programIndex];

        if (horiz)
        {
            // cross = vertical: align within inner_height
            varying float cross_off = slots[i].cross_align[programIndex]
                * max(chunks[i].inner_height - slots[i].avail_height[programIndex], 0.f);
            slots[i].abs_x[programIndex] = parent_x + cx + exclusive_scan_add(aw) + (float)programIndex * gap + slots[i].margin_left[programIndex];
            slots[i].abs_y[programIndex] = parent_y + cy + slots[i].margin_top[programIndex] + cross_off;
        }
        else
        {
            // cross = horizontal: align within inner_width
            varying float cross_off = slots[i].cross_align[programIndex]
                * max(chunks[i].inner_width - slots[i].avail_width[programIndex], 0.f);
            slots[i].abs_x[programIndex] = parent_x + cx + slots[i].margin_left[programIndex] + cross_off;
            slots[i].abs_y[programIndex] = parent_y + cy + exclusive_scan_add(ah) + (float)programIndex * gap + slots[i].margin_top[programIndex];
        }

        if (horiz) chunks[i].cursor_x = cx + reduce_add(aw) + (float)chunks[i].child_count * gap;
        else       chunks[i].cursor_y = cy + reduce_add(ah)  + (float)chunks[i].child_count * gap;
    }
}
