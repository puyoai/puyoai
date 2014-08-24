#include "wii/stdout_key_sender.h"

#include <iostream>

using namespace std;

void StdoutKeySender::sendKey(const KeySet& keySet)
{
    cout << "SendKey: " << keySet.toString() << endl;
}
