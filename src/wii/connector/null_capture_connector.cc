#include "capture/connector/null_capture_connector.h"

#include <iostream>

void NullCaptureConnector::sendKey(Key k)
{
    cout << k << endl;
}

void NullCaptureConnector::sendKey(Key k1, Key k2)
{
    cout << k1 << ' ' << k2 << endl;
}
