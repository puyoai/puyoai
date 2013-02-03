#include "field_deprecated.h"

#include <vector>

using namespace std;

void FieldDeprecated::GetPossibleFields(
    const FieldDeprecated& field, char c1, char c2,
    vector<pair<Decision, FieldDeprecated> >* ret) {
  c1 -= '0';
  c2 -= '0';
  FieldDeprecated f(field);

  int heights[Field::MAP_WIDTH];
  for (int x = 1; x <= Field::WIDTH; x++) {
    heights[x] = 100;
    for (int y = 1; y <= Field::HEIGHT + 2; y++) {
      if (f.Get(x, y) == EMPTY) {
        heights[x] = y;
        break;
      }
    }
  }

  for (int x = 1; x < 6; x++) {
    f.Set(x, heights[x], c1);
    f.Set(x + 1, heights[x + 1], c2);
    ret->push_back(make_pair(Decision(x, 1), f));
    f.Set(x, heights[x], EMPTY);
    f.Set(x + 1, heights[x + 1], EMPTY);
  }

  for (int x = 1; x <= 6; x++) {
    f.Set(x, heights[x], c1);
    f.Set(x, heights[x] + 1, c2);
    ret->push_back(make_pair(Decision(x, 0), f));
    f.Set(x, heights[x], EMPTY);
    f.Set(x, heights[x] + 1, EMPTY);
  }

  if (c1 != c2) {
    for (int x = 1; x < 6; x++) {
      f.Set(x, heights[x], c2);
      f.Set(x + 1, heights[x + 1], c1);
      ret->push_back(make_pair(Decision(x + 1, 3), f));
      f.Set(x, heights[x], EMPTY);
      f.Set(x + 1, heights[x + 1], EMPTY);
    }
    for (int x = 1; x <= 6; x++) {
      f.Set(x, heights[x], c2);
      f.Set(x, heights[x] + 1, c1);
      ret->push_back(make_pair(Decision(x, 2), f));
      f.Set(x, heights[x], EMPTY);
      f.Set(x, heights[x] + 1, EMPTY);
    }
  }
}
