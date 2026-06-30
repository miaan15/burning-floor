module;

extern "C" {
#include "./spines.h"
}

export module spines;

export using ::spn_TokenType;
export using ::spn_Token;
export using ::spn_FieldType;
export using ::spn_Field;
export using ::spn_Ident;
export using ::spn_Context;
export using ::spn_Group;

export using ::spn_destroy;
export using ::spn_parse;
export using ::spn_root;
export using ::spn_move;
export using ::spn_move_id;
export using ::spn_find;
export using ::spn_find_id;
export using ::spn_step;
export using ::spn_next;
export using ::spn_step_flat;
export using ::spn_next_flat;
export using ::spn_str;
