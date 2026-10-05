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

// node
typedef struct dt_map_entry {
    char    *key;
    dt_value value;
    struct dt_map_entry *next;
} dt_map_entry;


// bucket
struct dt_map {
    dt_map_entry **buckets;
    size_t bucket_count;
    char **order;
    size_t count;
    size_t order_capacity;
};

// given hash
static unsigned long long dt_map_hash(const char *key)
{
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    
    return h;
}

/*
 * dt_map_new builds an empty map. It returns NULL after an allocation failure.
 */
dt_map *dt_map_new(void)
{
    /* TODO: Return an allocated empty map. Return NULL after an allocation failure.
       dt_map_new()  -> a map whose dt_map_len is 0
       cases/normal/map_basics.case */
    
    // allocate memory
    dt_map *m = malloc(sizeof *m);
    if (!m) {
        return NULL;
    }

    // allocate 16 NULL buckets
    m->buckets = calloc(16, sizeof *m->buckets); 
    if (!m->buckets) {
        free(m);
        return NULL;
    }

    // record how many buckets
    m->bucket_count = 16;

    m->order_capacity = 16;

    // allocate memory for each slot
    m->order = malloc(m->order_capacity * sizeof *m->order);
    if (!m->order) {
        free(m->buckets);
        free(m);
        return NULL;
    }

    // set number of keys stored to 0
    m->count = 0;

    return m;
}

/*
 * dt_map_free releases each entry, copied key, order array, and map.
 * It accepts NULL. The environment owns the values.
 */
void dt_map_free(dt_map *m)
{
    /* TODO: Release each entry, copied key, order array, and map.
       Preserve the values. The environment owns them.
       a map holding a string value  -> the nodes and keys go, the string stays
       dt_map_free(NULL)             -> returns, having done nothing
       cases/cleanup/map_churn.case */

    // nothing to free, return immediately
    if (!m) {
        return;
    }

    // visit every index and free every bucket
    for (size_t b = 0; b < m->bucket_count; b++) {
        dt_map_entry *entry  = m->buckets[b];
        while (entry) {
            dt_map_entry *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }

    // free everything else
    free(m->buckets);
    free(m->order);
    free(m);
}

/*
 * dt_map_len returns the number of keys in constant time.
 */
size_t dt_map_len(const dt_map *m)
{
    /* TODO: Return the current key count.
       Replacing a value does not change this count.
       after put alpha, beta, gamma:  dt_map_len(m) -> 3
       after put beta again:          dt_map_len(m) -> 3, still
       after del alpha:               dt_map_len(m) -> 2
       cases/normal/map_basics.case */

    return m->count;
}



// helper function for looking up keys; used by dt_map_put and dt_map_get
static dt_map_entry *dt_map_find(const dt_map *m, const char *key)
{
    unsigned long long h = dt_map_hash(key);

    // calculate target bucket index and get first entry
    size_t bucket = h % m->bucket_count;
    dt_map_entry *entry = m->buckets[bucket];

    // run until found otherwise return NULL
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}


/*
 * dt_map_put binds v to key.
 * An existing key keeps its insertion position. A new key becomes the last key.
 * Copy each new key because the caller owns the source buffer.
 * Return DT_ERR_CAPACITY after an allocation failure.
 */
dt_status dt_map_put(dt_map *m, const char *key, dt_value v)
{
    /* TODO: Replace the value for an existing key.
       Add a new entry for a new key. Copy each new key.
       Hash the key. Select its bucket. Search the bucket chain.
       Add a new entry to the chain and insertion list.
       put "beta" -> 2 on an empty map    -> DT_OK, "beta" is last in order
       put "beta" -> 22 on that map       -> DT_OK, same position, new value
       an allocation failure              -> DT_ERR_CAPACITY, map unchanged
       cases/normal/map_basics.case */

    // if entry exists, place immediately
    dt_map_entry *exists = dt_map_find(m, key);
    if (exists) {
        exists->value = v;
        return DT_OK;
    }

    // store key length
    size_t key_len = strlen(key);

    // allocate with terminator in mind
    char *key_copy = malloc(key_len + 1);
    if (!key_copy) {
        return DT_ERR_CAPACITY;
    }

    // copy for the map
    memcpy(key_copy, key, key_len + 1);


    // new entry node
    dt_map_entry *entry = malloc(sizeof *entry);
    if (!entry) {
        free(key_copy);
        return DT_ERR_CAPACITY;
    }

    // copy values
    entry->key = key_copy;
    entry->value = v;

    // hash key
    unsigned long long h = dt_map_hash(key);

    // modulo to fit in the array
    size_t bucket = h % m->bucket_count;

    // set the new entry to the top
    entry->next = m->buckets[bucket];
    m->buckets[bucket] = entry;

    // check full array 
    if (m->count == m->order_capacity) {

        // calculate new capacity if full
        size_t new_capacity = m->order_capacity * 2;

        // resize array
        char **grown = realloc(m->order, new_capacity * sizeof *m->order);
        if (!grown) {
            m->buckets[bucket] = entry->next;
            free(entry->key);
            free(entry);
            return DT_ERR_CAPACITY;
        }

        // update 
        m->order = grown;
        m->order_capacity = new_capacity;
    }

    // write new key in the index
    m->order[m->count] = key_copy;
    m->count++;

    return DT_OK;
}

/*
 * dt_map_get writes the value for key to *out.
 * It returns DT_ERR_KEY and does not change *out when the key is absent.
 * An absent key differs from a nil value.
 */
dt_status dt_map_get(const dt_map *m, const char *key, dt_value *out)
{
    /* TODO: Return DT_ERR_KEY when the key is absent.
       Preserve *out after this error. A nil value can be present.
       after put "beta" -> 22:
         dt_map_get(m, "beta", &out)   -> DT_OK, *out is the integer 22
         dt_map_get(m, "ghost", &out)  -> DT_ERR_KEY, *out untouched
       cases/normal/map_basics.case, cases/boundary/map_missing_key.case */
    
    // check for key
    dt_map_entry *exists = dt_map_find(m, key);
    if (!exists) {
        return DT_ERR_KEY;
    }

    // assign value to out
    *out = exists->value;

    return DT_OK;
}

/*
 * dt_map_remove removes key from its bucket and insertion position.
 * It releases the copied key. It returns DT_ERR_KEY when the key is absent.
 */
dt_status dt_map_remove(dt_map *m, const char *key)
{
    /* TODO: Remove the entry from its bucket and insertion position.
       Release the copied key. Return DT_ERR_KEY when the key is absent.
       a map holding alpha, beta, gamma:
         dt_map_remove(m, "alpha")  -> DT_OK, order is now beta, gamma
         dt_map_remove(m, "ghost")  -> DT_ERR_KEY, nothing changes
       reinserting "alpha" appends it after "gamma"
       cases/normal/map_basics.case, cases/boundary/map_remove_missing_key.case */
    
    // hash to find bucket index
    unsigned long long h = dt_map_hash(key);
    size_t bucket = h % m->bucket_count;

    // check each entry
    dt_map_entry *cur = m->buckets[bucket];
    dt_map_entry *prev = NULL;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            break;
        }

        // move pointers
        prev = cur;
        cur = cur->next;
    }

    // error if not found
    if (!cur) {
        return DT_ERR_KEY;
    }

    // if prev exists, skip the current entry
    if (prev) {
        prev->next = cur->next;
    } else {
        m->buckets[bucket] = cur->next;
    }

    // find index of key
    size_t i = 0;
    while (i < m->count && m->order[i] != cur->key) {
        i++;
    }

    // move the order
    for (size_t j = i; j + 1 < m->count; j++) {
        m->order[j] = m->order[j + 1];
    }

    // decrement 
    m->count--;

    // release to remove
    free(cur->key);
    free(cur);

    return DT_OK;
}

/*
 * dt_map_key_at writes the key at insertion position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 */
dt_status dt_map_key_at(const dt_map *m, size_t index, const char **out)
{
    /* TODO: Write the key at the specified insertion position to *out.
       Return DT_ERR_RANGE for an invalid position. Preserve *out after this error.
       The printer uses this order.
       a map holding alpha, beta, gamma:
         dt_map_key_at(m, 0, &out)  -> DT_OK, *out = "alpha"
         dt_map_key_at(m, 3, &out)  -> DT_ERR_RANGE, *out untouched
       cases/normal/map_basics.case */

    // check if out of bound
    if (index >= m->count) {
        return DT_ERR_RANGE;
    }

    // fetch key from index
    *out = m->order[index];
    return DT_OK;
}

