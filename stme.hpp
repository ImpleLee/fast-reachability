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
        
        template<my_epic_generator<N> Generator>
        constexpr explicit stme(const Generator& gen) {
            static_for<N>([&](const auto i) {
                data[i] = std::invoke(gen, i);
            });
        }


        constexpr explicit stme( const std::integral auto& value ) noexcept{
            static_for<N>([&](const auto i) {
                data[i] = value;
            });
        }

        template< class U >
        constexpr explicit stme( const stme<U, N>& other ) noexcept {
            static_for<N>([&](const auto i) {
                data[i] = other[i];
            });
        }

        constexpr explicit stme (const std::array<T, N>& other) noexcept {
            static_for<N>([&](const auto i) {
                data[i] = other[i];
            });
        }


        inline constexpr const T& operator[](auto i) const {
            return data[i];
        }
        
        inline constexpr T& operator[](auto i) {
            return data[i];
        }

        inline constexpr stme operator-(const stme& other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] - other[i];
            });
            return ret;
        }
        
        inline constexpr stme operator+(const stme& other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] + other[i];
            });
            return ret;
        }
        
        inline constexpr stme operator|(const stme& other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] | other[i];
            });
            return ret;
        }
        
        inline constexpr stme operator&(const stme& other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] & other[i];
            });
            return ret;
        }
        
        inline constexpr stme operator^(const stme& other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] ^ other[i];
            });
            return ret;
        }

        inline constexpr stme operator>>(const std::integral auto& other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] >> other;
            });
            return ret;
        }

        inline constexpr void operator>>=(const stme& other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] >> other[i];
            });
            data = ret;
        }

        inline constexpr void operator>>=(const unsigned int& other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] >> other;
            });
            data = ret;
        }
        
        inline constexpr void operator<<=(const stme& other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] << other[i];
            });
            data = ret;
        }

        inline constexpr void operator<<=(const unsigned int& other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] << other;
            });
            data = ret;
        }

        inline constexpr stme operator==(const std::integral auto& other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] == other;
            });
            return ret;
        }
        
        inline constexpr stme operator!=(const std::integral auto& other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] != other;
            });
            return ret;
        }
        
        inline constexpr stme operator!=(const stme&& other) const {
            stme ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] != other[i];
            });
            return ret;
        }

        inline constexpr void operator&=(const stme& other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] & other[i];
            });
            data = ret;
        }
        
        inline constexpr void operator|=(const stme& other) {
            std::array<T,N> ret;
            static_for<N>([&](auto i) {
                ret[i] = data[i] | other[i];
            });
            data = ret;
        }

        inline constexpr stme operator~() const {
            stme ret{data};
            static_for<N>([&](auto i) {
                ret[i] = ~ret[i];
            });
            return ret;
        }

        inline constexpr auto begin() const {
            return data.begin();
        }

        inline constexpr auto end() const {
            return data.end();
        }

        inline constexpr auto size() const {
            return data.size();
        }

        inline constexpr bool any_of() const {
            T any{};
            static_for<N>([&](auto i) {
                any |= data[i];
            });

            return !!any;
        }

        inline constexpr bool all_of() const {
            T all = -1;
            static_for<N>([&](auto i) {
                all &= data[i];
            });

            return !!all;
        }

        [[no_unique_address]] std::array<T,N> data{};
    };

};