#ifdef SMART_EXPORTS
#define SMART_API extern "C" __declspec(dllexport)
#else
#define SMART_API extern "C" __declspec(dllimport)
#endif