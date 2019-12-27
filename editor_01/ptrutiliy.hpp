#pragma once
#include <type_traits>

template <typename ptr_t, typename = std::enable_if<std::is_pointer<ptr_t>::value>>
void null_delete(ptr_t & ptr)
{
	delete ptr;
	ptr = nullptr;
}

template <typename ptr_t, typename = std::enable_if<std::is_pointer<ptr_t>::value>>
void safe_delete(ptr_t & ptr)
{
	if (ptr)
	{
		delete ptr;
		ptr = nullptr;
	}
}
