#ifndef SOLVER_PUYOP_H_
#define SOLVER_PUYOP_H_

#include <string>
#include <vector>

class Decision;
class KumipuyoSeq;

// makePuyopURL makes a URL to show KumipuyoSeq and decisions in
// http://www.puyop.com/
std::string makePuyopURL(const KumipuyoSeq&, const std::vector<Decision>&);

#endif
