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
	// 大局的な主要情報
	mutable int  myPuyo;
	mutable int  myColorPuyo;
	mutable int  opColorPuyo;
	mutable bool hasAdvantage;
	mutable bool myZen;
	mutable int  myOjama;
	mutable bool opGoing;
	mutable int  opFinFrame;
	mutable int  baseFrameId;
	mutable PlayerState baseEnemy;

	// 探索情報
	mutable int bestThinkScore;
	mutable int bestThinkPos;

	mutable int firstPos;
	mutable int assassinScore[22];
	mutable int mainScore[22];

	Decision DECISIONS[22] = {
		Decision(2, 3), Decision(1, 1), Decision(5, 1), Decision(6, 3), Decision(4, 1), Decision(5, 3),
		Decision(3, 3), Decision(2, 1), Decision(3, 1), Decision(4, 3),
		Decision(1, 0), Decision(1, 2), Decision(2, 0), Decision(2, 2), Decision(6, 0), Decision(6, 2),
		Decision(5, 0), Decision(5, 2), Decision(4, 0), Decision(4, 2), Decision(3, 0), Decision(3, 2),
	};

	Kumipuyo ALL_KUMIPUYO_KINDS[10] = {
		Kumipuyo(PuyoColor::RED,    PuyoColor::RED),
		Kumipuyo(PuyoColor::BLUE,   PuyoColor::BLUE),
		Kumipuyo(PuyoColor::YELLOW, PuyoColor::YELLOW),
		Kumipuyo(PuyoColor::GREEN,  PuyoColor::GREEN),
		Kumipuyo(PuyoColor::RED,    PuyoColor::BLUE),
		Kumipuyo(PuyoColor::RED,    PuyoColor::YELLOW),
		Kumipuyo(PuyoColor::RED,    PuyoColor::GREEN),
		Kumipuyo(PuyoColor::BLUE,   PuyoColor::YELLOW),
		Kumipuyo(PuyoColor::BLUE,   PuyoColor::GREEN),
		Kumipuyo(PuyoColor::YELLOW, PuyoColor::GREEN),
	};

	int BIG_MINUS = -100000000;

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

		// 大局的な主要情報
		myPuyo       = f.countPuyos();
		myColorPuyo  = f.countColorPuyos();
		opColorPuyo  = enemy.field.countColorPuyos();
		hasAdvantage = ((myColorPuyo >= 20) && (myColorPuyo >= opColorPuyo * 1.5));
		myZen        = me.hasZenkeshi;
		myOjama      = me.totalOjama(enemy);
		opGoing      = enemy.isRensaOngoing();
		opFinFrame   = max(enemy.rensaFinishingFrameId() - frameId, 0);
		baseFrameId  = frameId;
		baseEnemy    = enemy;

		// 探索情報
		bestThinkScore = BIG_MINUS;
		bestThinkPos = 0;

		Decision decision;
		if(myPuyo == 0){
			// 初手、全消し直後の思考
			decision = dropFirst(seq);
		} else if(myPuyo < 8){
			// 序盤の思考
			decision = dropEarly(f, seq);
		} else {
			// 通常の思考
			decision = dropMain(f, seq);
		}

		return DropDecision(decision);
	}

	// 初手、全消し直後の思考
	Decision dropFirst(const KumipuyoSeq& seq) const
	{
		// 最初の3手のみ考慮する
		Kumipuyo now   = seq.get(0);
		Kumipuyo next1 = seq.get(1);
		Kumipuyo next2 = seq.get(2);

		if(now.axis == now.child){
			// 初手が1色
			if(next1.axis == next1.child){
				// 2手目が1色
				if(now.axis == next1.axis){
					// 2手全消し
					return Decision(3, 0);
				} else if((next1.axis == next2.axis) && (next2.axis == next2.child)){
					// 2手目、3手目が1色
					return Decision(1, 0);
				}
			}
			return Decision(2, 3);
		}

		if(next1.axis == next1.child){
			// 初手が2色で2手目が1色
			if(now.axis == next1.axis){
				// 2手目が1手目の axis と同じ色
				return Decision(1, 1);
			} else if(now.child == next1.axis){
				// 2手目が1手目の child と同じ色
				return Decision(2, 3);
			} else if(next2.axis == next2.child){
				// 3手目が1色
				if(next1.axis == next2.axis){
					// 2手目、3手目が1色
					return Decision(2, 0);
				} else if(now.axis == next2.axis){
					// 3手目が1手目の axis と同じ色
					return Decision(1, 1);
				} else if(now.child == next2.axis){
					// 3手目が1手目の child と同じ色
					return Decision(2, 3);
				}
			}
			// 上記のいずれにも該当しない場合は後続の処理へ
		}

		// 2手目、3手目で多く出てくる色を左にして横置き
		int axisCount =
			((now.axis == next1.axis)  ? 1 : 0) +
			((now.axis == next1.child) ? 1 : 0) +
			((now.axis == next2.axis)  ? 1 : 0) +
			((now.axis == next2.child) ? 1 : 0);

		int childCount =
			((now.child == next1.axis)  ? 1 : 0) +
			((now.child == next1.child) ? 1 : 0) +
			((now.child == next2.axis)  ? 1 : 0) +
			((now.child == next2.child) ? 1 : 0);

		if(axisCount > childCount){
			return Decision(1, 1);
		}
		return Decision(2, 3);
	}

	// 序盤の思考
	Decision dropEarly(const CoreField& f, const KumipuyoSeq& seq) const
	{
		dropEarlyRecursive(f, seq, 0, 0);

		return DECISIONS[bestThinkPos];
	}

	int dropEarlyRecursive(const CoreField& f, const KumipuyoSeq& seq, int depth, int scoreSum) const
	{
		int bestScore = BIG_MINUS;

		Kumipuyo puyo = seq.get(depth);
		for(int i = 0; i < 22; i++){
			if((puyo.axis == puyo.child) && (i % 2 == 1)){
				continue;
			}
			CoreField field = f;
			const Decision & decision = DECISIONS[i];
			if(!PuyoController::isReachable(field, decision)){
				continue;
			}
			if(field.isChigiriDecision(decision)){
				// ちぎる手は採用しない
				continue;
			}

			field.dropKumipuyo(decision, puyo);
			const RensaResult & result = field.simulate();
			if(!field.isEmpty(3, 12)){
				continue;
			}

			int evalScore = scoreSum + dropEarlyEval(field, result, depth);
			if((result.chains == 0) && (depth < 2)){
				// 評価値の半分を引き継いで再帰処理
				evalScore += dropEarlyRecursive(field, seq, depth + 1, evalScore * 0.5);
			}

			if(evalScore > bestScore){
				bestScore = evalScore;
			}
			if((depth == 0) && (evalScore > bestThinkScore)){
				bestThinkScore = evalScore;
				bestThinkPos = i;
			}
		}

		return bestScore;
	}

	int dropEarlyEval(const CoreField& field, const RensaResult& result, int depth) const
	{
		int score  = result.score;
		int chains = result.chains;

		if( myZen && (chains > 0) &&
		    ((myOjama == 0) || ((0 < opFinFrame) && (opFinFrame < 120))) ){
			// 全消しは早めに精算する（なるべく少ない連鎖で消す）
			score += 10000 * (100 - depth * 10) * (10 - chains);
		}

		if(field.isZenkeshi()){
			score += 2100;
		}

		// 同色の連結を重視する
		int count2, count3;
		field.countConnection(&count2, &count3);
		score += count2 * 100;
		score += count3 * 220;

		return score;
	}

	// 通常の思考
	Decision dropMain(const CoreField& f, const KumipuyoSeq& seq) const
	{
		for(int i = 0; i < 22; i++){
			assassinScore[i] = 0;
			mainScore[i] = 0;
		}

		dropMainRecursive(f, seq, 0, 0, seq.get(0));

		return DECISIONS[bestThinkPos];
	}

	int dropMainRecursive(const CoreField& f, const KumipuyoSeq& seq, int depth, int chigiriFrames,
		const Kumipuyo puyo) const
	{
		int bestScore = BIG_MINUS;

		for(int i = 0; i < 22; i++){
			if((puyo.axis == puyo.child) && (i % 2 == 1)){
				continue;
			}
			CoreField field = f;
			const Decision & decision = DECISIONS[i];
			if(!PuyoController::isReachable(field, decision)){
				continue;
			}

			int chiFr = chigiriFrames + getChigiriFrames(field, decision);

			field.dropKumipuyo(decision, puyo);
			const RensaResult & result = field.simulate();
			if(!field.isEmpty(3, 12)){
				continue;
			}

			int score  = result.score;
			int chains = result.chains;

			if(depth == 0){
				firstPos = i;
			}

			int evalScore = BIG_MINUS;
			if(chains > 0){
				// 消す手の評価
				evalScore = dropMainEvalErase(field, result, depth, chiFr);
				if((1 <= depth) && (depth <= 2)){
					// ネクストに見えている手で消す
					if(!opGoing && !hasAdvantage && (chains == 1) && (score >= 400)){
						assassinScore[firstPos] += score * 3 * (3 - depth);
					} else if(!opGoing && !hasAdvantage && (chains == 2) && (score >= 1210)){
						assassinScore[firstPos] += score * (3 - depth);
					}
					if(evalScore > mainScore[firstPos]){
						mainScore[firstPos] = evalScore;
					}
				} else if(depth >= 3){
					// ネクストに見えていない手で消せるかもしれない
					if(myPuyo > 40){
						// フィールドが狭くなってきたら、見えない手への期待度を下げる
						evalScore *= max((200 - myPuyo * 3), 0) * 0.01;
					}
					if(!opGoing && !hasAdvantage && (chains == 1) && (score >= 700)){
						assassinScore[firstPos] += score;
					}
					if(evalScore > mainScore[firstPos]){
						mainScore[firstPos] = evalScore;
					}
				}
			} else if(depth >= 3){
				// ネクストに見えていない手で消さない手は評価の対象外
				evalScore = BIG_MINUS;
			} else {
				int fieldScore = (depth == 0) ? dropMainEvalField(field, chiFr) : 0;
				if(depth < 2){
					evalScore = dropMainRecursive(field, seq, depth + 1, chiFr, seq.get(depth + 1));
				} else {
					for(const Kumipuyo & nextPuyo : ALL_KUMIPUYO_KINDS){
						evalScore = dropMainRecursive(field, seq, depth + 1, chiFr, nextPuyo);
						if(evalScore > bestScore){
							bestScore = evalScore;
						}
					}
				}
				if(depth == 0){
					// 今の手のポテンシャル（形と次の手以降の評価値）を評価
					evalScore = fieldScore + mainScore[firstPos] + assassinScore[firstPos];
				}
			}

			if(evalScore > bestScore){
				bestScore = evalScore;
			}
			if((depth == 0) && (evalScore > bestThinkScore)){
				bestThinkScore = evalScore;
				bestThinkPos = i;
			}
		}

		return bestScore;
	}

	int dropMainEvalErase(const CoreField& field, const RensaResult& result, int depth, int chigiriFrames) const
	{
		int score  = result.score;
		int chains = result.chains;
		int frames = result.frames;

		if(field.isZenkeshi()){
			score += 2100;
		}

		if(myZen && ((myOjama == 0) || ((0 < opFinFrame) && (opFinFrame < 120)))){
			// 全消しは早めに精算する（なるべく少ない連鎖で消す）
			score += 10000 * (100 - depth * 10) * (10 - chains);
		} else if(opGoing && (myOjama >= 6) && (depth <= 2)){
			// 相手の連鎖が終わるまでに発火する
			int ojamaLimit = (opFinFrame / NUM_FRAMES_OF_ONE_HAND) + 1;
			if((depth < ojamaLimit) && (ojamaLimit <= 3)){
				score *= 256.0 / pow(4, ojamaLimit - depth);
			}
		} else if((depth == 0) && (chains <= 2)){
			// 今の手で消す大きい1～2連鎖
			int puyoCount = field.countPuyos();
			int erasePuyo = myPuyo + (depth + 1) * 2 - puyoCount;
			int aveScore = (erasePuyo > 0) ? (score / erasePuyo) : 0;
			if(aveScore >= 40){
				// 連鎖終了までに相手が出せる最高得点
				const GazeResult& gazeResult = gazer_.gazeResult();
				int gazeScore = gazeResult.estimateMaxScore(baseFrameId + frames, baseEnemy);
				if(score >= gazeScore){
					if(chains == 1){
						score *= (aveScore - 30) * 300;
					} else if((chains == 2) && (aveScore >= 90)){
						score *= (aveScore - 50) * 200;
					}
				}
			}
		}

		if(chigiriFrames > 0){
			score -= chigiriFrames * 5;
		}

		return score;
	}

	int dropMainEvalField(const CoreField& field, int chigiriFrames) const
	{
		int score = 0;

		int h1 = field.height(1);
		int h2 = field.height(2);
		int h3 = field.height(3);
		int h4 = field.height(4);
		int h5 = field.height(5);
		int h6 = field.height(6);
		int compHeight[3][3] = {
			{h2, h1, 1000},
			{h5, h6, 1000},
			{h4, h5, 3000},
		};
		for(int i = 0; i < 3; i++){
			int height1     = compHeight[i][0];
			int height2     = compHeight[i][1];
			int penaltyRate = compHeight[i][2];
			int penaltyScore = (height1 - height2 + 1) * penaltyRate;
			if((height1 >= 12) && (height1 > height2)){
				// フィールドに使えないスペースができている（壁越えは動作が不安定なので考慮しない）
				score -= penaltyScore;
			}
			if((myPuyo < 48) && (height1 == 11) && (height1 >= height2)){
				// おじゃま1個でフィールドに使えないスペースができる
				score -= penaltyScore / 2;
			}
		}
		if(h3 >= 11){
			score -= 10000;
		}

		if(chigiriFrames > 0){
			score -= chigiriFrames * 5;
		}

		return score;
	}

	// ちぎりによってロスするフレーム数を取得
	int getChigiriFrames(const CoreField& field, const Decision& decision) const
	{
		int x1 = decision.axisX();
		int x2 = decision.childX();
		if(x1 == x2){
			return 0;
		}

		int height1 = field.height(x1);
		int height2 = field.height(x2);
		if(height1 == height2){
			return 0;
		}

		int diffHeight = (height1 > height2) ? (height1 - height2) : (height2 - height1);

		return FRAMES_TO_DROP[diffHeight] + FRAMES_GROUNDING;
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
