#include "duel/frame_context.h"

#include "duel/field_realtime.h"

void FrameContext::apply(FieldRealtime* me, FieldRealtime* opponent)
{
    int restOjama = me->reduceOjama(numSentOjama_);
    opponent->addPendingOjama(restOjama);

    if (ojamaCommitted_)
        opponent->commitOjama();
}



