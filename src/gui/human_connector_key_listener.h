#ifndef GUI_HUMAN_CONNECTOR_KEY_LISTENER_H_
#define GUI_HUMAN_CONNECTOR_KEY_LISTENER_H_

#include "base/base.h"
#include "gui/main_window.h"
class HumanConnector;

class HumanConnectorKeyListener : public MainWindow::EventListener {
public:
    explicit HumanConnectorKeyListener(HumanConnector*);
    virtual ~HumanConnectorKeyListener();

    virtual void handleAfterPollEvent() OVERRIDE;

private:
    HumanConnector* connector_;
};

#endif
