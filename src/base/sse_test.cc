#include "base/sse.h"

#include <gtest/gtest.h>

TEST(SSETest, inverseMovemask)
{
    for (int i = 0; i < 0x10000; ++i) {
        __m128i m = sse::inverseMovemask(i);
        int expected = _mm_movemask_epi8(m);

        EXPECT_EQ(expected, i);
    }
}
