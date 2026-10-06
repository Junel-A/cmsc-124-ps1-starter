/*
 * dt_map.c: Associative arrays for Unit 5, Section E.
 *
 * An array does not store its indices. This map stores its keys.
 * An array calculates a position with one subtraction.
 * The map calculates a hash and then compares keys in one bucket.
 *
 * Hashing turns the key into a bucket number. Compare all keys in that bucket
 * because two keys can select it. Use a linked list for each bucket. Start the
 * unsigned accumulator at 14695981039346656037ULL. For each unsigned byte,
 * exclusive-or the byte into it and multiply by 1099511628211ULL.
 *
 * A separate list stores insertion order for stable output. dt_map_key_at
 * reads this list. Updating a key preserves its position. Removing and
 * reinserting a key moves it to the end.
 */

#include "dt.h"

#include <stdlib.h>
#include <string.h>

#define DT_MAP_BUCKETS 16

struct dt_map {
    // Each node stores a dynamically allocated key, a value, and a next pointer
    struct dt_map_node {
        char *key;
        dt_value value;
        struct dt_map_node *next;
    } *buckets[DT_MAP_BUCKETS];

    struct dt_map_node **order; // Dynamic array storing node pointers in order
    size_t count;               // Number of active keys currently in the map
    size_t order_cap;           // Allocated capacity of the order pointer array
};

/*
 * dt_map_new builds an empty map. It returns NULL after an allocation failure.
 */
dt_map *dt_map_new(void)
{

    // Allocate memory for the main map container
    dt_map *m = malloc(sizeof *m);
    if (m == NULL) return NULL;

    /* Initialize all bucket heads to NULL */
    for (size_t i = 0; i < DT_MAP_BUCKETS; i++) {
        m->buckets[i] = NULL;
    }

    // Start with no order array allocated and zero items
    m->order = NULL;
    m->count = 0;
    m->order_cap = 0;

    return m;
}

/*
 * dt_map_free releases each entry, copied key, order array, and map.
 * It accepts NULL. The environment owns the values.
 */
void dt_map_free(dt_map *m)
{
    
        if (m == NULL) return;

    // Iterating through m->order allows freeing every key copy and node w/o needing to traverse all 64 bucket chains
    for (size_t i = 0; i < m->count; i++) {
        free(m->order[i]->key); // Free the copied key string
        free(m->order[i]);      // Free the node memory itself
    }

    free(m->order); // Free the dynamic insertion-order array
    free(m);        // Free the map container structure
}

/*
 * dt_map_len returns the number of keys in constant time.
 */
size_t dt_map_len(const dt_map *m)
{

   if (m == NULL) return 0;
    return m->count;    //O(1) lookup of active key count
}

/*
 * dt_map_put binds v to key.
 * An existing key keeps its insertion position. A new key becomes the last key.
 * Copy each new key because the caller owns the source buffer.
 * Return DT_ERR_CAPACITY after an allocation failure.
 */
dt_status dt_map_put(dt_map *m, const char *key, dt_value v)
{
    if (m == NULL || key == NULL) return DT_ERR_CAPACITY;

    // 64-bit FNV-1a Hash Calculation
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    size_t b = (size_t)(h % DT_MAP_BUCKETS); // Map hash to a valid bucket index 

    // Search the bucket chain for an existing key
    struct dt_map_node *curr = m->buckets[b];
    while (curr != NULL) {
        if (strcmp(curr->key, key) == 0) {
            // Key exists: update value only; insertion order remains unchanged
            curr->value = v;
            return DT_OK;
        }
        curr = curr->next;
    }

    // Grow the insertion order pointer array if full
    if (m->count == m->order_cap) {
        size_t new_cap = m->order_cap == 0 ? 8 : m->order_cap * 2;
        if (new_cap > (size_t)-1 / sizeof *m->order) return DT_ERR_CAPACITY;

        struct dt_map_node **grown = realloc(m->order, new_cap * sizeof *m->order);
        if (grown == NULL) return DT_ERR_CAPACITY;

        m->order = grown;
        m->order_cap = new_cap;
    }

    // Allocate node memory
    struct dt_map_node *node = malloc(sizeof *node);
    if (node == NULL) return DT_ERR_CAPACITY;

    // Allocate memory for key copy and duplicate string
    size_t len = strlen(key);
    node->key = malloc(len + 1);
    if (node->key == NULL) {
        free(node);
        return DT_ERR_CAPACITY;
    }
    memcpy(node->key, key, len + 1);
    node->value = v;

    // Push the new node onto the head of its bucket chain
    node->next = m->buckets[b];
    m->buckets[b] = node;

    // Append node pointer to insertion order tracking list
    m->order[m->count++] = node;

    return DT_OK;
}

/*
 * dt_map_get writes the value for key to *out.
 * It returns DT_ERR_KEY and does not change *out when the key is absent.
 * An absent key differs from a nil value.
 */
dt_status dt_map_get(const dt_map *m, const char *key, dt_value *out)
{

    if (m == NULL || key == NULL || out == NULL) return DT_ERR_KEY;

    // Calculate FNV-1a hash bucket
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    size_t b = (size_t)(h % DT_MAP_BUCKETS);

    // Search bucket chain for key
    struct dt_map_node *curr = m->buckets[b];
    while (curr != NULL) {
        if (strcmp(curr->key, key) == 0) {
            *out = curr->value; // Match found: write value to output parameter
            return DT_OK;
        }
        curr = curr->next;
    }

    // Key absent: leave *out untouched
    return DT_ERR_KEY;
}

/*
 * dt_map_remove removes key from its bucket and insertion position.
 * It releases the copied key. It returns DT_ERR_KEY when the key is absent.
 */
dt_status dt_map_remove(dt_map *m, const char *key)
{
    
    if (m == NULL || key == NULL) return DT_ERR_KEY;

    // Calculate FNV-1a hash bucket
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    size_t b = (size_t)(h % DT_MAP_BUCKETS);

    // Unlink node from bucket chain using pointer-to-pointer traversal
    struct dt_map_node **link = &m->buckets[b];
    while (*link != NULL && strcmp((*link)->key, key) != 0) {
        link = &(*link)->next;
    }

    // Key not found in bucket 
    if (*link == NULL) return DT_ERR_KEY;

    struct dt_map_node *target = *link;
    *link = target->next;   // Bypass and unlink target node

    // Find target's index inside insertion order array 
    size_t i = 0;
    while (i < m->count && m->order[i] != target) {
        i++;
    }

    // Shift remaining elements left to maintain contiguous insertion order 
    if (i < m->count) {
        memmove(&m->order[i], &m->order[i + 1],
                (m->count - i - 1) * sizeof *m->order);
        m->count--;
    }

    // Free node resources 
    free(target->key);
    free(target);
    return DT_OK;
}

/*
 * dt_map_key_at writes the key at insertion position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 */
dt_status dt_map_key_at(const dt_map *m, size_t index, const char **out)
{
    // Validate index boundaries against current key count 
    if (m == NULL || out == NULL || index >= m->count) {
        return DT_ERR_RANGE;
    }

    // Retrieve key string stored at specified insertion index
    *out = m->order[index]->key;
    return DT_OK;
}
