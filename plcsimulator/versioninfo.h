#pragma once

#include <QString>

namespace libros {

#ifdef ROS_SOFTWARE_VERSION
#define STR(x) #x
#define STRINGIFY(x) STR(x)
const QString tmpVersion(STRINGIFY(ROS_SOFTWARE_VERSION));
// Strip out the starting 'v' from tag-based version number
const QString rosSoftwareVersion = tmpVersion.startsWith('v') ? tmpVersion.mid(1) : tmpVersion;
#else
const QString rosSoftwareVersion = "undefined";
#endif

const QString softwareBuildTimestamp = QStringLiteral(__DATE__ " " __TIME__);

} // namespace libros
