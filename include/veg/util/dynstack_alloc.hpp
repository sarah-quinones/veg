#ifndef VEG_DYNSTACK_ALLOC_HPP_YYWN4MATS
#define VEG_DYNSTACK_ALLOC_HPP_YYWN4MATS

#include "veg/vec.hpp"
#include "veg/memory/dynamic_stack.hpp"

#define __VEG_IMPL_MAKE_STACK(vec, stack, stack_req)                           \
	VEG_NOM_SEMICOLON;                                                           \
	::veg::Vec<unsigned char> vec;                                               \
	vec.resize_for_overwrite(stack_req.alloc_req());                             \
	::veg::dynstack::DynStackMut stack{                                          \
			::veg::tags::from_slice_mut, vec.as_mut()};                              \
	VEG_NOM_SEMICOLON

#define VEG_MAKE_STACK(stack, stack_req)                                       \
	__VEG_IMPL_MAKE_STACK(VEG_ID(stack_storage), stack, stack_req)

#endif /* end of include guard VEG_DYNSTACK_ALLOC_HPP_YYWN4MATS */
