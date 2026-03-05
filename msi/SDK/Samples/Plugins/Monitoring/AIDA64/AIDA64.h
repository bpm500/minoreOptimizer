#ifdef AIDA64_EXPORTS
#define AIDA64_API extern "C" __declspec(dllexport)
#else
#define AIDA64_API extern "C" __declspec(dllimport)
#endif