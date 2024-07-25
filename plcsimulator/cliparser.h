#pragma once
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QString>

#include <versioninfo.h>

struct Configuration
{
    QString userSettingsRosToServer;
    QString userSettingsCraneToServer;
    QString userSettingsYardToServer;
    bool cliMode;
    QString json;
    bool fullLogging;
    QString logFile;
    QString db_rosToSever;
    QString db_serverToRos;
    QString db_craneToServer;
    QString db_serverToCrane;
    QString db_yardToServer;
    QString db_serverToYard;
    QString ip_videoServer;
    QString port_videoServerRos;
    QString port_videoServerCrane;
    QString port_videoServerYard;
    QString ip_ros;
    QString port_ros;
    QString ip_crane;
    QString port_crane;
    QString ip_yard;
    QString port_yard;
    uint messageIntervalOut;
    QString msgCounterRos;
    QString msgCounterCrane;
    QString msgCounterYard;
    QString rosConfigAddress;
    QString craneConfigAddress;
    QString yardConfigAddress;
    bool fuzzy;
    bool filterSpare;
    QString softwareVersion;
};

struct Defaults
{
    const QString APPLICATION_NAME = "PLCSimul";
    const QString APPLICATION_VERSION = libros::rosSoftwareVersion;
    const QString DEFAULT_IP = "127.0.0.1";
    const QString DEFAULT_ROS_PORT_ON_VIDEOSERVER = "2050";
    const QString DEFAULT_CRANE_PORT_ON_VIDEOSERVER = "2051";
    const QString DEFAULT_YARD_PORT_ON_VIDEOSERVER = "2052";
    const QString DEFAULT_PORT_ROS_PLC = "2000";
    const QString DEFAULT_PORT_CRANE_PLC = "2001";
    const QString DEFAULT_PORT_YARD_PLC = "2002";
    const QString DEFAULT_DB_FILE_ROS_TO_VIDEOSERVER = ":/DB311_ROS_PLC_to_AV_ROS.db"; //
    const QString DEFAULT_DB_FILE_VIDEOSERVER_TO_ROS = ":/DB344_AV_to_ROS_PLC_ROS.db"; //
    const QString DEFAULT_DB_FILE_CRANE_TO_VIDEOSERVER = ":/DB312_ROS_PLC_to_AV_Crane.db"; //
    const QString DEFAULT_DB_FILE_VIDEOSERVER_TO_CRANE = ":/DB345_AV_to_ROS_PLC_Crane.db"; //
    const QString DEFAULT_DB_FILE_YARD_TO_VIDEOSERVER = ":/DB313_YARD_PLC_to_AV_Yard.db"; //
    const QString DEFAULT_DB_FILE_VIDEOSERVER_TO_YARD = ":/DB346_AV_to_YARD_PLC_Yard.db"; //
    const QString DEFAULT_MSG_COUNTER_TAG = "WATCHDOG";
    const uint DEFAULT_INTERVAL_OUT = 50;
    const QString DEFAULT_LOG_FILE = "plcsimul.log";
    const QString DEFAULT_ROS_CONFIG_ADDRESS = "127.0.0.1:12000";
    const QString DEFAULT_CRANE_CONFIG_ADDRESS = "127.0.0.1:12001";
    const QString DEFAULT_YARD_CONFIG_ADDRESS = "127.0.0.1:12002";
};

class CLIParser
{
public:
    CLIParser();
    static Configuration parseCliOptions(Defaults defaults, QCoreApplication& app);
};
