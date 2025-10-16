#ifndef CONSULTAS_H
#define CONSULTAS_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QWidget>

namespace Ui {
class Consultas;
}

class Consultas : public QWidget
{
    Q_OBJECT

public:
    explicit Consultas(QSqlDatabase &db, QWidget *parent = nullptr);
    ~Consultas();
    void cargarComboBox(QComboBox* comboBox, const QString& tabla, const QString& columnaId, const QString& columnaNombre);
    void limitarFechasDateTimeEdit(QDateTimeEdit *dateTimeEdit);
    void cargarTiposCombustibleDeEstacion(QComboBox* comboBox, int idEstacion);
    void configurarConsulta_1();
    void configurarConsulta_2();
    void configurarConsulta_3();
    void configurarConsulta_4();
    void configurarConsulta_5();
    void configurarConsulta_6();
    void ConfigurarTablasConsultas();

    void ConfigurarFecha();

private slots:
    void on_BotonConsultar_Page1_clicked();
    void on_BotonConsultar_Page2_clicked();
    void on_BotonConsultar_Page3_clicked();
    void on_BotonConsultar_Page5_clicked();


    void on_BotonConsultar_Page4_clicked();

    void on_BotonConsultar_Page6_clicked();

private:
    Ui::Consultas *ui;
    QStandardItemModel* model1;
    QStandardItemModel* model2;
    QStandardItemModel* model4;
    QStandardItemModel* model5;
    QStandardItemModel* model6;

    QSqlDatabase db;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // CONSULTAS_H
