#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QDebug>

void crearProxy(QCoreApplication *app) {

    //consigue el directorio del exe del programa
    QString exeDir = QCoreApplication::applicationDirPath();
    QDir dir(exeDir);

    //entra al folder conectarDB y apunta al bat
    dir.cd("conectarDB");
    QString batFile = dir.filePath("conectar-db");

    //revisa si el puerto 3307 ya esta ocupado (puerto donde se conecta al DB)
    QProcess checkPort;
    checkPort.start("netstat", {"-ano"});
    checkPort.waitForFinished();
    QString output = checkPort.readAllStandardOutput();
    if (output.contains("3307")) {
        qDebug() << "Proxy ya conectado";
        return; //si ya hay proxy corriendo sale del metodo
    }

    //crea el proceso para correr el bat
    QProcess *proxyProcess = new QProcess(app);
    proxyProcess->start("cmd.exe", {"/c", batFile});

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    crearProxy(&a); //le pasa el qapplication al metodo

    MainWindow w;
    w.show();
    return a.exec();
}
