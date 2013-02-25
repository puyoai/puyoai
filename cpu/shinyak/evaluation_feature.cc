#include "evaluation_feature.h"

#include <glog/logging.h>
#include <sstream>
#include "evaluation_params.h"

using namespace std;

string EvaluationFeature::toString() const
{
    ostringstream ss;
    ss << get(MAX_CHAINS) << " ";
    ss << (1.0 / get(MAX_RENSA_NECESSARY_PUYOS)) << " ";
    // TODO(mayah): We have to implement this.

    return ss.str();
}
