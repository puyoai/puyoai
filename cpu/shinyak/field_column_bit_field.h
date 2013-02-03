#ifndef __FIELD_COLUMN_BIT_FIELD_H_
#define __FIELD_COLUMN_BIT_FIELD_H_

#include <string.h>
#include <glog/logging.h>
#include "field.h"

class FieldColumnBitField {
public:
    FieldColumnBitField() {
        memset(bitField, 0, sizeof(int) * 8 * 6);
    }

    FieldColumnBitField(const Field& field) {
        memset(bitField, 0, sizeof(int) * 8 * 6);
        setField(field);
    }

    void setField(const Field& field) {
        for (int x = 1; x <= Field::WIDTH; ++x) {
            for (int y = 1; y <= Field::HEIGHT; ++y)
                set(field.color(x, y), x, y);
        }
    }

    int get(PuyoColor color, int x) const {
        return bitField[static_cast<int>(color)][x-1];
    }
    void set(PuyoColor color, int x, int y) {
        bitField[static_cast<int>(color)][x-1] |= 1 << (y - 1);
    }

    // xxxx  2  5  8 11    11  8  5  2
    // xxxx  1  4  7 10    10  7  4  1 
    // xxxx  0  3  6  9     9  6  3  0
    int indexFor34(int leftX, int bottomY, PuyoColor a, PuyoColor b) {
        DCHECK(1 <= leftX && leftX <= 3);
        DCHECK(a < b);
        DCHECK(a != EMPTY && b != EMPTY);

        int indexA = 0, indexB = 0, indexC = 0;
        if (leftX != 3) {
            for (int offsetX = 0; offsetX < 4; ++offsetX) {
                int x = offsetX + leftX - 1;
                indexA |= (0x7 & (bitField[a][x] >> (bottomY - 1))) << (offsetX * 3);
                indexB |= (0x7 & (bitField[b][x] >> (bottomY - 1))) << (offsetX * 3);
                indexC |= (0x7 & (bitField[EMPTY][x] >> (bottomY - 1))) << (offsetX * 3);
            }
        } else {
            for (int offsetX = 0; offsetX < 4; ++offsetX) {
                int x = offsetX + leftX - 1;
                indexA |= (0x7 & (bitField[a][x] >> (bottomY - 1))) << ((3 - offsetX) * 3);
                indexB |= (0x7 & (bitField[b][x] >> (bottomY - 1))) << ((3 - offsetX) * 3);
                indexC |= (0x7 & (bitField[EMPTY][x] >> (bottomY - 1))) << ((3 - offsetX) * 3);
            }
        }

        return indexA | (indexB << 12) | indexC | (indexC << 12);
    }

    // xxx  3  7 11   11  7  3
    // xxx  2  6 10   10  6  2
    // xxx  1  5  9    9  5  1
    // xxx  0  4  8    8  4  0
    int indexFor43(int leftX, int bottomY, PuyoColor a, PuyoColor b) {
        DCHECK(1 <= leftX && leftX <= 4);
        DCHECK(a < b);
        DCHECK(a != EMPTY);
        DCHECK(b != EMPTY);

        int indexA = 0, indexB = 0, indexC = 0;
        if (leftX <= 2) {
            for (int offsetX = 0; offsetX < 3; ++offsetX) {
                int x = offsetX + leftX - 1;
                indexA |= (0xF & (bitField[a][x] >> (bottomY - 1))) << (offsetX * 4);
                indexB |= (0xF & (bitField[b][x] >> (bottomY - 1))) << (offsetX * 4);
                indexC |= (0xF & (bitField[EMPTY][x] >> (bottomY - 1))) << (offsetX * 4);
            }
        } else {
            for (int offsetX = 0; offsetX < 3; ++offsetX) {
                int x = offsetX + leftX - 1;
                indexA |= (0xF & (bitField[a][x] >> (bottomY - 1))) << ((2 - offsetX) * 4);
                indexB |= (0xF & (bitField[b][x] >> (bottomY - 1))) << ((2 - offsetX) * 4);
                indexC |= (0xF & (bitField[EMPTY][x] >> (bottomY - 1))) << ((2 - offsetX) * 4);
            }
        }

        return indexA | (indexB << 12) | indexC | (indexC << 12);
    }
private:
    int bitField[8][6];
};

#endif
