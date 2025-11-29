#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "databasemanager.h"
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnAdd_clicked();
    void on_btnSearch_clicked();
    void on_btnEdit_clicked();
    void on_btnDelete_clicked();
    void on_btnGenerate_clicked();
    void on_btnInfo_clicked();
    void on_btnClear_clicked();
    void on_btnPerformanceTest_clicked();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    DatabaseManager db;

    void refreshTables();
    void log(const QString& msg);
};
#endif // MAINWINDOW_H
