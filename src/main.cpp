#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtDebug>
#include <QThread>
#include "SocketWorker.h"
#include "FrameProcessor.h"
#include <QQueue>
#include "Frame.h"
#include <processthreadsapi.h>

int main(int argc, char *argv[])
{
    int rv = 0;

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    QQueue<Frame*> frame_queue;

    QThread *threadSocket = new QThread();
    SocketWorker *socketWorker = new SocketWorker(&frame_queue);
    FrameProcessor *frameProcessor = new FrameProcessor(&frame_queue);

//    qmlRegisterType<FrameProcessor>("src.FrameProcessorr", 1, 0, "FrameProcessor");

    QQmlApplicationEngine engine;
    QQmlContext* ctx = engine.rootContext();
    ctx->setContextProperty("FrameProcessor", frameProcessor);
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    socketWorker->moveToThread(threadSocket);
    qDebug()<<"Main thread,"<< QThread::currentThreadId();

    // Connect signals and slots.
    QObject::connect(socketWorker, SIGNAL(checkLocalIP(QStringList)),
        frameProcessor, SLOT(refreshLocalIP(QStringList)));
    QObject::connect(frameProcessor, SIGNAL(openLocalServer(QString)),
        socketWorker, SLOT(openLocalServer(QString)));
    QObject::connect(frameProcessor, SIGNAL(closeLocalServer()),
        socketWorker, SLOT(closeLocalServer()));
    QObject::connect(frameProcessor, SIGNAL(sendFrame(QString, Frame*)),
        socketWorker, SLOT(sendFrame(QString, Frame *)));

    QObject::connect(frameProcessor, SIGNAL(autoSaveData()),
        socketWorker, SLOT(autoSaveData()));
    QObject::connect(frameProcessor, SIGNAL(stopSaveData()),
        socketWorker, SLOT(stopSaveData()));
    QObject::connect(frameProcessor, SIGNAL(updateChannelCountsX(QVector<int>*)),
        socketWorker, SLOT(updateChannelCountsX(QVector<int>*)));
    QObject::connect(frameProcessor, SIGNAL(updateChannelCountsY(QVector<int>*)),
        socketWorker, SLOT(updateChannelCountsY(QVector<int>*)));

    QObject::connect(socketWorker, SIGNAL(frameReceived(QString, Frame*)),
        frameProcessor, SLOT(FrameIncoming(QString, Frame*)));

    threadSocket->start();

    socketWorker->updateLocalIP();

    rv = app.exec();
    threadSocket->quit();
    threadSocket->wait();

    delete threadSocket;
    qDebug() << "Delete Socket Thread";
    delete socketWorker;
    qDebug() << "Delete Socket SocketWorker";

    qDebug() << "End Application";

    return 0;
}
