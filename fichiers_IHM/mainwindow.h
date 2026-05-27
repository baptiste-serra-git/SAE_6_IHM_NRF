#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

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

private slots:
    void on_toolButton_clicked();
    void on_comboSTM_currentTextChanged(const QString &arg1);
    void handleReadyRead();
    void on_CONNECTButton_clicked();
    void on_DYSCONNECTButton_clicked();
    void on_dataButton_clicked();
    void on_EcrireConfigButton_clicked();
    void on_LireConfigButton_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *m_serial;
    QByteArray m_readData;
};

#endif // MAINWINDOW_H
