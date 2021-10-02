Tuple
=====

#include <veg/tuple.hpp>

.. role:: ccode(code)
   :language: cpp

.. cpp:namespace:: veg

.. cpp:class:: template <typename... Ts> Tuple

   inherits from :code:`IndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...>`,
   and inherits its constructors

.. cpp:namespace:: veg::tuple

.. cpp:class:: template <usize... Is, typename... Ts> IndexedTuple<meta::index_sequence<Is...>,Ts...>

  .. cpp:function:: IndexedTuple() = default;

  .. cpp:function:: explicit constexpr IndexedTuple(IndexedTuple const&) = default;

  .. cpp:function:: constexpr IndexedTuple(IndexedTuple&&) = default;

  .. cpp:function:: constexpr IndexedTuple(Direct, Ts... args) noexcept(see_below)

    | direct constructor: each member is constructed by forwarding the
      corresponding parameter :code:`Ts[I](FWD(args_i))`
    | viable if :cpp:concept:`movable\<Ts[I]\><veg::concepts::movable>` for all
      ``I``
    | ``noexcept`` if :cpp:concept:`nothrow_movable\<T[I]\><veg::concepts::movable>` for
      all ``I``

  .. cpp:function:: template <typename... Fns> constexpr IndexedTuple(InPlace, Fns... fns) noexcept(see_below)

    | in place constructor: each member is initialized from the result of
      invoking the corresponding parameter :code:`Ts[I](FWD(fn_i)())`
    | viable if `fn_once<Fn_i, Ts[I]>
      <https://en.cppreference.com/w/cpp/types/is_invocable>`__ for all ``I``
    | ``noexcept`` if `nothrow_fn_once<Fn_i, Ts[I]>
      <https://en.cppreference.com/w/cpp/types/is_invocable>`__ for all ``I``

  .. cpp:function:: constexpr auto as_ref() const & noexcept -> Tuple<Ref<Ts>...>

    | returns a tuple containing :code:`ref(elems[Is])...`

  .. cpp:function:: constexpr auto as_mut() const & noexcept -> Tuple<RefMut<Ts>...>

    | returns a tuple containing :code:`mut(elems[Is])...`

  .. cpp:function:: constexpr auto as_const()&& noexcept -> Tuple<Ref<Us>...>

    | viable if ``Ts[I] := RefMut<Us[I]>`` for all ``I`` returns a tuple containing :code:`elem[Is].as_const()...`


  .. cpp:function:: template <i64 I> constexpr operator[](Fix<I>) const& noexcept -> ith<I, Ts...> const&
  .. cpp:function:: template <i64 I> constexpr operator[](Fix<I>)& noexcept -> ith<I, Ts...> &
  .. cpp:function:: template <i64 I> constexpr operator[](Fix<I>)&& noexcept(see_below) -> ith<I, Ts...>

    | returns the ith element
    | the rvalue qualified overload is viable if
      :cpp:concept:`movable\<Ts[I]\><veg::concepts::movable>` for all ``I``
    | the rvalue qualified overload is  ``noexcept`` if
      :cpp:concept:`nothrow_movable\<Ts[I]\><veg::concepts::movable>` for all ``I``

  .. cpp:function:: template <typename Fn, typename Ret = meta::invoke_result_t<Fn, Ts...>> constexpr auto unpack(Fn fn)&& noexcept(see_below) -> Ret

    | calls :code:`FWD(fn)(FWD(elems_i)...)`
    | viable if :cpp:concept:`fn_once\<Fn, Ret\><veg::concepts::fn_once>`
    | ``noexcept`` if :cpp:concept:`nothrow_fn_once\<Fn, Ret\><veg::concepts::fn_once>`

  .. cpp:function:: template <typename Fn> constexpr void for_each(Fn fn)&& noexcept(see_below)

    | calls :code:`(fn(FWD(elems_i)), ...)`
    | viable if :cpp:concept:`fn_mut\<Fn, void, Ts[I]\><veg::concepts::fn_mut>` for all ``I``
    | ``noexcept`` if :cpp:concept:`nothrow_fn_mut\<Fn, void, Ts[I]\><veg::concepts::fn_mut>` for all ``I``

  .. cpp:function:: template <typename Fn> constexpr void for_each_i(Fn fn)&& noexcept(see_below)

    | calls :code:`(fn(Fix<Is>{}, FWD(elems[Is])), ...)`
    | viable if :cpp:concept:`fn_mut\<Fn, void, Fix\<Is\>, Ts[I]\><veg::concepts::fn_mut>` for all ``I``
    | ``noexcept`` if :cpp:concept:`nothrow_fn_mut\<Fn, void, Fix\<Is\>, Ts[I]\><veg::concepts::fn_mut>` for all ``I``

  .. cpp:function:: template <typename Fn, typename Rets = meta::invoke_result_t<Fn&, Ts>...> constexpr auto map(Fn fn)&& noexcept(see_below) -> Tuple<Rets...>

    | returns a tuple containing :code:`fn(FWD(elems_i))...`
    | viable if :cpp:concept:`fn_mut\<Fn, Rets[I], Ts[I]\><veg::concepts::fn_mut>` for all ``I``
    | ``noexcept`` if :cpp:concept:`nothrow_fn_mut\<Fn, Rets[I], Ts[I]\><veg::concepts::fn_mut>` for all ``I``

  .. cpp:function:: template <\
       typename Fn,\
       typename Rets = meta::invoke_result_t<Fn&, Fix<Is>, Ts>...\
     >\
     constexpr auto for_each_i(Fn fn)&& noexcept(see_below) -> Tuple<Rets...>

    | retusn a tuple containing :code:`fn(Fix<Is>{}, FWD(elems[Is]))...`
    | viable if :cpp:concept:`fn_mut\<Fn, Rets[I], Fix\<Is\>, Ts[I]\><veg::concepts::fn_mut>` for all ``I``
    | ``noexcept`` if :cpp:concept:`nothrow_fn_mut\<Fn, Rets[I], Fix\<Is\>, Ts[I]\><veg::concepts::fn_mut>` for all ``I``


.. cpp:function:: template <usize I, typename... Ts>\
                  constexpr auto get(Tuple<Ts...>& tup) noexcept -> Ti;

.. cpp:function:: template <usize I, typename... Ts>\
                  constexpr auto get(Tuple<Ts...> const& tup) noexcept -> Ti const&;

.. cpp:function:: template <usize I, typename... Ts>\
                  constexpr auto get(Tuple<Ts...>&& tup) noexcept -> Ti&&;

.. cpp:function:: template <usize I, typename... Ts>\
                  constexpr void get(Tuple<Ts...> const&& tup) = delete;

  | returns ith element. regular function template, visible to ADL

.. cpp:function:: template <typename... Ts, typename... Us>\
                  constexpr void swap(Tuple<Ts...>& t, Tuple<Us...>& u) noexcept(conditionally);

  | expression-equivalent to memberwise swap :cpp:func:`veg::swap`\ :code:`(t.elem_i, u.elem_i)`. regular function template, visible to ADL
  | not viable if some ``Ti``, or some ``Ui`` is a reference

.. cpp:function:: template <typename... Ts>\
                  constexpr auto cat(Ts... ts) noexcept(conditionally) -> see_below;

  | takes a variadic number of indexed tuples and returns a tuple whose elements are those of the first tuple, followed by the elements of the second and so on
  | viable if ``ts[I]`` is some ``IndexedTuple`` and is :cpp:concept:`movable\<Ts[I]\><veg::concepts::movable>` for all ``I``
  | ``noexcept`` if it's :cpp:concept:`nothrow_movable\<Ts[I]\><veg::concepts::movable>` for all ``I``

.. cpp:function:: template <typename... Ts>\
                  constexpr auto zip(Ts... t) noexcept(conditionally) -> see_below;

  | takes a variadic number of indexed tuples ``t[i]...`` and returns a tuple whose elements are ``((t[j][i]...)_j...)_i``
  | viable if ``ts[I]`` is some ``IndexedTuple``, is :cpp:concept:`movable\<Ts[I]\><veg::concepts::movable>` for all ``I``, and the tuples are all the same size
  | ``noexcept`` if it's :cpp:concept:`nothrow_movable\<Ts[I]\><veg::concepts::movable>` for all ``I``
