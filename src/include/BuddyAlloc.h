/*
 * Copyright 2021 Stanislav Paskalev <spaskalev@protonmail.com>
 */

/*
 * A binary buddy memory allocator
 *
 * To include and use it in your project do the following
 * 1. Add BuddyAlloc.h (this file) to your include directory
 * 2. Include the header in places where you need to use the allocator
 * 3. In one of your source files #define BUDDY_ALLOC_IMPLEMENTATION
 *    and then import the header. This will insert the implementation.
 */

#ifndef BUDDY_ALLOC_H
#define BUDDY_ALLOC_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
/*
#include <stdio.h>
#include <string.h>
*/

struct buddy;
/* Returns the size of a buddy required to manage of block of the specified size
 */
size_t buddy_sizeof(size_t memory_size);

/* Initializes a binary buddy memory allocator at the specified location */
struct buddy *buddy_init(unsigned char *at, unsigned char *main,
                         size_t memory_size);

/*
 * Initializes a binary buddy memory allocator embedded in the specified arena.
 * The arena's capacity is reduced to account for the allocator metadata.
 */
struct buddy *buddy_embed(unsigned char *main, size_t memory_size);

/* Resizes the arena and metadata to a new size. */
struct buddy *buddy_resize(struct buddy *buddy, size_t new_memory_size);

/* Tests if the allocation can be shrunk in half */
unsigned int buddy_can_shrink(struct buddy *buddy);

/* Tests if the allocation is completely empty */
unsigned int buddy_is_empty(struct buddy *buddy);

/* Reports the arena size */
size_t buddy_arena_size(struct buddy *buddy);

/*
 * Allocation functions
 */

/* Use the specified buddy to allocate memory. See malloc. */
void *buddy_malloc(struct buddy *buddy, size_t requested_size);

/* Use the specified buddy to allocate zeroed memory. See calloc. */
void *buddy_calloc(struct buddy *buddy, size_t members_count,
                   size_t member_size);

/* Realloc semantics are a joke. See realloc. */
void *buddy_realloc(struct buddy *buddy, void *ptr, size_t requested_size);

/* Realloc-like behavior that checks for overflow. See reallocarray*/
void *buddy_reallocarray(struct buddy *buddy, void *ptr, size_t members_count,
                         size_t member_size);

/* Use the specified buddy to free memory. See free. */
void buddy_free(struct buddy *buddy, void *ptr);

/* A (safer) free with a size. Will not free unless the size fits the target
 * span. */
void buddy_safe_free(struct buddy *buddy, void *ptr, size_t requested_size);

/*
 * Reservation functions
 */

/* Reserve a range by marking it as allocated. Useful for dealing with physical
 * memory. */
void buddy_reserve_range(struct buddy *buddy, void *ptr, size_t requested_size);

/* Release a reserved memory range. Unsafe, this can mess up other allocations
 * if called with wrong parameters! */
void buddy_unsafe_release_range(struct buddy *buddy, void *ptr,
                                size_t requested_size);

/*
 * Iteration functions
 */

/*
 * Iterate through the allocated slots and call the provided function for each
 * of them.
 *
 * If the provided function returns a non-NULL result the iteration stops and
 * the result is returned to called. NULL is returned upon completing iteration
 * without stopping.
 *
 * The iteration order is implementation-defined and may change between
 * versions.
 */
void *buddy_walk(struct buddy *buddy,
                 void *(fp)(void *ctx, void *addr, size_t slot_size),
                 void *ctx);

/*
 * Miscellaneous functions
 */

/*
 * Calculates the fragmentation in the allocator in a 0.0 - 1.0 range.
 * NOTE: if you are using a non-power-of-two sized arena the maximum upper bound
 * can be lower.
 */
float buddy_fragmentation(struct buddy *buddy);

/*
 * Configure the allocator to bias allocations on the left, lower side of its
 * arena.
 */
void buddy_set_left_bias(struct buddy *buddy);

/*
 * Configure the allocator to optimize allocations, instead of biasing them.
 * This is the default mode.
 */
void buddy_set_optimal_fit(struct buddy *buddy);

#define BUDDY_ALLOC_ALIGN (sizeof(size_t) * CHAR_BIT)

/*
 * Debug functions
 */

/* Implementation defined */
//  void buddy_debug(FILE *stream, struct buddy *buddy);

// struct buddy_tree;

struct buddy_tree {
  size_t upper_pos_bound;
  size_t size_for_order_offset;
  uint8_t order;
  size_t data[];
};
struct buddy_tree_pos {
  size_t index;
  size_t depth;
};

#define INVALID_POS ((struct buddy_tree_pos){0, 0})

struct buddy_tree_interval {
  struct buddy_tree_pos from;
  struct buddy_tree_pos to;
};

struct buddy_tree_walk_state {
  struct buddy_tree_pos starting_pos;
  struct buddy_tree_pos current_pos;
  unsigned int going_up;
  unsigned int walk_done;
};

/*
 * Initialization functions
 */

/* Returns the size of a buddy allocation tree of the desired order*/
 size_t buddy_tree_sizeof(uint8_t order);

/* Initializes a buddy allocation tree at the specified location */
 struct buddy_tree *buddy_tree_init(unsigned char *at, uint8_t order);

/* Indicates whether this is a valid position for the tree */
 unsigned int buddy_tree_valid(struct buddy_tree *t,
                                     struct buddy_tree_pos pos);

/* Returns the order of the specified buddy allocation tree */
 uint8_t buddy_tree_order(struct buddy_tree *t);

/* Resize the tree to the new order. When downsizing the left subtree is picked.
 */
/* Caller must ensure enough space for the new order. */
 void buddy_tree_resize(struct buddy_tree *t, uint8_t desired_order);

/*
 * Navigation functions
 */

/* Returns a position at the root of a buddy allocation tree */
 struct buddy_tree_pos buddy_tree_root(void);

/* Returns the leftmost child node */
 struct buddy_tree_pos buddy_tree_leftmost_child(struct buddy_tree *t);

/* Returns the tree depth of the indicated position */
 inline size_t buddy_tree_depth(struct buddy_tree_pos pos);

/* Returns the left child node position. Does not check if that is a valid
 * position */
 inline struct buddy_tree_pos buddy_tree_left_child(
    struct buddy_tree_pos pos);

/* Returns the right child node position. Does not check if that is a valid
 * position */
 inline struct buddy_tree_pos buddy_tree_right_child(
    struct buddy_tree_pos pos);

/* Returns the current sibling node position. Does not check if that is a valid
 * position */
 inline struct buddy_tree_pos buddy_tree_sibling(
    struct buddy_tree_pos pos);

/* Returns the parent node position or an invalid position if there is no parent
 * node */
 inline struct buddy_tree_pos buddy_tree_parent(
    struct buddy_tree_pos pos);

/* Returns the right adjacent node position or an invalid position if there is
 * no right adjacent node */
 struct buddy_tree_pos buddy_tree_right_adjacent(
    struct buddy_tree_pos pos);

/* Returns the at-depth index of the indicated position */
 size_t buddy_tree_index(struct buddy_tree_pos pos);

/* Return the interval of the deepest positions spanning the indicated position
 */
 struct buddy_tree_interval buddy_tree_interval(
    struct buddy_tree *t, struct buddy_tree_pos pos);

/* Checks if one interval contains another */
 unsigned int buddy_tree_interval_contains(
    struct buddy_tree_interval outer, struct buddy_tree_interval inner);

/* Return a walk state structure starting from the root of a tree */
 struct buddy_tree_walk_state buddy_tree_walk_state_root();

/* Walk the tree, keeping track in the provided state structure */
 unsigned int buddy_tree_walk(struct buddy_tree *t,
                                    struct buddy_tree_walk_state *state);

/*
 * Allocation functions
 */

/* Returns the free capacity at or underneath the indicated position */
 size_t buddy_tree_status(struct buddy_tree *t,
                                struct buddy_tree_pos pos);

/* Marks the indicated position as allocated and propagates the change */
 void buddy_tree_mark(struct buddy_tree *t, struct buddy_tree_pos pos);

/* Marks the indicated position as free and propagates the change */
 void buddy_tree_release(struct buddy_tree *t, struct buddy_tree_pos pos);

/* Returns a free position at the specified depth or an invalid position */
 struct buddy_tree_pos buddy_tree_find_free(struct buddy_tree *t,
                                                  uint8_t depth,
                                                  uint8_t left_bias);

/* Tests if the incidated position is available for allocation */
 unsigned int buddy_tree_is_free(struct buddy_tree *t,
                                       struct buddy_tree_pos pos);

/* Tests if the tree can be shrank in half */
 unsigned int buddy_tree_can_shrink(struct buddy_tree *t);

/*
 * Debug functions
 */

/* Implementation defined */
//  void buddy_tree_debug(FILE *stream, struct buddy_tree *t, struct
// buddy_tree_pos pos, size_t start_size);

/* Implementation defined */
 unsigned int buddy_tree_check_invariant(struct buddy_tree *t,
                                               struct buddy_tree_pos pos);

/* Report fragmentation in a 0.0 - 1.0 range */
 float buddy_tree_fragmentation(struct buddy_tree *t);

/*
 * A char-backed bitset implementation
 */

 size_t bitset_sizeof(size_t elements);

 void bitset_set_range(unsigned char *bitset, size_t from_pos,
                             size_t to_pos);

 void bitset_clear_range(unsigned char *bitset, size_t from_pos,
                               size_t to_pos);

 size_t bitset_count_range(unsigned char *bitset, size_t from_pos,
                                 size_t to_pos);

 inline void bitset_set(unsigned char *bitset, size_t pos);

 inline void bitset_clear(unsigned char *bitset, size_t pos);

 inline unsigned int bitset_test(const unsigned char *bitset, size_t pos);

 void bitset_shift_left(unsigned char *bitset, size_t from_pos,
                              size_t to_pos, size_t by);

 void bitset_shift_right(unsigned char *bitset, size_t from_pos,
                               size_t to_pos, size_t by);

/*
 * Debug functions
 */

/* Implementation defined */
//  void bitset_debug(FILE *stream, unsigned char *bitset, size_t length);

/*
 * Bits
 */

/* Returns the number of set bits in the given byte */
 unsigned int popcount_byte(unsigned char b);

/* Returns the index of the highest bit set (1-based) */
 size_t highest_bit_position(size_t value);

/* Returns the nearest larger or equal power of two */
 inline size_t ceiling_power_of_two(size_t value);

/*
 * Math
 */

/* Approximates the square root of a float */
 inline float approximate_square_root(float f);

/*
 Implementation
*/

const unsigned int BUDDY_RELATIVE_MODE = 1;
const unsigned int BUDDY_LEFT_BIAS = 2;

/*
 * A binary buddy memory allocator
 */

struct buddy {
  size_t memory_size;
  size_t virtual_slots;
  union {
    unsigned char *main;
    ptrdiff_t main_offset;
  } arena;
  size_t buddy_flags;
  uint64_t free;
  unsigned char buddy_tree[];
};

struct buddy_embed_check {
  unsigned int can_fit;
  size_t offset;
  size_t buddy_size;
};

 size_t buddy_tree_order_for_memory(size_t memory_size);
 size_t depth_for_size(struct buddy *buddy, size_t requested_size);
 inline size_t size_for_depth(struct buddy *buddy, size_t depth);
 unsigned char *address_for_position(struct buddy *buddy,
                                           struct buddy_tree_pos pos);
 struct buddy_tree_pos position_for_address(struct buddy *buddy,
                                                  const unsigned char *addr);
 unsigned char *buddy_main(struct buddy *buddy);
 unsigned int buddy_relative_mode(struct buddy *buddy);
 struct buddy_tree *buddy_tree(struct buddy *buddy);
 size_t buddy_effective_memory_size(struct buddy *buddy);
 void buddy_toggle_virtual_slots(struct buddy *buddy, unsigned int state);
 void buddy_toggle_range_reservation(struct buddy *buddy, void *ptr,
                                           size_t requested_size,
                                           unsigned int state);
 struct buddy *buddy_resize_standard(struct buddy *buddy,
                                           size_t new_memory_size);
 struct buddy *buddy_resize_embedded(struct buddy *buddy,
                                           size_t new_memory_size);
 unsigned int buddy_is_free(struct buddy *buddy, size_t from);
 unsigned int buddy_is_left_biased(struct buddy *buddy);
 struct buddy_embed_check buddy_embed_offset(size_t memory_size);
 struct buddy_tree_pos deepest_position_for_offset(struct buddy *buddy,
                                                         size_t offset);

#endif /* BUDDY_ALLOC_IMPLEMENTATION */
