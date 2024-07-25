#pragma once

#include <stdexcept>
#include <QString>

namespace libros {

// Exception triggered by problems during program startup,
// e.g. malformed configuration files. Unlike other errors,
// should not cause a stacktrace to be saved to disk.
class StartupException : public std::runtime_error
{
public:
    StartupException(const QString& error) : std::runtime_error(error.toStdString())
    {
    }
};

} // namespace libros
