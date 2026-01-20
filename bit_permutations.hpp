#ifndef CXX26_BIT_PERMUTATIONS_INCLUDE_GUARD
#define CXX26_BIT_PERMUTATIONS_INCLUDE_GUARD

#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>
#include <version>

// #define CXX26_BIT_PERMUTATIONS_DISABLE_BUILTINS
// #define CXX26_BIT_PERMUTATIONS_DISABLE_ARCH_INTRINSICS

// DETECT GNU COMPILERS AND BUILTINS
// =================================

#ifdef __GNUC__
#define CXX26_BIT_PERMUTATIONS_GNU __GNUC__

#ifndef CXX26_BIT_PERMUTATIONS_DISABLE_BUILTINS
#if CXX26_BIT_PERMUTATIONS_GNU >= 14
// GCC 14 in particular is quite important because it brings generic __builtins
#define CXX26_BIT_PERMUTATIONS_BUILTIN_CLZG
#define CXX26_BIT_PERMUTATIONS_BUILTIN_CTZG
#define CXX26_BIT_PERMUTATIONS_BUILTIN_POPCOUNTG
#endif
#define CXX26_BIT_PERMUTATIONS_BUILTIN_CLZ
#define CXX26_BIT_PERMUTATIONS_BUILTIN_CTZ
#define CXX26_BIT_PERMUTATIONS_BUILTIN_POPCOUNT
#define CXX26_BIT_PERMUTATIONS_BUILTIN_BSWAP
#endif // CXX26_BIT_PERMUTATIONS_DISABLE_BUILTINS

#endif // __GNUC__

// DETECT CLANG COMPILER AND BUILTINS
// ==================================

#ifdef __clang__
#define CXX26_BIT_PERMUTATIONS_CLANG __clang_major__
#define CXX26_BIT_PERMUTATIONS_BITINT

#if CXX26_BIT_PERMUTATIONS_CLANG >= 16
#define CXX_BIT_PERMUTATIONS_BITINT_BEYOND_128
#endif

#ifndef CXX26_BIT_PERMUTATIONS_DISABLE_BUILTINS
#define CXX26_BIT_PERMUTATIONS_BUILTIN_BITREVERSE
#endif // CXX26_BIT_PERMUTATIONS_DISABLE_BUILTINS

#endif // __clang__

// DETECT MICROSOFT COMPILER AND BUILTINS
// ======================================

#ifdef _MSC_VER
#define CXX26_BIT_PERMUTATIONS_MSVC _MSC_VER

#ifndef CXX26_BIT_PERMUTATIONS_DISABLE_BUILTINS
#define CXX26_BIT_PERMUTATIONS_BUILTIN_LZCNT
#define CXX26_BIT_PERMUTATIONS_BUILTIN_BSF
#define CXX26_BIT_PERMUTATIONS_BUILTIN_BYTESWAP
#define CXX26_BIT_PERMUTATIONS_BUILTIN_POPCNT
#endif // CXX26_BIT_PERMUTATIONS_DISABLE_BUILTINS

#endif // _MSC_VER

// DETECT ARCHITECTURE
// ===================

#ifndef CXX26_BIT_PERMUTATIONS_DISABLE_ARCH_INTRINSICS
#if defined(__x86_64__) || defined(_M_X64)
#define CXX26_BIT_PERMUTATIONS_X86_64
#define CXX26_BIT_PERMUTATIONS_X86
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define CXX26_BIT_PERMUTATIONS_X86
#endif

#if defined(_M_ARM) || defined(__arm__)
#define CXX26_BIT_PERMUTATIONS_ARM
#endif

// DETECT INSTRUCTION SET FEATURES
// ===============================

#ifdef __BMI__
#define CXX26_BIT_PERMUTATIONS_X86_BMI
#endif
#ifdef __BMI2__
#define CXX26_BIT_PERMUTATIONS_X86_BMI2
#endif
#ifdef __PCLMUL__
#define CXX26_BIT_PERMUTATIONS_X86_PCLMUL
#endif
#ifdef __POPCNT__
#define CXX26_BIT_PERMUTATIONS_X86_POPCNT
#endif
#ifdef __ARM_FEATURE_SVE2
#define CXX26_BIT_PERMUTATIONS_ARM_SVE2
#endif
#ifdef __ARM_FEATURE_SVE
#define CXX26_BIT_PERMUTATIONS_ARM_SVE
#endif
#ifdef __ARM_FEATURE_SME
#define CXX26_BIT_PERMUTATIONS_ARM_SME
#endif

// DEFINE INSTRUCTION SUPPORT BASED ON INSTRUCTION SET FEATURES
// ============================================================

#if defined(CXX26_BIT_PERMUTATIONS_X86_BMI2)
#define CXX26_BIT_PERMUTATIONS_X86_PDEP
#define CXX26_BIT_PERMUTATIONS_X86_PEXT
#endif

#if defined(__ARM_FEATURE_SVE2)
#define CXX26_BIT_PERMUTATIONS_ARM_BDEP
#define CXX26_BIT_PERMUTATIONS_ARM_BEXT
#define CXX26_BIT_PERMUTATIONS_ARM_BGRP
#endif

#if defined(CXX26_BIT_PERMUTATIONS_ARM_SME) || defined(__ARM_FEATURE_SVE)
#define CXX26_BIT_PERMUTATIONS_ARM_RBIT
#endif

#if defined(CXX26_BIT_PERMUTATIONS_ARM_SME) || defined(__ARM_FEATURE_SVE)
// support for 64-bit PMUL is optional, so we will
// only use up to the 32-bit variant of this
#define CXX26_BIT_PERMUTATIONS_ARM_PMUL
#endif

// DEFINE WHICH FUNCTIONS HAVE "FAST" SUPPORT

#if defined(CXX26_BIT_PERMUTATIONS_ARM_RBIT)
#define CXX26_BIT_PERMUTATIONS_FAST_REVERSE
#endif

#if defined(CXX26_BIT_PERMUTATIONS_X86_PEXT) || defined(CXX26_BIT_PERMUTATIONS_ARM_BEXT)
#define CXX26_BIT_PERMUTATIONS_FAST_COMPRESS
#endif

#if defined(CXX26_BIT_PERMUTATIONS_X86_PDEP) || defined(CXX26_BIT_PERMUTATIONS_ARM_BDEP)
#define CXX26_BIT_PERMUTATIONS_FAST_EXPAND
#endif

#if defined(CXX26_BIT_PERMUTATIONS_X86_POPCNT) || defined(CXX26_BIT_PERMUTATIONS_X86_BMI2)         \
    || defined(CXX26_BIT_PERMUTATIONS_ARM_SVE)
#define CXX26_BIT_PERMUTATIONS_FAST_POPCOUNT
#endif

// ARCHITECTURE INTRINSIC INCLUDES
// ===============================

#ifdef CXX26_BIT_PERMUTATIONS_X86
#include <immintrin.h>
#endif
#ifdef CXX26_BIT_PERMUTATIONS_ARM_RBIT
#include <arm_acle.h>
#endif
#ifdef CXX26_BIT_PERMUTATIONS_ARM_PMUL
#include <arm_neon.h>
#endif
#ifdef CXX26_BIT_PERMUTATIONS_ARM_SVE2
#include <arm_sve.h>
#endif
#endif

// COMPILER-SPECIFIC FEATURES
// ==========================

#ifdef CXX26_BIT_PERMUTATIONS_GNU
#define CXX26_BIT_PERMUTATIONS_ALWAYS_INLINE [[gnu::always_inline]]
#define CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL _Pragma("GCC unroll 16")

#elif defined(CXX26_BIT_PERMUTATIONS_MSVC)
#define CXX26_BIT_PERMUTATIONS_ALWAYS_INLINE __forceinline
#define CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL

#else
#define CXX26_BIT_PERMUTATIONS_ALWAYS_INLINE inline
#define CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
#endif

// COMPILER-AGNOSTICS uint128_t
// ============================

namespace cxx26bp::detail {

template <typename T>
inline constexpr int width_v = std::numeric_limits<std::make_unsigned_t<T>>::digits;

template <typename T>
inline constexpr int digits_v = std::numeric_limits<T>::digits;

// __int128 ADDITIONAL SUPPORT
// ===========================

#ifdef CXX26_BIT_PERMUTATIONS_GNU
#define CXX26_BIT_PERMUTATIONS_U128
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
using int128_t = __int128;
using uint128_t = unsigned __int128;
#pragma GCC diagnostic pop
#else
struct int128_t;
struct uint128_t;
#endif // CXX26_BIT_PERMUTATIONS_GNU

template <>
inline constexpr int digits_v<int128_t> = 127;
template <>
inline constexpr int digits_v<uint128_t> = 128;

template <>
inline constexpr int width_v<int128_t> = 128;
template <>
inline constexpr int width_v<uint128_t> = 128;

// _BitInt ADDITIONAL SUPPORT
// ==========================

template <typename T>
struct is_bit_int : std::false_type { };

#ifdef CXX26_BIT_PERMUTATIONS_BITINT
#ifndef __INTELLISENSE__ // IntelliSense support for _BitInt is broken

template <int N>
struct is_bit_int<signed _BitInt(N)> : std::true_type { };
template <int N>
struct is_bit_int<unsigned _BitInt(N)> : std::true_type { };

static_assert(is_bit_int<signed _BitInt(2)>::value);
static_assert(is_bit_int<unsigned _BitInt(8)>::value);

template <int N>
inline constexpr int digits_v<_BitInt(N)> = N - 1;
template <int N>
inline constexpr int digits_v<unsigned _BitInt(N)> = N;

static_assert(digits_v<_BitInt(128)> == 127);
static_assert(digits_v<unsigned _BitInt(128)> == 128);

template <int N>
inline constexpr int width_v<_BitInt(N)> = N;
template <int N>
inline constexpr int width_v<unsigned _BitInt(N)> = N;

static_assert(width_v<_BitInt(128)> == 128);
static_assert(width_v<unsigned _BitInt(128)> == 128);
#endif // __INTELLISENSE__
#endif // CXX26_BIT_PERMUTATIONS_BITINT

} // namespace cxx26bp::detail

// C++ VERSION-SPECIFIC FEATURES
// =============================

#ifdef __cpp_if_consteval
#define CXX26_BIT_PERMUTATIONS_CONSTANT_EVALUATED consteval
#define CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED !consteval
#else
// all vendors use the same builtin
#define CXX26_BIT_PERMUTATIONS_CONSTANT_EVALUATED (__builtin_is_constant_evaluated())
#define CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED (!__builtin_is_constant_evaluated())
#endif

// =================================================================================================
// =================================================================================================
// =================================================================================================
// END OF CONFIG === END OF CONFIG === END OF CONFIG === END OF CONFIG === END OF CONFIG === END OF
// =================================================================================================
// =================================================================================================
// =================================================================================================

namespace cxx26bp {

namespace detail {

/// @brief Returns `true` if x is a power of two or zero.
[[nodiscard]] constexpr int is_pow2_or_zero(int x) noexcept
{
    return (x & (x - 1)) == 0;
}

/// Computes `floor(log2(max(1, x)))` of an  integer `x`.
/// If x is zero or negative, returns zero.
[[nodiscard]] constexpr int log2_floor(int x) noexcept
{
    return x < 1 ? 0 : digits_v<unsigned> - std::countl_zero(static_cast<unsigned>(x)) - 1;
}

/// Computes `ceil(log2(max(1, x)))` of an integer `x`.
/// If `x` is zero or negative, returns zero.
[[nodiscard]] constexpr int log2_ceil(int x) noexcept
{
    return log2_floor(x) + !is_pow2_or_zero(x);
}

template <typename T>
concept standard_integer //
    = std::same_as<T, signed char> || std::same_as<T, unsigned char> //
    || std::same_as<T, short> || std::same_as<T, unsigned short> //
    || std::same_as<T, int> || std::same_as<T, unsigned int> //
    || std::same_as<T, long> || std::same_as<T, unsigned long> //
    || std::same_as<T, long long> || std::same_as<T, unsigned long long>;

template <typename T>
concept extended_integer = std::same_as<T, int128_t> || std::same_as<T, uint128_t>;

template <typename T>
concept bit_precise_integer = is_bit_int<T>::value;

template <typename T>
concept signed_or_unsigned_integer
    = standard_integer<T> || bit_precise_integer<T> || extended_integer<T>;

// We cannot use `is_unsigned_v` because that also requires `T`
// to be an arithmetic type,
// which `__int128` isn't for GCC.

template <typename T>
concept unsigned_integer = signed_or_unsigned_integer<T> && !std::is_signed_v<T>;

} // namespace detail

/// @brief Saturating left-shift.
/// This is necessary because left-shifting by the operand size or more is undefined behavior.
/// @tparam T the type
/// @param x the value to shift
/// @param s the shift amount
/// @return `x << s` if `s < N`, `0` otherwise.
/// If `s` is negative, shifts in the opposite direction.
template <detail::signed_or_unsigned_integer T, detail::signed_or_unsigned_integer S>
[[nodiscard]] constexpr T shl(T x, S s) noexcept
{
    constexpr auto width = static_cast<S>(detail::width_v<T>);
    if constexpr (std::is_signed_v<S>) {
        if (s < 0) {
            auto fill = static_cast<T>(x < 0 ? -1 : 0);
            return s <= -width ? fill : x >> -s;
        }
    }
    return s >= width ? static_cast<T>(0) : x << s;
}

/// @brief Equivalent to `shl(x, -s)`,
/// but doesn't overflow on `-s`.
template <detail::signed_or_unsigned_integer T, detail::signed_or_unsigned_integer S>
[[nodiscard]] constexpr T shr(T x, S s) noexcept
{
    constexpr auto width = static_cast<S>(detail::width_v<T>);
    if constexpr (std::is_signed_v<S>) {
        if (s < 0) {
            return s <= -width ? static_cast<T>(0) : x << -s;
        }
    }
    const auto fill = static_cast<T>(x < 0 ? -1 : 0);
    return s >= width ? fill : x >> s;
}

/// @brief Repeats a bit pattern.
/// @param x the bit-pattern, stored in the lest significant `length` bits.
/// @param length the length of the bit-pattern, in range [1, inf)
/// @return The bit pattern in `x`, repeated as many times as representable by `T`.
/// @throws Nothing.
template <detail::unsigned_integer T>
[[nodiscard]] constexpr T bit_repeat(T x, int length)
{
    constexpr int N = detail::digits_v<T>;
    constexpr T one = 1;

    if CXX26_BIT_PERMUTATIONS_CONSTANT_EVALUATED {
        if (length <= 0) {
            throw "length must be greater than zero";
        }
    }

    // Clear undesirable bits which are not part of the pattern.
    // For length == N, this does nothing.
    x &= shl(one, length) - 1;

    CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
    for (int i = length; i < N; i <<= 1) {
        x |= x << i;
    }

    return x;
}

namespace detail {

/// @brief Returns the high bits of the left-shift `x << s`.
template <unsigned_integer T>
[[nodiscard]] constexpr T shl_high(T x, int s)
{
    constexpr int N = detail::digits_v<T>;
    return (s < N) ? static_cast<T>(0) : x >> (N - s);
}

} // namespace detail

template <typename T>
struct wide_result {
    T low_bits;
    T high_bits;

    friend constexpr bool operator==(const wide_result&, const wide_result&) = default;
};

template <detail::unsigned_integer T>
[[nodiscard]] constexpr wide_result<T> clmul_wide(T x, T y)
{
    constexpr int N = detail::digits_v<T>;
    constexpr int N_native = detail::digits_v<std::size_t>;

#ifdef CXX26_BIT_PERMUTATIONS_X86_PCLMUL
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if constexpr (N <= 64) {
            const __m128i x_128 = _mm_set_epi64x(0, x);
            const __m128i neg1_128 = _mm_set_epi64x(0, -1);
            const __m128i result_128 = _mm_clmulepi64_si128(x_128, neg1_128, 0);
            const auto lo64 = static_cast<std::uint64_t>(_mm_extract_epi64(result_128, 0));
            const auto hi64 = static_cast<std::uint64_t>(_mm_extract_epi64(result_128, 1));
            const auto high = static_cast<T>(shr(lo64, N)) | static_cast<T>(shl(hi64, 64 - N));
            return {
                .low_bits = static_cast<T>(lo64),
                .high_bits = high,
            };
        }
        // TODO: Rather than falling back onto a completely naive implementation,
        //       it would make sense to do a digit-by-digit clmul here.
        //       This still involves way fewer operations
        //       than doing it entirely in software.
    }
#endif
    if constexpr (2 * N <= N_native) {
        constexpr std::size_t one = 1;
        std::size_t result = 0;
        for (int i = 0; i < N; ++i) {
            result ^= static_cast<std::size_t>(x) * (static_cast<std::size_t>(y) & (one << i));
        }
        return {
            .low_bits = static_cast<T>(result),
            .high_bits = static_cast<T>(result >> N),
        };
    }
    else {
        constexpr T one = 1;
        T low = x * (y & one);
        T high = 0;
        for (int i = 1; i < N; ++i) {
            if ((y >> i) & one) {
                low ^= x << i;
                high ^= x >> (N - i);
            }
        }
        return { .low_bits = low, .high_bits = high };
    }
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T clmul(T x, T y)
{
    constexpr int N = detail::digits_v<T>;
    constexpr int N_native = detail::digits_v<std::size_t>;

    // Hardware support for clmul always comes in a form
    // where both low and high bits are computed.
    // For now, we can just put all the builtin support into clmul_wide,
    // and hope that discarding the high bits optimizes nicely.
#ifdef CXX26_BIT_PERMUTATIONS_X86_PCLMUL
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        return clmul_wide(x, y).low_bits;
    }
#endif

    T result = 0;
    for (int i = 0; i < N; ++i) {
        if constexpr (N <= N_native) {
            // For lower widths, we assume that fast multiplication is available.
            // This software implementation is better because the entire
            // right-hand side of the ^= can be parallelized.
            result ^= x * (y & (T { 1 } << i));
        }
        else {
            // For greater widths, multi-digit multiplication would be very expensive,
            // so we do a bit test (which is O(1) no matter the size),
            // followed by a conditional XOR and shift,
            // both of which are relatively cheap even for large N.
            if ((y >> i) & 1) {
                result ^= x << i;
            }
        }
    }
    return result;
}

namespace detail {

/// @brief Creates a number with alternating groups of 0s and 1s.
/// For example, `alternate01<uint8_t>(1, 2) -> 0b01001001
template <unsigned_integer T>
[[nodiscard]] CXX26_BIT_PERMUTATIONS_ALWAYS_INLINE constexpr T alternate01(int zero_size,
                                                                           int one_size)
{
    constexpr int N = digits_v<T>;
    const int pattern_length = zero_size + one_size;

    if CXX26_BIT_PERMUTATIONS_CONSTANT_EVALUATED {
        if (one_size < 0 || one_size > N) {
            throw "one_size must be in range [0, N]";
        }
        if (zero_size < 0 || zero_size > N) {
            throw "zero_size must be in range [0, N]";
        }
        if (pattern_length == 0) {
            throw "alternate01(0, 0) is undefined behavior";
        }
    }

    const T ones = shl(T { 1 }, one_size) - 1;
    return bit_repeat(ones, pattern_length);
}

/// Precomputed table where `alternating_bit_mask_table[i]` equals
/// `alternate01<T>((1 << i), (1 << i))`.
/// This table has size `log2_ceil(N) + 1`, where the last element is always a mask with every bit
/// set.
/// This may simplify some code compared to using `alternate01` as above directly, because direct
/// use would involve a shift which is undefined behavior.
template <typename T>
inline constexpr auto alternating_bit_mask_table = [] {
    constexpr int log_N = log2_ceil(digits_v<T>);
    // + 1 so that we also get a mask which covers the whole operand.
    std::array<T, log_N + 1> result;
    for (int i = 0; i <= log_N; ++i) {
        result[static_cast<std::size_t>(i)]
            = i == log_N ? static_cast<T>(-1) : alternate01<T>((1 << i), (1 << i));
    }
    return result;
}();

/// @brief If byte-reversal for `T` is supported, returns `x` with its byte order reversed,
/// and the return type deduces to `T`.
/// Otherwise, return `void`.
template <unsigned_integer T>
[[nodiscard]] auto optional_byteswap([[maybe_unused]] T x) noexcept
{
    [[maybe_unused]] constexpr int N = digits_v<T>;

#ifdef CXX26_BIT_PERMUTATIONS_BUILTIN_BSWAP
    if constexpr (N == 16) {
        return static_cast<T>(__builtin_bswap16(x));
    }
    else if constexpr (N == 32) {
        return static_cast<T>(__builtin_bswap32(x));
    }
    else if constexpr (N == 64) {
        return static_cast<T>(__builtin_bswap64(x));
    }
#elif defined(CXX26_BIT_PERMUTATIONS_BUILTIN_BYTESWAP)
    if constexpr (N == digits_v<unsigned short>) {
        return static_cast<T>(_byteswap_ushort(x));
    }
    else if constexpr (N == digits_v<unsigned long>) {
        return static_cast<T>(_byteswap_ulong(x));
    }
    else if constexpr (N == 64) {
        return static_cast<T>(_byteswap_uint64(x));
    }
#else
    return void();
#endif
}

/// Each bit in `x` is converted to the parity a bit and all bits to its right.
/// This can also be expressed as `CLMUL(x, -1)` where `CLMUL` is a carry-less
/// multiplication.
template <unsigned_integer T>
[[nodiscard]] constexpr T bitwise_inclusive_right_parity(T x) noexcept
{
    constexpr int N = digits_v<T>;

#ifdef CXX26_BIT_PERMUTATIONS_X86_PCLMUL
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if constexpr (N <= 64) {
            const __m128i x_128 = _mm_set_epi64x(0, x);
            const __m128i neg1_128 = _mm_set_epi64x(0, -1);
            const __m128i result_128 = _mm_clmulepi64_si128(x_128, neg1_128, 0);
            return static_cast<T>(_mm_extract_epi64(result_128, 0));
        }
    }
#endif
    // TODO: Technically, ARM does have some support for polynomial multiplication prior to
    // SVE2.
    //       However, this support is fairly limited and it's not even clear whether it
    //       beats this implementation.
    for (int i = 1; i < N; i <<= 1) {
        x ^= x << i;
    }
    return x;
}

template <int N, unsigned_integer T>
[[nodiscard]] constexpr T bit_reverse_impl(T x) noexcept
{
    constexpr int N_actual = digits_v<T>;
    static_assert(N <= N_actual);

#ifdef CXX26_BIT_PERMUTATIONS_BUILTIN_BITREVERSE
    if constexpr (N <= 8) {
        return static_cast<T>(__builtin_bitreverse8(x) >> (8 - N));
    }
    else if constexpr (N <= 16) {
        return static_cast<T>(__builtin_bitreverse16(x) >> (16 - N));
    }
    else if constexpr (N <= 32) {
        return static_cast<T>(__builtin_bitreverse32(x) >> (32 - N));
    }
    else if constexpr (N <= 64) {
        return static_cast<T>(__builtin_bitreverse64(x) >> (64 - N));
    }
#elif defined(CXX26_BIT_PERMUTATIONS_ARM_RBIT)
    constexpr int N_ull = digits_v<unsigned long long>;
    if constexpr (N <= N_ull) {
        constexpr int N_u = digits_v<unsigned>;
        constexpr int N_ul = digits_v<unsigned long>;
        if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
            if constexpr (N <= N_u) {
                return static_cast<T>(__rbit(x) >> (N_u - N));
            }
            else if constexpr (N <= N_ul) {
                return static_cast<T>(__rbitl(x) >> (N_ul - N));
            }
            else if constexpr (N <= N_ull) {
                return static_cast<T>(__rbitll(x) >> (N_ull - N));
            }
        }
    }
#endif
    if constexpr (constexpr int N_native = digits_v<std::size_t>; N > N_native) {
        // N, rounded up to the next multiple of N_native.
        constexpr int N_ceil = N_native * (N / N_native + (N % N_native != 0));
        static_assert(N_ceil >= N);
        constexpr int shift = N_ceil - N;

        T most = 0;
        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int i = 0; i + N_native < N; i += N_native) {
            most <<= N_native;
            most |= bit_reverse_impl<N_native>(static_cast<std::size_t>(x));
            x >>= N_native;
        }
        const T last = bit_reverse_impl<N_native>(static_cast<std::size_t>(x));

        return (most << (N_native - shift)) | (last >> shift);
    }
    else if constexpr (is_pow2_or_zero(N)) {
        // Byte-swap and parallel swap technique for conventional architectures.
        // O(log N)
        constexpr int byte_bits = digits_v<unsigned char>;
        int start_pow = detail::log2_floor(N);

        // If byteswap does what we want, we can skip a few iterations of the subsequent loop.
        if constexpr (N >= byte_bits) {
            // Nested constexpr if avoids unnecessary instantiation of optional_byteswap.
            if constexpr (!std::is_same_v<decltype(optional_byteswap(x)), void>) {
                if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
                    x = optional_byteswap(x) >> (N_actual - N);
                    start_pow = detail::log2_floor(byte_bits);
                }
            }
        }

        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int i = start_pow - 1; i >= 0; --i) {
            const T lo = alternating_bit_mask_table<T>[i];
            x = ((x & ~lo) >> (1 << i)) | ((x & lo) << (1 << i));
        }

        return x;
    }
    else {
        //         [hi] [lo]
        // input:  -654 3210
        // rev:    456- 0123
        // return: -0123 456

        constexpr int M = std::bit_floor<unsigned>(N);
        constexpr int shift = (M * 2) - N;
        static_assert(M < N);
        static_assert(M > shift);
        constexpr T lo_mask = (T { 1 } << M) - 1;

        const T lo = bit_reverse_impl<M>(x & lo_mask);
        const T hi = bit_reverse_impl<M>(x >> M);

        const T result = (lo << (M - shift)) | (hi >> shift);

        return result;
    }
}

} // namespace detail

// `std::popcount` does not accept _BitInt or other extensions, so we make our own.
template <detail::unsigned_integer T>
[[nodiscard]] constexpr int popcount(T x) noexcept
{
#ifdef CXX26_BIT_PERMUTATIONS_BUILTIN_POPCOUNTG
    return __builtin_popcountg(x);
#else
    constexpr int N = detail::digits_v<T>;

#ifdef CXX26_BIT_PERMUTATIONS_BUILTIN_POPCOUNT
    if constexpr (N <= detail::digits_v<unsigned>) {
        return __builtin_popcount(x);
    }
    else if constexpr (N <= detail::digits_v<unsigned long>) {
        return __builtin_popcountl(x);
    }
    else if constexpr (N <= detail::digits_v<unsigned long long>) {
        return __builtin_popcountll(x);
    }
#elif defined(CXX26_BIT_PERMUTATIONS_BUILTIN_POPCNT)
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if constexpr (N <= detail::digits_v<unsigned short>) {
            return static_cast<int>(__popcnt16(x));
        }
        else if constexpr (N <= detail::digits_v<unsigned int>) {
            return static_cast<int>(__popcnt(x));
        }
        else if constexpr (N <= 64) {
            return static_cast<int>(__popcnt64(x));
        }
    }
#endif
    constexpr int N_native = detail::digits_v<std::size_t>;
    if constexpr (N > N_native) {
        int sum = 0;
        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int i = 0; i < N; i += N_native) {
            sum += popcount(static_cast<std::size_t>(x));
            x >>= N_native;
        }
        return sum;
    }
    else if constexpr (N == 1) {
        return x;
    }
    else if constexpr (N == 2) {
        return (x >> 1) + (x & 1);
    }
    else if constexpr (N == 3) {
        return (x >> 2) + ((x >> 1) & 1) + (x & 1);
    }
    else {
        constexpr int log_N = detail::log2_ceil(N);

        constexpr auto mask1 = detail::alternating_bit_mask_table<T>[0];
        constexpr auto mask2 = detail::alternating_bit_mask_table<T>[1];

        T result = x - ((x >> 1) & mask1);
        result = ((result >> 2) & mask2) + (result & mask2);

        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int i = 2; i < log_N; ++i) {
            const auto mask = detail::alternating_bit_mask_table<T>[i];
            result = ((result >> (1 << i)) + result) & mask;
        }
        return result;
    }
#endif // CXX26_BIT_PERMUTATIONS_BUILTIN_POPCOUNTG
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr int countr_zero(T x) noexcept
{
    constexpr int N = detail::digits_v<T>;

#ifdef CXX26_BIT_PERMUTATIONS_BUILTIN_CTZG
    return __builtin_ctzg(x, N);
#else
#ifdef CXX26_BIT_PERMUTATIONS_BUILTIN_CTZ
    constexpr int N_ull = detail::digits_v<unsigned long long>;
    if constexpr (N <= N_ull) {
        if (x == 0) {
            return N;
        }
        if constexpr (N <= detail::digits_v<unsigned>) {
            constexpr auto sentinel = (1u << (N - 1) << 1);
            return __builtin_ctz(x | sentinel);
        }
        else if constexpr (N <= detail::digits_v<unsigned long>) {
            constexpr auto sentinel = (1ul << (N - 1) << 1);
            return __builtin_ctzl(x | sentinel);
        }
        else if constexpr (N <= detail::digits_v<unsigned long long>) {
            constexpr auto sentinel = (1ull << (N - 1) << 1);
            return __builtin_ctzll(x | sentinel);
        }
    }
#elif defined(CXX26_BIT_PERMUTATIONS_X86_BMI)
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if constexpr (N <= 16) {
            constexpr auto sentinel = static_cast<unsigned short>(1u << (N - 1) << 1);
            return _tzcnt_u16(x | sentinel);
        }
        else if constexpr (N <= 32) {
            constexpr auto sentinel = 1u << (N - 1) << 1;
            return _tzcnt_u32(x | sentinel);
        }
        else if constexpr (N <= 64) {
            constexpr auto sentinel = 1ull << (N - 1) << 1;
            return _tzcnt_u64(x | sentinel);
        }
    }
#elif defined(CXX26_BIT_PERMUTATIONS_BUILTIN_BSF)
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if constexpr (N <= 32) {
            constexpr unsigned long sentinel = (1ul << (N - 1) << 1);
            unsigned long index;
            return _BitScanForward(&index, static_cast<unsigned long>(x) | sentinel)
                ? static_cast<int>(index)
                : N;
        }
        else if constexpr (N <= 64) {
            constexpr unsigned __int64 sentinel = (1ull << (N - 1) << 1);
            unsigned long index;
            return _BitScanForward64(&index, static_cast<__int64>(x) | sentinel)
                ? static_cast<int>(index)
                : N;
        }
    }
#endif
    constexpr int N_nat = detail::digits_v<std::size_t>;
    if constexpr (N > N_nat) {
        // Handle everything except for the most significant digit
        int result = 0;
        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int i = 0; i + N_nat < N; i += N_nat) {
            const int part = countr_zero(static_cast<std::size_t>(x));
            result += part;
            if (part != N_nat) {
                return result;
            }
            x >>= N_nat;
        }
        // Handle the most significant digit.
        constexpr auto sentinel
            = static_cast<std::size_t>(N % N_nat == 0 ? 0 : std::size_t { 1 } << (N % N_nat));
        return result
            + countr_zero(static_cast<std::size_t>(static_cast<std::size_t>(x) | sentinel));
    }
    // https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel
    else {
        constexpr int M = std::bit_ceil<unsigned>(N);
        constexpr int log_M = detail::log2_floor(M);
        int result = M;
        x &= -x; // isolate the lowest 1-bit
        result -= (x != 0);
        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int i = log_M - 1; i >= 0; --i) {
            const T mask = detail::alternating_bit_mask_table<T>[i];
            result -= ((x & mask) != 0) * (1 << i);
        }
        if constexpr (N == M) {
            return result;
        }
        else {
            return N < result ? N : result;
        }
    }
#endif // CXX26_BIT_PERMUTATIONS_BUILTIN_CTZG
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr int countr_one(T x) noexcept
{
    return countr_zero(static_cast<T>(~x));
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr int countl_zero(T x) noexcept
{
    constexpr int N = detail::digits_v<T>;

#ifdef CXX26_BIT_PERMUTATIONS_BUILTIN_CLZG
    return __builtin_clzg(x, N);
#else
#ifdef CXX26_BIT_PERMUTATIONS_BUILTIN_CLZ
    if (x == 0) {
        return N;
    }
    if constexpr (N <= detail::digits_v<unsigned>) {
        return __builtin_clz(x) - (detail::digits_v<unsigned> - N);
    }
    else if constexpr (N <= detail::digits_v<unsigned long>) {
        return __builtin_clzl(x) - (detail::digits_v<unsigned long> - N);
    }
    else if constexpr (N <= detail::digits_v<unsigned long long>) {
        return __builtin_clzll(x) - (detail::digits_v<unsigned long long> - N);
    }
#elif defined(CXX26_BIT_PERMUTATIONS_BUILTIN_LZCNT)
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if (x == 0) {
            return N;
        }
        if constexpr (N <= 16) {
            return static_cast<int>(__lzcnt16(x)) - (16 - N);
        }
        else if constexpr (N <= 32) {
            return static_cast<int>(__lzcnt(x)) - (32 - N);
        }
        else if constexpr (N <= 64) {
            return static_cast<int>(__lzcnt64(x)) - (64 - N);
        }
    }
#endif // CXX26_BIT_PERMUTATIONS_MSVC

    // TODO: ARM intrinsics

    // TODO: digit-by-digit loop for large sizes
    if (x == 0) {
        return N;
    }
    constexpr int start = std::bit_ceil<unsigned>(N);
    auto mask = static_cast<T>(-1);
    int result = 0;
    CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
    for (int i = start >> 1; i != 0; i >>= 1) {
        if (x & (mask << i)) {
            mask <<= i;
            result += i;
        }
    }
    return N - result - 1;
#endif // CXX26_BIT_PERMUTATIONS_BUILTIN_CLZG
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr int countl_one(T x) noexcept
{
    return countl_zero(static_cast<T>(~x));
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T bit_reverse(T x) noexcept
{
    constexpr int N = detail::digits_v<T>;
    return detail::bit_reverse_impl<N>(x);
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T next_bit_permutation(T x) noexcept
{
    // https://graphics.stanford.edu/~seander/bithacks.html#NextBitPermutation
    constexpr T one = 1;
    const T t = x | (x - one);
    if (t == static_cast<T>(-1)) {
        return 0;
    }
    // Two shifts are better than shifting by + 1. We must not shift by the operand size.
    return (t + one) | (((~t & -~t) - one) >> countr_zero(x) >> one);
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T prev_bit_permutation(T x) noexcept
{
    constexpr T one = 1;
    const T trailing_ones_cleared = x & (x + one);
    if (trailing_ones_cleared == 0) {
        return 0;
    }
    const T trailing_ones = x ^ trailing_ones_cleared;
    const int shift = countr_zero(trailing_ones_cleared) - countr_one(trailing_ones) - 1;

    return static_cast<T>(trailing_ones_cleared - one) >> shift << shift;
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T bit_compressr(T x, T m) noexcept
{
    constexpr int N = detail::digits_v<T>;

#ifdef CXX26_BIT_PERMUTATIONS_X86_PEXT
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if constexpr (N <= 32) {
            return static_cast<T>(_pext_u32(x, m));
        }
        else if constexpr (N <= 64) {
            return static_cast<T>(_pext_u64(x, m));
        }
    }
#endif

#ifdef CXX26_BIT_PERMUTATIONS_ARM_BEXT
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if constexpr (N <= 8) {
            auto sv_result = svbext_u8(svdup_u8(x), svdup_u8(m));
            return static_cast<T>(svorv_u8(svptrue_b8(), sv_result));
        }
        else if constexpr (N <= 16) {
            auto sv_result = svbext_u16(svdup_u16(x), svdup_u16(m));
            return static_cast<T>(svorv_u16(svptrue_b16(), sv_result));
        }
        else if constexpr (N <= 32) {
            auto sv_result = svbext_u32(svdup_u32(x), svdup_u32(m));
            return static_cast<T>(svorv_u32(svptrue_b32(), sv_result));
        }
        else if constexpr (N <= 64) {
            auto sv_result = svbext_u64(svdup_u64(x), svdup_u64(m));
            return static_cast<T>(svorv_u64(svptrue_b64(), sv_result));
        }
    }
#endif
    constexpr int N_native = detail::digits_v<std::size_t>;
    if constexpr (N > N_native) {
        // For integer sizes above the native size, we assume that a fast native implementation
        // is provided. We then perform the algorithm digit by digit, where a digit is a native
        // integer.
        T result = 0;
        int offset = 0;
        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int mask_bits = 0; mask_bits < N; mask_bits += N_native) {
            const auto compressed
                = bit_compressr(static_cast<std::size_t>(x), static_cast<std::size_t>(m));
            result |= static_cast<T>(compressed) << offset;
            offset += popcount(static_cast<std::size_t>(m));
            x >>= N_native;
            m >>= N_native;
        }

        return result;
    }
    else {
        x &= m;
        T mk = ~m << 1;

        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int i = 1; i < N; i <<= 1) {
            const T mk_parity = detail::bitwise_inclusive_right_parity(mk);

            const T move = mk_parity & m;
            m = (m ^ move) | (move >> i);

            const T t = x & move;
            x = (x ^ t) | (t >> i);

            mk &= ~mk_parity;
        }
        return x;
    }
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T bit_expandr(T x, T m) noexcept
{
    constexpr int N = detail::digits_v<T>;
    constexpr int log_N = detail::log2_floor(std::bit_ceil<unsigned>(N));

#ifdef CXX26_BIT_PERMUTATIONS_X86_PDEP
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if constexpr (N <= 32) {
            return _pdep_u32(x, m);
        }
        else if constexpr (N <= 64) {
            return _pdep_u64(x, m);
        }
    }
#endif

#ifdef CXX26_BIT_PERMUTATIONS_ARM_BDEP
    if CXX26_BIT_PERMUTATIONS_NOT_CONSTANT_EVALUATED {
        if constexpr (N <= 8) {
            auto sv_result = svbdep_u8(svdup_u8(x), svdup_u8(m));
            return static_cast<T>(svorv_u8(svptrue_b8(), sv_result));
        }
        else if constexpr (N <= 16) {
            auto sv_result = svbdep_u16(svdup_u16(x), svdup_u16(m));
            return static_cast<T>(svorv_u16(svptrue_b16(), sv_result));
        }
        else if constexpr (N <= 32) {
            auto sv_result = svbdep_u32(svdup_u32(x), svdup_u32(m));
            return static_cast<T>(svorv_u32(svptrue_b32(), sv_result));
        }
        else if constexpr (N <= 64) {
            auto sv_result = svbdep_u64(svdup_u64(x), svdup_u64(m));
            return static_cast<T>(svorv_u64(svptrue_b64(), sv_result));
        }
    }
#endif
    constexpr int N_native = detail::digits_v<std::size_t>;
    if constexpr (N > N_native) {
        // Digit-by-digit approach, same as in bit_expandr.
        T result = 0;
        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int mask_bits = 0; mask_bits < N; mask_bits += N_native) {
            const auto expanded
                = bit_expandr(static_cast<std::size_t>(x), static_cast<std::size_t>(m));
            result |= static_cast<T>(expanded) << mask_bits;
            x >>= popcount(static_cast<std::size_t>(m));
            m >>= N_native;
        }

        return result;
    }
    else {
        const T initial_m = m;

        T array[log_N];
        T mk = ~m << 1;

        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int i = 0; i < log_N; ++i) {
            const T mk_parity = detail::bitwise_inclusive_right_parity(mk);
            const T move = mk_parity & m;
            m = (m ^ move) | (move >> (1 << i));
            array[i] = move;
            mk &= ~mk_parity;
        }

        CXX26_BIT_PERMUTATIONS_AGGRESSIVE_UNROLL
        for (int i = log_N; i > 0;) {
            --i; // Normally, I would write (i-- > 0), but this triggers
                 // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=113581
            const T move = array[i];
            const T t = x << (1 << i);
            x = (x & ~move) | (t & move);
        }

        return x & initial_m;
    }
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T bit_compressl(T x, T m) noexcept
{
    constexpr int N = detail::digits_v<T>;

#if defined(CXX26_BIT_PERMUTATIONS_FAST_REVERSE) && !defined(CXX26_BIT_PERMUTATIONS_FAST_POPCOUNT)
    return bit_reverse(bit_compressr(bit_reverse(x)), bit_reverse(m)));
#else
    if (m == 0) { // Prevents shift which is >= the operand size.
        return 0;
    }
    int shift = N - popcount(m);
    return static_cast<T>(bit_compressr(x, m) << shift);
#endif
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T bit_expandl(T x, T m) noexcept
{
    constexpr int N = detail::digits_v<T>;

#if defined(CXX26_BIT_PERMUTATIONS_FAST_REVERSE) && !defined(CXX26_BIT_PERMUTATIONS_FAST_POPCOUNT)
    return bit_reverse(bit_expandr(bit_reverse(x)), bit_reverse(m)));
#else
    if (m == 0) {
        return 0;
    }
    const int shift = N - popcount(m);
    return bit_expandr(static_cast<T>(x >> shift), m);
#endif
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T bit_compress(T x, T m) noexcept
{
    return bit_compressr(x, m);
}

template <detail::unsigned_integer T>
[[nodiscard]] constexpr T bit_expand(T x, T m) noexcept
{
    return bit_expandr(x, m);
}

} // namespace cxx26bp

#endif
