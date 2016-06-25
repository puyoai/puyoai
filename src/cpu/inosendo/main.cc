#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cmath>

#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/puyo_controller.h"
#include "core/frame_request.h"

#include "gazer.h"

using namespace std;

class InosendoAI : public AI {
public:
	InosendoAI(int argc, char* argv[]) : AI(argc, argv, "inosendo") {}
	~InosendoAI() override {}

	void onGameWillBegin(const FrameRequest& frameRequest) override
	{
		gazer_.initialize(frameRequest.frameId);
	}

	DropDecision think(int frameId, const CoreField& f, const KumipuyoSeq& seq,
		const PlayerState& me, const PlayerState& enemy, bool fast) const override
	{
		UNUSED_VARIABLE(fast);

		int ojamaLimit = 0;
		if(enemy.isRensaOngoing() && (me.totalOjama(enemy) >= 6)){
			ojamaLimit = (max(enemy.rensaFinishingFrameId() - frameId, 0) / FPS) + 1;
		}

		const GazeResult& gazeResult = gazer_.gazeResult();
		int gazeScore1 = gazeResult.estimateMaxScore(frameId + 60, enemy);
		int gazeScore2 = gazeResult.estimateMaxScore(frameId + 120, enemy);

		int depth = 0;
		Kumipuyo puyo = seq.get(depth);
		int bestScore = -100000000;
		int bestPos = 0;
		Decision decision = dropRecursive(depth, 0, 0, 0, bestScore, bestPos, puyo, f, seq, ojamaLimit, gazeScore1, gazeScore2);

		return DropDecision(decision);
	}

	Decision dropRecursive(int depth, int deciPos, int scoreSum, int chigiriNum,
		int& bestScore, int& bestPos, const Kumipuyo& puyo, const CoreField& f, const KumipuyoSeq& seq,
		const int ojamaLimit, const int gazeScore1, const int gazeScore2) const
	{
		static const Decision DECISIONS[22] = {
			Decision(3, 0), Decision(3, 1), Decision(3, 3), Decision(4, 0), Decision(2, 0),
			Decision(4, 1), Decision(2, 3), Decision(5, 0), Decision(1, 0), Decision(5, 1), Decision(6, 0),
			Decision(3, 2), Decision(4, 3), Decision(2, 1), Decision(4, 2), Decision(2, 2),
			Decision(5, 3), Decision(1, 1), Decision(5, 2), Decision(1, 2), Decision(6, 3), Decision(6, 2),
		};

		static const Kumipuyo ALL_KUMIPUYO_KINDS[10] = {
			Kumipuyo(PuyoColor::RED, PuyoColor::RED),
			Kumipuyo(PuyoColor::RED, PuyoColor::BLUE),
			Kumipuyo(PuyoColor::RED, PuyoColor::YELLOW),
			Kumipuyo(PuyoColor::RED, PuyoColor::GREEN),
			Kumipuyo(PuyoColor::BLUE, PuyoColor::BLUE),
			Kumipuyo(PuyoColor::BLUE, PuyoColor::YELLOW),
			Kumipuyo(PuyoColor::BLUE, PuyoColor::GREEN),
			Kumipuyo(PuyoColor::YELLOW, PuyoColor::YELLOW),
			Kumipuyo(PuyoColor::YELLOW, PuyoColor::GREEN),
			Kumipuyo(PuyoColor::GREEN, PuyoColor::GREEN),
		};

		int posNum = (puyo.axis == puyo.child) ? 11 : 22;
		for(int i = 0; i < posNum; i++){
			CoreField field = f;
			auto & decision = DECISIONS[i];
			if(!PuyoController::isReachable(field, decision)){
				continue;
			}

			bool isChigiri = field.isChigiriDecision(decision);
			chigiriNum += isChigiri ? 1 : 0;

			field.dropKumipuyo(decision, puyo);
			const auto & result = field.simulate();
			if(!isFieldAvailable(field)){
				continue;
			}

			int decisionPos = (depth == 0) ? i : deciPos;

			int score = scoreSum + result.score;
			int chains = result.chains;
			if(chains == 0){
				int nextDepth = depth + 1;
				if((nextDepth <= seq.size()) && (nextDepth <= 2)){
					if((nextDepth < seq.size()) && (nextDepth < 2)){
						Kumipuyo nextPuyo = seq.get(nextDepth);
						dropRecursive(nextDepth, decisionPos, score, chigiriNum, bestScore, bestPos, nextPuyo, field, seq, ojamaLimit, gazeScore1, gazeScore2);
					} else {
						for(const auto& nextPuyo : ALL_KUMIPUYO_KINDS){
							dropRecursive(nextDepth, decisionPos, score, chigiriNum, bestScore, bestPos, nextPuyo, field, seq, ojamaLimit, gazeScore1, gazeScore2);
						}
					}
					continue;
				}
			}

			int evalScore = getEvalScore(depth, score, result.chains, chigiriNum, field, ojamaLimit, gazeScore1, gazeScore2);
			if(evalScore > bestScore){
				bestScore = evalScore;
				bestPos = decisionPos;
			}
		}

		return DECISIONS[bestPos];
	}

	bool isFieldAvailable(const CoreField& field) const
	{
		if(!field.isEmpty(3, 12)){
			return false;
		} else if((field.height(2) >= 12) && (field.height(1) < 12)){
			return false;
		} else if((field.height(4) >= 12) && (field.height(5) < 12)){
			return false;
		} else if((field.height(5) >= 12) && (field.height(6) < 12)){
			return false;
		}
		return true;
	}

	int getEvalScore(int depth, int score, int chains, int chigiriNum, const CoreField& field, const int ojamaLimit, const int gazeScore1, const int gazeScore2) const
	{
		int evalScore = score;

		if(ojamaLimit == 0){
			if((chains == 1) && (evalScore >= 600) && (evalScore >= gazeScore1)){
				evalScore *= 100;
			} else if((chains == 2) && (evalScore >= 1210) && (evalScore >= gazeScore2)){
				evalScore *= 10;
			}
			if(field.isZenkeshi()){
				evalScore += 2800;
			}
			if((depth == 0) && (chains <= 2) && (evalScore / chains > 1400)){
				evalScore *= 10;
			}
			int count2, count3;
			field.countConnection(&count2, &count3);
			evalScore += count2 * 10;
			evalScore += count3 * 30;
		} else if((ojamaLimit < 4) && (chains > 0) && (depth < ojamaLimit)){
			evalScore *= 1000 / pow(2, max(ojamaLimit - depth, 0));
		}

		evalScore -= chigiriNum * 50;

		static const int VALLEY_DEPTH[15] = {0, 0, -20, -50, -100, -200, -400, -600, -800, -1000, -1200, -1400, -1600, -1800, -2000};
		static const int RIDGE_HEIGHT[15] = {0, 0, -20, -50, -100, -200, -400, -600, -800, -1000, -1200, -1400, -1600, -1800, -2000};
		for(int x = 1; x <= 6; x++){
			evalScore += VALLEY_DEPTH[field.valleyDepth(x)];
			evalScore += RIDGE_HEIGHT[field.ridgeHeight(x)];
		}

		return evalScore;
	}

private:
	Gazer gazer_;
};

int main(int argc, char* argv[])
{
	google::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
	google::InstallFailureSignalHandler();
#endif

	InosendoAI(argc, argv).runLoop();
	return 0;
}
