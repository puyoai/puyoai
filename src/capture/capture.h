#ifndef CAPTURE_CAPTURE_H_
#define CAPTURE_CAPTURE_H_

#include <memory>
#include <pthread.h>

#include "base/base.h"
#include "base/lock.h"
#include "capture/analyzer.h"
#include "capture/analyzer_result_drawer.h"
#include "gui/drawer.h"
#include "gui/unique_sdl_surface.h"

class Analyzer;
class Source;
class Screen;

class Capture : public Drawer, public AnalyzerResultRetriever {
public:
    explicit Capture(Source* source, Analyzer* analyzer);
    virtual ~Capture() {}

    bool start();
    void stop();

    virtual void draw(Screen*) OVERRIDE;

    virtual std::unique_ptr<AnalyzerResult> analyzerResult() const OVERRIDE;

private:
    static void* runLoopCallback(void*);
    void runLoop();

    Source* source_;
    Analyzer* analyzer_;

    pthread_t th_;
    volatile bool shouldStop_;

    mutable Mutex mu_;
    UniqueSDLSurface surface_;
    std::deque<std::unique_ptr<AnalyzerResult>> results_;
};

#endif  // CAPTURE_CAPTURE_H_
