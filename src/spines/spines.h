#ifndef _SPINES_H
#define _SPINES_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _SPN_INLINE
#   if defined(__cplusplus)
#       define _SPN_INLINE inline
#   else
#       define _SPN_INLINE static inline
#   endif
#endif

#ifndef SPN_DISABLE_ERROR
extern char _spn_err_buffer[128];
#define _SPN_SET_ERROR(format, ...) \
    snprintf(_spn_err_buffer, sizeof(_spn_err_buffer), format, __VA_ARGS__)
_SPN_INLINE const char *spn_error(void) { return _spn_err_buffer; }
#else
#define _SPN_SET_ERROR(format, ...) ((void)0)
_SPN_INLINE const char *spn_error(void) { return ""; }
#endif

typedef enum {
    TOKEN_IDENT = 0,
    TOKEN_ID_IDENT,
    TOKEN_NUM,
    TOKEN_STR,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COLON,
    TOKEN_COMMA
} spn_TokenType;

typedef struct {
    uint8_t type;
    size_t len;
    size_t index;
} spn_Token;

typedef enum {
    FIELD_INT = 0,
    FIELD_FLOAT,
    FIELD_STR
} spn_FieldType;

typedef union {
    int64_t int_val;
    double float_val;
    struct {
        uint32_t begin, len;
    } str_val;
} spn_FieldVal;

typedef struct {
    size_t name_begin, name_len;
    size_t fields_begin, fields_len;
    size_t parent_len, parent;
} spn_Ident;

typedef struct {
    void *buffer;
    size_t buffer_offset;
    size_t buffer_cap;

    // Mem layout: idents -> field_vals -> field_types
    //             -> ident_names -> string_data
    spn_Ident *idents;
    size_t idents_cap;

    spn_FieldVal *field_vals;
    uint8_t *field_types;
    size_t fields_cap;

    char *ident_names;
    size_t ident_names_size;

    char *string_data;
    size_t string_data_size;
} spn_Context;

typedef struct {
    spn_FieldVal val;
    uint8_t type;
} spn_Field;

typedef struct {
    spn_Context *cxt;
    size_t index;
} spn_Mark;

/**
 * This is C, just remember to call this
 */
_SPN_INLINE void spn_destroy(spn_Context *cxt) {
    if (cxt->buffer) free(cxt->buffer);
    cxt->buffer = NULL;
}

/**
 * Minh lam tu xuong va da, ca phe, thuoc la va 250 lit do co con
 *
 * Con bao o trong cho ta dien vao?
 */
void spn_parse(spn_Context *cxt, const char *str_ptr, size_t str_len);

_SPN_INLINE spn_Field spn_get(spn_Mark *gr, size_t index) {
    size_t target = gr->cxt->idents[gr->index].fields_begin + index;
    return (spn_Field){ .val = gr->cxt->field_vals[target],
                        .type = gr->cxt->field_types[target] };
}

_SPN_INLINE spn_FieldVal *spn_get_raw(spn_Mark *gr, size_t index) {
    return &gr->cxt->field_vals[gr->cxt->idents[gr->index].fields_begin + index];
}

_SPN_INLINE int64_t spn_get_int(spn_Mark *gr, size_t index) {
    spn_Field field = spn_get(gr, index);
    if (field.type == FIELD_STR) {
#ifndef SPN_DISABLE_ERROR
         _SPN_SET_ERROR("spn_get_int(): field %zu cannot convert to INT, it was %s => Return 0",
                        gr->cxt->idents[gr->index].fields_begin + index, "STR");
#endif
        return 0;
    }

    if (field.type == FIELD_INT) return (int64_t)field.val.int_val;
    else                         return (int64_t)field.val.float_val;
}

_SPN_INLINE double spn_get_float(spn_Mark *gr, size_t index) {
    spn_Field field = spn_get(gr, index);
    if (field.type == FIELD_STR) {
#ifndef SPN_DISABLE_ERROR
         _SPN_SET_ERROR("spn_get_int(): field %zu cannot convert to FLOAT, it was %s => Return 0",
                        gr->cxt->idents[gr->index].fields_begin + index, "STR");
#endif
        return 0;
    }

    if (field.type == FIELD_INT) return (double)field.val.int_val;
    else                         return (double)field.val.float_val;
}

_SPN_INLINE const char *spn_get_str(spn_Mark *gr, size_t index) {
    spn_Field field = spn_get(gr, index);
    if (field.type != FIELD_STR) {
#ifndef SPN_DISABLE_ERROR
         _SPN_SET_ERROR("spn_get_int(): field %zu cannot convert to STR, it was %s => Return NULL",
                        gr->cxt->idents[gr->index].fields_begin + index, field.type == FIELD_INT ? "INT" : "FLOAT");
#endif
        return 0;
    }

    return gr->cxt->string_data + field.val.str_val.begin;
}

/**
 * Returns the string field's data
 */
_SPN_INLINE const char *spn_str(spn_Context *cxt, spn_FieldVal field) {
    return cxt->string_data + field.str_val.begin;
}

/**
 * Retrieves the top-level mark of the context
 */
_SPN_INLINE spn_Mark spn_root(spn_Context *cxt) {
    return (spn_Mark){cxt, 0};
}

/**
 * Mutates the mark to a child-mark
 *
 * Do nothing if error
 */
void spn_move(spn_Mark *gr, const char *dir);

/**
 * Mutates the mark to an auto-indexed child-mark
 *
 * Do nothing if error
 */
void spn_move_id(spn_Mark *gr, size_t id);

/**
 * Returns a new mark from the child-mark
 *
 * Return the same as original mark if error
 */
_SPN_INLINE spn_Mark spn_find(spn_Mark gr, const char *dir) {
    spn_move(&gr, dir);
    return gr;
}

/**
 * Returns a new mark from the child-mark
 *
 * Return the same as original mark if error
 */
_SPN_INLINE spn_Mark spn_find_id(spn_Mark gr, size_t id) {
    spn_move_id(&gr, id);
    return gr;
}

/**
 * Mutates the mark to its parent
 *
 * Do nothing if this is root mark
 */
_SPN_INLINE void spn_to_parent(spn_Mark *gr) {
    spn_Context *cxt = gr->cxt;
    assert(cxt);
    assert(cxt->buffer);
    gr->index = cxt->idents[gr->index].parent;
}

/**
 * Return parent mark
 *
 * Return the same as original mark if error
 */
_SPN_INLINE spn_Mark spn_parent(spn_Mark gr) {
    spn_to_parent(&gr);
    return gr;
}

/**
 * Mutates the mark to a sibling-mark
 *
 * Do nothing if error
 */
_SPN_INLINE void spn_move_sibling(spn_Mark *gr, const char *dir) {
    spn_to_parent(gr);
    spn_move(gr, dir);
}

/**
 * Mutates the mark to an auto-indexed sibling-mark
 *
 * Do nothing if error
 */
_SPN_INLINE void spn_move_sibling_id(spn_Mark *gr, size_t id) {
    spn_to_parent(gr);
    spn_move_id(gr, id);
}

/**
 * Returns a new mark from the child-mark
 *
 * Return the same as original mark if error
 */
_SPN_INLINE spn_Mark spn_find_sibling(spn_Mark gr, const char *dir) {
    spn_move_sibling(&gr, dir);
    return gr;
}

/**
 * Returns a new mark from the child-mark
 *
 * Return the same as original mark if error
 */
_SPN_INLINE spn_Mark spn_find_sibling_id(spn_Mark gr, size_t id) {
    spn_move_sibling_id(&gr, id);
    return gr;
}

/**
 * Advances mark to the next sibling mark (in the same level)
 *
 * Returns false and do nothing if the end is reached
 */
bool spn_step(spn_Mark *gr);

/**
 * Returns the next sibling mark (in the same level)
 *
 * Return the same as original mark if error
 */
_SPN_INLINE spn_Mark spn_next(spn_Mark gr) {
    spn_step(&gr);
    return gr;
}

/**
 * Advances mark to the absolute next mark
 *
 * Returns false and do nothing if the end is reached
 */
bool spn_step_flat(spn_Mark *gr);

/**
 * Returns the absolute next mark
 *
 * Return the same as original mark if error
 */
_SPN_INLINE spn_Mark spn_next_flat(spn_Mark gr) {
    spn_step_flat(&gr);
    return gr;
}

#endif
