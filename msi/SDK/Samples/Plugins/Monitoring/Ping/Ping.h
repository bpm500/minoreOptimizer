#ifdef PING_EXPORTS
#define PING_API extern "C" __declspec(dllexport)
#else
#define PING_API extern "C" __declspec(dllimport)
#endif