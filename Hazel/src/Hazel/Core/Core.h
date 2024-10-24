#pragma once

#include <memory>

#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
	#define HZ_PLATFORM_WINDOWS
	#else
		/* Windows x86 */
	#error "x86 Builds are not supported!"
	#endif
#endif

#ifdef HZ_PLATFORM_WINDOWS
#else
	#error Hazel only supports windows
#endif // HZ_PLATFORM_WINDOWS

#ifdef HZ_DEBUG
#define HZ_ENABLE_ASSERTS
#endif // HZ_DEBUG

#ifdef HZ_ENABLE_ASSERTS
//断言（如果x表示错误则语句运行，{0}占位的"__VA_ARGS__"代表"..."所输入的语句）
#define HZ_CORE_ASSERT(x, ...) \
		{if(!(x)){\
			HZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);\
			__debugbreak();}\
		}
#define HZ_ASSERT(x, ...)\
		{if(!(x)){\
			HZ_ERROR("Assertion Failed: {0}", __VA_ARGS__);\
			__debugbreak();}\
		}
#else
#define HZ_CORE_ASSERT(x, ...)
#define HZ_ASSERT(x, ...)
#endif // HZ_ENABLE_ASSERTS

#define BIT(x) (1 << x)

#define HZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Hazel
{
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);									// make_shared 的参数是 根据CreateRef所填参数 所动态调整的参数。这些来自CreateRef中的模版参数会被 std::forward 所完美转发
	}

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}