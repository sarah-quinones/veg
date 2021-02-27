utility
=======

#include <veg/assert.hpp>

.. cpp:namespace:: veg
.. cpp:function:: template <typename U, typename V> constexpr void swap(U&&, V&&) noexcept(conditionally);

   see `std::ranges::swap <https://en.cppreference.com/w/cpp/utility/ranges/swap>`__

.. cpp:function:: template <usize I, typename T> constexpr auto get(T&&) noexcept(conditionally) -> decltype(auto);

   see lookup of ``get`` for `structured bindings <https://en.cppreference.com/w/cpp/language/structured_binding>`__
.. cpp:class:: elems_t
.. cpp:class:: inplace_t
.. cpp:class:: template <i64 N> fix
