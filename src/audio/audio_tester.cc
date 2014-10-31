#include "audio/audio_server.h"

#include <iostream>
#include <string>

#include "internal/audio/internal_speaker.h"

using namespace std;

int main(void)
{
    InternalSpeaker speaker;
    AudioServer ac(&speaker);
    ac.start();

    ac.submit("進捗どうですか");
    ac.submit("進捗だめです");

    string s;
    getline(cin, s);
    // ac.stop();

    return 0;
}
