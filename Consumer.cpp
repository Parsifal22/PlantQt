#include "Consumer.h"
#include "mainwindow.h"

Consumer::Consumer() :  pCd(nullptr), pFileBuf(new vector<unsigned char>()) {}

void Consumer::operator() ()
{
    while (true) {

        if (pCd->state == 's')
        {
            break;
        }


        if (pCd->state == 'r')
        {
            unique_lock<mutex> lock(pCd->mx);
            pCd->cv.wait(lock, [&]() { return !pCd->pBuf->empty(); });
            // Open the file in binary mode for writing
            ofstream outFile(filePath, ios::binary | ios::app);

            if (outFile.is_open()) {

                // Write the buffer data to the file
                unsigned int concatenatedValue = (static_cast<unsigned char>((*pCd->pBuf)[0])) | (static_cast<int>((*pCd->pBuf)[1]) << 8) | (static_cast<int>((*pCd->pBuf)[2]) << 16) | static_cast<int>((*pCd->pBuf)[3] << 24);

                // Add to the number of bytes 8 bytes for time
                concatenatedValue += 8;

                // Divide the final number into 4 parts
                (*pCd->pBuf)[3] = (static_cast<int>(concatenatedValue) >> 24) & 0xFF;
                (*pCd->pBuf)[2] = (static_cast<int>(concatenatedValue) >> 16) & 0xFF;
                (*pCd->pBuf)[1] = (static_cast<int>(concatenatedValue) >> 8) & 0xFF;
                (*pCd->pBuf)[0] = static_cast<int>(concatenatedValue) & 0xFF;

                // Get the current time
                auto currentTime = std::chrono::system_clock::now();

                auto duration = currentTime.time_since_epoch();


                // Copy the seconds as a byte sequence to the vector
                unsigned char* data = reinterpret_cast<unsigned char*>(&duration);
                pCd->pBuf->insert(pCd->pBuf->end(), data, data + sizeof(duration));

                outFile.write(reinterpret_cast<const char*>(&(*pCd->pBuf)[0]), pCd->pBuf->size());

                outFile.close();
            }
            else {
                cerr << "Error opening the file for writing." << endl;
            }


            convertBufferData(pCd->pBuf);
            toString();
            cout << endl << endl;
        }

        pCd->pBuf->clear();
        pCd->cv.notify_one();

    }

}

void Consumer::sendDataToGUI(const QString& data) {
    // Ensure GUI is not null and logData is not empty before emitting the signal
    if (gui && !data.isEmpty()) {
        // Emit signal to update the Logbook in the GUI
        emit updateLogbookSignal(data);
    }
}

void Consumer::convertBufferData(vector<unsigned char>* pBuf)
{

    int offset = 0;
    int sumOfBytes = 0;
    // Loop until the iterator reaches the end of the vector
    while (offset < pBuf->size()-1) {
        inputDataTotalBytes = 0;
        inputDataNumChannels = 0;

        for (int i = 0; i < 4; i++)
        {

            inputDataTotalBytes |= static_cast<int>((*pBuf)[offset + i]) << 8 * i;
            inputDataNumChannels |= static_cast<int>((*pBuf)[(i + offset) + 4]) << 8 * i;

        }

        offset += 8; // Start offset for channel data

        Channel channel;

        // Loop through each channel
        for (unsigned int i = 0; i < inputDataNumChannels; ++i) {



            string channelName;

            int pointsNumber = 0;

            // Extract number of points and channel name
            for (int i = 0; i < 4; i++)
            {
                pointsNumber |= static_cast<unsigned char>((*pBuf)[i + offset]) << 8 * i;
            }

            offset += 4;

            while ((*pBuf)[offset] != '\0') {
                channelName += static_cast<char>((*pBuf)[offset]) & 0xFF;
                offset++;
            }
            offset++;


            Point* point = new Point();  // Allocate Point on the heap

            // Loop through each point in the channel
            for (int j = 0; j < pointsNumber; ++j) {



                string pointName;

                variant <int, double> pointValue;


                while ((*pBuf)[offset] != '\0') {

                    pointName += static_cast<char>((*pBuf)[offset]) & 0xFF;
                    offset++;
                }
                offset++;

                if (pointName == "Level")
                {
                    int tempValue = 0;
                    // Extract number of points and channel name
                    for (int i = 0; i < 4; i++)
                    {

                        tempValue |= static_cast<int>((*pBuf)[i + offset]) << 8 * i;
                    }

                    pointValue = tempValue;

                    offset += 4;
                }
                else if (pointName == "Input turbidity" || pointName == "Output turbidity")
                {
                    uint64_t tempValue = 0;
                    // Extract number of points and channel name
                    for (int i = 0; i < 8; i++)
                    {
                        tempValue |= static_cast<uint64_t>((*pBuf)[i + offset]) << (8 * i);
                    }

                    pointValue = *reinterpret_cast<double*>(&tempValue);

                    offset += 8;
                }
                else if (pointName == "Output air pressure")
                {
                    uint64_t tempValue = 0;
                    // Extract number of points and channel name
                    for (int i = 0; i < 8; i++)
                    {
                        tempValue |= static_cast<uint64_t>((*pBuf)[i + offset]) << (8 * i);
                    }

                    pointValue = *reinterpret_cast<double*>(&tempValue);

                    offset += 8;
                }

                // Assuming the time data is stored at the end of the vector
                unsigned long long start = 0;
                int time_data = inputDataTotalBytes - 7;
                for (int i = 0; i < 7; i++)
                {

                    start |= static_cast<unsigned long long>((*pBuf)[i + time_data + sumOfBytes]) << 8 * i;
                }


                std::chrono::system_clock::duration timePoint(start);

                std::chrono::system_clock::time_point readTime(timePoint);

                list<pair<variant<int, double>, chrono::system_clock::time_point>>* dataFroPoint =
                    new list<pair<variant<int, double>, chrono::system_clock::time_point>>{
                                                                                           {pointValue, readTime},
                                                                                           };

                point->insert(make_pair(pointName, dataFroPoint));


            }

            channel.insert(make_pair(channelName, point));
        }

        convertedData.push_back(channel);
        offset += 9;
        sumOfBytes += inputDataTotalBytes + 1;
    }
}




void Consumer::printAll(QString channel_name, QString point_name)
{
    std::stringstream sout;
    QString logData;
    if (convertedData.empty())
    {
        readFile();
        if(pFileBuf->empty())
        {
            sout << "File is empty" << endl;
            logData = QString::fromStdString(sout.str());
            sendDataToGUI(logData);
            sout.str("");
            return;
        }
        convertBufferData(pFileBuf);
    }

    for (const auto& dp : convertedData)
    {

        // Access the time information from the first element
        const auto& outerIt = dp.begin();
        const auto& innerMap = *outerIt->second;
        const auto& innerIt = innerMap.begin();
        const auto& innerTime = innerIt->second;

        if (innerTime->empty()) {
            cout << "No time information available." << endl;
            return;
        }

        const auto& listItem = innerTime->front();
        const auto& timePointValue = listItem.second;

        time_t t = chrono::system_clock::to_time_t(timePointValue);

        tm timeInfo;
        if (localtime_s(&timeInfo, &t) != 0) {
            cerr << "Failed to get local time" << endl;
        }

        sout << "Time Point: " << put_time(&timeInfo, "%T") << endl;
        logData = QString::fromStdString(sout.str());
        sendDataToGUI(logData);
        sout.str("");
        // Traversing the outer map
        for (const auto& outerPair : dp) {
            const string& outerKey = outerPair.first;
            if (channel_name != "")
            {
                if (outerKey.compare(channel_name.toStdString()))
                {
                    continue;
                }
            }
            const auto& innerMap = *outerPair.second;


            sout << "Channel: " << outerKey << endl;
            logData = QString::fromStdString(sout.str());
            sendDataToGUI(logData);
            sout.str("");
            // Traversing the inner map
            for (const auto& innerPair : innerMap) {

                const std::string& innerKey = innerPair.first;
                if (point_name != "")
                {

                    if (innerKey.compare(point_name.toStdString()))
                    {
                        continue;
                    }
                }
                const auto& innerList = *innerPair.second;

                // Traversing the list
                for (const auto& listItem : innerList) {
                    const std::variant<int, double>& variantValue = listItem.first;
                    //Print value
                    sout << innerKey << " = ";

                    visit([&](const auto& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            sout << arg << endl;
                        }
                        else if constexpr (std::is_same_v<T, double>) {
                            sout << arg << endl;
                        }
                    }, variantValue);

                    logData = QString::fromStdString(sout.str());
                    sendDataToGUI(logData);
                    sout.str("");
                }

            }
        }
        QString logData = QString::fromStdString(sout.str());
        sendDataToGUI(logData);
        sout.str("");
        sout << endl << endl;
    }


}

void Consumer::printLimits(QString channel_name, QString point_name)
{

    std::stringstream sout;

    if (convertedData.empty())
    {
        readFile();
        if(pFileBuf->empty())
        {
            sout << "File is empty" << endl;
            QString logData = QString::fromStdString(sout.str());
            sendDataToGUI(logData);
            return;
        }
        convertBufferData(pFileBuf);
    }

    for (const auto& dp : convertedData)
    {

        // Traversing the outer map
        for (const auto& outerPair : dp) {
            const string& outerKey = outerPair.first;
            if (channel_name != "")
            {
                if (outerKey.compare(channel_name.toStdString()))
                {
                    continue;
                }
            }
            const auto& innerMap = *outerPair.second;

            // Traversing the inner map
            for (const auto& innerPair : innerMap) {

                const std::string& innerKey = innerPair.first;
                if (point_name != "")
                {

                    if (innerKey.compare(point_name.toStdString()))
                    {
                        continue;
                    }
                }
                const auto& innerList = *innerPair.second;

                // Traversing the list
                for (const auto& listItem : innerList) {
                    const std::variant<int, double>& variantValue = listItem.first;

                    if (std::holds_alternative<int>(max) && std::holds_alternative<int>(variantValue)) {
                        int value = std::get<int>(max);
                        if (value == 0)
                        {
                            max = variantValue;
                            min = variantValue;
                        }


                    }

                    // Check if the variant has a double
                    if (!std::holds_alternative<double>(max) && std::holds_alternative<double>(variantValue)) {
                        max = variantValue;
                        min = variantValue;
                    }

                    // Check and update max
                    std::visit([&](const auto& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            if (std::holds_alternative<int>(max) && arg > std::get<int>(max)) {
                                max = arg;
                            }

                        }
                        else if constexpr (std::is_same_v<T, double>) {
                            if (std::holds_alternative<double>(max) && arg > std::get<double>(max)) {
                                max = arg;
                            }
                        }
                    }, variantValue);


                    // Check and update min
                    std::visit([&](const auto& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            if (std::holds_alternative<int>(min) && arg < std::get<int>(min)) {
                                min = arg;
                            }

                        }
                        else if constexpr (std::is_same_v<T, double>) {
                            if (std::holds_alternative<double>(min) && arg < std::get<double>(min)) {
                                min = arg;
                            }
                        }
                    }, variantValue);

                }
            }
        }
    }

    //Print value
    sout << "Channel: " << channel_name.toStdString() << endl;

    sout << point_name.toStdString() << " max" << " = ";

    visit([&](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            sout << arg << endl;
        }
        else if constexpr (std::is_same_v<T, double>) {
            sout << arg << endl;
        }
    }, max);

    //Print value
    sout << point_name.toStdString() << " min" << " = ";

    visit([&](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            sout << arg << endl;
        }
        else if constexpr (std::is_same_v<T, double>) {
            sout << arg << endl;
        }
    }, min);

    sout << endl << endl;
    QString logData = QString::fromStdString(sout.str());
    sout.str("");
    sendDataToGUI(logData);
}

void Consumer::toString()
{

    datapackage dp = convertedData.back();

    std::stringstream sout;

    // Access the converted data
    sout << "Total Bytes: " << inputDataTotalBytes << std::endl;
    sout << "Number of Channels: " << inputDataNumChannels << std::endl;

    // Access the time information from the first element
    const auto& outerIt = dp.begin();
    const auto& innerMap = *outerIt->second;
    const auto& innerIt = innerMap.begin();
    const auto& innerTime = innerIt->second;

    if (innerTime->empty()) {
        cout << "No time information available." << endl;
        return;
    }

    const auto& listItem = innerTime->front();
    const auto& timePointValue = listItem.second;

    time_t t = chrono::system_clock::to_time_t(timePointValue);

    tm timeInfo;
    if (localtime_s(&timeInfo, &t) != 0) {
        cerr << "Failed to get local time" << endl;
    }

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%T", &timeInfo);

    sout << "Time Point: " << put_time(&timeInfo, "%T") << endl;



    // Traversing the outer map
    for (const auto& outerPair : dp) {
        const string& outerKey = outerPair.first;
        const auto& innerMap = *outerPair.second;
        sout << "Channel: " << outerKey << endl;

        // Traversing the inner map
        for (const auto& innerPair : innerMap) {

            const std::string& innerKey = innerPair.first;
            const auto& innerList = *innerPair.second;

            // Traversing the list
            for (const auto& listItem : innerList) {
                const std::variant<int, double>& variantValue = listItem.first;


                //Print value
                sout << innerKey << " = ";

                std::visit([&](const auto& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, int>) {
                        sout << arg << endl;

                    }
                    else if constexpr (std::is_same_v<T, double>) {
                        sout << arg << endl;
                    }
                }, variantValue);          

            QString logData = QString::fromStdString(sout.str());
            sout.str("");
            sendDataToGUI(logData);


            }
        }
    }

}

void Consumer::readFile()
{
    // Open the binary file for reading in binary mode
    ifstream inputFile(filePath, ios::binary);

    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }


    inputFile.seekg(0, ios::end); // Move to the end of the file
    streampos fileSize = inputFile.tellg(); // Get the file size
    inputFile.seekg(0, ios::beg); // Move back to the beginning of the file

    // Resize the vector to accommodate the entire file content
    pFileBuf->resize(static_cast<size_t>(fileSize));

    // Read the content of the file into the vector
    inputFile.read(reinterpret_cast<char*>(pFileBuf->data()), fileSize);

    inputFile.close(); // Close the file
}

