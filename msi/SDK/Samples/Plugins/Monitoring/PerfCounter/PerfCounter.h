#ifdef PERFCOUNTER_EXPORTS
#define PERFCOUNTER_API extern "C" __declspec(dllexport)
#else
#define PERFCOUNTER_API extern "C" __declspec(dllimport)
#endif