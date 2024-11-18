#include "tcpfilesender.h"

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent)
{
    loadSize = 1024 * 4;
    totalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;
    // 初始化 UI 组件
    ipLineEdit = new QLineEdit(this);
    ipLineEdit->setPlaceholderText("輸入伺服器IP (如: 127.0.0.1)");

    portLineEdit = new QLineEdit(this);
    portLineEdit->setPlaceholderText("輸入伺服器端口 (如: 16998)");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    clientProgressBar = new QProgressBar;
    clientStatusLabel = new QLabel(QStringLiteral("客戶端就緒"));
    startButton = new QPushButton(QStringLiteral("開始"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    openButton = new QPushButton(QStringLiteral("開檔"));
    startButton->setEnabled(false);

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(openButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(ipLineEdit);
    mainLayout->addWidget(portLineEdit);
    mainLayout->addWidget(clientProgressBar);
    mainLayout->addWidget(clientStatusLabel);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("檔案傳送"));

    // 连接信号和槽
    connect(startButton, &QPushButton::clicked, this, &TcpFileSender::configureConnection);
    connect(openButton, &QPushButton::clicked, this, &TcpFileSender::openFile);
    connect(quitButton, &QPushButton::clicked, this, &TcpFileSender::close);
    connect(&tcpClient, &QTcpSocket::bytesWritten, this, &TcpFileSender::updateClientProgress);

}
// 动态配置 IP 和端口
void TcpFileSender::configureConnection()
{
    serverIP = ipLineEdit->text();
    serverPort = portLineEdit->text().toUShort();

    if (serverIP.isEmpty() || serverPort == 0) {
        QMessageBox::warning(this, QStringLiteral("配置錯誤"), QStringLiteral("請輸入有效的IP地址和端口號！"));
        return;
    }

    clientStatusLabel->setText(QStringLiteral("正在連接到 %1:%2...").arg(serverIP).arg(serverPort));
    tcpClient.connectToHost(serverIP, serverPort);

    if (!tcpClient.waitForConnected(3000)) {
        QMessageBox::critical(this, QStringLiteral("連接失敗"), tcpClient.errorString());
        return;
    }

    clientStatusLabel->setText(QStringLiteral("連接成功，準備傳輸"));
    startTransfer();
}
void TcpFileSender::openFile()
{
    fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) startButton->setEnabled(true);
}
void TcpFileSender::start()
{
    startButton->setEnabled(false);
    bytesWritten = 0;
    clientStatusLabel->setText(QStringLiteral("連接中..."));
    tcpClient.connectToHost(QHostAddress("127.0.0.1"), 16998);
}
void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly))
     {
        QMessageBox::warning(this,QStringLiteral("應用程式"),
                              QStringLiteral("無法讀取 %1:\n%2.").arg(fileName)
                              .arg(localFile->errorString()));
        return;
     }

    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);
    QString currentFile = fileName.right(fileName.size() -
                                         fileName.lastIndexOf("/")-1);
    sendOut <<qint64(0)<<qint64(0)<<currentFile;
    totalBytes += outBlock.size();

    sendOut.device()->seek(0);
    sendOut<<totalBytes<<qint64((outBlock.size()-sizeof(qint64)*2));
    bytesToWrite = totalBytes - tcpClient.write(outBlock);
    clientStatusLabel->setText(QStringLiteral("已連接"));
    qDebug() << currentFile <<totalBytes;
    outBlock.resize(0);
}
void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += (int) numBytes;
    if(bytesToWrite > 0)
    {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= (int) tcpClient.write(outBlock);
        outBlock.resize(0);
    }else
    {
        localFile->close();
    }

    clientProgressBar->setMaximum(totalBytes);
    clientProgressBar->setValue(bytesWritten);
    clientStatusLabel->setText(QStringLiteral("已傳送 %1 Bytes").arg(bytesWritten));
}

TcpFileSender::~TcpFileSender()
{

}



