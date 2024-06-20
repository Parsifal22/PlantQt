#ifndef CONSUMER_H
#define CONSUMER_H

#include <QThread>
#include <QTextEdit>
#include <QString>
#include "top.h"

class MainWindow;

class Consumer : public QThread
{
    Q_OBJECT

private:

    ControlData *pCd;
    vector<datapackage> convertedData;
    vector<unsigned char>* pFileBuf;

    MainWindow* gui;

    string filePath;
    int inputDataTotalBytes;
    int inputDataNumChannels;

    variant<int, double> max;
    variant<int, double> min;

    void convertBufferData(vector<unsigned char>* pBuf);

    void toString();

public:

    inline void setFilePath(QString path) { filePath = path.toStdString(); }
    inline void setBuffer(ControlData* Cd) { pCd = Cd; }
    inline void setGUI(MainWindow* ui) {gui = ui;}
    inline int getSizeConvertedData() { return convertedData.size(); }
    void readFile();
    void printAll(QString channel_name, QString point_name);
    void printLimits(QString channel_name, QString point_name);
    Consumer();
    void operator() ();

    void sendDataToGUI(const QString& data);

    std::vector<char> guiBuffer;

signals:
    void updateLogbookSignal(const QString& data);
};

#endif // CONSUMER_H
