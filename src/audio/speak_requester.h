#ifndef AUDIO_SPEAK_REQUESTER_H_
#define AUDIO_SPEAK_REQUESTER_H_

#include <string>

struct SpeakRequest {
    std::string text;
};

class SpeakRequester {
public:
    virtual ~SpeakRequester() {}
    virtual SpeakRequest requestSpeak() = 0;
};

#endif
