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
    
    // check if valid amount of fields
    if (field_count > DT_RECORD_MAX_FIELDS) {
        return NULL;
    }

    // allocate memory for record
    dt_record *r = malloc(sizeof *r);
    if (!r) {
        return NULL;
    }

    // initialize
    r->count = 0;

    // loop through fields
    for (size_t i = 0; i < field_count; i++) {

        // store length of field name
        size_t len = strlen(field_names[i]);

        // allocate buffer 
        char *copy = malloc(len + 1);
        if (!copy) {
            for (size_t j = 0; j < i; j++) {
                free(r->names[j]);
            }
            free(r);
            return NULL;
        }

        // copy to field names to buffer
        memcpy(copy, field_names[i], len + 1);

        // sotre names and initial value
        r->names[i] = copy;
        r->values[i] = dt_value_nil();
        r->count = i + 1;
    }

    return r;
}


/*
 * dt_record_free releases the copied field names and the record.
 * It accepts NULL. The environment owns the field values.
 */
void dt_record_free(dt_record *r)
{
    
    // return immediately if null
    if (!r) {
        return;
    }

    // free each field
    for (size_t i = 0; i < r->count; i++) {
        free(r->names[i]);
    }

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

    // check if out of bounds
    if (index >= r->count) {
        return DT_ERR_RANGE;
    }

    // fetch field name from index
    *out = r->names[index];
    return DT_OK;
}

// helper function for looking up field
static size_t dt_record_find(const dt_record *r, const char *field)
{
    // return the index of the field
    for (size_t i = 0; i < r->count; i++) {
        if (strcmp(r->names[i], field) == 0) {
            return i;
        }
    }

    // nothing found
    return r->count;
}

/*
 * dt_record_get writes the value of field to *out.
 * It returns DT_ERR_FIELD and does not change *out when the field is absent.
 */
dt_status dt_record_get(const dt_record *r, const char *field, dt_value *out)
{
    
    // check if field exists
    size_t i = dt_record_find(r, field);
    if (i == r->count) {
        return DT_ERR_FIELD;
    }

    // fetch value at the index
    *out = r->values[i];
    return DT_OK;
}

/*
 * dt_record_set replaces the value of field with v.
 * It returns DT_ERR_FIELD and changes nothing when the field is absent.
 * A record cannot gain fields after construction.
 */
dt_status dt_record_set(dt_record *r, const char *field, dt_value v)
{
    
    // check if field exists
    size_t i = dt_record_find(r, field);
    if (i == r->count) {
        return DT_ERR_FIELD;
    }

    // sets overwrites value
    r->values[i] = v;
    return DT_OK;

}
