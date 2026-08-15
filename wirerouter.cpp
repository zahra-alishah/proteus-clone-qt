#include "wirerouter.h"
#include <queue>
#include <vector>
#include <limits>
#include <algorithm>

namespace {


const int kDx[4] = { 0,  0, -1,  1 };
const int kDy[4] = {-1,  1,  0,  0 };


const long long kTurnPenalty = 4;
const long long kInf = std::numeric_limits<long long>::max() / 4;

}

QVector<Position> WireRouter::findPath(const Position &start, const Position &end,
                                       const QVector<QRect> &obstacles, int gridStep)
{
    QVector<Position> result;
    if (gridStep <= 0) gridStep = 10;


    int minX = std::min(0, std::min(start.x, end.x)) - gridStep;
    int minY = std::min(0, std::min(start.y, end.y)) - gridStep;
    int maxX = std::max(1000, std::max(start.x, end.x)) + gridStep;
    int maxY = std::max(1000, std::max(start.y, end.y)) + gridStep;

    int width  = (maxX - minX) / gridStep + 1;
    int height = (maxY - minY) / gridStep + 1;
    if (width <= 1 || height <= 1) return result;

    auto toGx = [&](int x) { return qRound(double(x - minX) / gridStep); };
    auto toGy = [&](int y) { return qRound(double(y - minY) / gridStep); };
    auto toWorldX = [&](int gx) { return minX + gx * gridStep; };
    auto toWorldY = [&](int gy) { return minY + gy * gridStep; };

    int startGx = toGx(start.x), startGy = toGy(start.y);
    int endGx = toGx(end.x), endGy = toGy(end.y);
    startGx = std::clamp(startGx, 0, width - 1);
    startGy = std::clamp(startGy, 0, height - 1);
    endGx = std::clamp(endGx, 0, width - 1);
    endGy = std::clamp(endGy, 0, height - 1);

    if (startGx == endGx && startGy == endGy) return result;


    std::vector<bool> blocked(size_t(width) * height, false);
    for (const QRect &r : obstacles) {
        int gxMin = std::clamp(toGx(r.left())  - 1, 0, width - 1);
        int gxMax = std::clamp(toGx(r.right()) + 1, 0, width - 1);
        int gyMin = std::clamp(toGy(r.top())    - 1, 0, height - 1);
        int gyMax = std::clamp(toGy(r.bottom()) + 1, 0, height - 1);
        for (int gy = gyMin; gy <= gyMax; ++gy) {
            for (int gx = gxMin; gx <= gxMax; ++gx) {
                int wx = toWorldX(gx), wy = toWorldY(gy);
                if (r.contains(QPoint(wx, wy))) {
                    blocked[size_t(gy) * width + gx] = true;
                }
            }
        }
    }

    blocked[size_t(startGy) * width + startGx] = false;
    blocked[size_t(endGy) * width + endGx] = false;


    auto stateIndex = [&](int gx, int gy, int dir) {
        return (size_t(gy) * width + gx) * 5 + dir;
    };

    size_t stateCount = size_t(width) * height * 5;
    std::vector<long long> dist(stateCount, kInf);
    std::vector<int> prevState(stateCount, -1);

    using QueueItem = std::pair<long long, int>; // (cost, stateIndex)
    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> pq;

    int startState = int(stateIndex(startGx, startGy, 0));
    dist[startState] = 0;
    pq.push({0, startState});

    int goalGx = endGx, goalGy = endGy;
    int foundState = -1;

    while (!pq.empty()) {
        auto [cost, st] = pq.top();
        pq.pop();
        if (cost > dist[st]) continue;

        int cellIdx = st / 5;
        int dir = st % 5;
        int gx = cellIdx % width;
        int gy = cellIdx / width;

        if (gx == goalGx && gy == goalGy) {
            foundState = st;
            break;
        }

        for (int d = 0; d < 4; ++d) {
            int nx = gx + kDx[d];
            int ny = gy + kDy[d];
            if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
            if (blocked[size_t(ny) * width + nx]) continue;

            int newDirCode = d + 1; // 1..4
            long long turnCost = (dir != 0 && dir != newDirCode) ? kTurnPenalty : 0;
            long long newCost = cost + 1 + turnCost;

            int ns = int(stateIndex(nx, ny, newDirCode));
            if (newCost < dist[ns]) {
                dist[ns] = newCost;
                prevState[ns] = st;
                pq.push({newCost, ns});
            }
        }
    }

    if (foundState == -1) return result;

    QVector<Position> fullPath;
    int cur = foundState;
    while (cur != -1) {
        int cellIdx = cur / 5;
        int gx = cellIdx % width;
        int gy = cellIdx / width;
        fullPath.push_back(Position(toWorldX(gx), toWorldY(gy)));
        cur = prevState[cur];
    }
    std::reverse(fullPath.begin(), fullPath.end());

    if (fullPath.size() <= 2) return result;


    for (int i = 1; i < fullPath.size() - 1; ++i) {
        Position prev = fullPath[i - 1];
        Position cur2 = fullPath[i];
        Position next = fullPath[i + 1];
        int dx1 = cur2.x - prev.x, dy1 = cur2.y - prev.y;
        int dx2 = next.x - cur2.x, dy2 = next.y - cur2.y;
        if (dx1 != dx2 || dy1 != dy2) {
            result.push_back(cur2);
        }
    }

    return result;
}