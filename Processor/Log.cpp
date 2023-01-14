#include <cstdio>
#include "Log.hpp"

#ifndef LOGFILE
#define LOGFILE "ProcessorLog.txt"
#endif

// Emulation Behaviour Logging ======================================
// Uncomment this to create a logfile entry for each clock tick of 
// the CPU. Beware: this slows down emulation considerably and
// generates extremely large files. I recommend "glogg" to view the
// data as it is designed to handle enormous files.

namespace CPU
{
    void log(std::string text)
    {
    #ifdef LOGMODE
        if (logfile == nullptr)
        {
            logfile = fopen(LOGFILE, "a+");
        }
        if (logfile != nullptr)
        {
            fprintf(logfile, "%s\n", text.c_str());
            fflush(logfile);
        }
    #endif
    }

};
