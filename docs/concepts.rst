Concepts
========

#include <veg/type_traits/invocable.hpp>

.. role:: ccode(code)
   :language: cpp

.. cpp:namespace:: veg::concepts

.. cpp:concept:: template <typename Fn, typename Ret, typename... Args> fn_once

   function object that can be called at least once,
   taking the arguments :cpp:expr:`Args...`
   and returning an object of type :cpp:expr:`Ret`

   .. code-block:: cpp

      requires(Fn fn, Args... args) {
        FWD(fn)(FWD(args)...); // same_as<Ret>
      }

.. cpp:concept:: template <typename Fn, typename Ret, typename... Args> fn_mut

   function object that can be repeatedly called, possibly mutating its state,
   taking the arguments :cpp:expr:`Args...`
   and returning an object of type :cpp:expr:`Ret`

   .. code-block:: cpp

      requires(Fn& fn, Args... args) {
        fn(FWD(args)...); // same_as<Ret>
      }

.. cpp:concept:: template <typename Fn, typename Ret, typename... Args> fn

   function object that can be repeatedly called, without mutating its state,
   taking the arguments :cpp:expr:`Args...`
   and returning an object of type :cpp:expr:`Ret`

   .. code-block:: cpp

      requires(Fn const& fn, Args... args) {
        fn(FWD(args)...); // same_as<Ret>
      }

#include <veg/type_traits/constructible.hpp>

.. cpp:concept:: template <typename T> nothrow_destructible

   object can be destroyed without throwing an exception

   .. code-block:: cpp

      requires(T* mem) {
        { mem->~T() } noexcept;
      }

.. cpp:concept:: template <typename T> trivially_copyable

   corresponds to the `[Trivially copyable]
   <https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable>`__ named requirement.

.. cpp:concept:: template <typename T> movable

   object can be move constructed.

   .. code-block:: cpp

      requires(T value) {
        { T(FWD(value)) } noexcept;
      }

.. cpp:concept:: template <typename T> copyable

   object can be copy constructed.

   .. code-block:: cpp

      requires(T const value) {
        { T(value) } noexcept;
      }
