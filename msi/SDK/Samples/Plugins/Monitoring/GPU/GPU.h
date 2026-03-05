#ifdef GPU_EXPORTS
#define GPU_API extern "C" __declspec(dllexport)
#else
#define GPU_API extern "C" __declspec(dllimport)
#endif