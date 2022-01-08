#ifndef __UTIL_JAVASCRIPT_HPP__
#define __UTIL_JAVASCRIPT_HPP__

#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace cef
{
	enum class JSValueType : uint32_t
	{
		Undefined = 0,
		Null,
		Bool,
		Int,
		Double,
		Date,
		String,
		Object,
		Array,
		Function
	};

	struct JSValue
	{
		JSValueType type;
		void *data = nullptr;
	};
};

#endif
