/*
 puyo.hpp
 yuricat(Katsuki Ohto)
 */

#ifndef PUYOAI_YURICAT_PUYO_HPP_
#define PUYOAI_YURICAT_PUYO_HPP_

#include <cassert>
#include <cmath>
#include <random>
#include <valarray>
#include <utility>
#include <tuple>
#include <climits>
#include <map>
#include <unordered_set>
#include <algorithm>
#include <thread>
#include <sys/time.h>
#include <atomic>
#include <mutex>

#define MINIMUM

#ifndef MINIMUM
#define COUT cout
#define CERR cerr
#else
#define CERR 0 && cerr
#define COUT 0 && cout
#endif

// 条件x、命令等y
#ifdef NDEBUG
#define ASSERT(X, Y)
#else
#define ASSERT(X, Y)  if(!(X)){ Y; assert(X); assert(0); }
#endif // NDEBUG

// 浮動小数点がまともな値を取っているかどうかのアサーション
#define FASSERT(f, o) ASSERT(!std::isinf(f) && !std::isnan(f), cerr << (f) << endl;{o});

// 浮動小数点が「ほぼ」同じ値を取っているかのチェック && FASSERT
#define FEQUALS(f0, f1, o) { FASSERT(f0, o); FASSERT(f1, o); if(fabs(f0 - f1) > 0.00001){ cerr << (f0) << " <-> " << (f1) << endl; {o} } );

/*
// クロック計測関数(64ビットで値を返す)
static uint64_t cputime(){
    unsigned int ax, dx;
    asm volatile("rdtsc\nmovl %%eax,%0\nmovl %%edx,%1":"=g"(ax),"=g"(dx): :"eax","edx");
    return ((uint64_t)(dx) << 32) + (uint64_t)(ax);
}

// タイムスタンプ読み込みにかかる時間
// 環境依存しそうなので0にしておく
constexpr uint64_t CLOCK_LATENCY = 0ULL;

// デバッグ用

uint64_t CLOCK_START;

static void tick()noexcept{
    CLOCK_START = cputime();
}

static void tock(){
    uint64_t CLOCK_END = cputime();
    printf("clock=%lld\n", CLOCK_END - CLOCK_START - CLOCK_LATENCY);
}*/

class ClockMS{
    // millisec単位
private:
    timeval t_start;
public:
    void start()noexcept{ gettimeofday(&t_start, NULL); }
    long stop()const noexcept{
        timeval t_end;
        gettimeofday(&t_end, NULL);
        long t = (t_end.tv_sec - t_start.tv_sec) * 1000 + (t_end.tv_usec - t_start.tv_usec) / 1000;
        return t;
    }
    long restart()noexcept{ // 結果を返し、0から再スタート
        timeval t_end;
        gettimeofday(&t_end, NULL);
        long t = (t_end.tv_sec - t_start.tv_sec) * 1000 + (t_end.tv_usec - t_start.tv_usec) / 1000;
        t_start = t_end;
        return t;
    }
    static long long now()noexcept{
        timeval t_now;
        gettimeofday(&t_now, NULL);
        long long t = t_now.tv_sec * 1000 + t_now.tv_usec / 1000;
        return t;
    }
    ClockMS(){}
    ClockMS(int m){ UNUSED_VARIABLE(m); start(); }
};

/*
class XorShift64{
private:
    uint64 x, y, z, t;
public:
    
    uint64 rand()noexcept{
        uint64 tmp = x ^ (x << 11);
        x = y;
        y = z;
        z = t;
        t = (t ^ (t >> 19)) ^ (tmp ^ (tmp >> 8));
        return t;
    }
    
    double drand()noexcept{
        uint64 tmp = x ^ (x << 11);
        x = y;
        y = z;
        z = t;
        t = (t ^ (t >> 19)) ^ (tmp ^ (tmp >> 8));
        return t / static_cast<double>(0xFFFFFFFFFFFFFFFFULL);
    }
    void srand(const uint64 s)noexcept{
        if (!s){ // seedが0だとまずい
            x = 0x0123456789ABCDEFULL;
        }
        else{
            x = (s << 32) ^ s;
        }
        y = (x << 8) | ((x & 0xff00000000000000ULL) >> 56);
        z = (y << 8) | ((y & 0xff00000000000000ULL) >> 56);
        t = (z << 8) | ((z & 0xff00000000000000ULL) >> 56);
    }
    constexpr static uint64 min()noexcept{ return 0ULL; }
    constexpr static uint64 max()noexcept{ return 0xFFFFFFFFFFFFFFFFULL; }
    
    constexpr XorShift64()noexcept
    :x(), y(), z(), t(){}
    
    XorShift64(const uint64 s)
    : x(), y(), z(), t(){
        srand(s);
    }
};
*/

using namespace std;

namespace Yuricat{
    
    // 評価点
    constexpr int SCORE_MATE     = 30000;
    constexpr int SCORE_INFINITE = 32783;
    
    // 端数おじゃまぷよの落下パターン
    constexpr char kLinePattern2[15] = {
        0b00000110, 0b00001010, 0b00001100, 0b00010010,
        0b00010100, 0b00011000, 0b00100010, 0b00100100,
        0b00101000, 0b00110000, 0b01000010, 0b01000100,
        0b01001000, 0b01010000, 0b01100000,
    };
    
    constexpr char kLinePattern3[20] = {
        0b00001110, 0b00010110, 0b00011010, 0b00011100,
        0b00100110, 0b00101010, 0b00101100, 0b00110010,
        0b00110100, 0b00111000, 0b01000110, 0b01001010,
        0b01001100, 0b01010010, 0b01010100, 0b01011000,
        0b01100010, 0b01100100, 0b01101000, 0b01110000,
    };
    
    constexpr char kLinePatternFull = 0b01111110;
    
    
    constexpr int kMateScoreDist = 5000; // この得点差ついたらおじゃまぷよで負け
    
    // 連鎖数からの得点の概算
    constexpr int kMinChainScore[20] = {
        0, 40, 360, 1000,
        2280, 4840, 8680, 13800,
        20200, 27880, 36840, 47080,
        58600, 71400, 85480, 100480,
        117480, 135400, 154600, 175080,
    };
    
    int kMaxChainScore[20];
    
    constexpr int kRequiredChain[20] = { // 相手の連鎖数に対して必要な連鎖数
        0, 0, 0, 0,
        0, 0, 5, 6,
        8, 9, 10, 11,
        12, 13, 14, 15,
        16, 17, 18, 19,
    };
    constexpr int kMateChain[20] = { // 相手の連鎖数に対してこの連鎖ができたら勝ち(時間も考慮の必要あり)
        12, 12, 12, 12,
        11, 11, 10, 10,
        10, 10, 10, 11,
        13, 14, 15, 16,
        17, 18, 19, 20,
    };
    
    int toDecisionIndex(const Decision& decision){
        return decision.x * 6 + decision.r;
    }
    const Decision DECISION_SUICIDE = Decision(3, 0);
    
    int decideOjamaComumn(int n, int r)noexcept{
        // 端数のおじゃまぷよの落ちる位置を決定する
        switch(n){
            case 0: assert(0); return 0; break;
            case 1: return 1 << (rand() % 6); break;
            case 2: return kLinePattern2[r % 15]; break;
            case 3: return kLinePattern3[r % 20]; break;
            case 4: return kLinePatternFull ^ kLinePattern2[r % 15]; break;
            case 5: return kLinePatternFull ^ (1 << (r % 6)); break;
            default: assert(0); return kLinePatternFull; break;
        }
        assert(0);
        return 0;
    }
    
    int fallFractionOjama(CoreField *const pfield, int ojamas){
        // 端数のおじゃまぷよを置く
        // フレーム数をちゃんと返す
        
        int dropHeight = 0;
        int cnt = 0;
        for(int x = 1; x <= FieldConstant::WIDTH; ++x){
            if(ojamas & (1 << x)){
                dropHeight = max(dropHeight, 12 - pfield->height(x));
                pfield->dropPuyoOnWithMaxHeight(x, PuyoColor::OJAMA, 13);
                ++cnt;
            }
        }
        return FRAMES_TO_DROP[dropHeight] + framesGroundingOjama(cnt);
    }
    
    // 得点から勝率に変換
    double scoreToWP(int score){
        return 1 / (1 + exp(-(score - 60000) / 3000));
    }
    double scoresToWP(int score0, int score1){
        return 1 / (1 + exp(-(score0 - score1) / 3000));
    }
    
    // 勝ち負け判定のための関数
    int framePuyoNumToMaxScore(){
        return 0;
    }
    
    template<class field_t>
    int calcActionFrame(){
        // 行動開始フレーム数を計算する
        return 0;
    }
    
    constexpr bool examChain(int c)noexcept{
        return (0 <= c && c < 20);
    }
    
    int init(){
        return 0;
    }
}

#endif // PUYOAI_YURICAT_PUYO_HPP_
