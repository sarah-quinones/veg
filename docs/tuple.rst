tuple
=====

#include <veg/tuple.hpp>

``constexpr`` support is provided starting from c++14

.. cpp:namespace:: veg
.. cpp:class:: template <typename... Ts> tuple

  .. cpp:function:: constexpr tuple(tuple const&) = default;
  .. cpp:function:: constexpr tuple(tuple&&) = default;
  .. cpp:function:: constexpr auto operator=(tuple const&) & -> tuple& = default;
  .. cpp:function:: constexpr auto operator=(tuple&&) & -> tuple& = default;
  .. cpp:function:: constexpr tuple() noexcept(conditionally)

    | default constructor: each member is default or value initialized
    | viable if
      `default_constructible<Ti> <https://en.cppreference.com/w/cpp/types/is_default_constructible>`__
      for all *i*
    | ``noexcept`` if `nothrow_default_constructible<Ti>
      <https://en.cppreference.com/w/cpp/types/is_default_constructible>`__

  .. cpp:function:: constexpr tuple(elems_t, Ts... args) noexcept(conditionally)

    | direct constructor: each member is constructed by forwarding the
      corresponding parameter ``Ti(FWD(args_i))``
    | viable if `move_constructible<Ti>
      <https://en.cppreference.com/w/cpp/types/is_move_constructible>`__ for all
      *i*
    | ``noexcept`` if `nothrow_move_constructible<Ti>
      <https://en.cppreference.com/w/cpp/types/is_move_constructible>`__ for
      all *i*

  .. cpp:function:: template<typename... Fn> constexpr tuple(inplace_t, Fn&&... fn) noexcept(conditionally)

    | inplace constructor: each member is initialized from the result of
      invoking the corresponding parameter ``Ti(FWD(fn_i)())``
    | viable if `invocable<Ti>
      <https://en.cppreference.com/w/cpp/types/is_invocable>`__ and
      `invoke_result_t<Fn_i> == Ti
        <https://en.cppreference.com/w/cpp/types/result_of>`__ for all *i*
    | ``noexcept`` if `nothrow_invocable<Ti>
      <https://en.cppreference.com/w/cpp/types/is_invocable>`__ for all *i*

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
          `i`
        | ``explicit`` if `convertible_to<Ti, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_convertible>`__ is
          ``false`` for at least one `i`
        | ``noexcept`` if `nothrow_constructible<Ti, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for
          all *i*

    .. tab:: lvalue

      .. cpp:function:: template <typename... Us>\
                        constexpr tuple(tuple<Us...>& tup) noexcept(conditionally);

        | converting lvalue constructor: each member is initialized from the
          corresponding member of the tuple parameter
          ``Ti(tup.elem_i)``
        | viable if `constructible<Ti, Ui&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for all
          `i`
        | ``explicit`` if `convertible_to<Ti, Ui&>
          <https://en.cppreference.com/w/cpp/types/is_convertible>`__ is
          ``false`` for at least one `i`
        | ``noexcept`` if `nothrow_constructible<Ti, Ui&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for
          all *i*

    .. tab:: forwarding

      .. cpp:function:: template <typename... Us>\
                        constexpr tuple(tuple<Us...>&& tup) noexcept(conditionally);

        | converting forwarding constructor: each member is initialized by
          forwarding the corresponding member of the tuple parameter
          ``Ti(static_cast<Ui&&>(tup.elem_i))``
        | viable if `constructible<Ti, Ui&&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for all
          `i`
        | ``explicit`` if `convertible_to<Ti, Ui&&>
          <https://en.cppreference.com/w/cpp/types/is_convertible>`__ is
          ``false`` for at least one `i`
        | ``noexcept`` if `nothrow_constructible<Ti, Ui&&>
          <https://en.cppreference.com/w/cpp/types/is_constructible>`__ for
          all *i*

    .. tab:: forwarding (deleted)

      .. cpp:function:: template <typename... Us>\
                       constexpr tuple(tuple<Us...>&&) = delete;

        | prevents selecting the const overload
        | viable when the previous constructor is not

  .. centered:: assignment operators

  .. tabs::

    .. tab:: copy

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...> const& tup) & noexcept(conditionally);

        | assignment operator: assigns to each member ``elem_i = tup.elem_i``
        | viable if ``Ti`` is not a reference and `assignable<Ti&, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all `i`
        | ``noexcept`` if `nothrow_assignable<Ti&, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all *i*

    .. tab:: copy (deleted)

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...> const& tup) & = delete;

        | prevents selecting the proxy overload
        | viable if the previous assignment operator is not

    .. tab:: forwarding

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...>&& tup) & noexcept(conditionally);

        | forwarding assignment operator: assigns to each member ``elem_i =
          static_cast<Ui&&>(tup.elem_i)``
        | viable if ``Ti`` is not a reference and `assignable<Ti&, U&&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all `i`
        | ``noexcept`` if `nothrow_assignable<Ti&, Ui&&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all *i*

    .. tab:: forwarding (deleted)

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...>&& tup) & = delete;

        | prevents selecting the proxy overload
        | viable if the previous assignment operator is not

  .. centered:: proxy assignment operators

  .. tabs::

    .. tab:: copy

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...> const& tup) const& noexcept(conditionally);

        | proxy assignment operator: assigns to each member ``elem_i = tup.elem_i``
        | viable if `assignable<Ti const&, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all `i`
        | ``noexcept`` if `nothrow_assignable<Ti const&, Ui const&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all *i*

    .. tab:: forwarding

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...>&& tup) const& noexcept(conditionally);

        | forwarding proxy assignment operator: assigns to each member ``elem_i =
          static_cast<Ui&&>(tup.elem_i)``
        | viable if `assignable<Ti const&, U&&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all `i`
        | ``noexcept`` if `nothrow_assignable<Ti const&, Ui&&>
          <https://en.cppreference.com/w/cpp/types/is_assignable>`__ for all *i*

    .. tab:: forwarding (deleted)

      .. cpp:function:: template <typename... Us>\
                        constexpr auto operator=(tuple<Us...>&& tup) const& = delete;

        | prevents selecting the const overload
        | viable if the previous assignment operator is not

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


.. cpp:function:: template <usize I, typename T>\
                  constexpr void __adl::get(T&& tup) noexcept(conditionally);

  | found through adl
  | viable if ``tup`` is a :cpp:class:`tuple\<Ti...>`, and expression-equivalent to
    ``FWD(tup)[fix<I>()]``

.. cpp:function:: template <typename U, typename V>\
                  constexpr void __adl::swap(U&& u, V&& v) noexcept(conditionally);

  | memberwise forwarding :cpp:func:`veg::swap`
  | let ``Ui``, be ``decltype(FWD(u).as_ref()[fix<I>()])``, and ``Vi``, be
    defined similarly
  | viable if ``u`` and ``v`` are :cpp:class:`tuple\<_>` and `swappable_with<Ui, Vi>
    <https://en.cppreference.com/w/cpp/types/is_swappable>`__ for all *i*
  | ``noexcept`` if `nothrow_swappable_with<Ui, Vi>
    <https://en.cppreference.com/w/cpp/types/is_swappable>`__ for all *i*
