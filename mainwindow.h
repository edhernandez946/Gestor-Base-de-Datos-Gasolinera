#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QStandardItemModel>
#include <QTabWidget>
#include "crud.h"
#include "consultas.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void conectarBD();

private slots:
    void on_botonCrud_clicked();
    void on_botonConsultas_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;    //atributo de conexion

    Crud *crudWindow;
    Consultas *consultasWindow;
};

#endif // MAINWINDOW_H
