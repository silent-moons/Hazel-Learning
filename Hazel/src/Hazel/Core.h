#pragma once

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
		{if(!x){\
			HZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);\
			__debugbreak();}\
		}
#define HZ_ASSERT(x, ...)\
		{if(!x){\
			HZ_ERROR("Assertion Failed: {0}", __VA_ARGS__);\
			__debugbreak();}\
		}
#else
#define HZ_CORE_ASSERT(x, ...)
#define HZ_ASSERT(x, ...)
#endif // HZ_ENABLE_ASSERTS

#define BIT(x) (1 << x)
