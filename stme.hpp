#pragma once

#include <array>
#include <cstddef>
#include <concepts>
#include <functional>
#include "utils.hpp"

#define AVOID_VECTOR_ASSIGN_CONSTEXPR 1
#ifdef AVOID_VECTOR_ASSIGN_CONSTEXPR
#define CONSTEXPR_MAYBE
#else
#define CONSTEXPR_MAYBE constexpr
#endif

namespace Shak {
    using namespace reachability;

    template<typename Generator, std::size_t N>
    concept my_epic_generator = []<std::size_t... Indices>(std::index_sequence<Indices...>) {
        /* Can be called with every index. */
        return (
            std::invocable<Generator &, const std::integral_constant<std::size_t, Indices> &> &&
            ...
        );
    }(std::make_index_sequence<N>{});

    // same type multiple element (simd but worse)
    template<typename T, std::size_t N>
    struct stme {
        using data_t [[gnu::vector_size(N * sizeof(T))]] = T;
        static_assert(sizeof(data_t) == N * sizeof(T));
        explicit constexpr stme() = default;
        public:
        template<my_epic_generator<N> Generator>
        CONSTEXPR_MAYBE explicit stme(Generator gen) {
            static_for<N>([&](const auto i) {
                data[int(i)] = std::invoke(gen, i);
            });
        }


        CONSTEXPR_MAYBE explicit stme( std::integral auto value ) noexcept{
            static_for<N>([&](const auto i) {
                data[int(i)] = value;
            });
        }

        template< class U >
        constexpr explicit stme( stme<U, N> other ) noexcept {
            data = other.data;
        }

        CONSTEXPR_MAYBE explicit stme (std::array<T, N> other) noexcept {
            static_for<N>([&](const auto i) {
                data[int(i)] = other[i];
            });
        }

        constexpr explicit stme (data_t other) noexcept {
            data = other;
        }

        constexpr T operator[](int i) const {
            return data[i];
        }

        friend CONSTEXPR_MAYBE void assign(stme &d, int i, T value) {
            d.data[i] = value;
        }

        constexpr stme operator-(stme other) const {
            return stme{data - other.data};
        }

        constexpr stme operator+(stme other) const {
            return stme{data + other.data};
        }

        constexpr stme operator|(stme other) const {
            return stme{data | other.data};
        }

        constexpr stme operator&(stme other) const {
            return stme{data & other.data};
        }

        constexpr stme operator^(stme other) const {
            return stme{data ^ other.data};
        }

        constexpr stme operator>>(std::integral auto other) const {
            return stme{data >> other};
        }

        constexpr void operator>>=(stme other) {
            data >>= other.data;
        }

        constexpr void operator>>=(std::integral auto other) {
            data >>= other;
        }

        constexpr void operator<<=(stme other) {
            data <<= other.data;
        }

        constexpr void operator<<=(std::integral auto other) {
            data <<= other;
        }

        constexpr stme operator<<(std::integral auto other) const {
            return stme{data << other};
        }

        stme operator<<(unsigned int other) {
            return stme{data << other};
        }

        constexpr stme operator==(std::integral auto other) const {
            return stme{data == other};
        }

        constexpr stme operator!=(std::integral auto other) const {
            return stme{data != other};
        }

        constexpr stme operator!=(stme other) const {
            return stme{data != other.data};
        }

        constexpr void operator&=(stme other) {
            data &= other.data;
        }

        constexpr void operator|=(stme other) {
            data |= other.data;
        }

        constexpr stme operator~() const {
            return stme{~data};
        }

        constexpr static auto size() {
            return N;
        }

        friend constexpr bool any_of(stme self) {
            T any{};
            static_for<N>([&](auto i) {
                any |= self.data[int(i)];
            });

            return !!any;
        }

        friend constexpr bool all_of(stme self) {
            T all = -1;
            static_for<N>([&](auto i) {
                all &= self.data[int(i)];
            });

            return !!all;
        }

        data_t data;
    };

};