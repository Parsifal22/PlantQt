#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "top.h"
#include "Emulator.h"
#include "Consumer.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateLogbookSlot(const QString& data);

private slots:
    void on_Open_clicked();

    void on_Close_clicked();

    void on_Exit_clicked();

    void on_Connect_clicked();

    void on_Disconnect_clicked();

    void on_Start_clicked();

    void on_Stop_clicked();

    void on_Resume_clicked();

    void on_Break_clicked();

    void updateLogData(const QString &logData);

    void on_Show_clicked();

    void on_Limits_clicked();

    void on_Clear_clicked();

private:
    Ui::MainWindow *ui;
    QString fileName;
    Emulator e;
    Consumer c;
    ControlData *cd;
    std::thread consumerThread;
};
#endif // MAINWINDOW_H
