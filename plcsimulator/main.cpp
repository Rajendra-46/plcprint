#include <memory>

#include <QCommandLineParser>
#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTreeView>

#include "cliparser.h"
#include "plcmessage.h"
#include "signalhandlers.h"
#include "treemodel.h"
#include "workerthread.h"

int main(int argc, char* argv[])
{
    signalhandlers::installSignalHandlers();
    signal(SIGTERM, signalhandlers::quitqAppSignalhandler);

    Defaults defaults;
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QScopedPointer<QCoreApplication> app(new QCoreApplication(argc, argv));
    QCoreApplication::setApplicationName(defaults.APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(defaults.APPLICATION_VERSION);

    std::cout << "Starting " << defaults.APPLICATION_NAME.toStdString() << " "
              << defaults.APPLICATION_VERSION.toStdString() << std::endl;

    const Configuration configuration = CLIParser::parseCliOptions(defaults, *app);
    if (!configuration.cliMode)
    {
        app.reset();
        app.reset(new QApplication(argc, argv));
        qApp->setWindowIcon(QIcon(":/C_logo_48x48x32.ico"));
        qApp->setApplicationDisplayName("PLC Simulator " + libros::rosSoftwareVersion);
    }

    std::shared_ptr<PLCMessage> msg_rosToVideoserver = std::make_shared<PLCMessage>(
            configuration.db_rosToSever, "ROS->SERVER");
    msg_rosToVideoserver->setMessageCounter(configuration.msgCounterRos);

    std::shared_ptr<PLCMessage> msg_craneToVideoserver = std::make_shared<PLCMessage>(
            configuration.db_craneToServer, "CRANE->SERVER");
    msg_craneToVideoserver->setMessageCounter(configuration.msgCounterCrane);

    std::shared_ptr<PLCMessage> msg_yardToVideoserver = std::make_shared<PLCMessage>(
            configuration.db_yardToServer, "YARD->SERVER");
    msg_yardToVideoserver->setMessageCounter(configuration.msgCounterYard);

    if (!configuration.rosConfigAddress.isEmpty())
    {
        msg_rosToVideoserver->enableConfigServer(configuration.rosConfigAddress);
    }
    if (!configuration.craneConfigAddress.isEmpty())
    {
        msg_craneToVideoserver->enableConfigServer(configuration.craneConfigAddress);
    }
    if (!configuration.yardConfigAddress.isEmpty())
    {
        msg_yardToVideoserver->enableConfigServer(configuration.yardConfigAddress);
    }

    std::shared_ptr<TestLogger> logger = std::make_shared<TestLogger>(
            configuration.logFile, true, true);

    std::shared_ptr<PLCMessage> msg_videoserverToRos = std::make_shared<PLCMessage>(
            configuration.db_serverToRos, "SERVER->ROS");
    msg_videoserverToRos->enableLogging(logger, !configuration.fullLogging);
    msg_videoserverToRos->filterLogging(configuration.msgCounterRos);

    std::shared_ptr<PLCMessage> msg_videoserverToCrane = std::make_shared<PLCMessage>(
            configuration.db_serverToCrane, "SERVER->CRANE");
    msg_videoserverToCrane->enableLogging(logger, !configuration.fullLogging);
    msg_videoserverToCrane->filterLogging(configuration.msgCounterCrane);

    std::shared_ptr<PLCMessage> msg_videoserverToYard = std::make_shared<PLCMessage>(
            configuration.db_serverToYard, "SERVER->YARD");
    msg_videoserverToYard->enableLogging(logger, !configuration.fullLogging);
    msg_videoserverToYard->filterLogging(configuration.msgCounterYard);

    WorkerThread worker_ros(configuration.ip_videoServer,
            configuration.port_videoServerRos,
            msg_rosToVideoserver,
            configuration.messageIntervalOut,
            configuration.ip_ros,
            configuration.port_ros,
            msg_videoserverToRos);

    WorkerThread worker_crane(configuration.ip_videoServer,
            configuration.port_videoServerCrane,
            msg_craneToVideoserver,
            configuration.messageIntervalOut,
            configuration.ip_crane,
            configuration.port_crane,
            msg_videoserverToCrane);

    WorkerThread worker_yard(configuration.ip_videoServer,
            configuration.port_videoServerYard,
            msg_yardToVideoserver,
            configuration.messageIntervalOut,
            configuration.ip_yard,
            configuration.port_yard,
            msg_videoserverToYard);

    if (!configuration.userSettingsRosToServer.isEmpty())
    {
        msg_rosToVideoserver->addUserSettings(configuration.userSettingsRosToServer);
    }
    if (!configuration.userSettingsCraneToServer.isEmpty())
    {
        msg_craneToVideoserver->addUserSettings(configuration.userSettingsCraneToServer);
    }
    if (!configuration.userSettingsYardToServer.isEmpty())
    {
        msg_yardToVideoserver->addUserSettings(configuration.userSettingsYardToServer);
    }

    int retval = 1;
    if (!configuration.json.isEmpty())
    {
        if (configuration.json == "ros")
        {
            msg_rosToVideoserver->printDataStructureAsJson();
        }
        else if (configuration.json == "crane")
        {
            msg_craneToVideoserver->printDataStructureAsJson();
        }
        else if (configuration.json == "yard")
        {
            msg_yardToVideoserver->printDataStructureAsJson();
        }
        else
        {
            std::cout << "Invalid parameter.\n";
        }
    }

    if (configuration.cliMode)
    {
        worker_ros.startSending();
        worker_ros.startReceiving();

        worker_crane.startSending();
        worker_crane.startReceiving();

        worker_yard.startSending();
        worker_yard.startReceiving();

        retval = qApp->exec();
    }

    else
    {
        QObject::connect(
                qApp, &QApplication::lastWindowClosed, &worker_ros, &WorkerThread::stopReceiving);
        QObject::connect(qApp,
                &QApplication::lastWindowClosed,
                &worker_crane,
                &WorkerThread::stopReceiving);
        QObject::connect(qApp,
                &QApplication::lastWindowClosed,
                &worker_yard,
                &WorkerThread::stopReceiving);

        TreeModel model_rosToServer(msg_rosToVideoserver,
                TreeModel::ModelType::Sending,
                configuration.filterSpare,
                configuration.fuzzy,
                configuration.messageIntervalOut);
        TreeModel model_craneToServer(msg_craneToVideoserver,
                TreeModel::ModelType::Sending,
                configuration.filterSpare,
                configuration.fuzzy,
                configuration.messageIntervalOut);
        TreeModel model_yardToServer(msg_yardToVideoserver,
                TreeModel::ModelType::Sending,
                configuration.filterSpare,
                configuration.fuzzy,
                configuration.messageIntervalOut);

        TreeModel model_serverToRos(msg_videoserverToRos,
                TreeModel::ModelType::Receiving,
                configuration.filterSpare,
                false);
        TreeModel model_serverToCrane(msg_videoserverToCrane,
                TreeModel::ModelType::Receiving,
                configuration.filterSpare,
                false);
        TreeModel model_serverToYard(msg_videoserverToYard,
                TreeModel::ModelType::Receiving,
                configuration.filterSpare,
                false);

        QQmlApplicationEngine engine;

        engine.rootContext()->setContextProperty("model_rosToServer", &model_rosToServer);
        engine.rootContext()->setContextProperty("model_serverToRos", &model_serverToRos);

        engine.rootContext()->setContextProperty("model_craneToServer", &model_craneToServer);
        engine.rootContext()->setContextProperty("model_serverToCrane", &model_serverToCrane);

        engine.rootContext()->setContextProperty("model_yardToServer", &model_yardToServer);
        engine.rootContext()->setContextProperty("model_serverToYard", &model_serverToYard);

        engine.rootContext()->setContextProperty("worker_ros", &worker_ros);
        engine.rootContext()->setContextProperty("worker_crane", &worker_crane);
        engine.rootContext()->setContextProperty("worker_yard", &worker_yard);

        engine.rootContext()->setContextProperty("softwareVersion", configuration.softwareVersion);

        const QUrl url(QStringLiteral("qrc:/main.qml"));
        QObject::connect(
                &engine,
                &QQmlApplicationEngine::objectCreated,
                qApp,
                [url](QObject* obj, const QUrl& objUrl) {
                    if (!obj && url == objUrl)
                        QCoreApplication::exit(-1);
                },
                Qt::QueuedConnection);

        engine.load(url);

        retval = qApp->exec();
    }
    std::cout << "Exiting " << defaults.APPLICATION_NAME.toStdString() << " " << retval
              << std::endl;
    return retval;
}
