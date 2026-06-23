module;

export module common;

export import assert;

export import std;
export import glm;
export import sdl;
export import box2d;
export import log;

export namespace stdf = std::filesystem;
export namespace stdr = std::ranges;
export namespace stdv = std::views;

export namespace _ {}
export using namespace _;

export using namespace glm;

export using size_t = std::size_t;

export using i8  = std::int8_t;
export using i16 = std::int16_t;
export using i32 = std::int32_t;
export using i64 = std::int64_t;
export using iptr = std::intptr_t;

export using u8  = std::uint8_t;
export using u16 = std::uint16_t;
export using u32 = std::uint32_t;
export using u64 = std::uint64_t;
export using uptr = std::uintptr_t;

export const stdf::path root_path = stdf::path{__FILE__}.parent_path() / "..";
export const stdf::path src_path = root_path / "src";
export const stdf::path asset_path = root_path / "asset";
