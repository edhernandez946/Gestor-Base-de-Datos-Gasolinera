#include "crud.h"
#include "ui_crud.h"
#include <QtSql/QSqlError>
#include <QDebug>
#include <QtSql/QSqlQuery>
#include <QMessageBox>

Crud::Crud(QSqlDatabase &db, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Crud),
    db(db)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    configurarMenuVentas();
    configurarMenuEntregas();
    configurarMenuPrecios();
    configurarMenuLecturas();

    ui->dateTimeEditTurno->setCalendarPopup(true);

    srand(time(nullptr));

    connect(ui->dateTimeEditTurno, &QDateTimeEdit::dateTimeChanged,this, [this](const QDateTime &){this->actualizarPrecio();});


}

Crud::~Crud()
{
    delete ui;
}

void Crud::cargarComboBox(QComboBox* comboBox, const QString& tabla, const QString& columnaId, const QString& columnaNombre)
{
    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    QSqlQuery query(db);
    QString sql = QString("SELECT %1, %2 FROM %3").arg(columnaId, columnaNombre, tabla);

    if (!query.exec(sql)) {
        qDebug() << "Error al obtener datos de" << tabla << ":" << query.lastError().text();
        return;
    }

    comboBox->clear();
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        comboBox->addItem(nombre, id);
    }
    comboBox->setCurrentIndex(-1);
    qDebug() << "ComboBox cargado desde" << tabla;
}

void Crud::cargarTiposCombustibleDeEstacion(QComboBox* comboBox, int idEstacion)
{
    comboBox->clear();
    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    //une tanque y tipocombustible filtrando por estación
    QSqlQuery query(db);
    query.prepare("SELECT DISTINCT tc.idTipoCombustible, tc.nombre "
                  "FROM Tanque t "
                  "JOIN TipoCombustible tc ON t.idTipoCombustible = tc.idTipoCombustible "
                  "WHERE t.idEstacion = :idEstacion");
    query.bindValue(":idEstacion", idEstacion);

    if (!query.exec()) {
        qDebug() << "Error al obtener tipos de combustible de la estación:" << query.lastError().text();
        return;
    }

    //llena el combo box
    while (query.next()) {
        int idTipoCombustible = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        comboBox->addItem(nombre, idTipoCombustible);
    }

    qDebug() << "Combo de combustibles cargado para estación" << idEstacion;
}

void Crud::actualizarPrecio()
{
    int idEstacion = ui->comboBox_Venta_Estacion->currentData().toInt();
    int idCombustible = ui->comboBox_Venta_Combustible->currentData().toInt();
    QDateTime fechaHora = ui->dateTimeEditTurno->dateTime();

    if (!db.isOpen()) {
        ui->Precio_Label->setText("L0.00");
        return;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT precio
        FROM PrecioCombustible
        WHERE idEstacion = :idEstacion
          AND idTipoCombustible = :idCombustible
          AND fechaInicio <= :fechaHora
          AND (fechaFin IS NULL OR fechaFin > :fechaHora)
        ORDER BY fechaInicio DESC
        LIMIT 1
    )");
    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);
    query.bindValue(":fechaHora", fechaHora);

    if (!query.exec()) {
        ui->Precio_Label->setText("L0.00");
        return;
    }

    if (query.next()) {
        double precio = query.value(0).toDouble();
        ui->Precio_Label->setText("L" + QString::number(precio, 'f', 2));
    } else {
        // No hay precio para esa fecha, usamos el más reciente
        QSqlQuery queryUltimo(db);
        queryUltimo.prepare(R"(
            SELECT precio
            FROM PrecioCombustible
            WHERE idEstacion = :idEstacion
              AND idTipoCombustible = :idCombustible
            ORDER BY fechaInicio DESC
            LIMIT 1
        )");
        queryUltimo.bindValue(":idEstacion", idEstacion);
        queryUltimo.bindValue(":idCombustible", idCombustible);

        if (queryUltimo.exec() && queryUltimo.next()) {
            double precio = queryUltimo.value(0).toDouble();
            ui->Precio_Label->setText("L" + QString::number(precio, 'f', 2));
        } else {
            ui->Precio_Label->setText("L0.00");
        }
    }
}

void Crud::cargarSurtidores(QComboBox* comboBox, int idEstacion, int idCombustible)
{
    if (!db.isOpen()) {
        qDebug() << "La base de datos no está abierta";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT s.idSurtidor, s.numeroSurtidor "
                  "FROM Surtidor s "
                  "JOIN Manguera m ON m.idSurtidor = s.idSurtidor "
                  "WHERE s.idEstacion = :idEstacion "
                  "AND m.idTipoCombustible = :idCombustible "
                  "ORDER BY s.numeroSurtidor ASC");
    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);

    if (!query.exec()) {
        qDebug() << "Error al obtener surtidores:" << query.lastError().text();
        return;
    }

    comboBox->clear();
    while (query.next()) {
        int idSurtidor = query.value(0).toInt();
        int numeroSurtidor = query.value(1).toInt();
        comboBox->addItem(QString::number(numeroSurtidor), idSurtidor);
    }
    qDebug() << "Combo de surtidores cargado correctamente";
}


void Crud::cargarSurtidores(QComboBox* comboBox, int idEstacion)
{
    if (!db.isOpen()) {
        qDebug() << "La base de datos no está abierta";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT idSurtidor, numeroSurtidor FROM Surtidor "
                  "WHERE idEstacion = :idEstacion "
                  "ORDER BY numeroSurtidor ASC");
    query.bindValue(":idEstacion", idEstacion);

    if (!query.exec()) {
        qDebug() << "Error al obtener surtidores:" << query.lastError().text();
        return;
    }

    comboBox->clear();
    while (query.next()) {
        int idSurtidor = query.value(0).toInt();
        int numeroSurtidor = query.value(1).toInt();
        comboBox->addItem(QString::number(numeroSurtidor), idSurtidor);
    }
    qDebug() << "Combo de surtidores cargado correctamente";
}
void Crud::actualizarPrecio(QComboBox* comboEstacion, QComboBox* comboCombustible, QLabel* labelDestino)
{
    int idEstacion = comboEstacion->currentData().toInt();
    int idCombustible = comboCombustible->currentData().toInt();

    qDebug() << "Actualizar precio para estación ID:" << idEstacion
             << "y combustible ID:" << idCombustible;

    if (!db.isOpen()) {
        qDebug() << "La base de datos no está abierta";
        labelDestino->setText("L0.00");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT precio FROM PrecioCombustible "
                  "WHERE idEstacion = :idEstacion "
                  "AND idTipoCombustible = :idCombustible "
                  "ORDER BY idPrecio DESC LIMIT 1");

    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);

    if (!query.exec()) {
        qDebug() << "Error al obtener precio:" << query.lastError().text();
        labelDestino->setText("L0.00");
        return;
    }

    if (query.next()) {
        double precio = query.value(0).toDouble();
        qDebug() << "Último precio registrado:" << precio;

        QString textoPrecio = "L" + QString::number(precio, 'f', 2);
        labelDestino->setText(textoPrecio);
    } else {
        qDebug() << "No hay precio registrado para esta estación y combustible.";
        labelDestino->setText("L0.00");
    }
}

void Crud::actualizarPrecioTotal(){
    {
        // Obtener litros desde el spinbox
        double litros = ui->spinBox_Venta_Litros->value();

        // Obtener precio desde el label
        QString p=ui->Precio_Label->text();
        QString sinMoneda = p.mid(1);
        double precio = sinMoneda.toDouble();

        // Calcular total
        double total = litros * precio;

        // mostrar el resultado en el label de precio total
        ui->PrecioTotal_Label->setText("L" + QString::number(total, 'f', 2));
        qDebug() << total;
    }
}

int Crud::obtenerIdManguera()
{
    int idSurtidor = ui->comboBox_Venta_Surtidor->currentData().toInt();
    int idTipoCombustible = ui->comboBox_Venta_Combustible->currentData().toInt();

    if (!db.isOpen()) {
        qDebug() << "La base de datos no está abierta";
        return -1;
    }

    QSqlQuery query(db);
    query.prepare("SELECT idManguera FROM Manguera "
                  "WHERE idSurtidor = :idSurtidor "
                  "AND idTipoCombustible = :idTipoCombustible");
    query.bindValue(":idSurtidor", idSurtidor);
    query.bindValue(":idTipoCombustible", idTipoCombustible);

    if (!query.exec()) {
        qDebug() << "Error al obtener idManguera:" << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    } else {
        qDebug() << "No se encontró manguera para surtidor y tipo de combustible seleccionados";
        return -1;
    }
}

void Crud::limitarFechasDateTimeEdit(QDateTimeEdit *dateTimeEdit) {
    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT DISTINCT fecha FROM Turno ORDER BY fecha ASC");

    if (!query.exec()) {
        qDebug() << "Error al obtener fechas de turno:" << query.lastError().text();
        return;
    }

    QList<QDate> fechasPermitidas;
    while (query.next()) {
        QDate fecha = query.value(0).toDate();
        fechasPermitidas.append(fecha);
    }

    if (fechasPermitidas.isEmpty()) {
        qDebug() << "No hay fechas disponibles";
        return;
    }

    // Limitar el QDateTimeEdit al rango de fechas mínimo y máximo
    QDate minFecha = fechasPermitidas.first();
    QDate maxFecha = fechasPermitidas.last();

    dateTimeEdit->setMinimumDate(minFecha);
    dateTimeEdit->setMaximumDate(maxFecha);

    qDebug() << "Rango limitado entre" << minFecha << "y" << maxFecha;
}

void Crud::configurarMenuVentas() {
    ui->dateTimeEditTurno->setDateTime(QDateTime::currentDateTime());

    // Carga el combo box de estaciones
    cargarComboBox(ui->comboBox_Venta_Estacion, "Estacion", "idEstacion", "nombre");

    // Carga el combo box de tipos de combustible por estación
    int idEstacionVenta = ui->comboBox_Venta_Estacion->currentData().toInt();
    cargarTiposCombustibleDeEstacion(ui->comboBox_Venta_Combustible, idEstacionVenta);

    // Carga el combo box de medios de pago
    cargarComboBox(ui->comboBox_Venta_MedioPago, "MedioPago", "idMedioPago", "descripcion");

    // Carga surtidores inicial
    int idCombustible = ui->comboBox_Venta_Combustible->currentData().toInt();
    cargarSurtidores(ui->comboBox_Venta_Surtidor, idEstacionVenta, idCombustible);

    // Conexión: al cambiar estación
    connect(ui->comboBox_Venta_Estacion, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
        int idEst = ui->comboBox_Venta_Estacion->currentData().toInt();

        // Actualiza combustibles disponibles
        cargarTiposCombustibleDeEstacion(ui->comboBox_Venta_Combustible, idEst);

        int idComb = ui->comboBox_Venta_Combustible->currentData().toInt();
        cargarSurtidores(ui->comboBox_Venta_Surtidor, idEst, idComb);

        actualizarPrecio();
        actualizarPrecioTotal();
    });

    // Conexión: al cambiar combustible
    connect(ui->comboBox_Venta_Combustible, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
        int idEst = ui->comboBox_Venta_Estacion->currentData().toInt();
        int idComb = ui->comboBox_Venta_Combustible->currentData().toInt();
        cargarSurtidores(ui->comboBox_Venta_Surtidor, idEst, idComb);

        actualizarPrecio();
        actualizarPrecioTotal();
    });

    // cambiar litros
    connect(ui->spinBox_Venta_Litros, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &Crud::actualizarPrecioTotal);

    // Limitar fechas disponibles en el DateTimeEdit
    limitarFechasDateTimeEdit();

    // Reset inicial
    ui->comboBox_Venta_Combustible->setCurrentIndex(-1);
    ui->comboBox_Venta_Estacion->setCurrentIndex(-1);
}


#include <QRandomGenerator>
#include <cmath> // para std::round

void Crud::limitarFechasDateTimeEdit() {
    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT DISTINCT fecha FROM Turno ORDER BY fecha ASC");

    if (!query.exec()) {
        qDebug() << "Error al obtener fechas de turno:" << query.lastError().text();
        return;
    }

    QList<QDate> fechasPermitidas;
    while (query.next()) {
        QDate fecha = query.value(0).toDate();
        fechasPermitidas.append(fecha);
    }

    if (fechasPermitidas.isEmpty()) {
        qDebug() << "No hay fechas disponibles";
        return;
    }

    // Limitar el QDateTimeEdit al rango de fechas mínimo y máximo
    QDate minFecha = fechasPermitidas.first();
    QDate maxFecha = fechasPermitidas.last();

    ui->dateTimeEditTurno->setMinimumDate(minFecha);
    ui->dateTimeEditTurno->setMaximumDate(maxFecha);

    qDebug() << "Rango limitado entre" << minFecha << "y" << maxFecha;
}


void Crud::on_Boton_RegistroVenta_clicked()
{
    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    int idEstacion = ui->comboBox_Venta_Estacion->currentData().toInt();
    int idCombustible = ui->comboBox_Venta_Combustible->currentData().toInt();
    int idMedioPago = ui->comboBox_Venta_MedioPago->currentData().toInt();
    int idManguera = obtenerIdManguera();
    double litros = ui->spinBox_Venta_Litros->value();
    QDateTime fechaHora = ui->dateTimeEditTurno->dateTime();

    QString precioTexto = ui->Precio_Label->text();
    QString sinMoneda = precioTexto.mid(1);
    double precioAplicado = sinMoneda.toDouble();

    if (idManguera == -1) {
        QMessageBox::warning(this, "Campos vacios", "Debe seleccionar todos los campos");
        return;
    } else if (litros == 0) {
        QMessageBox::warning(this, "Campos vacios", "Debe seleccionar la cantidad de litros");
        return;
    }

    QSqlQuery query(db);

    query.prepare("SELECT idTurno FROM Turno "
                  "WHERE idEstacion = :idEstacion "
                  "AND fecha = :fecha "
                  "AND ((horaFin > horaInicio AND horaInicio <= :hora AND horaFin > :hora) "
                  "OR (horaFin < horaInicio AND (horaInicio <= :hora OR horaFin > :hora))) "
                  "LIMIT 1");
    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":fecha", fechaHora.date());
    query.bindValue(":hora", fechaHora.time());

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Error", "No se encontro turno para la fecha y hora seleccionada");
        return;
    }

    int idTurno = query.value(0).toInt();

    query.prepare("SELECT idTanque, litrosDisponibles FROM Tanque "
                  "WHERE idEstacion = :idEstacion AND idTipoCombustible = :idCombustible");
    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Error", "No se pudo consultar el tanque");
        return;
    }

    int idTanque = query.value(0).toInt();
    double litrosDisponibles = query.value(1).toDouble();

    if (litrosDisponibles < litros) {
        QMessageBox::warning(this, "Error", "No hay suficiente combustible en el tanque");
        return;
    }

    query.prepare("UPDATE Tanque "
                  "SET litrosDisponibles = litrosDisponibles - :litros "
                  "WHERE idEstacion = :idEstacion AND idTipoCombustible = :idCombustible");
    query.bindValue(":litros", litros);
    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "No se pudo actualizar el tanque");
        return;
    }

    query.prepare("INSERT INTO Venta (idTurno, idManguera, idMedioPago, fechaHora, litros, precioAplicado) "
                  "VALUES (:idTurno, :idManguera, :idMedioPago, :fechaHora, :litros, :precio)");
    query.bindValue(":idTurno", idTurno);
    query.bindValue(":idManguera", idManguera);
    query.bindValue(":idMedioPago", idMedioPago);
    query.bindValue(":fechaHora", fechaHora);
    query.bindValue(":litros", litros);
    query.bindValue(":precio", precioAplicado);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Error al registrar la venta: " + query.lastError().text());
        return;
    }

    QSqlQuery insertTanque(db);
    insertTanque.prepare("INSERT INTO LecturaTanque (idTanque, idTurno, fechaHora, volumenMedido) "
                         "VALUES (:idTanque, :idTurno, :fechaHora, :volumenMedido)");
    insertTanque.bindValue(":idTanque", idTanque);
    insertTanque.bindValue(":idTurno", idTurno);
    insertTanque.bindValue(":fechaHora", fechaHora);
    insertTanque.bindValue(":volumenMedido", litrosDisponibles - litros);
    insertTanque.exec();

    double perdida = (rand() % 5) / 100.0;



    double medidorFinal = litros - perdida;

    QSqlQuery insertManguera(db);
    insertManguera.prepare("INSERT INTO LecturaManguera (idManguera, idTurno, fechaHora, medidorFinal) "
                           "VALUES (:idManguera, :idTurno, :fechaHora, :medidorFinal)");
    insertManguera.bindValue(":idManguera", idManguera);
    insertManguera.bindValue(":idTurno", idTurno);
    query.bindValue(":fechaHora", fechaHora.toString("yyyy-MM-dd HH:mm:ss"));

    insertManguera.bindValue(":medidorFinal", medidorFinal);
    insertManguera.exec();

    QMessageBox::information(this, "Exito", "Venta registrada con exito");
    ui->spinBox_Venta_Litros->setValue(0);
    ui->PrecioTotal_Label->setText("L0.00");
}

//Entregas


void Crud::configurarMenuEntregas(){
    cargarComboBox(ui->comboBox_Entrega_Proveedor,"Proveedor","idProveedor","nombre");

    //carga el combo box de tipos de combustible por estacion
    int idEstacionVenta = ui->comboBox_Entrega_Estacion->currentData().toInt();
    cargarTiposCombustibleDeEstacion(ui->comboBox_Entrega_Combustible, idEstacionVenta);

    cargarComboBox(ui->comboBox_Entrega_Estacion, "Estacion", "idEstacion", "nombre");
    ui->dateTimeEditEntrega->setDateTime(QDateTime::currentDateTime());

    //conexion para combustibles por estacion
    connect(ui->comboBox_Entrega_Estacion, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
        int idEst = ui->comboBox_Entrega_Estacion->currentData().toInt();
        cargarTiposCombustibleDeEstacion(ui->comboBox_Entrega_Combustible, idEst);
    });
    limitarFechasDateTimeEdit2();
}

void Crud::limitarFechasDateTimeEdit2() {
    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT DISTINCT fecha FROM Turno ORDER BY fecha ASC");

    if (!query.exec()) {
        qDebug() << "Error al obtener fechas de turno:" << query.lastError().text();
        return;
    }

    QList<QDate> fechasPermitidas;
    while (query.next()) {
        QDate fecha = query.value(0).toDate();
        fechasPermitidas.append(fecha);
    }

    if (fechasPermitidas.isEmpty()) {
        qDebug() << "No hay fechas disponibles";
        return;
    }

    // Limitar el QDateTimeEdit al rango de fechas mínimo y máximo
    QDate minFecha = fechasPermitidas.first();

    ui->dateTimeEditEntrega->setMinimumDate(minFecha);
}

void Crud::on_Boton_RegistroEntrega_clicked()
{
    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    int idProveedor = ui->comboBox_Entrega_Proveedor->currentData().toInt();
    int idTipoCombustible = ui->comboBox_Entrega_Combustible->currentData().toInt();
    int idEstacion = ui->comboBox_Entrega_Estacion->currentData().toInt();
    double volumen = ui->doubleSpinBox_Entrega_Volumen->value();
    double temperatura = ui->doubleSpinBox_Entrega_Temperatura->value();
    double densidad = ui->doubleSpinBox_Entrega_Densidad->value();
    double costo = ui->doubleSpinBox_Entrega_Costo->value();
    QDateTime fechaHora = ui->dateTimeEditEntrega->dateTime();
    QSqlQuery query(db);

    if (volumen <= 0 || idEstacion == 0 || densidad == 0 || idProveedor == 0 ||
        idTipoCombustible == 0 || costo == 0 || temperatura == 0) {
        QMessageBox::warning(this, "Campos vacíos", "Debe completar todos los campos y volumen > 0");
        return;
    }

    query.prepare("SELECT idTanque, litrosDisponibles, capacidad "
                  "FROM Tanque "
                  "WHERE idEstacion = :idEstacion AND idTipoCombustible = :idTipoCombustible");
    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idTipoCombustible", idTipoCombustible);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Error", "No se encontró el tanque correspondiente.");
        return;
    }

    int idTanque = query.value("idTanque").toInt();
    double litrosDisponibles = query.value("litrosDisponibles").toDouble();
    double capacidadMaxima = query.value("capacidad").toDouble();

    if (litrosDisponibles + volumen > capacidadMaxima) {
        QMessageBox::warning(this, "Tanque lleno", "No se puede registrar la entrega: excede la capacidad máxima del tanque.");
        return;
    }

    query.prepare("INSERT INTO Entrega (idProveedor, fechaHora, volumen, temperatura, densidad, costo, idEstacion, idTipoCombustible) "
                  "VALUES (:idProveedor, :fechaHora, :volumen, :temperatura, :densidad, :costo, :idEstacion, :idTipoCombustible)");
    query.bindValue(":idProveedor", idProveedor);
    query.bindValue(":fechaHora", fechaHora.toString("yyyy-MM-dd HH:mm:ss"));

    query.bindValue(":volumen", volumen);
    query.bindValue(":temperatura", temperatura);
    query.bindValue(":densidad", densidad);
    query.bindValue(":costo", costo);
    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idTipoCombustible", idTipoCombustible);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo registrar la entrega.");
        return;
    }

    query.prepare("UPDATE Tanque "
                  "SET litrosDisponibles = litrosDisponibles + :litros "
                  "WHERE idEstacion = :idEstacion AND idTipoCombustible = :idTipoCombustible");
    query.bindValue(":litros", volumen);
    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idTipoCombustible", idTipoCombustible);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "La entrega se registró pero no se pudo actualizar el tanque.");
        return;
    }

    // Crear lectura del tanque con el nuevo volumen
    QSqlQuery insertLectura(db);
    insertLectura.prepare("INSERT INTO LecturaTanque (idTanque, idTurno, fechaHora, volumenMedido) "
                          "VALUES (:idTanque, NULL, :fechaHora, :volumenMedido)");
    insertLectura.bindValue(":idTanque", idTanque);
    insertLectura.bindValue(":fechaHora", fechaHora);
    insertLectura.bindValue(":volumenMedido", litrosDisponibles + volumen);
    insertLectura.exec();

    QMessageBox::information(this, "Entrega registrada", "Entrega registrada y tanque actualizado correctamente.");

    ui->doubleSpinBox_Entrega_Volumen->setValue(0);
    ui->doubleSpinBox_Entrega_Temperatura->setValue(0);
    ui->doubleSpinBox_Entrega_Densidad->setValue(0);
    ui->doubleSpinBox_Entrega_Costo->setValue(0);
    ui->dateTimeEditEntrega->setDateTime(QDateTime::currentDateTime());
}


//Precio

void Crud::configurarMenuPrecios() {
    ui->dateTimeEdit_Precio_FechaInicio->setReadOnly(true);
    ui->dateTimeEdit_Precio_FechaInicio->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->dateTimeEdit_Precio_FechaInicio->setReadOnly(true);

    ui->dateTimeEdit_Precio_FechaFin->setDateTime(QDateTime::currentDateTime());


    cargarComboBox(ui->comboBox_Precio_Estacion, "Estacion", "idEstacion", "nombre");

    //carga el combo box de tipos de combustible por estacion
    int idEstacionVenta = ui->comboBox_Precio_Estacion->currentData().toInt();
    cargarTiposCombustibleDeEstacion(ui->comboBox_Precio_Combustible, idEstacionVenta);

    //conexion para combustibles por estacion
    connect(ui->comboBox_Precio_Estacion, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
        int idEst = ui->comboBox_Precio_Estacion->currentData().toInt();
        cargarTiposCombustibleDeEstacion(ui->comboBox_Precio_Combustible, idEst);
    });

    // Actualiza las fechas
    connect(ui->comboBox_Precio_Estacion, &QComboBox::currentIndexChanged,
            this, &Crud::CargarFechasPrecio);
    connect(ui->comboBox_Precio_Combustible, &QComboBox::currentIndexChanged,
            this, &Crud::CargarFechasPrecio);

    // Actualizan el precio de los labels
    connect(ui->comboBox_Precio_Estacion, &QComboBox::currentIndexChanged, this, [this]() {
        actualizarPrecio(ui->comboBox_Precio_Estacion, ui->comboBox_Precio_Combustible, ui->UltimoPrecio_Label);
    });

    connect(ui->comboBox_Precio_Combustible, &QComboBox::currentIndexChanged, this, [this]() {
        actualizarPrecio(ui->comboBox_Precio_Estacion, ui->comboBox_Precio_Combustible, ui->UltimoPrecio_Label);
    });
}

//Precio



void Crud::CargarFechasPrecio(){
    int idEstacion = ui->comboBox_Precio_Estacion->currentData().toInt();
    int idCombustible = ui->comboBox_Precio_Combustible->currentData().toInt();

    if(idEstacion <= 0 || idCombustible <= 0)
        return;

    if(!db.isOpen()){
        qDebug() << "Base de datos no abierta en CargarFechasPrecio";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT fechaInicio, fechaFin "
                  "FROM PrecioCombustible "
                  "WHERE idEstacion = :idEstacion AND idTipoCombustible = :idCombustible "
                  "ORDER BY fechaInicio DESC LIMIT 1");
    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);

    if(query.exec() && query.next()){
        QDateTime fechaInicio = query.value(0).toDateTime();
        ui->dateTimeEdit_Precio_FechaInicio->setDateTime(fechaInicio);
    }
}

void Crud::on_Boton_RegistroPrecio_clicked()
{
    if (!db.isOpen()) {
        qDebug() << "Error al abrir la base de datos";
        QMessageBox::critical(this, "Error", "La base de datos no está abierta.");
        return;
    }

    int idEstacion = ui->comboBox_Precio_Estacion->currentData().toInt();
    int idCombustible = ui->comboBox_Precio_Combustible->currentData().toInt();
    double nuevoPrecio = ui->DoubleSpinBox_Precios_NuevosPrecios->value();

    if (idEstacion <= 0 || idCombustible <= 0 || nuevoPrecio <= 0) {
        QMessageBox::warning(this, "Datos no validos", "Debe seleccionar estación, combustible y un precio valido.");
        return;
    }

    QDateTime fechaActual = QDateTime::currentDateTime();


    {
        QSqlQuery queryUpdate(db);
        queryUpdate.prepare("UPDATE PrecioCombustible "
                            "SET fechaFin = :fechaFin "
                            "WHERE idEstacion = :idEstacion "
                            "AND idTipoCombustible = :idCombustible "
                            "AND fechaFin IS NULL");
        queryUpdate.bindValue(":fechaFin", fechaActual);
        queryUpdate.bindValue(":idEstacion", idEstacion);
        queryUpdate.bindValue(":idCombustible", idCombustible);
        if(!queryUpdate.exec()) {
            qDebug() << "Error al cerrar precio anterior:" << queryUpdate.lastError().text();
            QMessageBox::critical(this, "Error", "No se pudo cerrar el precio anterior.");
            return;
        }
    }

    // ⿢ Insertar nuevo precio
    {
        QSqlQuery queryInsert(db);
        queryInsert.prepare("INSERT INTO PrecioCombustible "
                            "(idEstacion, idTipoCombustible, precio, fechaInicio, fechaFin) "
                            "VALUES (:idEstacion, :idTipoCombustible, :precio, :fechaInicio, NULL)");
        queryInsert.bindValue(":idEstacion", idEstacion);
        queryInsert.bindValue(":idTipoCombustible", idCombustible);
        queryInsert.bindValue(":precio", nuevoPrecio);
        queryInsert.bindValue(":fechaInicio", fechaActual);
        if(!queryInsert.exec()) {
            qDebug() << "Error al registrar nuevo precio:" << queryInsert.lastError().text();
            QMessageBox::critical(this, "Error", "No se pudo registrar el nuevo precio.");
            return;
        }
    }

    QMessageBox::information(this, "Exito", "Nuevo precio registrado correctamente");

    // Actualizar GUI
    CargarFechasPrecio();
    actualizarPrecio(ui->comboBox_Precio_Estacion, ui->comboBox_Precio_Combustible, ui->UltimoPrecio_Label);
}

//Lecturas

void Crud::configurarMenuLecturas() {
    ui->dateTimeEdit_Lecturas_FechaHora->setDateTime(QDateTime::currentDateTime());

    cargarComboBox(ui->comboBox_Lecturas_Estacion, "Estacion", "idEstacion", "nombre");

    //carga el combo box de tipos de combustible por estacion
    int idEstacionVenta = ui->comboBox_Lecturas_Estacion->currentData().toInt();
    cargarTiposCombustibleDeEstacion(ui->comboBox_Lecturas_Combustible, idEstacionVenta);

    //conexion para combustibles por estacion
    connect(ui->comboBox_Lecturas_Estacion, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
        int idEst = ui->comboBox_Lecturas_Estacion->currentData().toInt();
        cargarTiposCombustibleDeEstacion(ui->comboBox_Lecturas_Combustible, idEst);
    });

    //carga surtidores
    idEstacionVenta = ui->comboBox_Lecturas_Estacion->currentData().toInt();
    cargarSurtidores(ui->comboBox_Lecturas_Surtidor, idEstacionVenta);

    connect(ui->comboBox_Lecturas_Estacion, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
        int idEst = ui->comboBox_Lecturas_Estacion->currentData().toInt();
        cargarSurtidores(ui->comboBox_Lecturas_Surtidor, idEst);
    });
}

void Crud::on_Boton_RegistroLecturaTanque_clicked()
{
    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    int idEstacion = ui->comboBox_Lecturas_Estacion->currentData().toInt();
    int idCombustible = ui->comboBox_Lecturas_Combustible->currentData().toInt();
    int volumenMedido = ui->spinBox_Lecturas_LecturaTanque->value();
    QDateTime fechaHora = ui->dateTimeEdit_Lecturas_FechaHora->dateTime();

    // Buscar turno correspondiente
    QSqlQuery query(db);
    query.prepare(
        "SELECT idTurno FROM Turno "
        "WHERE idEstacion = :idEstacion "
        "AND fecha = :fecha "
        "AND ( "
        "  (horaInicio <= horaFin AND :hora BETWEEN horaInicio AND horaFin) "
        "  OR "
        "  (horaInicio > horaFin AND (:hora >= horaInicio OR :hora <= horaFin)) "
        ") "
        "LIMIT 1"
        );

    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":fecha", fechaHora.date());
    query.bindValue(":hora", fechaHora.time());

    if (!query.exec()) {
        qDebug() << "Error al buscar turno:" << query.lastError().text();
        return;
    }

    int idTurno = -1;
    if (query.next()) {
        idTurno = query.value(0).toInt();
    } else {
        qDebug() << "No se encontro turno para la fecha y hora seleccionada";
        return;
    }

    //buscar idTanque usando idEstacion e idCombustible
    query.prepare("SELECT idTanque FROM Tanque "
                  "WHERE idEstacion = :idEstacion "
                  "AND idTipoCombustible = :idCombustible "
                  "LIMIT 1");

    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);

    if (!query.exec()) {
        qDebug() << "Error al buscar tanque:" << query.lastError().text();
        return;
    }

    int idTanque = -1;
    if (query.next()) {
        idTanque = query.value(0).toInt();
    } else {
        qDebug() << "No se encontro tanque para la estacion y combustible";
        return;
    }

    //insertar el registro de tanque

    {
        QSqlQuery queryInsert(db);
        queryInsert.prepare("INSERT INTO LecturaTanque"
                            "(idTanque, idTurno, fechaHora, volumenMedido)"
                            "VALUES (:idTanque, :idTurno, :fechaHora, :volumenMedido)");
        queryInsert.bindValue(":idTanque", idTanque);
        queryInsert.bindValue(":idTurno", idTurno);
        queryInsert.bindValue(":fechaHora", fechaHora);
        queryInsert.bindValue(":volumenMedido", volumenMedido);
        if(!queryInsert.exec()) {
            qDebug() << "Error al registrar lectura:" << queryInsert.lastError().text();
            QMessageBox::critical(this, "Error", "No se pudo registrar la lectura.");
            return;
        }
    }

}


void Crud::on_Boton_RegistroLecturaMedidor_clicked()
{

    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    int idEstacion = ui->comboBox_Lecturas_Estacion->currentData().toInt();
    int idCombustible = ui->comboBox_Lecturas_Combustible->currentData().toInt();
    int medidorFinal = ui->spinBox_Lecturas_LecturaMedidor->value();
    int idSurtidor = ui->comboBox_Lecturas_Surtidor->currentData().toInt();
    QDateTime fechaHora = ui->dateTimeEdit_Lecturas_FechaHora->dateTime();

    // Buscar turno correspondiente
    QSqlQuery query(db);
    query.prepare(
        "SELECT idTurno FROM Turno "
        "WHERE idEstacion = :idEstacion "
        "AND fecha = :fecha "
        "AND ( "
        "  (horaInicio <= horaFin AND :hora BETWEEN horaInicio AND horaFin) "
        "  OR "
        "  (horaInicio > horaFin AND (:hora >= horaInicio OR :hora <= horaFin)) "
        ") "
        "LIMIT 1"
        );

    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":fecha", fechaHora.date());
    query.bindValue(":hora", fechaHora.time());

    if (!query.exec()) {
        qDebug() << "Error al buscar turno:" << query.lastError().text();
        return;
    }

    int idTurno = -1;
    if (query.next()) {
        idTurno = query.value(0).toInt();
    } else {
        qDebug() << "No se encontro turno para la fecha y hora seleccionada";
        return;
    }

    //buscar idTanque usando idEstacion e idCombustible
    query.prepare("SELECT idTanque FROM Tanque "
                  "WHERE idEstacion = :idEstacion "
                  "AND idTipoCombustible = :idCombustible "
                  "LIMIT 1");

    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);

    if (!query.exec()) {
        qDebug() << "Error al buscar tanque:" << query.lastError().text();
        return;
    }

    int idTanque = -1;
    if (query.next()) {
        idTanque = query.value(0).toInt();
    } else {
        qDebug() << "No se encontro tanque para la estacion y combustible";
        return;
    }

    //buscar idTanque usando idEstacion e idCombustible
    query.prepare("SELECT idManguera FROM Manguera "
                  "WHERE idSurtidor = :idSurtidor "
                  "AND idTipoCombustible = :idCombustible "
                  "LIMIT 1");

    query.bindValue(":idSurtidor", idSurtidor);
    query.bindValue(":idCombustible", idCombustible);

    if (!query.exec()) {
        qDebug() << "Error al buscar tanque:" << query.lastError().text();
        return;
    }

    //insertar el registro de manguera

    {
        QSqlQuery queryInsert(db);
        queryInsert.prepare("INSERT INTO LecturaManguera"
                            "(idManguera, idTurno, fechaHora, medidorFinal)"
                            "VALUES (:idTanque, :idTurno, :fechaHora, :medidorFinal)");
        queryInsert.bindValue(":idTanque", idTanque);
        queryInsert.bindValue(":idTurno", idTurno);
        queryInsert.bindValue(":fechaHora", fechaHora);
        queryInsert.bindValue(":medidorFinal", medidorFinal);
        if(!queryInsert.exec()) {
            qDebug() << "Error al registrar lectura:" << queryInsert.lastError().text();
            QMessageBox::critical(this, "Error", "No se pudo registrar la lectura.");
            return;
        }
    }
}

