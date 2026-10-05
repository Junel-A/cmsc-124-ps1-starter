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
   if (length == SIZE_MAX) {
        return NULL;
    }
    dt_str *s = malloc(sizeof(dt_str));
    if (!s) {
        return NULL;
    }
    s->capacity = length + 1;
    s->bytes = malloc(s->capacity);
    if (!s->bytes) {
        free(s);
        return NULL;
    }
    if (length > 0 && bytes != NULL) {
        memcpy(s->bytes, bytes, length);
    }
    s->bytes[length] = '\0';
    s->length = length;
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
    if (!s) {
        return 0;
    }
    return s->length;
}

/*
 * dt_str_bytes returns the string bytes. Internal storage can include a final
 * zero byte. Callers must use dt_str_len with this pointer.
 */
const char *dt_str_bytes(const dt_str *s)
{
    if (!s) {
        return "";
    }
    return s->bytes;
}

/*
 * dt_str_append adds `length` bytes and grows the buffer when necessary. It
 * returns DT_ERR_CAPACITY when allocation or size representation fails.
 * The function does not change the string after a failure.
 */
dt_status dt_str_append(dt_str *s, const char *bytes, size_t length)
{
   if (!s) {
        return DT_ERR_CAPACITY;
    }
    if (length == 0) {
        return DT_OK;
    }
    if (length > SIZE_MAX - s->length) {
        return DT_ERR_CAPACITY;
    }
    size_t new_len = s->length + length;
    if (new_len > SIZE_MAX - 1) {
        return DT_ERR_CAPACITY;
    }
    size_t req_cap = new_len + 1;

    if (req_cap > s->capacity) {
        size_t new_cap = s->capacity;
        while (new_cap < req_cap) {
            if (new_cap > SIZE_MAX / 2) {
                new_cap = req_cap;
                if (new_cap < req_cap) {
                    return DT_ERR_CAPACITY;
                }
                break;
            }
            new_cap *= 2;
        }
        char *new_bytes = realloc(s->bytes, new_cap);
        if (!new_bytes) {
            return DT_ERR_CAPACITY;
        }
        s->bytes = new_bytes;
        s->capacity = new_cap;
    }

    if (bytes != NULL) {
        memcpy(s->bytes + s->length, bytes, length);
    }
    s->length = new_len;
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
    /* TODO: Return DT_ERR_RANGE when the requested range exceeds the source.
       Two size_t values can wrap. First compare start with the source length.
       Then compare length with the remaining length.
       s holds "hello" (length 5):
         dt_str_substr(s, 3, 2, &out)  -> DT_OK, *out is "lo"
         dt_str_substr(s, 5, 0, &out)  -> DT_OK, *out is a valid empty string
         dt_str_substr(s, 3, 5, &out)  -> DT_ERR_RANGE, *out untouched
       an allocation failure           -> DT_ERR_CAPACITY, *out untouched
       cases/boundary/substr_exact_end.case, cases/boundary/substr_past_end.case */
    (void)s;
    (void)start;
    (void)length;
    (void)out;
    return DT_ERR_RANGE;
}

/*
 * dt_str_eq reports whether both strings hold the same bytes.
 * The stored lengths let the comparison include embedded zero bytes.
 */
bool dt_str_eq(const dt_str *a, const dt_str *b)
{
    /* TODO: Compare the lengths first. Then use memcmp.
       strcmp ends at an embedded zero byte and can report unequal data as equal.
       "world" and "world"  -> true
       "hello" and "world"  -> false
       "a\0b" and "a"       -> false because their lengths are 3 and 1
       cases/normal/string_building.case, cases/capacity/embedded_zero_byte.case */
    (void)a;
    (void)b;
    return false;
}
