#include "cliparser.h"

Configuration CLIParser::parseCliOptions(Defaults defaults, QCoreApplication& app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("\nSiemens S7 PLC Message Simulator."
                                     "\n \nDefault values in braces.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption userSettingsRosOption(QStringList() << "file-ros",
            QString("User Setting Json for ROS to Video Server"),
            "file");

    parser.addOption(userSettingsRosOption);

    QCommandLineOption userSettingsCraneOption(QStringList() << "file-crane",
            QString("User Setting Json for Crane to Video Server"),
            "file");

    parser.addOption(userSettingsCraneOption);

    QCommandLineOption userSettingsYardOption(QStringList() << "file-yard",
            QString("User Setting Json for Yard to Video Server"),
            "file");

    parser.addOption(userSettingsYardOption);

    QCommandLineOption cliModeOption(QStringList() << "c"
                                                   << "cli",
            "Start in command line mode");

    parser.addOption(cliModeOption);

    QCommandLineOption printJsonOption(QStringList() << "j"
                                                     << "json",
            "Print user settings template",
            "ros|crane|yard");
    parser.addOption(printJsonOption);

    QCommandLineOption fuzzyOption(QStringList() << "fuzzy", "Enable fuzzy in GUI by default.");
    parser.addOption(fuzzyOption);

    QCommandLineOption filterSpareOption(
            QStringList() << "filter", QString("Filter SPARE_* values from displayed values"));
    parser.addOption(filterSpareOption);

    QCommandLineOption fullLoggingOption(QStringList() << "full-logging", "Enable full logging");
    parser.addOption(fullLoggingOption);

    QCommandLineOption logFileOption(QStringList() << "logfile",
            QString("File for logs (%0)").arg(defaults.DEFAULT_LOG_FILE),
            "string");
    parser.addOption(logFileOption);

    QCommandLineOption videoserverIpOption(QStringList() << "ip-videoserver",
            QString("IP of Video Server (%0)").arg(defaults.DEFAULT_IP),
            "ip");

    parser.addOption(videoserverIpOption);

    QCommandLineOption videoserverPortRosOption(QStringList() << "port-videoserver-ros",
            QString("ROS PLC Port on Video Server (%0)")
                    .arg(defaults.DEFAULT_ROS_PORT_ON_VIDEOSERVER),
            "port");

    parser.addOption(videoserverPortRosOption);

    QCommandLineOption videoserverPortCraneOption(QStringList() << "port-videoserver-crane",
            QString("Crane PLC Port on Video Server (%0)")
                    .arg(defaults.DEFAULT_CRANE_PORT_ON_VIDEOSERVER),
            "port");

    parser.addOption(videoserverPortCraneOption);

    QCommandLineOption videoserverPortYardOption(QStringList() << "port-videoserver-yard",
            QString("Yard PLC Port on Video Server (%0)")
                    .arg(defaults.DEFAULT_YARD_PORT_ON_VIDEOSERVER),
            "port");

    parser.addOption(videoserverPortYardOption);

    QCommandLineOption rosIpOption(QStringList() << "ip-ros",
            QString("IP of simulated ROS PLC (%0)").arg(defaults.DEFAULT_IP),
            "ip");
    parser.addOption(rosIpOption);

    QCommandLineOption craneIpOption(QStringList() << "ip-crane",
            QString("IP of simulated Crane PLC (%0)").arg(defaults.DEFAULT_IP),
            "ip");
    parser.addOption(craneIpOption);

    QCommandLineOption yardIpOption(QStringList() << "ip-yard",
            QString("IP of simulated Yard PLC (%0)").arg(defaults.DEFAULT_IP),
            "ip");
    parser.addOption(yardIpOption);

    QCommandLineOption rosPortOption(QStringList() << "port-ros",
            QString("Port of simulated ROS PLC (%0)").arg(defaults.DEFAULT_PORT_ROS_PLC),
            "port");
    parser.addOption(rosPortOption);

    QCommandLineOption cranePortOption(QStringList() << "port-crane",
            QString("Port of simulated Crane PLC (%0)").arg(defaults.DEFAULT_PORT_CRANE_PLC),
            "port");
    parser.addOption(cranePortOption);

    QCommandLineOption yardPortOption(QStringList() << "port-yard",
            QString("Port of simulated Yard PLC (%0)").arg(defaults.DEFAULT_PORT_YARD_PLC),
            "port");

    parser.addOption(yardPortOption);

    QCommandLineOption intervalOutOption(QStringList() << "interval-out",
            QString("Interval of sending messages (%0 ms)").arg(defaults.DEFAULT_INTERVAL_OUT),
            "interval in ms");
    parser.addOption(intervalOutOption);

    QCommandLineOption dbRosToServerOption(QStringList() << "db-out-ros",
            QString("Datablock PLC TO VIDEOSERVER (%0)")
                    .arg(defaults.DEFAULT_DB_FILE_ROS_TO_VIDEOSERVER),
            ".AWL file");
    parser.addOption(dbRosToServerOption);

    QCommandLineOption dbServerToPlcOption(QStringList() << "db-in-ros",
            QString("Datablock VIDEOSERVER TO PLC (%0)")
                    .arg(defaults.DEFAULT_DB_FILE_VIDEOSERVER_TO_ROS),
            ".AWL file");
    parser.addOption(dbServerToPlcOption);

    QCommandLineOption dbCraneToServerOption(QStringList() << "db-out-crane",
            QString("Datablock CRANE TO VIDEOSERVER (%0)")
                    .arg(defaults.DEFAULT_DB_FILE_CRANE_TO_VIDEOSERVER),
            ".AWL file");
    parser.addOption(dbCraneToServerOption);

    QCommandLineOption dbServerToCraneOption(QStringList() << "db-in-crane",
            QString("Datablock VIDEOSERVER TO CRANE (%0)")
                    .arg(defaults.DEFAULT_DB_FILE_VIDEOSERVER_TO_CRANE),
            ".AWL file");
    parser.addOption(dbServerToCraneOption);

    QCommandLineOption dbYardToServerOption(QStringList() << "db-out-yard",
            QString("Datablock YARD TO VIDEOSERVER (%0)")
                    .arg(defaults.DEFAULT_DB_FILE_YARD_TO_VIDEOSERVER),
            ".AWL file");
    parser.addOption(dbYardToServerOption);

    QCommandLineOption dbServerToYardOption(QStringList() << "db-in-yard",
            QString("Datablock VIDEOSERVER TO YARD (%0)")
                    .arg(defaults.DEFAULT_DB_FILE_VIDEOSERVER_TO_YARD),
            ".AWL file");

    parser.addOption(dbServerToYardOption);

    QCommandLineOption msgCounterRosOption(QStringList() << "msgcounter-ros",
            QString("Counter tag in db-out-ros (%0)").arg(defaults.DEFAULT_MSG_COUNTER_TAG),
            "string");
    parser.addOption(msgCounterRosOption);

    QCommandLineOption msgCounterCraneOption(QStringList() << "msgcounter-crane",
            QString("Counter tag in db-out-crane (%0)").arg(defaults.DEFAULT_MSG_COUNTER_TAG),
            "string");
    parser.addOption(msgCounterCraneOption);

    QCommandLineOption msgCounterYardOption(QStringList() << "msgcounter-yard",
            QString("Counter tag in db-out-yard (%0)").arg(defaults.DEFAULT_MSG_COUNTER_TAG),
            "string");
    parser.addOption(msgCounterYardOption);

    QCommandLineOption configServerRosOption(QStringList() << "config-server-ros",
            QString("Address for ros runtime configuration interface (%0)")
                    .arg(defaults.DEFAULT_ROS_CONFIG_ADDRESS),
            "address");
    parser.addOption(configServerRosOption);

    QCommandLineOption configServerCraneOption(QStringList() << "config-server-crane",
            QString("Address for crane runtime configuration interface (%0)")
                    .arg(defaults.DEFAULT_CRANE_CONFIG_ADDRESS),
            "address");
    parser.addOption(configServerCraneOption);

    QCommandLineOption configServerYardOption(QStringList() << "config-server-yard",
            QString("Address for yard runtime configuration interface (%0)")
                    .arg(defaults.DEFAULT_YARD_CONFIG_ADDRESS),
            "address");
    parser.addOption(configServerYardOption);

    parser.process(app);

    const auto userSettingsRosToServer = parser.isSet(userSettingsRosOption)
            ? parser.value(userSettingsRosOption)
            : QString();
    const auto userSettingsCraneToServer = parser.isSet(userSettingsCraneOption)
            ? parser.value(userSettingsCraneOption)
            : QString();
    const auto userSettingsYardToServer = parser.isSet(userSettingsYardOption)
            ? parser.value(userSettingsYardOption)
            : QString();

    const auto cliMode = parser.isSet(cliModeOption);
    const auto json = parser.isSet(printJsonOption) ? parser.value(printJsonOption) : QString();
    const auto fuzzy = parser.isSet(fuzzyOption) ? true : false;
    const auto filterSpare = parser.isSet(filterSpareOption) ? true : false;
    const auto fullLogging = parser.isSet(fullLoggingOption);

    const auto logFile = parser.isSet(logFileOption) ? parser.value(logFileOption)
                                                     : defaults.DEFAULT_LOG_FILE;

    const auto db_rosToSever = parser.isSet(dbRosToServerOption)
            ? parser.value(dbRosToServerOption)
            : defaults.DEFAULT_DB_FILE_ROS_TO_VIDEOSERVER;

    const auto db_serverToRos = parser.isSet(dbServerToPlcOption)
            ? parser.value(dbServerToPlcOption)
            : defaults.DEFAULT_DB_FILE_VIDEOSERVER_TO_ROS;

    const auto db_craneToServer = parser.isSet(dbCraneToServerOption)
            ? parser.value(dbCraneToServerOption)
            : defaults.DEFAULT_DB_FILE_CRANE_TO_VIDEOSERVER;

    const auto db_serverToCrane = parser.isSet(dbServerToCraneOption)
            ? parser.value(dbServerToCraneOption)
            : defaults.DEFAULT_DB_FILE_VIDEOSERVER_TO_CRANE;

    const auto db_yardToServer = parser.isSet(dbYardToServerOption)
            ? parser.value(dbYardToServerOption)
            : defaults.DEFAULT_DB_FILE_YARD_TO_VIDEOSERVER;

    const auto db_serverToYard = parser.isSet(dbServerToYardOption)
            ? parser.value(dbServerToYardOption)
            : defaults.DEFAULT_DB_FILE_VIDEOSERVER_TO_YARD;

    const auto ip_videoServer = parser.isSet(videoserverIpOption)
            ? parser.value(videoserverIpOption)
            : defaults.DEFAULT_IP;

    const auto port_videoServerRos = parser.isSet(videoserverPortRosOption)
            ? parser.value(videoserverPortRosOption)
            : defaults.DEFAULT_ROS_PORT_ON_VIDEOSERVER;

    const auto port_videoServerCrane = parser.isSet(videoserverPortCraneOption)
            ? parser.value(videoserverPortCraneOption)
            : defaults.DEFAULT_CRANE_PORT_ON_VIDEOSERVER;

    const auto port_videoServerYard = parser.isSet(videoserverPortYardOption)
            ? parser.value(videoserverPortYardOption)
            : defaults.DEFAULT_YARD_PORT_ON_VIDEOSERVER;

    const auto ip_ros = parser.isSet(rosIpOption) ? parser.value(rosIpOption) : defaults.DEFAULT_IP;

    const auto port_ros = parser.isSet(rosPortOption) ? parser.value(rosPortOption)
                                                      : defaults.DEFAULT_PORT_ROS_PLC;

    const auto ip_crane = parser.isSet(craneIpOption) ? parser.value(craneIpOption)
                                                      : defaults.DEFAULT_IP;

    const auto port_crane = parser.isSet(cranePortOption) ? parser.value(cranePortOption)
                                                          : defaults.DEFAULT_PORT_CRANE_PLC;

    const auto ip_yard = parser.isSet(yardIpOption) ? parser.value(yardIpOption)
                                                    : defaults.DEFAULT_IP;

    const auto port_yard = parser.isSet(yardPortOption) ? parser.value(yardPortOption)
                                                        : defaults.DEFAULT_PORT_YARD_PLC;

    const auto messageIntervalOut = parser.value(intervalOutOption).toUInt() == 0
            ? defaults.DEFAULT_INTERVAL_OUT
            : parser.value(intervalOutOption).toUInt();

    const auto msgCounterRos = parser.isSet(msgCounterRosOption) ? parser.value(msgCounterRosOption)
                                                                 : defaults.DEFAULT_MSG_COUNTER_TAG;
    const auto msgCounterCrane = parser.isSet(msgCounterCraneOption)
            ? parser.value(msgCounterCraneOption)
            : defaults.DEFAULT_MSG_COUNTER_TAG;

    const auto msgCounterYard = parser.isSet(msgCounterYardOption)
            ? parser.value(msgCounterYardOption)
            : defaults.DEFAULT_MSG_COUNTER_TAG;
    const auto configAddressRos = parser.isSet(configServerRosOption)
            ? parser.value(configServerRosOption)
            : defaults.DEFAULT_ROS_CONFIG_ADDRESS;
    const auto configAddressCrane = parser.isSet(configServerCraneOption)
            ? parser.value(configServerCraneOption)
            : defaults.DEFAULT_CRANE_CONFIG_ADDRESS;
    const auto configAddressYard = parser.isSet(configServerYardOption)
            ? parser.value(configServerYardOption)
            : defaults.DEFAULT_YARD_CONFIG_ADDRESS;

    const auto version = libros::rosSoftwareVersion != "" ? libros::rosSoftwareVersion : "unknown";

    Configuration const configuration{userSettingsRosToServer,
            userSettingsCraneToServer,
            userSettingsYardToServer,
            cliMode,
            json,
            fullLogging,
            logFile,
            db_rosToSever,
            db_serverToRos,
            db_craneToServer,
            db_serverToCrane,
            db_yardToServer,
            db_serverToYard,
            ip_videoServer,
            port_videoServerRos,
            port_videoServerCrane,
            port_videoServerYard,
            ip_ros,
            port_ros,
            ip_crane,
            port_crane,
            ip_yard,
            port_yard,
            messageIntervalOut,
            msgCounterRos,
            msgCounterCrane,
            msgCounterYard,
            configAddressRos,
            configAddressCrane,
            configAddressYard,
            fuzzy,
            filterSpare,
            version};

    return configuration;
}
