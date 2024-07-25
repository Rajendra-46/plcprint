#pragma once

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>

#ifdef linux
#include <cstdio>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#define CRASH_LOG_OUTPUT_DIRECTORY "/var/log/kcav"
#endif

class signalhandlers
{
public:
    inline static void quitqAppSignalhandler(int sig)
    {
        Q_UNUSED(sig)
#ifdef linux
        fprintf(stderr, "Shutting down (signal %d)\n", sig);
        // Try to exit QApplication gracefully
        if (qApp)
        {
            // Remove own handling for signal to prevent crash loop
            signal(sig, SIG_DFL);
            qApp->quit();
        }
        else
        {
            exit(0);
        }
#endif
    }

    inline static void stackTraceSignalHandler(int sig)
    {
        Q_UNUSED(sig)
#ifdef linux
        void* array[50];
        int size;

        // Get void*'s for all entries on the stack
        size = backtrace(array, sizeof(array) / sizeof(void*));

        // Print out all the frames to stderr
        fprintf(stderr, "Error: crash with signal \"%s\" (%d):\n", strsignal(sig), sig);
        backtrace_symbols_fd(array, size, STDERR_FILENO);

        // log backtrace to file
        writeCrashLog(sig, array, size);

        // Remove own handling for signal and reraise it
        signal(sig, SIG_DFL);
        raise(sig);
#endif
    }

    inline static bool installSignalHandlers()
    {
#ifdef linux
        if (signal(SIGINT, quitqAppSignalhandler) == SIG_ERR)
            return false;
        if (signal(SIGSEGV, stackTraceSignalHandler) == SIG_ERR)
            return false;
        if (signal(SIGFPE, stackTraceSignalHandler) == SIG_ERR)
            return false;
        if (signal(SIGABRT, stackTraceSignalHandler) == SIG_ERR)
            return false;
        if (signal(SIGBUS, stackTraceSignalHandler) == SIG_ERR)
            return false;
        if (signal(SIGILL, stackTraceSignalHandler) == SIG_ERR)
            return false;
        if (signal(SIGSYS, stackTraceSignalHandler) == SIG_ERR)
            return false;
#endif
        return true;
    }

#ifdef linux
    inline static void writeCrashLog(int sig, void* backtraceArray[], int size)
    {
        static const char* destdir = CRASH_LOG_OUTPUT_DIRECTORY;
        QString fname = QString("crashlog_%1_%2.log")
                                .arg(QDateTime::currentDateTime().toString("yyyy-MM-ddTHH.mm.ss"))
                                .arg(QCoreApplication::applicationName());
        QString fpath = QDir(destdir).filePath(fname);

        FILE* file = fopen(fpath.toLocal8Bit().constData(), "w");
        if (!file)
            fprintf(stderr, "Error: cannot create file \"%s\"\n", fpath.toLocal8Bit().constData());
        else
        {
            backtrace_symbols_fd(backtraceArray, size, fileno(file));
            fprintf(file, "\n\n Error: crash with signal \"%s\" (%d):\n", strsignal(sig), sig);
            fclose(file);
        }
    }
#endif
};
