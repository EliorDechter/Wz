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
    float min_w[CHUNK_SIZE];
    float min_h[CHUNK_SIZE];
    float flex[CHUNK_SIZE];
    float margin_l[CHUNK_SIZE];    // per-slot margin (outside the box)
    float margin_r[CHUNK_SIZE];
    float margin_t[CHUNK_SIZE];
    float margin_b[CHUNK_SIZE];
    float max_w[CHUNK_SIZE];     // 0 = unlimited
    float max_h[CHUNK_SIZE];     // 0 = unlimited
    float cross_align[CHUNK_SIZE]; // 0=start, 0.5=center, 1=end (cross axis only)
    float avail_w[CHUNK_SIZE];   // scratch: margin-inclusive allocated size (pass 2)
    float avail_h[CHUNK_SIZE];
    float abs_w[CHUNK_SIZE];     // output: final box size, margins excluded (pass 3)
    float abs_h[CHUNK_SIZE];
    float abs_x[CHUNK_SIZE];     // output: final box position, inside margin (pass 5)
    float abs_y[CHUNK_SIZE];
};

// ---------------------------------------------------------------------------
// Per-chunk scalar metadata.
// ---------------------------------------------------------------------------
struct WzChunk {
    float        pad_left, pad_right, pad_top, pad_bottom;
    float        border_l, border_r, border_t, border_b;
    float        child_gap;
    float        min_w, min_h;   // chunk's own inherent minimum size
    float        flex_total;     // scratch – zero before call
    float        inner_w;        // scratch – cross-axis inner size, written in pass 2
    float        inner_h;        // scratch – cross-axis inner size, written in pass 2
    float        cursor_x;       // init to pad_left + border_l before call
    float        cursor_y;       // init to pad_top  + border_t before call
    int          shrink_w;       // if 1: clamp avail_w to min_w at start of pass 2
    int          shrink_h;       // if 1: clamp avail_h to min_h at start of pass 2
    unsigned int child_count;
    unsigned int par_chunk;
    unsigned int par_slot;
    int          is_horiz;
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
        varying float v_tw = 0.f, v_th = 0.f;
        varying float v_bw = 0.f, v_bh = 0.f;
        varying float v_tf = 0.f;

        foreach (j = 0 ... CHUNK_SIZE)
        {
            float w = slots[i].min_w[j] + slots[i].margin_l[j] + slots[i].margin_r[j];
            float h = slots[i].min_h[j] + slots[i].margin_t[j] + slots[i].margin_b[j];
            v_tw      += w;
            v_th      += h;
            v_bw       = max(v_bw, w);
            v_bh       = max(v_bh, h);
            v_tf      += slots[i].flex[j];
        }

        uniform unsigned int pc   = chunks[i].par_chunk;
        uniform unsigned int ps   = chunks[i].par_slot;

        chunks[pc].flex_total += reduce_add(v_tf);

        uniform float pad_w   = chunks[i].pad_left + chunks[i].pad_right + chunks[i].border_l + chunks[i].border_r;
        uniform float pad_h   = chunks[i].pad_top  + chunks[i].pad_bottom + chunks[i].border_t + chunks[i].border_b;
        uniform float gaps    = chunks[i].child_gap * (float)(chunks[i].child_count - 1);

        if (chunks[i].is_horiz)
        {
            slots[pc].min_w[ps] = max(reduce_add(v_tw) + pad_w + gaps, max(chunks[i].min_w, slots[pc].min_w[ps]));
            slots[pc].min_h[ps] = max(reduce_max(v_bh) + pad_h,        max(chunks[i].min_h, slots[pc].min_h[ps]));
        }
        else
        {
            slots[pc].min_w[ps] = max(reduce_max(v_bw) + pad_w,        max(chunks[i].min_w, slots[pc].min_w[ps]));
            slots[pc].min_h[ps] = max(reduce_add(v_th) + pad_h + gaps, max(chunks[i].min_h, slots[pc].min_h[ps]));
        }
    }

    // -----------------------------------------------------------------------
    // Pass 2 – top-down available-space distribution
    // -----------------------------------------------------------------------
    for (uniform int i = 0; i < num_chunks; ++i)
    {
        uniform unsigned int pc    = chunks[i].par_chunk;
        uniform unsigned int ps    = chunks[i].par_slot;
        uniform bool         horiz = chunks[i].is_horiz;

        // shrink-to-content: clamp parent avail to its own min before distributing
        if (chunks[i].shrink_w) slots[pc].avail_w[ps] = min(slots[pc].avail_w[ps], slots[pc].min_w[ps]);
        if (chunks[i].shrink_h) slots[pc].avail_h[ps] = min(slots[pc].avail_h[ps], slots[pc].min_h[ps]);

        uniform float space_w = slots[pc].avail_w[ps] - chunks[i].pad_left - chunks[i].pad_right - chunks[i].border_l - chunks[i].border_r;
        uniform float space_h = slots[pc].avail_h[ps] - chunks[i].pad_top  - chunks[i].pad_bottom - chunks[i].border_t - chunks[i].border_b;
        uniform float gap     = chunks[i].child_gap;
        uniform int   cnt     = (int)chunks[i].child_count;

        // store inner cross-axis size for pass 5 alignment
        if (horiz) chunks[i].inner_h = max(space_h, 0.f);
        else       chunks[i].inner_w = max(space_w, 0.f);

        if (horiz) space_w -= gap * (float)(cnt - 1);
        else       space_h -= gap * (float)(cnt - 1);
        space_w = max(space_w, 0.f);
        space_h = max(space_h, 0.f);

        varying float v_mw = 0.f, v_mh = 0.f;

        foreach (j = 0 ... CHUNK_SIZE)
        {
            float mw = slots[i].min_w[j] + slots[i].margin_l[j] + slots[i].margin_r[j];
            float mh = slots[i].min_h[j] + slots[i].margin_t[j] + slots[i].margin_b[j];
            slots[i].avail_w[j] = mw;
            slots[i].avail_h[j] = mh;
            v_mw              += mw;
            v_mh              += mh;
        }

        if (horiz) space_w = max(space_w - reduce_add(v_mw), 0.f);
        else       space_h = max(space_h - reduce_add(v_mh), 0.f);

        uniform float tf        = chunks[pc].flex_total;
        uniform float w_per_flex = (tf > 0.f) ? (space_w / tf) : 0.f;
        uniform float h_per_flex = (tf > 0.f) ? (space_h / tf) : 0.f;

        foreach (j = 0 ... CHUNK_SIZE)
        {
            float f             = slots[i].flex[j];
            slots[i].avail_w[j] += horiz ? f * w_per_flex : 0.f;
            slots[i].avail_h[j] += horiz ? 0.f            : f * h_per_flex;
        }
    }

    // -----------------------------------------------------------------------
    // Pass 3 – available -> absolute  (copy)
    // -----------------------------------------------------------------------
    for (uniform int i = 0; i < num_chunks; ++i)
    {
        foreach (j = 0 ... CHUNK_SIZE)
        {
            float bw = max(slots[i].avail_w[j] - slots[i].margin_l[j] - slots[i].margin_r[j], 0.f);
            float bh = max(slots[i].avail_h[j] - slots[i].margin_t[j] - slots[i].margin_b[j], 0.f);
            if (slots[i].max_w[j] > 0.f) bw = min(bw, slots[i].max_w[j]);
            if (slots[i].max_h[j] > 0.f) bh = min(bh, slots[i].max_h[j]);
            slots[i].abs_w[j] = bw;
            slots[i].abs_h[j] = bh;
        }
    }

    // -----------------------------------------------------------------------
    // Pass 4 – assert non-negative parent space  (debug)
    // -----------------------------------------------------------------------
    for (uniform int i = 1; i < num_chunks; ++i)
    {
        uniform unsigned int pc = chunks[i].par_chunk;
        uniform unsigned int ps = chunks[i].par_slot;
        assert(slots[pc].avail_w[ps] >= 0.f);
        assert(slots[pc].avail_h[ps] >= 0.f);
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
        uniform unsigned int pc       = chunks[i].par_chunk;
        uniform unsigned int ps       = chunks[i].par_slot;
        uniform bool         horiz    = chunks[i].is_horiz;
        uniform float        gap      = chunks[i].child_gap;
        uniform float        parent_x = slots[pc].abs_x[ps];
        uniform float        parent_y = slots[pc].abs_y[ps];
        uniform float        cx       = chunks[i].cursor_x;
        uniform float        cy       = chunks[i].cursor_y;

        // Use avail (margin-inclusive) for scan so siblings are spaced correctly.
        // Actual box position is then offset inward by the slot's own margin.
        varying float aw = slots[i].avail_w[programIndex];
        varying float ah = slots[i].avail_h[programIndex];

        if (horiz)
        {
            // cross = vertical: align within inner_h
            varying float cross_off = slots[i].cross_align[programIndex]
                * max(chunks[i].inner_h - slots[i].avail_h[programIndex], 0.f);
            slots[i].abs_x[programIndex] = parent_x + cx + exclusive_scan_add(aw) + (float)programIndex * gap + slots[i].margin_l[programIndex];
            slots[i].abs_y[programIndex] = parent_y + cy + slots[i].margin_t[programIndex] + cross_off;
        }
        else
        {
            // cross = horizontal: align within inner_w
            varying float cross_off = slots[i].cross_align[programIndex]
                * max(chunks[i].inner_w - slots[i].avail_w[programIndex], 0.f);
            slots[i].abs_x[programIndex] = parent_x + cx + slots[i].margin_l[programIndex] + cross_off;
            slots[i].abs_y[programIndex] = parent_y + cy + exclusive_scan_add(ah) + (float)programIndex * gap + slots[i].margin_t[programIndex];
        }

        if (horiz) chunks[i].cursor_x = cx + reduce_add(aw) + (float)CHUNK_SIZE * gap;
        else       chunks[i].cursor_y = cy + reduce_add(ah)  + (float)CHUNK_SIZE * gap;
    }
}
