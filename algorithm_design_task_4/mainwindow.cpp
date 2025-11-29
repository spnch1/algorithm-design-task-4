#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTime>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
{
    ui->setupUi(this);
    refreshTables();
    log("System initialized.");
}

MainWindow::~MainWindow()
{
}

void MainWindow::refreshTables()
{
    QList<IndexEntry> index = db.getIndexTable();
    ui->tableIndex->setRowCount(index.size());
    for (int i = 0; i < index.size(); ++i) {
        ui->tableIndex->setItem(i, 0, new QTableWidgetItem(QString::number(index[i].key)));
        ui->tableIndex->setItem(i, 1, new QTableWidgetItem(QString::number(index[i].offset)));
    }

    QList<Record> data = db.getDataTable();
    int limit = qMin(data.size(), 1000);
    ui->tableData->setRowCount(limit);
    for (int i = 0; i < limit; ++i) {
        ui->tableData->setItem(i, 0, new QTableWidgetItem(QString::number(data[i].key)));
        ui->tableData->setItem(i, 1, new QTableWidgetItem(QString::fromUtf8(data[i].data)));
    }
    if (data.size() > limit) {
        log(QString("Displaying first %1 records of %2.").arg(limit).arg(data.size()));
    }
}

void MainWindow::log(const QString& msg)
{
    ui->textLog->append(QTime::currentTime().toString() + ": " + msg);
}

void MainWindow::on_btnAdd_clicked()
{
    int key = ui->spinKey->value();
    QString data = ui->editData->text();

    if (data.isEmpty()) {
        QMessageBox::warning(this, "Error", "Data cannot be empty.");
        return;
    }

    if (db.add(key, data)) {
        log(QString("Added record: Key=%1, Data=%2").arg(key).arg(data));
        refreshTables();
        ui->editData->clear();
    } else {
        QMessageBox::warning(this, "Error", "Key already exists or error writing.");
    }
}

void MainWindow::on_btnSearch_clicked()
{
    int key = ui->spinKey->value();
    int comparisons = 0;
    QString result = db.search(key, comparisons);

    if (!result.isEmpty()) {
        log(QString("Found: Key=%1, Data=%2 (Comparisons: %3)").arg(key).arg(result).arg(comparisons));
        QMessageBox::information(this, "Found", QString("Key: %1\nData: %2\nComparisons: %3").arg(key).arg(result).arg(comparisons));
    } else {
        log(QString("Not Found: Key=%1 (Comparisons: %2)").arg(key).arg(comparisons));
        QMessageBox::information(this, "Not Found", QString("Key %1 not found.\nComparisons: %2").arg(key).arg(comparisons));
    }
}

void MainWindow::on_btnEdit_clicked()
{
    int key = ui->spinKey->value();
    QString data = ui->editData->text();

    if (data.isEmpty()) {
        QMessageBox::warning(this, "Error", "Data cannot be empty.");
        return;
    }

    if (db.edit(key, data)) {
        log(QString("Edited record: Key=%1, New Data=%2").arg(key).arg(data));
        refreshTables();
    } else {
        QMessageBox::warning(this, "Error", "Key not found.");
    }
}

void MainWindow::on_btnDelete_clicked()
{
    int key = ui->spinKey->value();

    if (db.remove(key)) {
        log(QString("Deleted record: Key=%1").arg(key));
        refreshTables();
    } else {
        QMessageBox::warning(this, "Error", "Key not found.");
    }
}

void MainWindow::on_btnGenerate_clicked()
{
    log("Generating 10,000 random records... Please wait.");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    db.generateRandom(10000);
    QApplication::restoreOverrideCursor();
    log("Generation complete.");
    refreshTables();
    
    int totalComparisons = 0;
    int tests = 25;
    QList<IndexEntry> index = db.getIndexTable();
    if (index.isEmpty()) return;

    for (int i = 0; i < tests; ++i) {
        int randIdx = QRandomGenerator::global()->bounded(index.size());
        int key = index[randIdx].key;
        int comps = 0;
        (void)db.search(key, comps);
        totalComparisons += comps;
    }
    double avg = (double)totalComparisons / tests;
    log(QString("Average comparisons for %1 searches: %2").arg(tests).arg(avg));
    QMessageBox::information(this, "Performance", QString("Average comparisons (25 searches): %1").arg(avg));
}

void MainWindow::on_btnInfo_clicked()
{
    QString info = "Complexity:\n" + db.getComplexity();
    
    ui->textLog->append("--------------------------------------------------");
    ui->textLog->append(info);
    ui->textLog->append("--------------------------------------------------");
}

void MainWindow::on_btnClear_clicked()
{
    db.clear();
    refreshTables();
    log("Database cleared.");
}
