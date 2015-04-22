#include "wii/wii_connect_server.h"

#include <gtest/gtest.h>

#include "core/decision.h"
#include "core/kumipuyo_pos.h"
#include "core/plain_field.h"

TEST(WiiConnectorServerTest, calculateDropPosition)
{
    PlainField pf("..O..."
                  "..O..."
                  "..O...");

    EXPECT_EQ(KumipuyoPos(3, 4, 1), WiiConnectServer::calculateDropPosition(pf, Decision(3, 1)));
    EXPECT_EQ(KumipuyoPos(4, 4, 3), WiiConnectServer::calculateDropPosition(pf, Decision(4, 3)));
}
