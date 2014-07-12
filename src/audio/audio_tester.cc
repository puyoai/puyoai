#include "audio/audio_commentator.h"

#include <iostream>
#include <string>

#include "internal/audio/internal_speaker.h"

using namespace std;

int main(void)
{
    InternalSpeaker speaker;
    AudioCommentator ac(&speaker);
    ac.addText("進捗どうですか");
    ac.addText("進捗だめです");

    ac.start();

    string s;
    getline(cin, s);
    // ac.stop();

    return 0;
}
