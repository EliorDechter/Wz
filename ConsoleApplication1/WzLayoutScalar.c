// WzLayoutScalar.c
// Scalar C translation of WzLayout.ispc — no SIMD, one value at a time.
//
// Translation rules applied:
//   uniform T x      -> T x          (just a scalar variable)
//   varying T x      -> T x          (scalar accumulator in a loop)
//   foreach (j=0...N) -> for (int j = 0; j < N; j++)
//   reduce_add(v)    -> v            (after scalar loop, v already holds the sum)
//   reduce_max(v)    -> v            (after scalar loop, v already holds the max)
//   exclusive_scan_add(v) -> prefix sum array computed before the loop
//   programIndex     -> j            (the loop variable)
//   max(a,b)         -> fmaxf(a,b)
//   min(a,b)         -> fminf(a,b)

#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include "WzLayoutScalar.h"

#define CHUNK_SIZE WZ_CHUNK_SIZE

void wz_layout_chunks_c(
    struct WzChunk *chunks,   // [num_chunks]
    struct WzSlot  *slots,    // [num_chunks]
    int             num_chunks)
{
    // -----------------------------------------------------------------------
    // Pass 1 – bottom-up minimum-size accumulation
    // -----------------------------------------------------------------------
    for (int i = num_chunks - 1; i > 0; --i)
    {
        float sum_child_width  = 0.f, sum_child_height = 0.f;
        float max_child_width  = 0.f, max_child_height = 0.f;
        float sum_flex         = 0.f;

        for (int j = 0; j < CHUNK_SIZE; j++)
        {
            float w = slots[i].min_width[j]  + slots[i].margin_left[j] + slots[i].margin_right[j];
            float h = slots[i].min_height[j] + slots[i].margin_top[j]  + slots[i].margin_bottom[j];
            sum_child_width  += w;
            sum_child_height += h;
            max_child_width   = w > max_child_width  ? w : max_child_width;
            max_child_height  = h > max_child_height ? h : max_child_height;
            sum_flex         += slots[i].flex[j];
        }

        unsigned int parent_chunk_idx = chunks[i].parent_chunk;
        unsigned int parent_slot_idx  = chunks[i].parent_slot;

        chunks[parent_chunk_idx].flex_total += sum_flex;

        float pad_w   = chunks[i].pad_left  + chunks[i].pad_right  + chunks[i].border_left  + chunks[i].border_right;
        float pad_h   = chunks[i].pad_top   + chunks[i].pad_bottom + chunks[i].border_top   + chunks[i].border_bottom;
        float gaps    = chunks[i].child_gap * (float)(chunks[i].child_count - 1);
        bool   horizontal   = chunks[i].is_horizontal;

        float new_w = horizontal ? sum_child_width  + pad_w + gaps : max_child_width  + pad_w;
        float new_h = horizontal ? max_child_height + pad_h        : sum_child_height + pad_h + gaps;
        float old_w = slots[parent_chunk_idx].min_width[parent_slot_idx];
        float old_h = slots[parent_chunk_idx].min_height[parent_slot_idx];
        slots[parent_chunk_idx].min_width[parent_slot_idx]  = fmaxf(new_w, fmaxf(chunks[i].min_width,  old_w));
        slots[parent_chunk_idx].min_height[parent_slot_idx] = fmaxf(new_h, fmaxf(chunks[i].min_height, old_h));
    }

    // -----------------------------------------------------------------------
    // Pass 2 – top-down available-space distribution
    // -----------------------------------------------------------------------
    for (int i = 0; i < num_chunks; ++i)
    {
        unsigned int parent_chunk_idx = chunks[i].parent_chunk;
        unsigned int parent_slot_idx  = chunks[i].parent_slot;
        bool          horizontal            = chunks[i].is_horizontal;

        // Continuation chunk: its head already computed the flex rate.
        // Copy cross-axis inner size from the head and apply cached per-flex value.
        if (chunks[i].is_continuation)
        {
            int   head = chunks[i].overflow_group_head;
            float wf   = chunks[head].w_per_flex_cache;
            float hf   = chunks[head].h_per_flex_cache;
            chunks[i].inner_width  = chunks[head].inner_width;
            chunks[i].inner_height = chunks[head].inner_height;

            for (int j = 0; j < CHUNK_SIZE; j++)
            {
                float mw = slots[i].min_width[j]  + slots[i].margin_left[j] + slots[i].margin_right[j];
                float mh = slots[i].min_height[j] + slots[i].margin_top[j]  + slots[i].margin_bottom[j];
                slots[i].avail_width[j]  = mw + (horizontal ? slots[i].flex[j] * wf : 0.f);
                slots[i].avail_height[j] = mh + (horizontal ? 0.f : slots[i].flex[j] * hf);
            }
            continue;
        }

        // Shrink-to-content: clamp parent avail to its own min before distributing.
        float aw = slots[parent_chunk_idx].avail_width[parent_slot_idx];
        float ah = slots[parent_chunk_idx].avail_height[parent_slot_idx];
        slots[parent_chunk_idx].avail_width[parent_slot_idx]  = chunks[i].shrink_width  ? fminf(aw, slots[parent_chunk_idx].min_width[parent_slot_idx])  : aw;
        slots[parent_chunk_idx].avail_height[parent_slot_idx] = chunks[i].shrink_height ? fminf(ah, slots[parent_chunk_idx].min_height[parent_slot_idx]) : ah;

        float space_w = slots[parent_chunk_idx].avail_width[parent_slot_idx]
                      - chunks[i].pad_left - chunks[i].pad_right
                      - chunks[i].border_left - chunks[i].border_right;
        float space_h = slots[parent_chunk_idx].avail_height[parent_slot_idx]
                      - chunks[i].pad_top  - chunks[i].pad_bottom
                      - chunks[i].border_top - chunks[i].border_bottom;
        float gap = chunks[i].child_gap;

        // Store inner cross-axis size for pass 5 alignment.
        chunks[i].inner_width  = horizontal ? chunks[i].inner_width  : fmaxf(space_w, 0.f);
        chunks[i].inner_height = horizontal ? fmaxf(space_h, 0.f)    : chunks[i].inner_height;

        // Use total_child_count for gap subtraction when this is an overflow head chunk.
        int   effective_cnt = chunks[i].total_child_count > 0 ? chunks[i].total_child_count : (int)chunks[i].child_count;
        float gap_total     = gap * (float)(effective_cnt - 1);
        space_w -= horizontal ? gap_total : 0.f;
        space_h -= horizontal ? 0.f : gap_total;
        space_w  = fmaxf(space_w, 0.f);
        space_h  = fmaxf(space_h, 0.f);

        // Assign min size to each slot and accumulate totals.
        float v_mw = 0.f, v_mh = 0.f;
        for (int j = 0; j < CHUNK_SIZE; j++)
        {
            float mw = slots[i].min_width[j]  + slots[i].margin_left[j] + slots[i].margin_right[j];
            float mh = slots[i].min_height[j] + slots[i].margin_top[j]  + slots[i].margin_bottom[j];
            slots[i].avail_width[j]  = mw;
            slots[i].avail_height[j] = mh;
            v_mw += mw;
            v_mh += mh;
        }

        // Subtract children's minimum from available flex space.
        int   overflow = chunks[i].total_children_min_width > 0.f;
        float sub_w    = overflow ? chunks[i].total_children_min_width  : v_mw;
        float sub_h    = overflow ? chunks[i].total_children_min_height : v_mh;
        space_w = horizontal ? fmaxf(space_w - sub_w, 0.f) : space_w;
        space_h = horizontal ? space_h : fmaxf(space_h - sub_h, 0.f);

        float tf         = chunks[parent_chunk_idx].flex_total;
        float w_per_flex = tf > 0.f ? space_w / tf : 0.f;
        float h_per_flex = tf > 0.f ? space_h / tf : 0.f;

        // Cache so continuation chunks can reuse the same rate.
        chunks[i].w_per_flex_cache = w_per_flex;
        chunks[i].h_per_flex_cache = h_per_flex;

        // Distribute flex space to each slot.
        for (int j = 0; j < CHUNK_SIZE; j++)
        {
            float f = slots[i].flex[j];
            slots[i].avail_width[j]  += horizontal ? f * w_per_flex : 0.f;
            slots[i].avail_height[j] += horizontal ? 0.f            : f * h_per_flex;
        }
    }

    // -----------------------------------------------------------------------
    // Pass 3 – available -> absolute (copy, strip margins, clamp to max)
    // -----------------------------------------------------------------------
    for (int i = 0; i < num_chunks; ++i)
    {
        for (int j = 0; j < CHUNK_SIZE; j++)
        {
            float bw = fmaxf(slots[i].avail_width[j]  - slots[i].margin_left[j] - slots[i].margin_right[j],  0.f);
            float bh = fmaxf(slots[i].avail_height[j] - slots[i].margin_top[j]  - slots[i].margin_bottom[j], 0.f);
            bw = slots[i].max_width[j]  > 0.f ? fminf(bw, slots[i].max_width[j])  : bw;
            bh = slots[i].max_height[j] > 0.f ? fminf(bh, slots[i].max_height[j]) : bh;
            slots[i].abs_width[j]  = bw;
            slots[i].abs_height[j] = bh;
        }
    }

    // -----------------------------------------------------------------------
    // Pass 4 – assert non-negative parent space (debug)
    // -----------------------------------------------------------------------
    for (int i = 1; i < num_chunks; ++i)
    {
        unsigned int parent_chunk_idx = chunks[i].parent_chunk;
        unsigned int parent_slot_idx  = chunks[i].parent_slot;
        assert(slots[parent_chunk_idx].avail_width[parent_slot_idx]  >= 0.f);
        assert(slots[parent_chunk_idx].avail_height[parent_slot_idx] >= 0.f);
    }

    // -----------------------------------------------------------------------
    // Pass 5 – write absolute positions
    //
    // exclusive_scan_add(v) -> prefix sum array:
    //   prefix[0] = 0
    //   prefix[j] = v[0] + v[1] + ... + v[j-1]
    // -----------------------------------------------------------------------
    for (int i = 0; i < num_chunks; ++i)
    {
        unsigned int parent_chunk_idx = chunks[i].parent_chunk;
        unsigned int parent_slot_idx  = chunks[i].parent_slot;
        bool         horizontal    = chunks[i].is_horizontal;
        float        gap      = chunks[i].child_gap;
        float        parent_x = slots[parent_chunk_idx].abs_x[parent_slot_idx];
        float        parent_y = slots[parent_chunk_idx].abs_y[parent_slot_idx];
        float        cx       = chunks[i].cursor_x;
        float        cy       = chunks[i].cursor_y;

        // Compute exclusive prefix sums of avail_width and avail_height.
        float prefix_w[CHUNK_SIZE], prefix_h[CHUNK_SIZE];
        prefix_w[0] = prefix_h[0] = 0.f;
        for (int j = 1; j < CHUNK_SIZE; j++)
        {
            prefix_w[j] = prefix_w[j-1] + slots[i].avail_width[j-1];
            prefix_h[j] = prefix_h[j-1] + slots[i].avail_height[j-1];
        }

        // Total = reduce_add(avail) = last prefix + last element.
        float total_w = prefix_w[CHUNK_SIZE-1] + slots[i].avail_width[CHUNK_SIZE-1];
        float total_h = prefix_h[CHUNK_SIZE-1] + slots[i].avail_height[CHUNK_SIZE-1];

        for (int j = 0; j < CHUNK_SIZE; j++)
        {
            float cross_off = slots[i].cross_align[j] * (horizontal
                ? fmaxf(chunks[i].inner_height - slots[i].avail_height[j], 0.f)
                : fmaxf(chunks[i].inner_width  - slots[i].avail_width[j],  0.f));
            slots[i].abs_x[j] = parent_x + cx + slots[i].margin_left[j] + (horizontal ? prefix_w[j] + (float)j * gap : cross_off);
            slots[i].abs_y[j] = parent_y + cy + slots[i].margin_top[j]  + (horizontal ? cross_off : prefix_h[j] + (float)j * gap);
        }

        chunks[i].cursor_x = cx + (horizontal ? total_w + (float)chunks[i].child_count * gap : 0.f);
        chunks[i].cursor_y = cy + (horizontal ? 0.f : total_h + (float)chunks[i].child_count * gap);
    }
}
