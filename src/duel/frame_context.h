#ifndef DUEL_FRAME_CONTEXT_H_
#define DUEL_FRAME_CONTEXT_H_

class FieldRealtime;

class FrameContext {
public:
    void sendOjama(int num) { numSentOjama_ += num; }
    void commitOjama() { ojamaCommitted_ = true; }

    void apply(FieldRealtime* me, FieldRealtime* opponent);

private:
    int numSentOjama_ = 0;
    bool ojamaCommitted_ = false;
};

#endif
