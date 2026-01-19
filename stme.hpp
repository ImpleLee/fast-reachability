#pragma once

#include <array>
#include <cstddef>
#include <concepts>
#include <functional>
#include "utils.hpp"

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
        explicit constexpr stme() = default;
        public:
        template<my_epic_generator<N> Generator>
        constexpr explicit stme(Generator gen) {
            static_for<N>([&](const auto i) {
                data[i] = std::invoke(gen, i);
            });
        }


        constexpr explicit stme( std::integral auto value ) noexcept{
            static_for<N>([&](const auto i) {
                data[i] = value;
            });
        }

        template< class U >
        constexpr explicit stme( stme<U, N> other ) noexcept {
            static_for<N>([&](const auto i) {
                data[i] = other[i];
            });
        }

        constexpr explicit stme (std::array<T, N> other) noexcept {
            static_for<N>([&](const auto i) {
                data[i] = other[i];
            });
        }


        constexpr const T& operator[](auto i) const {
            return data[i];
        }
        
        constexpr T& operator[](auto i) {
            return data[i];
        }

        constexpr stme operator-(stme other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] - other[i];
            });
            return ret;
        }
        
        constexpr stme operator+(stme other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] + other[i];
            });
            return ret;
        }
        
        constexpr stme operator|(stme other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] | other[i];
            });
            return ret;
        }
        
        constexpr stme operator&(stme other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] & other[i];
            });
            return ret;
        }
        
        constexpr stme operator^(stme other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] ^ other[i];
            });
            return ret;
        }
        
        constexpr stme operator>>(std::integral auto other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] >> other;
            });
            return ret;
        }
        template <std::integral auto other>
        constexpr stme right_shift() const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] >> other;
            });
            return ret;
        }

        constexpr void operator>>=(stme other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] >> other[i];
            });
            data = ret;
        }

        constexpr void operator>>=(std::integral auto other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] >> other;
            });
            data = ret;
        }
        
        constexpr void operator<<=(stme other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] << other[i];
            });
            data = ret;
        }

        constexpr void operator<<=(std::integral auto other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] << other;
            });
            data = ret;
        }

        constexpr stme operator<<(std::integral auto other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] << other;
            });
            return ret;
        }

        stme operator<<(unsigned int other) {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] << other;
            });
            return ret;
        }

        constexpr stme operator==(std::integral auto other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] == other;
            });
            return ret;
        }
        
        constexpr stme operator!=(std::integral auto other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] != other;
            });
            return ret;
        }
        
        constexpr stme operator!=(stme other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] != other[i];
            });
            return ret;
        }

        constexpr void operator&=(stme other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] & other[i];
            });
            data = ret;
        }
        
        constexpr void operator|=(stme other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] | other[i];
            });
            data = ret;
        }

        constexpr stme operator~() const {
            stme ret{data};
            static_for<N>([&](auto i) {
                ret[i] = ~ret[i];
            });
            return ret;
        }

        constexpr auto begin() const {
            return data.begin();
        }

        constexpr auto end() const {
            return data.end();
        }

        constexpr auto size() const {
            return data.size();
        }

        constexpr bool any_of() const {
            T any{};
            static_for<N>([&](auto i) {
                any |= data[i];
            });

            return !!any;
        }

        constexpr bool all_of() const {
            T all = -1;
            static_for<N>([&](auto i) {
                all &= data[i];
            });

            return !!all;
        }

        [[no_unique_address]] std::array<T,N> data{};
    };

};