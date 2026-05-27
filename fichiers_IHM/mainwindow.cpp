#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    m_serial = new QSerialPort();
    ui->setupUi(this);

    QList<QSerialPortInfo> listePorts = QSerialPortInfo::availablePorts();
    QStringList listePortNames;
    for (const QSerialPortInfo &info : listePorts) {
        listePortNames << info.portName();
    }
    ui->comboSTM->addItems(listePortNames);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Génération aléatoire de la clé AES (32 caractères hex = 16 octets)
void MainWindow::on_toolButton_clicked()
{
    const int length = 32;
    const QString hexChars = "abcdef0123456789";
    QString KEY;
    for (int i = 0; i < length; i++) {
        int index = QRandomGenerator::global()->bounded(hexChars.length());
        KEY.append(hexChars.at(index));
    }
    ui->lineEdit_2->setText(KEY);
}

// Changement de port série sélectionné
void MainWindow::on_comboSTM_currentTextChanged(const QString &arg1)
{
    qDebug() << "set port name " << arg1;
    m_serial->setPortName(arg1);
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
        if (portInfo.portName() == arg1) {
            ui->label_description->setText(portInfo.description());
            ui->label_manufacturer->setText(portInfo.manufacturer());
            ui->label_serial->setText(portInfo.serialNumber());
            ui->label_location->setText(portInfo.systemLocation());
            ui->label_vendor->setNum(portInfo.vendorIdentifier());
            ui->label_product->setNum(portInfo.productIdentifier());
        }
    }
}

// Connexion au port série
void MainWindow::on_CONNECTButton_clicked()
{
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setParity(QSerialPort::NoParity);

    if (!m_serial->open(QIODeviceBase::ReadWrite)) {
        qDebug() << "Erreur connexion";
        return;
    }
    qDebug() << "Bien connecté";
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::handleReadyRead);
}

// Déconnexion
void MainWindow::on_DYSCONNECTButton_clicked()
{
    m_serial->close();
    qDebug() << "Bien déconnecté";
}

// Envoi d'un octet test
void MainWindow::on_dataButton_clicked()
{
    m_serial->write("a");
    m_serial->waitForBytesWritten(3000);
}

// Bouton "Écrire config" — envoie la trame complète au nœud NRF
void MainWindow::on_EcrireConfigButton_clicked()
{
    QString nodeIDStr = ui->lineEdit->text();
    QString aesKeyStr = ui->lineEdit_2->text();

    if (nodeIDStr.isEmpty() || aesKeyStr.isEmpty()) {
        qDebug() << "NodeID ou clé AES manquant";
        return;
    }

    if (!m_serial->isOpen()) {
        qDebug() << "Port série non connecté";
        return;
    }

    // Conversion NodeID (2 octets)
    bool ok;
    uint16_t nodeID = nodeIDStr.toUShort(&ok, 16);
    if (!ok) {
        qDebug() << "NodeID invalide";
        return;
    }

    // Conversion KEY (16 octets = 32 caractères hex)
    if (aesKeyStr.length() != 32) {
        qDebug() << "La clé doit faire 32 caractères hex (16 octets)";
        return;
    }
    QByteArray keyBytes;
    for (int i = 0; i < 32; i += 2) {
        keyBytes.append((char)aesKeyStr.mid(i, 2).toUInt(&ok, 16));
        if (!ok) {
            qDebug() << "Clé invalide";
            return;
        }
    }

    // Construction de la trame (23 octets total)
    QByteArray trame;

    // 4 octets de start
    trame.append((char)0xAA);
    trame.append((char)0xBB);
    trame.append((char)0xCC);
    trame.append((char)0xDD);

    // 1 octet de commande : écrire = 0x01
    trame.append((char)0x01);

    // 2 octets de NodeID (big endian)
    trame.append((char)((nodeID >> 8) & 0xFF));
    trame.append((char)(nodeID & 0xFF));

    // 16 octets de KEY
    trame.append(keyBytes);

    m_serial->write(trame);
    m_serial->waitForBytesWritten(3000);

    qDebug() << "Trame envoyée :" << trame.toHex(' ');
}

// Bouton "Lire config" — demande la config au nœud NRF
void MainWindow::on_LireConfigButton_clicked()
{
    if (!m_serial->isOpen()) {
        qDebug() << "Port série non connecté";
        return;
    }

    m_readData.clear();

    QByteArray trame;

    // 4 octets de start
    trame.append((char)0xAA);
    trame.append((char)0xBB);
    trame.append((char)0xCC);
    trame.append((char)0xDD);

    // 1 octet de commande : lire = 0x02
    trame.append((char)0x02);

    // 2 octets NodeID à zéro (on veut justement les récupérer)
    trame.append((char)0x00);
    trame.append((char)0x00);

    // 16 octets KEY à zéro (idem)
    for (int i = 0; i < 16; i++) {
        trame.append((char)0x00);
    }

    m_serial->write(trame);
    m_serial->waitForBytesWritten(3000);

    qDebug() << "Trame lire envoyée :" << trame.toHex(' ');
    // La réponse arrivera dans handleReadyRead()
        
}

// Réception et parsing de la trame de réponse
void MainWindow::handleReadyRead()
{
    m_readData.append(m_serial->readAll());

    // On attend les 23 octets complets
    if (m_readData.size() < 23) return;

    // Vérification des octets de start
    if ((quint8)m_readData[0] != 0xAA || (quint8)m_readData[1] != 0xBB ||
        (quint8)m_readData[2] != 0xCC || (quint8)m_readData[3] != 0xDD) {
        qDebug() << "Trame invalide";
        m_readData.clear();
        return;
    }

    quint8   cmd    = (quint8)m_readData[4];
    uint16_t nodeID = ((quint8)m_readData[5] << 8) | (quint8)m_readData[6];
    QByteArray key  = m_readData.mid(7, 16);

    qDebug() << "Commande :" << Qt::hex << cmd;
    qDebug() << "NodeID   :" << Qt::hex << nodeID;
    qDebug() << "KEY      :" << key.toHex(' ');

    // Affichage dans l'IHM
    ui->lineEdit->setText(QString("%1").arg(nodeID, 4, 16, QChar('0')).toUpper());
    ui->lineEdit_2->setText(QString(key.toHex()));

    m_readData.clear();
}
