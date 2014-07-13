#ifndef AUDIO_SPEAK_REQUESTER_H_
#define AUDIO_SPEAK_REQUESTER_H_

#include <string>

struct SpeakRequest {
    SpeakRequest() {}
    explicit SpeakRequest(std::string text) : text(text) {}
    std::string text;
};

class SpeakRequester {
public:
    virtual ~SpeakRequester() {}
    virtual SpeakRequest requestSpeak() = 0;
};

#endif
