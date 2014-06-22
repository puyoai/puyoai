#ifndef GUI_HUMAN_CONNECTOR_KEY_LISTENER_H_
#define GUI_HUMAN_CONNECTOR_KEY_LISTENER_H_

#include "base/base.h"
#include "core/server/connector/human_connector.h"
#include "gui/main_window.h"

class HumanConnectorKeyListener : public MainWindow::EventListener {
public:
    explicit HumanConnectorKeyListener(HumanConnector*);
    virtual ~HumanConnectorKeyListener();

    virtual void handleEvent(const SDL_Event&);
    virtual void handleAfterPollEvent() OVERRIDE;

private:
    HumanConnector* connector_;
    KeySet keySet_;
};

#endif
