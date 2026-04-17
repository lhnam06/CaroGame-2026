/*
 * Bot - Alpha-beta search + tactical pattern evaluator (no OOP)
 */

#include "Defs/Defs.h"
#include "Bot/Bot.h"
#include <algorithm>
#include <cstdlib>
#include <vector>

struct CandidateMove {
    int r;
    int c;
    int score;
};

static const int kDirsR[4] = { 0, 1, 1, 1 };
static const int kDirsC[4] = { 1, 0, 1, -1 };
static const int kWinScore = 100000000;
static const int kInfScore = 1000000000;

static bool InBounds(int r, int c) {
    return (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE);
}

static bool BoardEmpty(void) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c != 0) return false;
        }
    }
    return true;
}

static bool BoardFull(void) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c == 0) return false;
        }
    }
    return true;
}

static bool HasNeighbor(int r, int c, int radius) {
    for (int dr = -radius; dr <= radius; dr++) {
        for (int dc = -radius; dc <= radius; dc++) {
            if (dr == 0 && dc == 0) continue;
            int rr = r + dr;
            int cc = c + dc;
            if (!InBounds(rr, cc)) continue;
            if (_A[rr][cc].c != 0) return true;
        }
    }
    return false;
}

static void AnalyzeLine(int r, int c, int dr, int dc, int piece, int& count, int& openEnds) {
    count = 1;
    openEnds = 0;

    int rr = r + dr;
    int cc = c + dc;
    while (InBounds(rr, cc) && _A[rr][cc].c == piece) {
        count++;
        rr += dr;
        cc += dc;
    }
    if (InBounds(rr, cc) && _A[rr][cc].c == 0) openEnds++;

    rr = r - dr;
    cc = c - dc;
    while (InBounds(rr, cc) && _A[rr][cc].c == piece) {
        count++;
        rr -= dr;
        cc -= dc;
    }
    if (InBounds(rr, cc) && _A[rr][cc].c == 0) openEnds++;
}

static int PatternScore(int count, int openEnds) {
    if (count >= 5) return 12000000;
    if (count == 4 && openEnds == 2) return 1500000;
    if (count == 4 && openEnds == 1) return 280000;
    if (count == 3 && openEnds == 2) return 50000;
    if (count == 3 && openEnds == 1) return 5000;
    if (count == 2 && openEnds == 2) return 1200;
    if (count == 2 && openEnds == 1) return 120;
    if (count == 1 && openEnds == 2) return 20;
    return 0;
}

static bool HasFiveAt(int r, int c, int piece) {
    for (int d = 0; d < 4; d++) {
        int cnt = 1;
        int rr = r + kDirsR[d];
        int cc = c + kDirsC[d];
        while (InBounds(rr, cc) && _A[rr][cc].c == piece) {
            cnt++;
            rr += kDirsR[d];
            cc += kDirsC[d];
        }
        rr = r - kDirsR[d];
        cc = c - kDirsC[d];
        while (InBounds(rr, cc) && _A[rr][cc].c == piece) {
            cnt++;
            rr -= kDirsR[d];
            cc -= kDirsC[d];
        }
        if (cnt >= 5) return true;
    }
    return false;
}

static bool HasAnyFive(int piece) {
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (_A[r][c].c != piece) continue;
            if (HasFiveAt(r, c, piece)) return true;
        }
    }
    return false;
}

static bool WouldHaveFive(int r, int c, int piece) {
    if (!InBounds(r, c) || _A[r][c].c != 0) return false;
    _A[r][c].c = piece;
    bool ok = HasFiveAt(r, c, piece);
    _A[r][c].c = 0;
    return ok;
}

static int EvaluatePoint(int r, int c, int piece) {
    if (_A[r][c].c != 0) return -kInfScore;

    _A[r][c].c = piece;
    int score = 0;
    for (int d = 0; d < 4; d++) {
        int count = 0;
        int openEnds = 0;
        AnalyzeLine(r, c, kDirsR[d], kDirsC[d], piece, count, openEnds);
        score += PatternScore(count, openEnds);
    }
    _A[r][c].c = 0;

    int center = BOARD_SIZE / 2;
    int dist = abs(r - center) + abs(c - center);
    score -= dist * 3;

    return score;
}

static int EvaluateBoard(void) {
    if (HasAnyFive(1)) return kWinScore;
    if (HasAnyFive(-1)) return -kWinScore;

    int score = 0;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (_A[r][c].c != 0) continue;
            if (!HasNeighbor(r, c, 2)) continue;

            int atk = EvaluatePoint(r, c, 1);
            int def = EvaluatePoint(r, c, -1);
            score += atk;
            score -= (def * 11) / 10;
        }
    }
    return score;
}

static std::vector<CandidateMove> GenerateCandidates(int piece, int maxCount) {
    std::vector<CandidateMove> moves;

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (_A[r][c].c != 0) continue;
            if (!HasNeighbor(r, c, 2)) continue;

            int attack = EvaluatePoint(r, c, piece);
            int block = EvaluatePoint(r, c, -piece);
            int score = attack + block / 2;
            if (WouldHaveFive(r, c, piece)) score += kWinScore / 2;
            if (WouldHaveFive(r, c, -piece)) score += kWinScore / 3;

            CandidateMove cm = { r, c, score };
            moves.push_back(cm);
        }
    }

    if (moves.empty()) {
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                if (_A[r][c].c == 0) {
                    CandidateMove cm = { r, c, 0 };
                    moves.push_back(cm);
                    return moves;
                }
            }
        }
    }

    std::sort(moves.begin(), moves.end(), [](const CandidateMove& a, const CandidateMove& b) {
        return a.score > b.score;
    });

    if ((int)moves.size() > maxCount) {
        moves.resize(maxCount);
    }
    return moves;
}

static int Minimax(int depth, int alpha, int beta, bool maximizingBot) {
    int eval = EvaluateBoard();
    if (depth == 0 || BoardFull() || eval >= kWinScore || eval <= -kWinScore) {
        return eval;
    }

    if (maximizingBot) {
        int best = -kInfScore;
        std::vector<CandidateMove> cand = GenerateCandidates(1, 10);
        for (size_t i = 0; i < cand.size(); i++) {
            int r = cand[i].r;
            int c = cand[i].c;
            _A[r][c].c = 1;

            int value;
            if (HasFiveAt(r, c, 1)) value = kWinScore - (3 - depth);
            else value = Minimax(depth - 1, alpha, beta, false);

            _A[r][c].c = 0;

            if (value > best) best = value;
            if (best > alpha) alpha = best;
            if (beta <= alpha) break;
        }
        return best;
    } else {
        int best = kInfScore;
        std::vector<CandidateMove> cand = GenerateCandidates(-1, 10);
        for (size_t i = 0; i < cand.size(); i++) {
            int r = cand[i].r;
            int c = cand[i].c;
            _A[r][c].c = -1;

            int value;
            if (HasFiveAt(r, c, -1)) value = -kWinScore + (3 - depth);
            else value = Minimax(depth - 1, alpha, beta, true);

            _A[r][c].c = 0;

            if (value < best) best = value;
            if (best < beta) beta = best;
            if (beta <= alpha) break;
        }
        return best;
    }
}

static int DecideSearchDepth(void) {
    int stones = 0;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (_A[r][c].c != 0) stones++;
        }
    }

    if (stones < 8) return 2;
    if (stones < 40) return 3;
    return 2;
}

void GetBotMove(int& outR, int& outC) {
    if (BoardEmpty()) {
        outR = BOARD_SIZE / 2;
        outC = BOARD_SIZE / 2;
        return;
    }

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (_A[r][c].c == 0 && WouldHaveFive(r, c, 1)) {
                outR = r;
                outC = c;
                return;
            }
        }
    }

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (_A[r][c].c == 0 && WouldHaveFive(r, c, -1)) {
                outR = r;
                outC = c;
                return;
            }
        }
    }

    int depth = DecideSearchDepth();
    std::vector<CandidateMove> root = GenerateCandidates(1, 12);

    int bestScore = -kInfScore;
    int bestR = -1;
    int bestC = -1;

    for (size_t i = 0; i < root.size(); i++) {
        int r = root[i].r;
        int c = root[i].c;

        _A[r][c].c = 1;
        int value;
        if (HasFiveAt(r, c, 1)) value = kWinScore;
        else value = Minimax(depth - 1, -kInfScore, kInfScore, false);
        _A[r][c].c = 0;

        if (value > bestScore || (value == bestScore && (rand() & 1))) {
            bestScore = value;
            bestR = r;
            bestC = c;
        }
    }

    if (bestR < 0 || bestC < 0) {
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                if (_A[r][c].c == 0) {
                    outR = r;
                    outC = c;
                    return;
                }
            }
        }
    }

    outR = bestR;
    outC = bestC;
}
