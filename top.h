#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <variant>
#include <map>
#include <list>
#include <chrono>
#include <future>
#include <exception>
#include <thread>
#include <utility>

using namespace std;

#ifndef TOP_H
#define TOP_H

struct ControlData
{
    mutex mx;
    condition_variable cv;
    atomic<char> state;
    vector<unsigned char>* pBuf;
    promise<void>* pProm;
};

typedef map<string, map<string, list<pair<variant<int, double>, chrono::system_clock::time_point> >* >* > datapackage;
typedef map<string, map<string, list<pair<variant<int, double>, chrono::system_clock::time_point> >* >* > Channel;
typedef map<string, list<pair <variant<int, double>, chrono::system_clock::time_point> >* > Point;

#endif // TOP_H
