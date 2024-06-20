#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QShortcut>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&c, &Consumer::updateLogbookSignal, this, &MainWindow::updateLogbookSlot);

    // Create a shortcut for the Exit button
    QShortcut *exitShortcut = new QShortcut(QKeySequence("x"), this);
    connect(exitShortcut, &QShortcut::activated, this, &MainWindow::on_Exit_clicked);
    // Initializing ControlData variable

    cd = new ControlData();

    c.setGUI(this);

    // Start the keyboard input thread
    fileName = "";

    // Disable the button
    ui->Close->setEnabled(false);
    ui->Disconnect->setEnabled(false);
    ui->Connect->setEnabled(false);
    ui->Start->setEnabled(false);
    ui->Break->setEnabled(false);
    ui->Resume->setEnabled(false);
    ui->Stop->setEnabled(false);
    ui->Limits->setEnabled(false);
    ui->Show->setEnabled(false);
    ui->Clear->setEnabled(false);

}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Open_clicked()
{
    // Open a file dialog to choose a file
    fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Binary Files (*.bin);;Text Files (*.txt)");

    // Check if a file was chosen
    if (!fileName.isEmpty()) {
        // Disable the button
        ui->Open->setEnabled(false);
        ui->Close->setEnabled(true);
        ui->Connect->setEnabled(true);
        ui->Limits->setEnabled(true);
        ui->Show->setEnabled(true);
    }

    // Display the chosen file name (for demonstration purposes)
    qDebug() << "Chosen file: " << fileName;
    c.setFilePath(fileName);
}


void MainWindow::on_Close_clicked()
{
    fileName = "";
    // Check if a file was chosen
    if (fileName.isEmpty()) {
        // Disable the button
        ui->Close->setEnabled(false);
        ui->Open->setEnabled(true);
        ui->Connect->setEnabled(false);
        ui->Limits->setEnabled(false);
        ui->Show->setEnabled(false);
        ui->Start->setEnabled(false);
    }
    // Display the chosen file name (for demonstration purposes)
    qDebug() << "Close File";
}


void MainWindow::on_Exit_clicked()
{
    QCoreApplication::quit();
}


void MainWindow::on_Connect_clicked()
{

    e.getDLL(L"C:/Users/Nikita/source/Qt/DLL/IAS0410PlantEmulator.dll");

    e.setEmulator("C:/Users/Nikita/source/Qt/DLL/IAS0410Plants.txt", 4);

    if (ifstream("C:\\Users\\Nikita\\source\\repos\\PlantSTL\\result.bin", ios::binary | ios::ate).tellg() != 0 && c.getSizeConvertedData() == 0) {
        c.readFile();

    }

    ui->Connect->setEnabled(false);
    ui->Disconnect->setEnabled(true);
    ui->Start->setEnabled(true);
    ui->Limits->setEnabled(false);
    ui->Show->setEnabled(false);
}


void MainWindow::updateLogData(const QString &logData)
{
    // Update your UI with the received log data
    ui->Logbook->appendPlainText(logData);
}

void MainWindow::on_Disconnect_clicked()
{
    e.disconnectDLL();
    ui->Connect->setEnabled(true);
    ui->Disconnect->setEnabled(false);
    ui->Limits->setEnabled(true);
    ui->Show->setEnabled(true);
    ui->Break->setEnabled(false);
    ui->Stop->setEnabled(false);
}


void MainWindow::on_Start_clicked()
{

    cd->state = 'r';
    // Allocate memory for pBuf
    cd->pBuf = new vector<unsigned char>();
    // Allocate memory for pProm
    cd->pProm = new promise<void>();

    c.setBuffer(cd);
    e.setBuffer(cd);
    // Launch consumer in a separate thread
    consumerThread = std::thread(&Consumer::operator(), &c);
    consumerThread.detach();

    // Launch emulator asynchronously
    future<void> emulatorFuture = async(launch::async, &Emulator::runEmulator, &e);


    ui->Start->setEnabled(false);
    ui->Break->setEnabled(true);
    ui->Stop->setEnabled(true);
}


void MainWindow::on_Stop_clicked()
{
    cd->state = 's';
    ui->Start->setEnabled(true);
    ui->Break->setEnabled(false);
    ui->Stop->setEnabled(false);
    ui->Resume->setEnabled(false);
}


void MainWindow::on_Resume_clicked()
{
    cd->state = 'r';

    ui->Break->setEnabled(true);
    ui->Stop->setEnabled(true);
    ui->Resume->setEnabled(false);
    ui->Start->setEnabled(false);
}


void MainWindow::on_Break_clicked()
{
    cd->state = 'b';
    ui->Break->setEnabled(false);
    ui->Stop->setEnabled(true);
    ui->Resume->setEnabled(true);
    ui->Start->setEnabled(false);
}

void MainWindow::updateLogbookSlot(const QString& data) {
    // Update the Logbook widget with the received data
    ui->Logbook->appendPlainText(data);

    // Make the Logbook widget read-only
    ui->Logbook->setReadOnly(true);

    ui->Clear->setEnabled(true);
}

void MainWindow::on_Show_clicked()
{
    c.printAll(ui->Channel->text(), ui->Point->text());

}


void MainWindow::on_Limits_clicked()
{
    c.printLimits(ui->Channel->text(), ui->Point->text());
}


void MainWindow::on_Clear_clicked()
{
    ui->Logbook->clear();
    ui->Clear->setEnabled(false);
}

