/*
 * dt_record.c: Records for Unit 5, Section F.
 *
 * A record selects fields by name. A compiled language can replace a field
 * access with a fixed offset. That access requires no run-time search.
 *
 * This implementation keeps a field-name array.
 * A lookup searches the array and returns an index.
 *
 * An undeclared field returns DT_ERR_FIELD. Records cannot add fields after
 * construction. An associative array can add keys.
 */

#include "dt.h"

#include <stdlib.h>
#include <string.h>

struct dt_record {
    char    *names[DT_RECORD_MAX_FIELDS];
    dt_value values[DT_RECORD_MAX_FIELDS];
    size_t   count;
};

/*
 * dt_record_new builds a record with the specified fields in declaration order.
 * It sets each field to nil and copies each field name.
 * It returns NULL for too many fields or an allocation failure.
 */
dt_record *dt_record_new(const char **field_names, size_t field_count)
{

    if (field_count > DT_RECORD_MAX_FIELDS) return NULL;  // too many fields: capacity refusal
    if (field_count > 0 && field_names == NULL) return NULL;
    dt_record *r = malloc(sizeof *r);
    if (r == NULL) return NULL;
    r->count = 0;   // count only fully built fields so cleanup is safe
    for (size_t i = 0; i < field_count; i++) {
        size_t len = strlen(field_names[i]);
        char *copy = malloc(len + 1);  // own a copy of the name 
        if (copy == NULL) {
            dt_record_free(r); // releases the names copied so far 
            return NULL;
        }
        memcpy(copy, field_names[i], len + 1);
        r->names[i] = copy;
        r->values[i] = dt_value_nil();  // every field starts as nil
        r->count++;  // field i is now fully built
    }
    return r;
}

/*
 * dt_record_free releases the copied field names and the record.
 * It accepts NULL. The environment owns the field values.
 */
void dt_record_free(dt_record *r)
{

    if (r == NULL) return;
    for (size_t i = 0; i < r->count; i++) free(r->names[i]);  // free the copied names, not the values
    free(r);
}

/*
 * dt_record_field_count returns the stored field count in constant time.
 */
size_t dt_record_field_count(const dt_record *r)
{

    return r->count;
}

/*
 * dt_record_field_name writes the field name at declaration position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 * The record prints fields in declaration order.
 */
dt_status dt_record_field_name(const dt_record *r, size_t index, const char **out)
{

    if (index >= r->count) return DT_ERR_RANGE;  // valid positions are 0..count-1
    *out = r->names[index];
    return DT_OK;
}

/*
 * dt_record_get writes the value of field to *out.
 * It returns DT_ERR_FIELD and does not change *out when the field is absent.
 */
dt_status dt_record_get(const dt_record *r, const char *field, dt_value *out)
{

    for (size_t i = 0; i < r->count; i++) {
        if (strcmp(r->names[i], field) == 0) {
            *out = r->values[i];  // same index as the matching name
            return DT_OK;
        }
    }
    return DT_ERR_FIELD;
}

/*
 * dt_record_set replaces the value of field with v.
 * It returns DT_ERR_FIELD and changes nothing when the field is absent.
 * A record cannot gain fields after construction.
 */
dt_status dt_record_set(dt_record *r, const char *field, dt_value v)
{

    for (size_t i = 0; i < r->count; i++) {
        if (strcmp(r->names[i], field) == 0) {
            r->values[i] = v;  // replace the value; schema never grows
            return DT_OK;
        }
    }
    return DT_ERR_FIELD;
}
