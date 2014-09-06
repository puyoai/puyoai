#ifndef CAPTURE_ANALYZER_RESULT_DRAWER
#define CAPTURE_ANALYZER_RESULT_DRAWER

#include <memory>

#include "base/base.h"
#include "gui/drawer.h"

class AnalyzerResult;
class Capture;
class Screen;

class AnalyzerResultRetriever {
public:
    virtual ~AnalyzerResultRetriever() {}
    virtual std::unique_ptr<AnalyzerResult> analyzerResult() const = 0;
};

class AnalyzerResultDrawer : public Drawer {
public:
    // Don't take ownership of Capture.
    explicit AnalyzerResultDrawer(AnalyzerResultRetriever*);

    void draw(Screen*) override;

private:
    AnalyzerResultRetriever* retriever_;
};

#endif
