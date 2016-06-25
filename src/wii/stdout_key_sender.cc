#include "wii/stdout_key_sender.h"

#include <iostream>

using namespace std;

void StdoutKeySender::sendWait(int ms)
{
    cout << "Wait: " << ms << endl;
}

void StdoutKeySender::sendKeySet(const KeySet& keySet, bool forceSend)
{
    UNUSED_VARIABLE(forceSend);

    cout << "SendKey: " << keySet.toString() << endl;
}

void StdoutKeySender::sendKeySetSeq(const KeySetSeq& keySetSeq)
{
    cout << "SendKey: " << keySetSeq.toString() << endl;
}
