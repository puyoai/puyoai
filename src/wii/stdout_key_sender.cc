#include "wii/stdout_key_sender.h"

#include <iostream>

using namespace std;

void StdoutKeySender::sendKeySet(const KeySet& keySet)
{
    cout << "SendKey: " << keySet.toString() << endl;
}

void StdoutKeySender::sendWait(int ms)
{
    cout << "Wait: " << ms << endl;
}
