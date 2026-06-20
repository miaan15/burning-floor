module;

export module spines;

import common;
import dyn_array;
import allocator;

export enum TokenType : u16 {
    // all the ascii can be token type
    TOKEN_IDENT = 256,
    TOKEN_ID_IDENT,
    TOKEN_NUM,
    TOKEN_STR
};

export struct Token {
    u16 type = 0;
    size_t len = 0;
    size_t index = 0;
};

export struct SpinesParseErr {
    enum Type { // FIXME: name
        NONE = 0,
        ERR_IDENT,
        ERR_FIELD_VAL,
        ERR_SYNTAX,
    } type = NONE;
    size_t column = 1;
    size_t line = 1;
};

export struct SpinesContext {
    enum FieldType : u8 {
        FIELD_INT,
        FIELD_FLOAT,
        FIELD_STRING
    };

    union Field {
        int int_val;
        float float_val;
        size_t string_val;
    };

    struct Ident {
        size_t name_begin;
        size_t name_len;

        size_t fields_begin;
        size_t fields_len;

        size_t parent_len;
    };

    Arena arena{};
    // Packed layout: idents -> field_vals -> field_types
    //                -> ident_names -> string_data
    Ident *idents = nullptr;
    size_t idents_cap = 0;

    Field *field_vals = nullptr;
    FieldType *field_types = nullptr;
    size_t fields_cap = 0;

    char *ident_names = nullptr;
    size_t ident_names_size = 0;

    char *string_data = nullptr;
    size_t string_data_size = 0;

    void destroy() {
        arena.destroy();
    }

    void print_debug() {
        std::cout << "idents - cap: " << idents_cap << std::endl;
        for (size_t i = 0; i < idents_cap; ++i) {
            Ident p = idents[i];
            if (p.name_begin != (size_t)-1) {
                std::cout
                    << "\t " << i << ": ["
                    << std::string_view(ident_names + p.name_begin, p.name_len)
                    << "]: "
                    << "fields " << p.fields_begin << " - " << p.fields_len
                    << " | parent of " << p.parent_len << std::endl;
            } else {
                std::cout
                    << "\t " << i << ": [."
                    << p.name_len
                    << "]: "
                    << "fields " << p.fields_begin << " - " << p.fields_len
                    << " | parent of " << p.parent_len << std::endl;
            }
        }

        std::cout << "Fields - cap: " << fields_cap << std::endl;
        for (size_t i = 0; i < fields_cap; ++i) {
            Field p = field_vals[i];
            FieldType t = field_types[i];
            std::string name = "int";
            if (t == FIELD_FLOAT) name = "float";
            if (t == FIELD_STRING) name = "string";
            std::cout << "\t " << i << ": [" << name << "]: ";
            if (t == FIELD_INT) std::cout << p.int_val;
            if (t == FIELD_FLOAT) std::cout << p.float_val;
            if (t == FIELD_STRING)
                std::cout << string_data + p.string_val;
            std::cout << std::endl;
        }
    }

    SpinesParseErr parse(std::string_view src) {
        auto cal_pos = [&](size_t index) -> std::pair<size_t, size_t> {
            size_t column = 1, line = 1;
            for (size_t i = 0; i < index; ++i) {
                if (src[i] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
            }
            return {column, line};
        };

        DynArray<Token> tokens;

        // LEXER
        // =====================================================================
        {
        std::string_view strv = src;
        size_t cur_index = 0;
        u16 last = 0;

        auto move_forward = [&](bool add_token, u16 token, size_t len) {
            if (token == TOKEN_IDENT || token == TOKEN_ID_IDENT) ++idents_cap;
            if (token == TOKEN_IDENT) ident_names_size += len;
            if (token == TOKEN_NUM || token == TOKEN_STR) ++fields_cap;
            if (token == TOKEN_STR) string_data_size += len + 1;

            if (add_token)
                tokens.append(Token{token, len, cur_index});
            cur_index += len;
            last = token;
            strv.remove_prefix(len);
        };
        auto lexer_err = [&](SpinesParseErr::Type type) {
            auto [column, line] = cal_pos(cur_index);
            tokens.destroy();
            return SpinesParseErr{type, column, line};
        };

        //   INS{}=.,0
        // I 011110011
        // N 000111111
        // S 000111111
        // { 100000000
        // } 011010010
        // = 100000000
        // . 000111111
        // , 011010000

        while (true) {
            while (!strv.empty()) {
                size_t first_non_space = strv.find_first_not_of(" \t\n\r");
                if (first_non_space == std::string_view::npos) {
                    strv = {};
                    break;
                }
                cur_index += first_non_space;
                strv.remove_prefix(first_non_space);

                if (strv.starts_with("//")) {
                    size_t end_cmt = strv.find('\n');
                    if (end_cmt == std::string_view::npos) {
                        strv = {};
                        break;
                    }
                    cur_index += end_cmt + 1;
                    strv.remove_prefix(end_cmt + 1);

                    continue;
                }

                break;
            }
            if (strv.empty()) break;

            char front = strv.front();

            // Ident
            if (std::isalpha(front) || front == '_') {
                if (last == TOKEN_IDENT || last == '=')
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);
                size_t len = strv.find_first_of(",.{} \t\n\r");
                move_forward(true, TOKEN_IDENT, len);
                continue;
            }

            // Number
            if (std::isdigit(front) || front == '-') {
                if (last == TOKEN_IDENT
                    || last == TOKEN_NUM || last == TOKEN_STR)
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);
                size_t len = strv.find_first_of(",.{} \t\n\r");
                move_forward(true, TOKEN_NUM, len);
                continue;
            }

            switch (front) {
            case '{':
                if (last != TOKEN_IDENT)
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);
                move_forward(false, front, 1);
            break;

            case '}':
                if (last != TOKEN_NUM && last != TOKEN_STR
                    && last != '}' && last != ',')
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);
                move_forward(true, front, 1);
            break;

            case '=':
                if (last != TOKEN_IDENT)
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);
                move_forward(true, front, 1);
            break;

            case '.':
                if (last == TOKEN_IDENT
                    || last == TOKEN_NUM || last == TOKEN_STR)
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);
                move_forward(true, TOKEN_ID_IDENT, 2);
            break;

            case ',':
                if (last != TOKEN_NUM && last != TOKEN_STR && last != '}')
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);
                move_forward(false, front, 1);
            break;

            // String
            case '\"': {
                if (last == TOKEN_IDENT
                    || last == TOKEN_NUM || last == TOKEN_STR)
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);

                ++cur_index;
                strv.remove_prefix(1);

                size_t len = strv.find_first_of("\'\"\n\r");
                if (strv[len] != '\"')
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);
                move_forward(true, TOKEN_STR, len);

                ++cur_index;
                strv.remove_prefix(1);
            } break;
            case '\'': {
                if (last == TOKEN_IDENT
                    || last == TOKEN_NUM || last == TOKEN_STR)
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);

                ++cur_index;
                strv.remove_prefix(1);

                size_t len = strv.find_first_of("\'\"\n\r");
                if (strv[len] != '\'')
                    return lexer_err(SpinesParseErr::ERR_SYNTAX);
                move_forward(true, TOKEN_STR, len);

                ++cur_index;
                strv.remove_prefix(1);
            } break;

            default:
                return lexer_err(SpinesParseErr::ERR_SYNTAX);
            break;
            }
        }
        }

        // MEMORY
        // =====================================================================
        size_t arena_size = idents_cap * sizeof(Ident);
        arena_size = (arena_size + alignof(Field) - 1) & ~(alignof(Field) - 1);
        arena_size += fields_cap * sizeof(Field);
        arena_size += fields_cap
                      + ident_names_size
                      + string_data_size;
        arena.init(arena_size);

        idents = (Ident *)arena.alloc(idents_cap * sizeof(Ident),
                                      alignof(Ident));
        field_vals = (Field *)arena.alloc(fields_cap * sizeof(Field),
                                          alignof(Field));
        field_types = (FieldType *)arena.alloc(fields_cap, 1);
        ident_names = (char *)arena.alloc(ident_names_size, 1);
        string_data = (char *)arena.alloc(string_data_size, 1);
        size_t idents_offset = 0;
        size_t fields_offset = 0;
        size_t ident_names_offset = 0;
        size_t string_data_offset = 0;

        // PARSER
        // =====================================================================
        struct IdentStackEntry { size_t ident; size_t next_id_ident; };
        std::vector<IdentStackEntry> ident_stack_container;
        ident_stack_container.reserve(idents_cap);
        std::stack<IdentStackEntry, std::vector<IdentStackEntry>>
            ident_stack{ident_stack_container};

        size_t next_ident = 0;

        auto handle_end_ident = [&]() {
            if (ident_stack.empty()) return;

            _assert(ident_stack.top().ident < idents_offset,
                    "ident_stack.top() out of bounds.");
            Ident old_ident = idents[ident_stack.top().ident];
            size_t old_fields_len = old_ident.fields_len;
            size_t old_parent_len = old_ident.parent_len;

            ident_stack.pop();

            if (ident_stack.empty()) return;
            _assert(ident_stack.top().ident < idents_offset,
                    "ident_stack.top() out of bounds.");
            Ident *cur_ident = &idents[ident_stack.top().ident];
            cur_ident->fields_len += old_fields_len;
            cur_ident->parent_len += old_parent_len;
        };
        auto handle_new_ident = [&]() {
            ident_stack.push({next_ident, 0});
            ++next_ident;
        };

        SpinesParseErr::Type err_type = SpinesParseErr::NONE;
        size_t err_index = 0;
        u8 just_after_assign_ident = 0;
        for (Token token : tokens) {
            switch (token.type) {

            case TOKEN_IDENT: {
                idents[idents_offset++] = Ident{
                    .name_begin = ident_names_offset,
                    .name_len = token.len,
                    .fields_begin = fields_offset,
                    .fields_len = 0,
                    .parent_len = 1};
                _assert(idents_offset <= idents_cap,
                        "idents_len out of bounds");

                for (size_t i = 0; i < token.len; ++i) {
                    ident_names[ident_names_offset + i] = src[token.index + i];
                }
                ident_names_offset += token.len;
                _assert(ident_names_offset <= ident_names_size,
                        "ident_names_offset out of bounds");

                handle_new_ident();
            } break;

            case TOKEN_ID_IDENT: {
                idents[idents_offset++] = Ident{
                    .name_begin = (size_t)-1,
                    .name_len = ident_stack.top().next_id_ident,
                    .fields_begin = fields_offset,
                    .fields_len = 0,
                    .parent_len = 1};
                _assert(idents_offset <= idents_cap,
                        "idents_len out of bounds");

                ++ident_stack.top().next_id_ident;
                handle_new_ident();
            } break;

            case TOKEN_NUM: {
                std::string_view strv(src.data() + token.index, token.len);

                int i_val;
                auto res_i = std::from_chars(strv.data(),
                                             strv.data() + strv.size(),
                                             i_val);

                if (res_i.ptr == strv.data() + strv.size()) {
                    field_types[fields_offset] = FIELD_INT;
                    field_vals[fields_offset].int_val = i_val;
                    ++fields_offset;
                    _assert(fields_offset <= fields_cap,
                            "fields_len out of bounds");
                } else {
                    float f_val;
                    auto res_f = std::from_chars(strv.data(),
                                                 strv.data() + strv.size(),
                                                 f_val);

                    if (res_f.ptr == strv.data() + strv.size()) {
                        field_types[fields_offset] = FIELD_FLOAT;
                        field_vals[fields_offset].float_val = f_val;
                        ++fields_offset;
                        _assert(fields_offset <= fields_cap,
                                "fields_len out of bounds");
                    } else {
                        err_type = SpinesParseErr::ERR_FIELD_VAL;
                        err_index = token.index;
                        goto _ERROR_PATH;
                    }
                }

                _assert(!ident_stack.empty(),
                        "ident stack should not be empty yet");
                _assert(ident_stack.top().ident < idents_offset,
                        "ident_stack.top() out of bounds.");
                ++idents[ident_stack.top().ident].fields_len;
                if (just_after_assign_ident) handle_end_ident();
            } break;

            case TOKEN_STR: {
                std::string_view strv(src.data() + token.index, token.len);

                field_types[fields_offset] = FIELD_STRING;
                field_vals[fields_offset].string_val = string_data_offset;
                ++fields_offset;
                _assert(fields_offset <= fields_cap,
                        "fields_len out of bounds");

                for (char c : strv) {
                    string_data[string_data_offset++] = c;
                }
                string_data[string_data_offset++] = '\0';
                _assert(string_data_offset <= string_data_size,
                        "string_data_offset out of bounds");

                _assert(!ident_stack.empty(),
                        "ident stack should not be empty yet");
                _assert(ident_stack.top().ident < idents_offset,
                        "ident_stack.top() out of bounds.");
                ++idents[ident_stack.top().ident].fields_len;
                if (just_after_assign_ident) handle_end_ident();
            } break;

            case '}': {
                handle_end_ident();
            } break;

            case '=': {
                just_after_assign_ident = 2;
            } break;

            default: {
                err_type = SpinesParseErr::ERR_SYNTAX;
                err_index = token.index;
                goto _ERROR_PATH;
            } break;

            }

            if (just_after_assign_ident > 0) --just_after_assign_ident;
        }

        tokens.destroy();
        return {};

    _ERROR_PATH:
        tokens.destroy();
        arena.destroy();
        idents = nullptr;
        field_vals = nullptr;
        field_types = nullptr;
        ident_names = string_data = nullptr;
        idents_cap = fields_cap = ident_names_size = string_data_size = 0;
        auto [column, line] = cal_pos(err_index);
        return SpinesParseErr{err_type, column, line};
    }
};
