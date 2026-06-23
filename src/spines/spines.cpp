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

export struct SpinesContext {
    enum FieldType : u8 {
        FIELD_INT,
        FIELD_FLOAT,
        FIELD_STRING
    };

    union Field {
        i64 int_val;
        f64 float_val;
        u64 string_val;
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

    void init(std::string_view src) {
        if (arena.buffer) {
            log_err("SpinesContext.init: already initiated");
            return;
        }

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
        auto lexer_err = [&]() {
            auto [col, line] = cal_pos(cur_index);
            tokens.destroy();
            log_err("SpinesContext.parse: lexer error - syntax error at %d:%d",
                    col, line);
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
                if (last == TOKEN_IDENT || last == '=') {
                    lexer_err();
                    return;
                }
                auto it = stdr::find_if(strv, [](char c) {
                    return !(std::isalpha(c) || std::isdigit(c) || c == '_');
                });
                size_t len = std::distance(strv.begin(), it);
                move_forward(true, TOKEN_IDENT, len);
                continue;
            }

            // Number
            if (std::isdigit(front) || front == '-') {
                if (last == TOKEN_IDENT
                    || last == TOKEN_NUM || last == TOKEN_STR) {
                    lexer_err();
                    return;
                }
                size_t len = strv.find_first_of(",.{} \t\n\r");
                move_forward(true, TOKEN_NUM, len);
                continue;
            }

            switch (front) {
            case '{':
                if (last != TOKEN_IDENT) {
                    lexer_err();
                    return;
                }
                move_forward(false, front, 1);
            break;

            case '}':
                if (last != TOKEN_NUM && last != TOKEN_STR
                    && last != '}' && last != ',') {
                    lexer_err();
                    return;
                }
                move_forward(true, front, 1);
            break;

            case '=':
                if (last != TOKEN_IDENT) {
                    lexer_err();
                    return;
                }
                move_forward(true, front, 1);
            break;

            case '.':
                if (last == TOKEN_IDENT
                    || last == TOKEN_NUM || last == TOKEN_STR) {
                    lexer_err();
                    return;
                }
                move_forward(true, TOKEN_ID_IDENT, 2);
            break;

            case ',':
                if (last != TOKEN_NUM && last != TOKEN_STR && last != '}') {
                    lexer_err();
                    return;
                }
                move_forward(false, front, 1);
            break;

            // String
            case '\"': {
                if (last == TOKEN_IDENT
                    || last == TOKEN_NUM || last == TOKEN_STR) {
                    lexer_err();
                    return;
                }

                ++cur_index;
                strv.remove_prefix(1);

                size_t len = strv.find_first_of("\'\"\n\r");
                if (strv[len] != '\"') {
                    lexer_err();
                    return;
                }
                move_forward(true, TOKEN_STR, len);

                ++cur_index;
                strv.remove_prefix(1);
            } break;
            case '\'': {
                if (last == TOKEN_IDENT
                    || last == TOKEN_NUM || last == TOKEN_STR) {
                    lexer_err();
                    return;
                }

                ++cur_index;
                strv.remove_prefix(1);

                size_t len = strv.find_first_of("\'\"\n\r");
                if (strv[len] != '\'') {
                    lexer_err();
                    return;
                }
                move_forward(true, TOKEN_STR, len);

                ++cur_index;
                strv.remove_prefix(1);
            } break;

            default:
                lexer_err();
                return;
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

        std::string err_mess{};
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
                        err_mess = "invalid data field";
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
                err_mess = "unrecognizable symbols";
                err_index = token.index;
                goto _ERROR_PATH;
            } break;

            }

            if (just_after_assign_ident > 0) --just_after_assign_ident;
        }

        tokens.destroy();
        return;

    _ERROR_PATH:
        tokens.destroy();
        arena.destroy();
        idents = nullptr;
        field_vals = nullptr;
        field_types = nullptr;
        ident_names = string_data = nullptr;
        idents_cap = fields_cap = ident_names_size = string_data_size = 0;
        auto [col, line] = cal_pos(err_index);
        log_err("SpinesContext.init: %s at %d:%d", err_mess.c_str(), col, line);
        return;
    }

    struct Group {
        bool err = false;
        SpinesContext *ref = nullptr;
        size_t ident = (size_t)-1;

        [[nodiscard]] Group into(std::string_view directive) const {
            if (err) return Group{.err = true};

            _assert(ref, "ref is null");
            _assert(ident == (size_t)-1 || ident < ref->idents_cap,
                    "ident out of bounds");

            size_t new_ident = ident;
            for (auto subrange : directive | stdv::split('/')) {
                auto strv = std::string_view{subrange};

                size_t id = (size_t)-1;
                if (strv.front() == '.') {
                    auto r = std::from_chars(strv.data(),
                                             strv.data() + strv.length(),
                                             id);
                    if (r.ptr != strv.data() + strv.length()) {
                        log_err("SpinesContext::Group.into(): wrong id format: %.*s",
                                (int)strv.length(), strv.data());
                        return Group{.err = true};
                    }
                }

                size_t s_begin = new_ident != (size_t)-1 ? 1 : 0;
                size_t s_end = new_ident != (size_t)-1 ?
                                    ref->idents[new_ident].parent_len
                                    : ref->idents_cap;
                size_t s_base = new_ident != (size_t)-1 ? new_ident : 0;
                _assert(s_begin <= s_end, "search begin > end");
                _assert(s_base + s_end <= ref->idents_cap,
                        "search_end out of bounds");
                for (size_t i = s_base + s_begin; i < s_base + s_end;) {
                    Ident p = ref->idents[i];
                    if (id != (size_t)-1) {
                        if (p.name_begin == (size_t)-1 && id == p.name_len) {
                            new_ident = i;
                            break;                        }
                    } else {
                        if (p.name_begin != (size_t)-1
                            && strv == std::string_view{ref->ident_names
                                                        + p.name_begin,
                                                        p.name_len}) {
                            new_ident = i;
                            break;
                        }
                    }

                    i += p.parent_len;

                    if (i >= s_base + s_end) {
                        log_err("SpinesContext::Group.into(): not found: \"%.*s\"",
                                (int)strv.length(), strv.data());
                        return Group{.err = true};
                    }
                }
            }
            return Group{err, ref, new_ident};
        }

        [[nodiscard]] Group into(size_t id) const {
            if (err) return Group{.err = true};

            _assert(ref, "ref is null");
            _assert(ident == (size_t)-1 || ident < ref->idents_cap,
                    "ident out of bounds");

            size_t s_begin = ident != (size_t)-1 ? 1 : 0;
            size_t s_end = ident != (size_t)-1 ?
                                ref->idents[ident].parent_len
                                : ref->idents_cap;
            size_t s_base = ident != (size_t)-1 ? ident : 0;
            _assert(s_begin <= s_end, "search begin > end");
            _assert(s_base + s_end <= ref->idents_cap,
                    "search_end out of bounds");
            for (size_t i = s_base + s_begin; i < s_base + s_end;) {
                Ident p = ref->idents[i];
                if (p.name_begin == (size_t)-1 && id == p.name_len) {
                    return Group {err, ref, i};
                }

                i += p.parent_len;
            }

            log_err("SpinesContext::Group.into(): not found .%d", id);
            return Group{.err = true};
        }

        [[nodiscard]] size_t len() const {
            return ident == (size_t)-1 ? ref->fields_cap
                                         : ref->idents[ident].fields_len;
        }

        [[nodiscard]] size_t child_count() const {
            return ident == (size_t)-1 ? ref->idents_cap - 1
                                         : ref->idents[ident].parent_len - 1;
        }


        [[nodiscard]] std::pair<Field, FieldType> get(size_t index) const {
            if (err || !ref) {
                log_err("SpinesContext:Group.get_naked(): Group is error");
                return {Field{}, (FieldType)0};
            }

            size_t base =
                ident == (size_t)-1 ? 0 : ref->idents[ident].fields_begin;
            size_t dest = base + index;
            if (index >= len()) {
                log_warn("SpinesContext:Group.get_naked(): index out of bounds; return first group's field");
                dest = base;
            }
            return {ref->field_vals[dest], ref->field_types[dest]};
        }

        [[nodiscard]] Field get_naked(size_t index) const {
            if (err || !ref) {
                log_err("SpinesContext:Group.get_naked(): Group is error");
                return Field{};
            }

            size_t base =
                ident == (size_t)-1 ? 0 : ref->idents[ident].fields_begin;
            size_t dest = base + index;
            if (index >= len()) {
                log_warn("SpinesContext:Group.get_naked(): index out of bounds; return first group's field");
                dest = base;
            }
            return ref->field_vals[dest];
        }

        [[nodiscard]] const char *get_string(size_t index) const {
            if (err || !ref) {
                log_err("SpinesContext:Group.get_string(): Group is error");
                return nullptr;
            }

            size_t base =
                ident == (size_t)-1 ? 0 : ref->idents[ident].fields_begin;
            size_t dest = base + index;
            if (index >= len()) {
                log_warn("SpinesContext:Group.get_string(): index out of bounds; return first group's field");
                dest = base;
            }
            return &ref->string_data[ref->field_vals[dest].string_val];
        }

        template <typename T> [[nodiscard]] T get_as(size_t index) const {
            if (err || !ref) {
                log_err("SpinesContext:Group.get_as(): Group is error");
                return T{};
            }

            size_t base =
                ident == (size_t)-1 ? 0 : ref->idents[ident].fields_begin;
            size_t dest = base + index;
            if (index >= len()) {
                log_warn("SpinesContext:Group.get_as(): index out of bounds; return first group's field");
                dest = base;
            }

            SpinesContext::FieldType type = ref->field_types[dest];
            Field val = ref->field_vals[dest];
            if constexpr (std::is_arithmetic_v<T>) {
                if (type == FIELD_INT) {
                    return static_cast<T>(val.int_val);
                } else if (type == FIELD_FLOAT) {
                    return static_cast<T>(val.float_val);
                }
            }
            else if constexpr (std::is_constructible_v<T, const char*>) {
                if (type == SpinesContext::FIELD_STRING) {
                    const char *str = &ref->string_data[val.string_val];
                    return T{str};
                }
            }

            log_warn("SpinesContext:Group.get_as(): type missmatched, return default");
            return T{};
        }

        struct iterator {
            using iterator_category = std::random_access_iterator_tag;
            using value_type        = std::pair<Field, FieldType>;
            using difference_type   = std::ptrdiff_t;
            using reference         = value_type;

            const Group* group = nullptr;
            size_t index = 0;

            reference operator*() const { return group->get(index); }

            iterator& operator++() { ++index; return *this; }
            iterator operator++(int) { iterator tmp = *this; ++index; return tmp; }
            iterator& operator--() { --index; return *this; }
            iterator operator--(int) { iterator tmp = *this; --index; return tmp; }

            iterator& operator+=(difference_type n) { index += n; return *this; }
            iterator& operator-=(difference_type n) { index -= n; return *this; }
            friend iterator operator+(iterator it, difference_type n) { it += n; return it; }
            friend iterator operator+(difference_type n, iterator it) { it += n; return it; }
            friend iterator operator-(iterator it, difference_type n) { it -= n; return it; }

            friend difference_type operator-(const iterator& a, const iterator& b) {
                return a.index - b.index;
            }

            bool operator==(const iterator& o) const = default;
            auto operator<=>(const iterator& o) const { return index <=> o.index; }
        };

        [[nodiscard]] iterator begin() const { return iterator{this, 0}; }
        [[nodiscard]] iterator end() const { return iterator{this, len()}; }
    };

    [[nodiscard]] Group group() {
        return Group{false, this, (size_t)-1};
    }
    [[nodiscard]] Group into(std::string_view directive) {
        return group().into(directive);
    }
    [[nodiscard]] Group into(size_t id) {
        return group().into(id);
    }
};
