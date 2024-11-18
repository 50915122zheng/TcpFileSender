#ifndef TCPFILESENDER_H
#define TCPFILESENDER_H

#include <QDialog>
#include <QtNetwork>
#include <QtWidgets>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    TcpFileSender(QWidget *parent = 0);
    ~TcpFileSender();
public slots:
    void configureConnection(); // 动态配置 IP 和端口
    void start();
    void startTransfer();
    void updateClientProgress(qint64 numBytes);
    void openFile();
private:
    QLineEdit        *ipLineEdit;       // 输入 IP 的文本框
    QLineEdit        *portLineEdit;
    QProgressBar     *clientProgressBar;
    QLabel           *clientStatusLabel;
    QPushButton      *startButton;
    QPushButton      *quitButton;
    QPushButton      *openButton;
    QDialogButtonBox *buttonBox;
    QTcpSocket       tcpClient;

    QString          serverIP;          // 存储 IP 地址
    quint16          serverPort;

    qint64           totalBytes;
    qint64           bytesWritten;
    qint64           bytesToWrite;
    qint64           loadSize;
    QString          fileName;
    QFile            *localFile;
    QByteArray       outBlock;
};

#endif // TCPFILESENDER_H
