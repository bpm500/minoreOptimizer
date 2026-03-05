#ifdef CPU_EXPORTS
#define CPU_API extern "C" __declspec(dllexport)
#else
#define CPU_API extern "C" __declspec(dllimport)
#endif