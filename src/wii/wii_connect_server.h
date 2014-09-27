#ifndef WII_WII_CONNECTOR_SERVER_H_
#define WII_WII_CONNECTOR_SERVER_H_

#include <array>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "base/base.h"
#include "capture/analyzer_result_drawer.h"
#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/puyo_color.h"
#include "core/real_color.h"
#include "core/server/connector/connector_manager.h"
#include "gui/drawer.h"
#include "gui/unique_sdl_surface.h"

class Analyzer;
class AnalyzerResult;
class KeySender;
class Source;

class WiiConnectServer : public Drawer, public AnalyzerResultRetriever {
public:
    WiiConnectServer(Source*, Analyzer*, KeySender*, const std::string& p1, const std::string& p2);
    ~WiiConnectServer();

    virtual void draw(Screen*) override;
    virtual std::unique_ptr<AnalyzerResult> analyzerResult() const override;

    bool start();
    void stop();

private:
    void reset();
    void runLoop();

    bool playForUnknown(int frameId);
    bool playForLevelSelect(int frameId, const AnalyzerResult&);
    bool playForPlaying(int frameId, const AnalyzerResult&);
    bool playForFinished(int frameId);

    ConnectorFrameRequest makeFrameRequestFor(int playerId, int frameId, const AnalyzerResult&);
    void outputKeys(int playerId, const AnalyzerResult&, const std::vector<ConnectorFrameResponse>&);

    PuyoColor toPuyoColor(RealColor, bool allowAllocation = false);

    std::thread th_;
    volatile bool shouldStop_;
    std::unique_ptr<ConnectorManager> connector_;

    // These 3 field should be used for only drawing.
    mutable std::mutex mu_;
    UniqueSDLSurface surface_;
    std::deque<std::unique_ptr<AnalyzerResult>> analyzerResults_;

    Source* source_;
    Analyzer* analyzer_;
    KeySender* keySender_;

    std::map<RealColor, PuyoColor> colorMap_;
    std::array<bool, 4> colorsUsed_;

    bool isAi_[2];
    Decision lastDecision_[2];
};

#endif
