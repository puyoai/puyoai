#include "duel/field_realtime.h"

#include <string>

#include <gtest/gtest.h>

#include "core/constant.h"
#include "core/kumipuyo.h"
#include "core/state.h"
#include "duel/field_realtime.h"

using namespace std;

class FieldRealtimeTest : public ::testing::Test {
protected:
    virtual void SetUp()
    {
        string sequence = "11223344";
        f_ = new FieldRealtime(0, sequence);
    }

    virtual void TearDown()
    {
        delete f_;
    }

    FieldRealtime* f_;
};

