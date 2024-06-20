#ifndef EMULATOR_H
#define EMULATOR_H
#include "top.h"


class Emulator
{
public:

    Emulator() : hDLL(nullptr), pCd(nullptr) {}
    ~Emulator();

    void getDLL(const wchar_t* dllPath);
    void setEmulator(const char* textPath, int plantNumber);
    void runEmulator();
    void disconnectDLL();

    inline void setBuffer(ControlData* Cd) { pCd = Cd; }
    inline vector <unsigned char> getBuf() { return *pCd->pBuf; }

    inline int getDLLstatus() { return 1 ? hDLL == nullptr : 0; }
    inline mutex& getMutex() { return pCd->mx; }

private:
    HMODULE hDLL;
    ControlData *pCd;
};

#endif // EMULATOR_H
