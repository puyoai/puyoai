#ifndef SOLVER_PUYOP_H_
#define SOLVER_PUYOP_H_

#include <string>
#include <vector>

class CoreField;
class Decision;
class KumipuyoSeq;

// makePuyopURL makes a URL in http://www.puyop.com/ to simulate the control.

std::string makePuyopURL(const CoreField&, const KumipuyoSeq&, const std::vector<Decision>&);
// Assuming empty field
std::string makePuyopURL(const KumipuyoSeq&, const std::vector<Decision>&);

#endif
