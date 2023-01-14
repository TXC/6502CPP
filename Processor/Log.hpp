#include <cstdio>
#include <string>

namespace CPU
{
#ifdef LOGMODE
    static FILE* logfile = nullptr;
#endif

    void log(std::string text);

};
