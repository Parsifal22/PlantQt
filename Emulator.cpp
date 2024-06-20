#include "Emulator.h"


Emulator::~Emulator()
{
    if (hDLL != nullptr)
    {
        FreeLibrary(hDLL);
    }
}

void Emulator::disconnectDLL()
{
    if (hDLL != nullptr)
    {
        FreeLibrary(hDLL);
        hDLL = nullptr;
        cout << "Emulator disabled successfully" << endl;
    }
    else
    {
        cout << "No connection to emulator" << endl;
    }
}

void Emulator::getDLL(const wchar_t* dllPath)
{
    if (hDLL != nullptr)
    {
        cout << "The DLL is already attached " << endl;
        return;
    }
    // Get the DLL handle.
    hDLL = LoadLibrary(dllPath);


    if (hDLL == NULL)
    {
        cout << "IAS0410PlantEmulator1.dll not found, error " << GetLastError() << endl;
        return;
    }
}


void Emulator::setEmulator(const char* textPath, int plantNumber)
{
    // Get the pointer to the function you want to import
    FARPROC setFun = GetProcAddress(hDLL, "SetIAS0410PlantEmulator");


    if (setFun == NULL)
    {
        FreeLibrary(hDLL);
        cout << "Function SetIAS0410PlantEmulator() not found, error  " << GetLastError() << endl;
        return;
    }



    // Call the imported function
    try
    {
        // Provide the necessary parameters for SetIAS0410PlantEmulator
        ((void(*)(string, int))setFun)(textPath, plantNumber);

        cout << "SetIAS0410PlantEmulator called successfully" << endl;
    }
    catch (const std::bad_alloc& ex)
    {
        cout << "Memory allocation error: " << ex.what() << endl;
    }
    catch (const std::exception& ex)
    {
        cout << "Error calling SetIAS0410PlantEmulator: " << ex.what() << endl;
    }
}


void Emulator::runEmulator()
{
    // Get the pointer to the function you want to import
    FARPROC setFun = GetProcAddress(hDLL, "RunIAS0410PlantEmulator");


    if (setFun == NULL)
    {
        FreeLibrary(hDLL);
        cout << "Function RunIAS0410PlantEmulator() not found, error  " << GetLastError() << endl;
        return;
    }

    // Assuming you have a vector and a promise ready
    std::vector<unsigned char> myVector;
    std::promise<void> myPromise;


    // Call the imported function
    try
    {

        // Provide the necessary parameters for SetIAS0410PlantEmulator
        ((void(*)(ControlData*))setFun)(pCd);

        cout << "RunIAS0410PlantEmulator called successfully" << endl;
    }
    catch (const bad_alloc& ex)
    {
        cout << "Memory allocation error: " << ex.what() << endl;
        delete pCd->pBuf;
        delete pCd->pProm;
        delete pCd;
    }
    catch (const std::exception& ex)
    {
        cout << "Error calling RunIAS0410PlantEmulator: " << ex.what() << endl;
        delete pCd->pBuf;
        delete pCd->pProm;
        delete pCd;
    }
}
