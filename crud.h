#ifndef CRUD_H
#define CRUD_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QWidget>

namespace Ui {
class Crud;
}

class Crud : public QWidget
{
    Q_OBJECT

public:
    explicit Crud(QSqlDatabase &db, QWidget *parent = nullptr);
    ~Crud();
    void cargarEstaciones(QComboBox* comboBox);
    void actualizarPrecio(QComboBox* comboBox);
    void cargarSurtidores(QComboBox* comboBox,int idEstacion);
    void cargarCombustibles(QComboBox* comboBox);
    void cargarSurtidores(QComboBox* comboBox, int idEstacion, int idCombustible);
    void cargarMediosPago(QComboBox* comboBox);
    void cargarComboBox(QComboBox* comboBox, const QString& tabla, const QString& columnaId, const QString& columnaNombre);
    void cargarFechas();
    void actualizarPrecio();
    void actualizarPrecio(QComboBox* comboEstacion,QComboBox* comboCombustible,QLabel* labelDestino);
    void cargarTiposCombustibleDeEstacion(QComboBox* comboBox, int idEstacion);
    void actualizarPrecioTotal( );
    int obtenerIdManguera();
    void limitarFechasDateTimeEdit(QDateTimeEdit *dateTimeEdit);
    void limitarFechasDateTimeEdit();
    void limitarFechasDateTimeEdit2();
    void configurarMenuVentas();
    void configurarMenuEntregas();
    void configurarMenuPrecios();
    void configurarMenuLecturas();
    void CargarFechasPrecio();



private slots:
    void on_Boton_RegistroVenta_clicked();
    void on_Boton_RegistroEntrega_clicked();
    void on_Boton_RegistroPrecio_clicked();

    void on_Boton_RegistroLecturaTanque_clicked();

    void on_Boton_RegistroLecturaMedidor_clicked();

private:
    Ui::Crud *ui;
    QSqlDatabase db;
};

#endif // CRUD_H
