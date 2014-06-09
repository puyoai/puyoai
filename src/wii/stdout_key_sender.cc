#include "wii/stdout_key_sender.h"

#include <iostream>

using namespace std;

void StdoutKeySender::sendKey(Key k1)
{
    cout << "SendKey: " << toString(k1) << endl;
}

void StdoutKeySender::sendKey(Key k1, Key k2)
{
    cout << "SendKey: " << toString(k1) << " " << toString(k2) << endl;
}
