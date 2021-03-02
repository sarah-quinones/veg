tuple
=====

#include <veg/tuple.hpp>

``constexpr`` support is provided starting from c++14

.. cpp:namespace:: veg
.. cpp:class:: template <typename... Ts> tuple : __::tuple_base

  .. cpp:function:: constexpr tuple(tuple const&) = default;
  .. cpp:function:: constexpr tuple(tuple&&) = default;
  .. cpp:function:: constexpr tuple() noexcept(conditionally)

    | default constructor: each member is default or value initialized
    | viable if
      `default_constructible<Ti> <https://en.cppreference.com/w/cpp/types/is_default_constructible>`__
      for all ``i``
    | ``noexcept`` if `nothrow_default_constructible<Ti>
      <https://en.cppreference.com/w/cpp/types/is_default_constructible>`__

  .. cpp:function:: constexpr tuple(elems_t, Ts... args) noexcept(conditionally)

    | direct constructor: each member is constructed by forwarding the
      corresponding parameter ``Ti(FWD(args_i))``
    | viable if `move_constructible<Ti>
      <https://en.cppreference.com/w/cpp/types/is_move_constructible>`__ for all
      ``i``
    | ``noexcept`` if `nothrow_move_constructible<Ti>
      <https://en.cppreference.com/w/cpp/types/is_move_constructible>`__ for
      all ``i``

  .. cpp:function:: template<typename... Fn> constexpr tuple(inplace_t, Fn&&... fn) noexcept(conditionally)

    | inplace constructor: each member is initialized from the result of
      invoking the corresponding parameter ``Ti(FWD(fn_i)())``
    | viable if `invocable<Ti>
      <https://en.cppreference.com/w/cpp/types/is_invocable>`__ and
      `invoke_result_t<Fn_i> == Ti
        <https://en.cppreference.com/w/cpp/types/result_of>`__ for all ``i``
    | ``noexcept`` if `nothrow_invocable<Ti>
      <https://en.cppreference.com/w/cpp/types/is_invocable>`__ for all ``i``

  .. centered:: converting constructors

  .. tabs::

    .. tab:: copy

      .. cpp:function:: template <typename... Us>\
                        constexpr tuple(tuple<Us...> const& tup) noexcept(conditionally);

        | converting copy constructor: each member is initialized from the
          corresponding member of the tuple parameter
          ``Ti(tup.elem_i)``
        | viable if `constructible<Ti, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for all
          ``i``
        | ``explicit`` if `convertible_to<Ui const&, Ti>
          <https://en.cppreference.com/w/cpp/types/is_convertible>`__ is
          ``false`` for at least one ``i``
        | ``noexcept`` if `nothrow_constructible<Ti, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for
          all ``i``

    .. tab:: lvalue

      .. cpp:function:: template <typename... Us>\
                        constexpr tuple(tuple<Us...>& tup) noexcept(conditionally);

        | converting lvalue constructor: each member is initialized from the
          corresponding member of the tuple parameter
          ``Ti(tup.elem_i)``
        | viable if `constructible<Ti, Ui&> && !constructible<Ti, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for all
          ``i``

        .. warning::
          when ``Ti == Ui`` for all ``i``, this is shadowed by the copy constructor

        | ``explicit`` if `convertible_to<Ui&, Ti>
          <https://en.cppreference.com/w/cpp/types/is_convertible>`__ is
          ``false`` for at least one ``i``
        | ``noexcept`` if `nothrow_constructible<Ti, Ui&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for
          all ``i``

    .. tab:: forwarding

      .. cpp:function:: template <typename... Us>\
                        constexpr tuple(tuple<Us...>&& tup) noexcept(conditionally);

        | converting forwarding constructor: each member is initialized by
          forwarding the corresponding member of the tuple parameter
          ``Ti(static_cast<Ui&&>(tup.elem_i))``
        | viable if `constructible<Ti, Ui&&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for all
          ``i``
        | ``explicit`` if `convertible_to<Ui&&, Ti>
          <https://en.cppreference.com/w/cpp/types/is_convertible>`__ is
          ``false`` for at least one ``i``
        | ``noexcept`` if `nothrow_constructible<Ti, Ui&&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for
          all ``i``

    .. tab:: forwarding (deleted)

      .. cpp:function:: template <typename... Us>\
                        constexpr tuple(tuple<Us...> const&&) = delete;

        | prevents implicit ``rvalue -> lvalue`` conversions

  .. centered:: assignment operators

  .. tabs::

    .. tab:: copy

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...> const& tup) & noexcept(conditionally) -> tuple&;

        | assignment operator: assigns to each member ``elem_i = tup.elem_i``
        | viable if `assignable<Ti&, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all ``i``
        | ``noexcept`` if `nothrow_assignable<Ti&, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all ``i``

      .. cpp:function:: template <typename... Us>\
                        void operator=(__::tuple_base<Us...> const& tup) & = delete;

        | prevents implicit conversions

    .. tab:: forwarding

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...>&& tup) & noexcept(conditionally) -> tuple&;

        | forwarding assignment operator: assigns to each member ``elem_i =
          static_cast<Ui&&>(tup.elem_i)``
        | viable if `assignable<Ti&, U&&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all ``i``
        | ``noexcept`` if `nothrow_assignable<Ti&, Ui&&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all ``i``

      .. cpp:function:: template <typename... Us>\
                        void operator=(__::tuple_base<Us...>&& tup) & = delete;

        | prevents implicit conversions

    .. tab:: copy\|move

      .. cpp:function:: constexpr auto operator=(tuple const&) & noexcept(conditionally) -> tuple&;

      .. cpp:function:: constexpr auto operator=(tuple&&) & noexcept(conditionally) -> tuple&;

        | equivalent to the last two overloads
        | default compiler-generated functions are used when none of the ``Ti`` is a reference

  .. centered:: proxy assignment operators

  .. tabs::

    .. tab:: copy

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...> const&& tup) const&& noexcept(conditionally) -> tuple const&&;

        | proxy assignment operator: assigns to each member ``FORWARD(elem_i) = tup.elem_i``
        | viable if `assignable<Ti const&&, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all ``i``
        | ``noexcept`` if `nothrow_assignable<Ti const&&, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all ``i``

    .. tab:: forwarding

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...>&& tup) const&& noexcept(conditionally) -> tuple const&&;

        | forwarding proxy assignment operator: assigns to each member ``FORWARD(elem_i) =
          FORWARD(tup.elem_i)``
        | viable if `assignable<Ti const&&, U&&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all ``i``
        | ``noexcept`` if `nothrow_assignable<Ti const&&, Ui&&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all ``i``

  .. centered:: access operator

  .. tabs::

    .. tab:: const lvalue

      .. cpp:function:: template <i64 I>\
                        constexpr auto operator[](fix<i64>) const& noexcept -> T_I const&;

        | returns a shallow-const reference to the Ith element
        | viable if ``0 <= I < sizeof...(Ts)``

    .. tab:: mutable lvalue

      .. cpp:function:: template <i64 I>\
                        constexpr auto operator[](fix<i64>) & noexcept -> T_I&;

        | returns a reference to the Ith element
        | viable if ``0 <= I < sizeof...(Ts)``

    .. tab:: rvalue

      .. cpp:function:: template <i64 I>\
                        constexpr auto operator[](fix<i64>) && noexcept(conditionally) -> T_I;

        | returns a shallow-const reference to the Ith element
        | viable if ``0 <= I < sizeof...(Ts)`` and `move_constructible<T_I>
          <https://en.cppreference.com/w/cpp/types/is_move_constructible>`__
        | ``noexcept`` if `nothrow_move_constructible<T_I>
          <https://en.cppreference.com/w/cpp/types/is_move_constructible>`__

    .. tab:: rvalue (deleted)

      .. cpp:function:: template <i64 I>\
                        constexpr void operator[](fix<i64>) && = delete;

        | prevents selecting the const overload
        | viable if ``0 <= I < sizeof...(Ts)`` and `move_constructible<T_I>
          <https://en.cppreference.com/w/cpp/types/is_move_constructible>`__ is ``false``

  .. centered:: to reference

  .. tabs::

    .. tab:: const lvalue

      .. cpp:function:: constexpr auto as_ref() const& noexcept -> tuple<Ts const&...>;

    .. tab:: mutable lvalue

      .. cpp:function:: constexpr auto as_ref() && noexcept -> tuple<Ts&&...>;

    .. tab:: rvalue

      .. cpp:function:: constexpr auto as_ref() & noexcept -> tuple<Ts&...>;

  | returns tuple of references to the members, or in the case of
    references, to the objects they point to


.. cpp:function:: template <usize I, typename... Ts>\
                  constexpr auto __adl::get(tuple<Ts...>& tup) noexcept -> Ti;

.. cpp:function:: template <usize I, typename... Ts>\
                  constexpr auto __adl::get(tuple<Ts...> const& tup) noexcept -> Ti const&;

.. cpp:function:: template <usize I, typename... Ts>\
                  constexpr auto __adl::get(tuple<Ts...>&& tup) noexcept -> Ti&&;

.. cpp:function:: template <usize I, typename... Ts>\
                  constexpr void __adl::get(tuple<Ts...> const&& tup) = delete;

  | returns ith element

.. cpp:function:: template <typename... Ts, typename... Us>\
                  constexpr void __adl::swap(tuple<Ts...> const&& u, tuple<Us...>& v) noexcept(conditionally);

  | expression-equivalent to memberwise swap :cpp:func:`veg::swap(FORWARD(t.elem_i), u.elem_i)`

.. cpp:function:: template <typename... Ts, typename... Us>\
                  constexpr void __adl::swap(tuple<Ts...>& t, tuple<Us...>& u) noexcept(conditionally);

  | expression-equivalent to memberwise swap :cpp:func:`veg::swap(t.elem_i, u.elem_i)`

.. cpp:function:: template <typename... Ts, typename... Us>\
                  constexpr void __adl::swap(tuple<Ts...>& u, tuple<Us...>const&& v) noexcept(conditionally);

  | expression-equivalent to memberwise swap :cpp:func:`veg::swap(t.elem_i, FORWARD(u.elem_i))`

.. cpp:function:: template <typename... Ts, typename... Us>\
                  constexpr void __adl::swap(tuple<Ts...> const&& u, tuple<Us...>& v) noexcept(conditionally);

  | expression-equivalent to memberwise swap :cpp:func:`veg::swap(FORWARD(t.elem_i), u.elem_i)`

.. cpp:function:: template <typename... Ts, typename... Us>\
                  constexpr void __adl::swap(tuple<Ts...> const&& u, tuple<Us...> const&& v) noexcept(conditionally);

  | expression-equivalent to memberwise swap :cpp:func:`veg::swap(FORWARD(t.elem_i), FORWARD(u.elem_i))`
