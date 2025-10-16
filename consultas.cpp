#include "consultas.h"
#include "ui_consultas.h"
#include <QtSql/QSqlError>
#include <QDebug>
#include <QtSql/QSqlQuery>
#include <QMessageBox>

Consultas::Consultas(QSqlDatabase &db, QWidget *parent):
    QWidget(parent),
    ui(new Ui::Consultas),
    db(db)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    configurarConsulta_1();
    configurarConsulta_2();
    configurarConsulta_3();
    configurarConsulta_4();
    configurarConsulta_5();
    configurarConsulta_6();

    model1 = new QStandardItemModel(this);
    model2 = new QStandardItemModel(this);
    model4 = new QStandardItemModel(this);
    model5 = new QStandardItemModel(this);
    model6 = new QStandardItemModel(this);









    ConfigurarTablasConsultas();
}

Consultas::~Consultas()
{
    delete ui;

    delete model1;
    delete model2;
    delete model4;
    delete model5;
}

void Consultas::cargarComboBox(QComboBox* comboBox, const QString& tabla, const QString& columnaId, const QString& columnaNombre)
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

void Consultas::limitarFechasDateTimeEdit(QDateTimeEdit *dateTimeEdit) {
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

void Consultas::cargarTiposCombustibleDeEstacion(QComboBox* comboBox, int idEstacion)
{
    comboBox->clear();
    if (!db.isOpen()) {
        qDebug() << "Base de datos no abierta";
        return;
    }

    //une tanque y tipocombustible filtrando por estacion
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

void Consultas::ConfigurarTablasConsultas(){
    ui->table_VentasPeriodo_Page1->setModel(model1);
    model1->setColumnCount(2);
    model1->setHorizontalHeaderLabels({"Litros", "Monto"});
    ui->table_VentasPeriodo_Page1->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_VentasPeriodo_Page1->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->table_ComparativoEstaciones_Page2->setModel(model2);
    model2->setColumnCount(2);
    model2->setHorizontalHeaderLabels({"Litros", "Monto"});
    ui->table_ComparativoEstaciones_Page2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_ComparativoEstaciones_Page2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->table_ComparativoEstaciones_Page4->setModel(model4);
    model4->setColumnCount(2);
    model4->setHorizontalHeaderLabels({"Litros", "Monto"});
    ui->table_ComparativoEstaciones_Page4->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_ComparativoEstaciones_Page4->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->table_ComparativoEstaciones_Page5->setModel(model5);
    model5->setColumnCount(4);
    model5->setHorizontalHeaderLabels({"Estacion","Precio Aplicado", "Costo entrega", "Ganancia"});
    ui->table_ComparativoEstaciones_Page5->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_ComparativoEstaciones_Page5->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->table_ComparativoEstaciones_Page6->setModel(model6);
    model6->setColumnCount(3);
    model6->setHorizontalHeaderLabels({"Litros vendidos","Medidor", "Estado"});
    ui->table_ComparativoEstaciones_Page6->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_ComparativoEstaciones_Page6->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void Consultas::configurarConsulta_1(){
    limitarFechasDateTimeEdit(ui->dateTimeEdit_FechaInicio_Page1);
    limitarFechasDateTimeEdit(ui->dateTimeEdit_FechaFin_Page1);

    //carga el combo box de estaciones
    cargarComboBox(ui->comboBox_Estacion_Page1, "Estacion", "idEstacion", "nombre");

    //carga el combo box de tipos de combustible por estacion
    int idEstacionVenta = ui->comboBox_Estacion_Page1->currentData().toInt();
    cargarTiposCombustibleDeEstacion(ui->comboBox_Combustible_Page1, idEstacionVenta);

    //conexion para combustibles por estacion
    connect(ui->comboBox_Estacion_Page1, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
        int idEst = ui->comboBox_Estacion_Page1->currentData().toInt();
        cargarTiposCombustibleDeEstacion(ui->comboBox_Combustible_Page1, idEst);
    });

    //limitaamos fechas disponibles en el DateTimeEdit
    ui->comboBox_Combustible_Page1->setCurrentIndex(-1);
    ui->comboBox_Estacion_Page1->setCurrentIndex(-1);
}

void Consultas::on_BotonConsultar_Page1_clicked()
{
    int idEstacion = ui->comboBox_Estacion_Page1->currentData().toInt();
    int idCombustible = ui->comboBox_Combustible_Page1->currentData().toInt();
    QDateTime fechaInicio = ui->dateTimeEdit_FechaInicio_Page1->dateTime();
    QDateTime fechaFin = ui->dateTimeEdit_FechaFin_Page1->dateTime();

    QSqlQuery query(db);
    query.prepare("SELECT v.litros, v.precioAplicado "
                  "FROM Venta AS v, Manguera AS m, Surtidor AS s "
                  "WHERE v.idManguera = m.idManguera "
                  "AND m.idSurtidor = s.idSurtidor "
                  "AND s.idEstacion = :idEstacion "
                  "AND m.idTipoCombustible = :idCombustible "
                  "AND v.fechaHora BETWEEN :fechaInicio AND :fechaFin");

    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);
    query.bindValue(":fechaInicio", fechaInicio);
    query.bindValue(":fechaFin", fechaFin);

    if (!query.exec()) {
        qDebug() << "Error al consultar ventas:" << query.lastError().text();
        return;
    }

    model1->clear();
    model1->setHorizontalHeaderLabels({"Litros", "Monto"});
    double Sumlitros=0;
    double SumMonto=0;

    while (query.next()) {
        double Litros = query.value(0).toDouble();
        double precio_combustible = query.value(1).toDouble();
        double monto = Litros * precio_combustible;

        QList<QStandardItem*> fila;
        fila << new QStandardItem(QString::number(Litros)+ " Litros");
        fila << new QStandardItem("L" + QString::number(monto, 'f', 2));
        model1->appendRow(fila);

        Sumlitros += Litros;
        SumMonto += monto;
    }

    ui->SumaLitros_Label->setText(QString::number(Sumlitros)+ " Litros");
    ui->SumaVentas_label->setText("L" + QString::number(SumMonto, 'f', 2));
}

void Consultas::configurarConsulta_2(){
    limitarFechasDateTimeEdit(ui->dateTimeEdit_FechaInicio_Page2);
    limitarFechasDateTimeEdit(ui->dateTimeEdit_FechaFin_Page2);

    ui->comboBox_Ranking_Page2->addItems({"Litros", "Monto"});
    ui->comboBox_Ranking_Page2->setCurrentIndex(-1);
}

void Consultas::on_BotonConsultar_Page2_clicked()
{
    model2->clear();
    int opcion = ui->comboBox_Ranking_Page2->currentIndex();

    QString order=" ";
    if (opcion==0){//si es por litros
        model2->setHorizontalHeaderLabels({"Estacion", "Litros"});
        order="TotalLitros DESC";
    }else if (opcion==1){
        model2->setHorizontalHeaderLabels({"Estacion", "Monto"});
        order="TotalMonto DESC";
    }
    QDateTime fechaInicio = ui->dateTimeEdit_FechaInicio_Page2->dateTime();
    QDateTime fechaFin = ui->dateTimeEdit_FechaFin_Page2->dateTime();

    QSqlQuery query(db);

    query.prepare("SELECT e.nombre AS Estacion, "
                  "SUM(v.litros) AS TotalLitros, "
                  "SUM(v.litros * v.precioAplicado) AS TotalMonto "
                  "FROM Venta AS v, Manguera AS m, Surtidor AS s, Estacion AS e "
                  "WHERE v.idManguera = m.idManguera "
                  "AND m.idSurtidor = s.idSurtidor "
                  "AND s.idEstacion = e.idEstacion "
                  "AND v.fechaHora BETWEEN :fechaInicio AND :fechaFin "
                  "GROUP BY e.idEstacion, e.nombre "
                  "ORDER BY " + order);

    query.bindValue(":fechaInicio", fechaInicio);
    query.bindValue(":fechaFin", fechaFin);

    if (!query.exec()) {
        qDebug() << "Error al consultar ventas:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        QString estacion = query.value(0).toString();
        double Litros = query.value(1).toDouble();
        double monto = query.value(2).toDouble();

        qDebug() << "estacion: " << estacion << "Litros:" << Litros
                 << " Monto:" << monto;
        QList<QStandardItem*> fila;
        if (opcion==0){//si es por Litros
            fila << new QStandardItem(estacion);
            fila << new QStandardItem(QString::number(Litros)+ " Litros");
        }else if (opcion==1){
            fila << new QStandardItem(estacion);
            fila << new QStandardItem("L" + QString::number(monto, 'f', 2));
        }
        model2->appendRow(fila);
    }
}

void Consultas::configurarConsulta_3(){
    limitarFechasDateTimeEdit(ui->dateEdit_Page3);

    cargarComboBox(ui->comboBox_Estacion_Page3, "Estacion", "idEstacion", "nombre");

    //carga el combo box de tipos de combustible por estacion
    int idEstacion = ui->comboBox_Estacion_Page3->currentData().toInt();
    cargarTiposCombustibleDeEstacion(ui->comboBox_Combustible_Page3, idEstacion);

    //conexion para combustibles por estacion
    connect(ui->comboBox_Estacion_Page3, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
        int idEst = ui->comboBox_Estacion_Page3->currentData().toInt();
        cargarTiposCombustibleDeEstacion(ui->comboBox_Combustible_Page3, idEst);
    });
}

void Consultas::on_BotonConsultar_Page3_clicked()
{
    int idEstacion = ui->comboBox_Estacion_Page3->currentData().toInt();
    int idCombustible = ui->comboBox_Combustible_Page3->currentData().toInt();
    QDateTime fecha = ui->dateEdit_Page3->dateTime();

    QSqlQuery query(db);
    query.prepare("SELECT precio FROM PrecioCombustible "
                  "WHERE idEstacion = :idEstacion "
                  "AND idTipoCombustible = :idCombustible "
                  "AND :fechaConsulta BETWEEN fechaInicio AND fechaFin "
                  "ORDER BY idPrecio DESC LIMIT 1");

    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":idCombustible", idCombustible);
    query.bindValue(":fechaConsulta", fecha);

    qDebug() << "precio: " << ui->PrecioVigente_Label_Page3->text().toStdString();

    if (!query.exec()) {
        qDebug() << "Error al obtener precio:" << query.lastError().text();
        ui->PrecioVigente_Label_Page3->setText("L0.00");
        return;
    }
    while (query.next()) {
        double precio = query.value(0).toDouble();
        ui->PrecioVigente_Label_Page3->setText("L" + QString::number(precio, 'f', 2));
    }
}

void Consultas::configurarConsulta_5(){
    cargarComboBox(ui->comboBox_Combustible_Page5, "TipoCombustible", "idTipoCombustible", "nombre");
}

void Consultas::on_BotonConsultar_Page5_clicked()
{
    int idCombustible = ui->comboBox_Combustible_Page5->currentData().toInt();
    qDebug() << idCombustible;
    QSqlQuery query(db);
    query.prepare("SELECT est.nombre as Estacion, "
                  "AVG(e.costo / e.volumen) as CostoPromedio, " // avg para sacar el promedio de costo de entrega
                  "AVG(p.precio) as PrecioPromedio " // avg para sacar el promedio de precio de ventas
                  "FROM Entrega as e, Estacion as est, PrecioCombustible as p "
                  "WHERE p.idEstacion = est.idEstacion "
                  "AND est.idEstacion = e.idEstacion "
                  "AND e.idTipoCombustible = :idCombustible "
                  "GROUP BY est.idEstacion, est.nombre "
                  "ORDER BY est.nombre");

    query.bindValue(":idCombustible", idCombustible);

    if (!query.exec()) {
        qDebug() << "Error en consulta costo vs precio:" << query.lastError().text();
        return;
    }
    model5->clear();
    model5->setHorizontalHeaderLabels({"Estacion","Precio Aplicado", "Costo entrega", "Ganancia"});

    while (query.next()) {

        QString Estacion = query.value(0).toString();
        double costoEntrega = query.value(1).toDouble();
        double precioAplicado = query.value(2).toDouble();

        QList<QStandardItem*> fila;
        fila << new QStandardItem(Estacion);
        fila << new QStandardItem("L" + QString::number(precioAplicado, 'f', 2));
        fila << new QStandardItem("L" + QString::number(costoEntrega, 'f', 2));
        fila << new QStandardItem("L" + QString::number(precioAplicado - costoEntrega, 'f', 2));

        model5->appendRow(fila);

        qDebug() << "Estacion:" << Estacion
                 << "CostoUnitario:" << costoEntrega
                 << "PrecioAplicado:" << precioAplicado;
    }
}

void Consultas::configurarConsulta_4(){
    limitarFechasDateTimeEdit(ui->dateEdit_Page4);
    limitarFechasDateTimeEdit(ui->dateEdit2_Page4);

    cargarComboBox(ui->comboBox_Estacion_Page4, "Estacion", "idEstacion", "nombre");
    ui->comboBox_Ranking_Page4->addItems({"Litros", "Monto"});
    ui->comboBox_Ranking_Page4->setCurrentIndex(-1);
}

void Consultas::on_BotonConsultar_Page4_clicked()
{
    model4->clear();
    int opcion = ui->comboBox_Ranking_Page4->currentIndex();

    QString order=" ";
    if (opcion==0){//si es por litros
        model4->setHorizontalHeaderLabels({"Combustible", "Litros"});
        order="TotalLitros DESC";
    }else if (opcion==1){//si es por monto
        model4->setHorizontalHeaderLabels({"Combustible", "Monto"});
        order="TotalMonto DESC";
    }

    int idEstacion = ui->comboBox_Estacion_Page4->currentData().toInt();
    QDateTime fechaInicio = ui->dateEdit_Page4->dateTime();
    QDateTime fechaFin = ui->dateEdit2_Page4->dateTime();

    QSqlQuery query(db);
    query.prepare("SELECT t.nombre, Sum(v.litros) AS TotalLitros, SUM(v.litros * v.precioAplicado) AS TotalMonto "
                  "FROM Venta AS v, Manguera AS m, Surtidor AS s, TipoCombustible as t "
                  "WHERE v.idManguera = m.idManguera "
                  "AND m.idSurtidor = s.idSurtidor "
                  "AND s.idEstacion = :idEstacion "
                  "and t.idTipoCombustible=m.idTipoCombustible "
                  "AND v.fechaHora BETWEEN :fechaInicio AND :fechaFin "
                  "group by m.idTipoCombustible "
                  "order by " + order);

    query.bindValue(":idEstacion", idEstacion);
    query.bindValue(":fechaInicio", fechaInicio);
    query.bindValue(":fechaFin", fechaFin);

    if (!query.exec()) {
        qDebug() << "Error al consultar ventas:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        QString tipoCombustible = query.value(0).toString();
        double Litros = query.value(1).toDouble();
        double monto = query.value(2).toDouble();

        qDebug() << "estacion: " << tipoCombustible << "Litros:" << Litros
                 << " Monto:" << monto;
        QList<QStandardItem*> fila;
        if (opcion==0){//si es por Litros
            fila << new QStandardItem(tipoCombustible);
            fila << new QStandardItem(QString::number(Litros)+ " Litros");
        }else if (opcion==1){//si e spor monto
            fila << new QStandardItem(tipoCombustible);
            fila << new QStandardItem("L" + QString::number(monto, 'f', 2));
        }
        model4->appendRow(fila);
    }
}

void Consultas::configurarConsulta_6(){
    cargarComboBox(ui->comboBox_Estacion_Page6, "Estacion", "idEstacion", "nombre");

    //carga el combo box de tipos de combustible por estacion
    int idEstacion = ui->comboBox_Estacion_Page6->currentData().toInt();
    cargarTiposCombustibleDeEstacion(ui->comboBox_Combustible_Page6, idEstacion);

    //conexion para combustibles por estacion
    connect(ui->comboBox_Estacion_Page6, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int){
        int idEst = ui->comboBox_Estacion_Page6->currentData().toInt();
        cargarTiposCombustibleDeEstacion(ui->comboBox_Combustible_Page6, idEst);
    });

    limitarFechasDateTimeEdit(ui->dateTimeEdit_FechaTurno_Page6);

    QSqlQuery query(db);
    query.prepare("SELECT MIN(numeroTurno), MAX(numeroTurno) FROM Turno");

    if (!query.exec()) {
        qDebug() << "Error en consulta costo vs precio:" << query.lastError().text();
        return;
    }
    int minTurno = 0;
    int maxTurno = 0;//sacamos el max y el min de turnos q hay por dia

    while (query.next()) {
        int minTurno = query.value(0).toInt();
        int maxTurno = query.value(1).toInt();

        ui->spinBox_NumTurno->setMinimum(minTurno);
        ui->spinBox_NumTurno->setMaximum(maxTurno);
    }
}

void Consultas::on_BotonConsultar_Page6_clicked()
{
    model6->setRowCount(0);

    int idEstacion = ui->comboBox_Estacion_Page6->currentData().toInt();
    int idCombustible = ui->comboBox_Combustible_Page6->currentData().toInt();
    QDateTime fechaTurno = ui->dateTimeEdit_FechaTurno_Page6->dateTime();
    int NumTurno = ui->spinBox_NumTurno->value();

    QSqlQuery query(db);

    query.prepare("select t.idTurno "
                  "from Turno t , Estacion e "
                  "where t.fecha= :fecha and t.idEstacion=e.idEstacion and t.numeroTurno= :numTurno and e.idEstacion = :idEstacion");

    query.bindValue(":fecha", fechaTurno.date());
    query.bindValue(":numTurno", NumTurno);
    query.bindValue(":idEstacion", idEstacion);

    if (!query.exec()) {
        return;
    }
    int idTurno=0;
    while (query.next()) {
        idTurno = query.value(0).toInt();
    }

    query.prepare("select sum(lm.medidorFinal), lm.idManguera, tc.nombre from LecturaManguera lm, Manguera m, TipoCombustible tc "
        "where lm.idManguera=m.idManguera and tc.idTipoCombustible=m.idTipoCombustible "
        "and lm.idTurno= :idTurno and tc.idTipoCombustible= :idTipoCombustible group by lm.idManguera, tc.nombre");


    query.bindValue(":idTurno", idTurno);
    query.bindValue(":idTipoCombustible", idCombustible);

    if (!query.exec()) {
        return;
    }
    double medidor=0;
    int idManguera=0;
    while (query.next()) {
        medidor = query.value(0).toDouble();
        idManguera = query.value(1).toInt();
    }

    query.prepare("select sum(v.litros), date(v.fechaHora), tc.nombre from Venta v, Manguera m, TipoCombustible tc "
                  "where v.idManguera=m.idManguera and tc.idTipoCombustible=m.idTipoCombustible "
                  "and idTurno= :idTurno  and tc.idTipoCombustible= :idTipoCombustible group by date(v.fechaHora), tc.nombre");

    query.bindValue(":idTurno", idTurno);
    query.bindValue(":idTipoCombustible", idCombustible);

    if (!query.exec()) {
        return;
    }
    double litros=0;
    while (query.next()) {
        litros = query.value(0).toDouble();
    }

    double diferencia = qAbs(medidor - litros);//diferencia siempre positiva
    QString estado="";
    if (diferencia > 5 || diferencia > (litros * 0.1)) { //si la diferencia absoluta es mayor a 5 litros o mayor al 10% de los litros que fueron vendidos entra
        estado = "ALERTA";
        QString diferenciaStr = QString::number(diferencia, 'f', 2);
        QString litrosStr = QString::number(litros, 'f', 2);

        QString descripcion = "Diferencia: " + diferenciaStr + " litros en venta de " + litrosStr + " litros";

        QSqlQuery queryAlerta(db);
        queryAlerta.prepare(
            "INSERT INTO Alerta (idTurno, fechaHora, idTipoCombustible, idManguera, tipo, descripcion) "
            "VALUES (:idTurno, :fechaHora, :idTipoCombustible, :idManguera, :tipo, :descripcion)"
            );
        queryAlerta.bindValue(":idTurno", idTurno);
        queryAlerta.bindValue(":fechaHora", fechaTurno);
        queryAlerta.bindValue(":idTipoCombustible", idCombustible);
        queryAlerta.bindValue(":idManguera", idManguera);
        queryAlerta.bindValue(":tipo", "DISCREPANCIA_MEDIDOR");
        queryAlerta.bindValue(":descripcion", descripcion);

        if (!queryAlerta.exec()) {
            qDebug() << "Error al insertar alerta:" << queryAlerta.lastError().text();
        }
    }else{
        estado = "OK";
    }
    QList<QStandardItem*> fila;
    fila << new QStandardItem(QString::number(litros, 'f', 2));
    fila << new QStandardItem(QString::number(medidor, 'f', 2));
    fila << new QStandardItem(estado);
    model6->appendRow(fila);
}

void Consultas::closeEvent(QCloseEvent *event){
    ui->SumaLitros_Label->setText("");
    ui->SumaVentas_label->setText("");
    model1->setRowCount(0);
    model2->setRowCount(0);
    model4->setRowCount(0);
    model5->setRowCount(0);
    model6->setRowCount(0);

    ui->comboBox_Combustible_Page1->setCurrentIndex(-1);
    ui->comboBox_Estacion_Page1->setCurrentIndex(-1);
    ui->comboBox_Ranking_Page2->setCurrentIndex(-1);
    ui->comboBox_Estacion_Page3->setCurrentIndex(-1);
    ui->comboBox_Combustible_Page3->setCurrentIndex(-1);
    ui->comboBox_Estacion_Page4->setCurrentIndex(-1);
    ui->comboBox_Ranking_Page4->setCurrentIndex(-1);
    ui->comboBox_Combustible_Page5->setCurrentIndex(-1);
    ui->comboBox_Combustible_Page6->setCurrentIndex(-1);
    ui->comboBox_Estacion_Page6->setCurrentIndex(-1);
    ui->PrecioVigente_Label_Page3->setText("");
}

