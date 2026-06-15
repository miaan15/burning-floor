module;

export module dyn_array;

import common;

export template <typename T>
requires std::is_trivially_copyable_v<T>
struct DynArray {
    T *data = nullptr;
    size_t len = 0;
    size_t cap = 0;


};

export template <typename T>
[[nodiscard]] DynArray<T> DynArray_make() noexcept {
}
