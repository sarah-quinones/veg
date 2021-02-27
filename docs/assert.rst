assert
======

#include <veg/assert.hpp>

.. c:macro:: VEG_ASSERT(Cond)
.. c:macro:: VEG_EXPECT(Cond)
.. c:macro:: VEG_ASSERT_ELSE(Message, Cond)
.. c:macro:: VEG_EXPECT_ELSE(Message, Cond)
.. c:macro:: VEG_ASSERT_ALL_OF(Cond...)
.. c:macro:: VEG_EXPECT_ALL_OF(Cond...)
.. c:macro:: VEG_ASSERT_ELSE_ALL_OF(Message, Cond...)
.. c:macro:: VEG_EXPECT_ELSE_ALL_OF(Message, Cond...)

| assert that the condition(s) hold, where each condition is of the form
  ``[explicitly-convertible-to-bool]``
| expressions written in the form ``[lhs OP rhs]``, where ``OP`` is one of
  ``==``, ``!=``, ``<``, ``<=``, ``>``, ``>=``, are decomposed and the ``lhs``
  and ``rhs`` are extracted separately to provide better diagnostics
| the macros expand to an expression of type ``void``
| in the multi-condition variants, condition expressions may be optionally
  surrounded by parentheses for proper comma handling within macros

| 
| 

.. c:macro:: VEG_DEBUG_ASSERT(Cond)
.. c:macro:: VEG_DEBUG_EXPECT(Cond)
.. c:macro:: VEG_DEBUG_ASSERT_ELSE(Message, Cond)
.. c:macro:: VEG_DEBUG_EXPECT_ELSE(Message, Cond)
.. c:macro:: VEG_DEBUG_ASSERT_ALL_OF(Cond...)
.. c:macro:: VEG_DEBUG_EXPECT_ALL_OF(Cond...)
.. c:macro:: VEG_DEBUG_ASSERT_ELSE_ALL_OF(Message, Cond...)
.. c:macro:: VEG_DEBUG_EXPECT_ELSE_ALL_OF(Message, Cond...)

| debug versions of the previous macros
| when ``NDEBUG`` is not defined, they expand to assertion macros
| otherwise, they expand to ``static_cast<void>(0)``
| 
| like most header files, the first ``#include`` of this header must be in the
  global namespace
| subsequent inclusions can be made in any context the language allows, and
  will toggle debug assertion macros depending on the value of ``NDEBUG``
