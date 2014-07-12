#ifndef AUDIO_SPEAKER_H_
#define AUDIO_SPEAKER_H_

#include <string>

// Speaker is an interface to speak something.
class Speaker {
public:
    virtual ~Speaker() {}
    virtual void speak(const std::string&) = 0;
};

#endif
