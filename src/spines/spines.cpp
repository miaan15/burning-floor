module;

export module spines;

import common;
import dyn_array;
import arena;

export enum TokenType : u16 {
    TOKEN_IDENTIFIER = 256,
    TOKEN_NUMBER,
    TOKEN_STRING
};
export struct Token {
    u16 type = 0;
    size_t len = 0;

    size_t index = 0;
    size_t column = 1;
    size_t line = 1;
};

export struct SpinesParseError {
    enum Type {
        NONE = 0,
        INVALID_IDENTIFIER_NAME,
        INVALID_FIELD_VAL,
        INVALID_SYNTAX
    } type = NONE;

    size_t index = 0;
    size_t column = 1;
    size_t line = 1;
};
export struct SpinesContext {
    struct Field {
        char type; // 0 = int
                   // 1 = float
                   // 2 = string (size_t)
        union {
            int int_val;
            float float_val;
            size_t string_val;
        };
    };
    struct Identifier {
        size_t name_begin;
        size_t name_len;

        size_t fields_begin;
        size_t fields_len;

        size_t parent_len;
    };

    Arena arena{};

    Identifier *identifiers = nullptr;
    size_t identifiers_cap = 0;

    Field *fields = nullptr;
    size_t fields_cap = 0;

    char *identifier_names = nullptr;
    size_t identifier_names_size = 0;

    char *string_data = nullptr;
    size_t string_data_size = 0;

    void destroy() {
        arena.destroy();
    }

    void print_debug() {
        std::cout << "Identifiers - cap: " << identifiers_cap << "\n";
        for (size_t i = 0; i < identifiers_cap; ++i) {
            Identifier p = identifiers[i];
            std::cout
                << "\t " << i << ": ["
                << std::string_view(identifier_names + p.name_begin, p.name_len)
                << "]: "
                << "fields " << p.fields_begin << " - " << p.fields_len
                << " | parent of " << p.parent_len << "\n";
        }
        std::cout << "Fields - cap: " << fields_cap << "\n";
        for (size_t i = 0; i < fields_cap; ++i) {
            Field p = fields[i];
            std::string name = "int";
            if (p.type == 1) name = "float";
            if (p.type == 2) name = "string";
            std::cout << "\t " << i << ": [" << name << "]: ";
            if (p.type == 0) std::cout << p.int_val;
            if (p.type == 1) std::cout << p.float_val;
            if (p.type == 2)
                std::cout << string_data + p.string_val;
            std::cout << "\n";
        }
    }

    SpinesParseError parse(std::string_view source) {
        DynArray<Token> tokens;

        // LEXER
        // =====================================================================
        {
        enum {
            EXPECT_IDENTIFIER,
            EXPECT_FIELD_OR_ARR,
            AFTER_IDENTIFIER,
            AFTER_FIELD,
            AFTER_SEPARATOR,
            AFTER_ARRAY_BEGIN,
            AFTER_ARRAY_END,
        } state = EXPECT_IDENTIFIER;

        std::string_view strv = source;
        Token token;

        auto advance_loc = [&](size_t v) {
            token.index += v;
            token.column += v;
            strv.remove_prefix(v);
        };
        auto append_token = [&](TokenType type, size_t len) {
            if (type == TOKEN_IDENTIFIER) {
                ++identifiers_cap;
                identifier_names_size += len;
            }
            if (type == TOKEN_NUMBER || type == TOKEN_STRING) ++fields_cap;
            if (type == TOKEN_STRING) string_data_size += len + 1;
            token.type = type;
            token.len = len;
            tokens.append(token);
        };

        auto get_identifier_len = [&](std::string_view strv) -> size_t {
            size_t len = 0;
            for (auto c : strv) {
                if (std::isalnum((unsigned char)c) || c == '_') ++len;
                else break;
            }
            return len;
        };

        auto might_be_value = [&](char c) -> bool {
            return std::isdigit(c) || c == '-' || c == '\"';
        };

        auto handle_remove_spaces =
            [&](bool *line_breaked) -> bool {
                if (strv.empty()) return true;
                auto s_front = strv.front();
                if (s_front == '\n' || s_front == '\r'
                    || s_front == '\t' || s_front == ' ') {
                    constexpr size_t TAB_WIDTH = 4;
                    size_t removed = 0;
                    for (auto c : strv) {
                        if (c == '\n') {
                            ++token.line;
                            token.column = 1;
                            *line_breaked = true;
                        } else if (c == '\t') {
                            token.column +=
                                TAB_WIDTH - ((token.column - 1) % TAB_WIDTH);
                        } else if (c == ' ') {
                            ++token.column;
                        } else {
                            break;
                        }
                        ++token.index;
                        ++removed;
                    }
                    strv.remove_prefix(removed);
                    if (strv.empty()) return true;
                }
                return false;
            };

        // =====================================================================
        while (true) {
            bool line_breaked = false;
            if (handle_remove_spaces(&line_breaked)) break;

            switch (state) {

            std::cout << "cur state: " << (int)state 
                      << " - " << token.line << ":" << token.column << "\n";

            case EXPECT_IDENTIFIER: {
                if (might_be_value(strv.front())) {
                    tokens.destroy();
                    return SpinesParseError{
                        SpinesParseError::INVALID_IDENTIFIER_NAME,
                        token.index, token.column, token.line
                    };
                }

                size_t len = get_identifier_len(strv);
                append_token(TOKEN_IDENTIFIER, len);
                advance_loc(len);
                state = AFTER_IDENTIFIER;
            } break;

            case EXPECT_FIELD_OR_ARR: {
                switch (strv.front()) {
                case '{': {
                    append_token((TokenType)strv.front(), 1);
                    advance_loc(1);
                    state = AFTER_ARRAY_BEGIN;
                } break;
                case '\"': {
                    advance_loc(1);
                    size_t end_string_i = strv.find_first_of("\"");
                    if (end_string_i != 0)
                        append_token(TOKEN_STRING, end_string_i);
                    advance_loc(end_string_i + 1);
                    state = AFTER_FIELD;
                } break;
                default: {
                    if (might_be_value(strv.front())) {
                        size_t num_len = strv.find_first_of(" \t\n\r,}");
                        append_token(TOKEN_NUMBER, num_len);
                        advance_loc(num_len);
                        state = AFTER_FIELD;
                    } else {
                        tokens.destroy();
                        return SpinesParseError{
                            SpinesParseError::INVALID_FIELD_VAL,
                            token.index, token.column, token.line
                        };
                    }
                } break;
                }
            } break;

            case AFTER_IDENTIFIER: {
                if (strv.front() != '=' && strv.front() != '{') {
                    tokens.destroy();
                    return SpinesParseError{
                        SpinesParseError::INVALID_SYNTAX,
                        token.index, token.column, token.line
                    };
                }
                if (strv.front() == '=') advance_loc(1);
                state = EXPECT_FIELD_OR_ARR;
            } break;

            case AFTER_FIELD: {
                switch (strv.front()) {
                case ',':
                    advance_loc(1);
                    state = AFTER_SEPARATOR;
                    break;
                case '}':
                    append_token((TokenType)strv.front(), 1);
                    advance_loc(1);
                    state = AFTER_ARRAY_END;
                    break;
                default:
                    state = EXPECT_IDENTIFIER;
                    break;
                }
            } break;

            case AFTER_SEPARATOR: {
                if (might_be_value(strv.front())) {
                    state = EXPECT_FIELD_OR_ARR;
                } else if (strv.front() == '}') {
                    advance_loc(1);
                    state = AFTER_ARRAY_END;
                } else {
                    state = EXPECT_IDENTIFIER;
                }
            } break;

            case AFTER_ARRAY_BEGIN: {
                if (might_be_value(strv.front())) {
                    state = EXPECT_FIELD_OR_ARR;
                } else if (strv.front() == '}') {
                    advance_loc(1);
                    state = AFTER_ARRAY_END;
                } else {
                    state = EXPECT_IDENTIFIER;
                }
            } break;

            case AFTER_ARRAY_END: {
                if (strv.front() == ',') {
                    advance_loc(1);
                    state = AFTER_SEPARATOR;
                } else {
                    state = EXPECT_IDENTIFIER;
                }
            } break;

            }
        }
        }

        // =====================================================================
        // Packed layout: identifiers -> field_vals -> identifier_names -> string_data
        size_t arena_size = identifiers_cap * sizeof(Identifier);
        arena_size = (arena_size + alignof(Field) - 1) & ~(alignof(Field) - 1);
        arena_size += fields_cap * sizeof(Field);
        arena_size += identifier_names_size + string_data_size;
        arena.init(arena_size);

        identifiers =
            (Identifier *)arena.alloc(identifiers_cap * sizeof(Identifier),
                                      alignof(Identifier));
        fields = (Field *)arena.alloc(fields_cap * sizeof(Field),
                                             alignof(Field));
        identifier_names = (char *)arena.alloc(identifier_names_size, 1);
        string_data = (char *)arena.alloc(string_data_size, 1);
        size_t identifiers_offset = 0;
        size_t fields_offset = 0;
        size_t identifier_names_offset = 0;
        size_t string_data_offset = 0;

        // =====================================================================
        std::vector<size_t> identifier_stack_container;
        identifier_stack_container.reserve(tokens.len);
        std::stack<size_t, std::vector<size_t>>
            identifier_stack{identifier_stack_container};

        size_t next_identifier_id = 0;

        auto handle_go_out_of_identifier = [&]() {
            if (identifier_stack.empty()) return;

            _assert(identifier_stack.top() < identifiers_offset,
                    "identifier_stack.top() out of bounds.");
            Identifier old_identifier = identifiers[identifier_stack.top()];
            size_t old_fields_len = old_identifier.fields_len;
            size_t old_parent_len = old_identifier.parent_len;

            identifier_stack.pop();

            if (identifier_stack.empty()) return;
            _assert(identifier_stack.top() < identifiers_offset,
                    "identifier_stack.top() out of bounds.");
            Identifier *cur_identifier = &identifiers[identifier_stack.top()];
            cur_identifier->fields_len += old_fields_len;
            cur_identifier->parent_len += old_parent_len;
        };

        // =====================================================================
        u8 just_after_identifier = 0; // this should just be a flag-bool but
                                      // I need a "hack" so it is u8
        for (Token token : tokens) {
            switch (token.type) {

            case TOKEN_IDENTIFIER: {
                identifiers[identifiers_offset++] = Identifier{
                    .name_begin = identifier_names_offset,
                    .name_len = token.len,
                    .fields_begin = fields_offset,
                    .fields_len = 0,
                    .parent_len = 1};
                _assert(identifiers_offset <= identifiers_cap,
                        "identifiers_len out of bounds");

                for (size_t i = 0; i < token.len; ++i) {
                    identifier_names[identifier_names_offset + i] =
                        source[token.index + i];
                }
                identifier_names_offset += token.len;
                _assert(identifier_names_offset <= identifier_names_size,
                        "identifier_names_offset out of bounds");

                identifier_stack.push(next_identifier_id);
                ++next_identifier_id;

                just_after_identifier = 2; // hack
            } break;

            case TOKEN_NUMBER: {
                std::string_view strv(source.data() + token.index, token.len);

                int i_val;
                auto res_i = std::from_chars(strv.data(),
                                             strv.data() + strv.size(),
                                             i_val);

                if (res_i.ptr == strv.data() + strv.size()) {
                    fields[fields_offset++] = Field{.type = 0,
                                                 .int_val = i_val};
                    _assert(fields_offset <= fields_cap,
                            "fields_len out of bounds");
                } else {
                    float f_val;
                    auto res_f = std::from_chars(strv.data(),
                                                 strv.data() + strv.size(),
                                                 f_val);

                    if (res_f.ptr == strv.data() + strv.size()) {
                        fields[fields_offset++] = Field{.type = 1,
                                                     .float_val = f_val};
                        _assert(fields_offset <= fields_cap,
                                "fields_len out of bounds");
                    } else {
                        tokens.destroy();
                        return SpinesParseError{
                            SpinesParseError::INVALID_FIELD_VAL,
                            token.index, token.column, token.line
                        };
                    }
                }

                _assert(!identifier_stack.empty(),
                        "identifier stack should not be empty yet");
                _assert(identifier_stack.top() < identifiers_offset,
                        "identifier_stack.top() out of bounds.");
                ++identifiers[identifier_stack.top()].fields_len;
                if (just_after_identifier) handle_go_out_of_identifier();
            } break;

            case TOKEN_STRING: {
                std::string_view strv(source.data() + token.index, token.len);

                fields[fields_offset++] = Field{.type = 2,
                                             .string_val = string_data_offset};
                _assert(fields_offset <= fields_cap,
                        "fields_len out of bounds");

                for (char c : strv) {
                    string_data[string_data_offset++] = c;
                }
                string_data[string_data_offset++] = '\0';
                _assert(string_data_offset <= string_data_size,
                        "string_data_offset out of bounds");

                _assert(!identifier_stack.empty(),
                        "identifier stack should not be empty yet");
                _assert(identifier_stack.top() < identifiers_offset,
                        "identifier_stack.top() out of bounds.");
                ++identifiers[identifier_stack.top()].fields_len;
                if (just_after_identifier) handle_go_out_of_identifier();
            } break;

            case '{': {
            } break;

            case '}': {
                handle_go_out_of_identifier();
            } break;

            default: {
                tokens.destroy();
                return SpinesParseError{
                    SpinesParseError::INVALID_SYNTAX,
                    token.index, token.column, token.line
                };
            } break;

            }

            if (just_after_identifier > 0) --just_after_identifier;
        }

        tokens.destroy();
        return SpinesParseError{};
    }
};
