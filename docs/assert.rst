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

  assert that the condition(s) hold, where each condition is of the form
  ``[explicitly-convertible-to-bool]``.

  expressions written in the form ``[lhs OP rhs]``, where ``OP`` is one of
  ``==``, ``!=``, ``<``, ``<=``, ``>``, ``>=``, are decomposed and the ``lhs``
  and ``rhs`` are extracted separately to provide better diagnostics.

.. c:macro:: VEG_DEBUG_ASSERT(Cond)
.. c:macro:: VEG_DEBUG_EXPECT(Cond)
.. c:macro:: VEG_DEBUG_ASSERT_ELSE(Message, Cond)
.. c:macro:: VEG_DEBUG_EXPECT_ELSE(Message, Cond)
.. c:macro:: VEG_DEBUG_ASSERT_ALL_OF(Cond...)
.. c:macro:: VEG_DEBUG_EXPECT_ALL_OF(Cond...)
.. c:macro:: VEG_DEBUG_ASSERT_ELSE_ALL_OF(Message, Cond...)
.. c:macro:: VEG_DEBUG_EXPECT_ELSE_ALL_OF(Message, Cond...)

