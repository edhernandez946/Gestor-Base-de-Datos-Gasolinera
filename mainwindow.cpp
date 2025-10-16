#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtSql/QSqlError>
#include <QDebug>
#include <QtSql/QSqlQuery>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , crudWindow(nullptr)
    , consultasWindow(nullptr)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    conectarBD(); //Llamada a la conexion al crear la ventana
}

MainWindow::~MainWindow()
{
    if (db.isOpen()) {
        db.close();
    }
    delete ui;
}

void MainWindow::conectarBD()
{

    db = QSqlDatabase::addDatabase("QMYSQL", "miConexion");
    db.setHostName("127.0.0.1");
    db.setPort(3307);
    db.setDatabaseName("Gasolinera");
    db.setUserName("root");
    db.setPassword("Gabriel200!");

    if (!db.open()) {
        qDebug() << "Error de conexión:" << db.lastError().text();
    } else {
        qDebug() << "Conectado a MySQL correctamente!";
    }
}


void MainWindow::on_botonCrud_clicked()
{
    if (!crudWindow) crudWindow = new Crud(db);
    crudWindow->show();
    crudWindow->raise();
    crudWindow->activateWindow();
}


void MainWindow::on_botonConsultas_clicked()
{
    if (!consultasWindow) consultasWindow = new Consultas(db);
    consultasWindow->show();
    consultasWindow->raise();
    consultasWindow->activateWindow();
}
