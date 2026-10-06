/*
 * dt_str.c: Length-carrying strings for Unit 5, Section B.
 *
 * A C string is a null-terminated character sequence stored in an array.
 * An array expression usually converts to a pointer to its first character.
 * strlen reads only through the first zero byte.
 * A pointer does not store the array capacity.
 *
 * This type stores the length and capacity with the bytes. dt_str_len reads a
 * field. A zero byte is data. Append operations use the stored capacity.
 *
 * An implementation can store a final zero byte after the data.
 * The public interface requires callers to use dt_str_len.
 */

#include "dt.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct dt_str {
    char  *bytes;
    size_t length;
    size_t capacity;
};

/*
 * dt_str_new copies the first `length` bytes. A zero byte is data. The function
 * returns NULL when allocation or size representation fails.
 */
dt_str *dt_str_new(const char *bytes, size_t length)
{

    // avoid bad values
    if (length == SIZE_MAX) {
        return NULL; 
    }

    // allocate memory and bail if fail
    dt_str *s = malloc(sizeof *s);
    if (!s) {
        return NULL;
    }

    // allocate buffer
    s->bytes = malloc(length + 1);
    if (!s->bytes) {
        free(s);
        return NULL;
    }

    // check if worth copying
    if (length > 0) {
        memcpy(s->bytes, bytes, length);
    }

    // prevent run off
    s->bytes[length] = '\0';

    // record length and buffer size
    s->length = length;
    s->capacity = length + 1;
    return s;
}

/*
 * dt_str_free releases the buffer and handle. It accepts NULL.
 */
void dt_str_free(dt_str *s)
{

    if (!s) {
        return;
    }
    free(s->bytes);
    free(s);
}

/*
 * dt_str_len returns the stored byte count in constant time.
 */
size_t dt_str_len(const dt_str *s)
{

    return s->length;
    
}

/*
 * dt_str_bytes returns the string bytes. Internal storage can include a final
 * zero byte. Callers must use dt_str_len with this pointer.
 */
const char *dt_str_bytes(const dt_str *s)
{

    return s->bytes;
    return "";
}

/*
 * dt_str_append adds `length` bytes and grows the buffer when necessary. It
 * returns DT_ERR_CAPACITY when allocation or size representation fails.
 * The function does not change the string after a failure.
 */
dt_status dt_str_append(dt_str *s, const char *bytes, size_t length)
{

    // check for overflow 
    if (length > SIZE_MAX - s->length - 1) {
        return DT_ERR_CAPACITY; 
    }

    // length after appending
    size_t new_length = s->length + length;
    size_t needed = new_length + 1;

    // adjust buffer if needed is larger
    if (needed > s->capacity) {

        // baseline, prevent doubling zero capacity
        size_t new_capacity = s->capacity == 0 ? 16 : s->capacity;


        // double capacity until it can hold needed
        while (new_capacity < needed) {

            // guard against overflow, keep it half of max size
            if (new_capacity > SIZE_MAX / 2) {
                new_capacity = needed;
                break;
            }
            new_capacity *= 2;
        }

        // regrow buffer
        char *grown = realloc(s->bytes, new_capacity);

        // return if fail, s is unchanged
        if (!grown) {
            return DT_ERR_CAPACITY; 
        }

        // replace pointer and capacity
        s->bytes = grown;
        s->capacity = new_capacity;
    }

    // copy 
    if (length > 0) {
        memcpy(s->bytes + s->length, bytes, length);
    }

    // update length and terminator
    s->length = new_length;
    s->bytes[s->length] = '\0';

    return DT_OK;
}

/*
 * dt_str_substr builds a new string from length bytes at start.
 * It returns DT_ERR_RANGE when the requested range exceeds the source.
 * It returns DT_ERR_CAPACITY after an allocation failure.
 * The function does not change the source string.
 */
dt_status dt_str_substr(const dt_str *s, size_t start, size_t length, dt_str **out)
{

    // check if equal or exceeds
    if (start > s->length) {
        return DT_ERR_RANGE;
    }

    // ensure it fits
    if (length > s->length - start) {
        return DT_ERR_RANGE;
    }

    // build new string
    dt_str *result = dt_str_new(s->bytes + start, length);
    if (!result) {
        return DT_ERR_CAPACITY;
    }

    *out = result;
    return DT_OK;
}

/*
 * dt_str_eq reports whether both strings hold the same bytes.
 * The stored lengths let the comparison include embedded zero bytes.
 */
bool dt_str_eq(const dt_str *a, const dt_str *b)
{
    // bail if length not equal
    if (a->length != b->length) {
        return false;
    }


    return memcmp(a->bytes, b->bytes, a->length) == 0;
}
