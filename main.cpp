#include <iostream>
#include <vector>
#include <memory>


using Grid = std::vector<std::vector<char>>;

enum class Result {
    NOTHING,
    FIRST,
    SECOND,
    DRAW,
    ERROR
};

const char DEFAULT = '.';
const char FIRST = 'x';
const char SECOND = 'o';

struct Settings {
    int size = 3;
    int lineLen = 3;
};

struct Cell {
    int i = 0;
    int j = 0;
};

auto makeGrid(int size) {
    return Grid(size, std::vector<char>(size, '.'));
}

void print(const Grid& grid) {
    std::cout << ' ';
    for (int i = 0; i < std::min(static_cast<int>(grid.size()), 10); ++i) {
        std::cout << i;
    }
    std::cout << '\n';
    int i = 0;
    for (const auto& vec : grid) {
        if (i < 10) {
            std::cout << i;
            ++i;
        }
        for (const auto& el : vec) {
            std::cout << el;
        }
        std::cout << '\n';
    }
}

void finish(Result result, Grid& grid) {
    print(grid);
    switch (result) {
        case Result::NOTHING:
            throw std::logic_error("NOTHING in finish");
        case Result::FIRST:
            std::cout << "First wins!\n";
            break;
        case Result::SECOND:
            std::cout << "Second wins!\n";
            break;
        case Result::ERROR:
            throw;
        case Result::DRAW:
            std::cout << "Draw.\n";
            break;
        default:
            throw std::runtime_error("Can't be default.");
    }
}

class Player {
public:
    virtual void init(const Settings& settings, char mark) = 0;

    virtual Result move(Grid& grid) = 0;

protected:
    char mark_ = '\0';
    bool isFirst_ = true;
    Settings settings_;

    [[nodiscard]] static bool validMove(const Grid& grid, size_t i, size_t j) {
        return grid[i][j] == DEFAULT;
        // Add that new cell must be near to the old cells
    }

    bool forEveryLine(const Grid& grid) const {
        for (int pos = 0; pos < settings_.size; ++pos) {
            // Row
            Cell start{pos, 0};
            int stepI = 0;
            int stepJ = 1;
            if (checkWinLine(grid, start, stepI, stepJ)) {
                return true;
            }
            // Diagonal
            stepI = 1;
            stepJ = 1;
            if (checkWinLine(grid, start, stepI, stepJ)) {
                return true;
            }
            // Diagonal
            stepI = 1;
            stepJ = -1;
            if (checkWinLine(grid, start, stepI, stepJ)) {
                return true;
            }

            // Col
            stepI = 1;
            stepJ = 0;
            start.i = 0;
            start.j = pos;
            if (checkWinLine(grid, start, stepI, stepJ)) {
                return true;
            }
            // Diagonal
            stepI = 1;
            stepJ = 1;
            if (checkWinLine(grid, start, stepI, stepJ)) {
                return true;
            }
            // Diagonal
            stepI = 1;
            stepJ = -1;
            if (checkWinLine(grid, start, stepI, stepJ)) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool checkWin(const Grid& grid) {
        return forEveryLine(grid);
    }

private:
    [[nodiscard]] bool checkWinLine(const Grid& grid, Cell start, int stepI, int stepJ) const {
        int count = 0;
        while (okBorders(start)) {
            char cur = grid[start.i][start.j];
            if (cur == mark_) {
                ++count;
            } else {
                count = 0;
            }
            if (count >= settings_.lineLen) {
                return true;
            }
            start.i += stepI;
            start.j += stepJ;
        }
        return false;
    }

    [[nodiscard]] bool okBorders(const Cell& cell) const {
        return cell.i >= 0 && cell.i < settings_.size
               && cell.j >= 0 && cell.j < settings_.size;
    }
};

using PlayerPtr = std::unique_ptr<Player>;

class HumanPlayer final : public Player {
public:
    void init(const Settings& settings, char mark) final {
        settings_ = settings;
        mark_ = mark;
        isFirst_ = mark_ == FIRST;
    }

    Result move(Grid& grid) final {
        print(grid);
        size_t i = 0, j = 0;

        std::cout << "Human player " + std::string{mark_} + " move (i, j):\n";
        while (true) {
            std::cin >> i >> j;
            if (validMove(grid, i, j)) {
                break;
            }
            std::cout << "Try again (i, j):\n";
        }
        grid[i][j] = mark_;
        if (checkWin(grid)) {
            return isFirst_ ? Result::FIRST : Result::SECOND;
        }
        return Result::NOTHING;
        // Draw?
    }
};

class ComputerPlayer : public Player {
public:
    void init(const Settings& settings, char mark) final {
        settings_ = settings;
        own_.resize(settings.size, std::vector<char>(settings.size, DEFAULT));
        mark_ = mark;
        isFirst_ = mark == FIRST;
    }

    Result move(Grid& grid) final {
        return Result::NOTHING;
    }

private:
    Grid own_;
};

void game(PlayerPtr first, PlayerPtr second, Grid& grid) {
    Result result = Result::NOTHING;
    auto numEmptyCells = grid.size() * grid.size();
    while (result == Result::NOTHING) {
        result = first->move(grid);
        --numEmptyCells;
        if (result != Result::NOTHING) {
            return finish(result, grid);
        }
        if (numEmptyCells == 0) {
            return finish(Result::DRAW, grid);
        }
        result = second->move(grid);
        --numEmptyCells;
        if (numEmptyCells == 0) {
            return finish(Result::DRAW, grid);
        }
    }
    finish(result, grid);
}

void startGame(PlayerPtr first, PlayerPtr second) {
    int size = 0;
    std::cout << "Grid size (3 -- 100):\n";
    std::cin >> size;
    if (size < 3 || size > 100) {
        throw std::range_error("Grid size is " + std::to_string(size));
    }
    int lineLen = 0;
    std::cout << "Number of cells to win (3 -- " + std::to_string(size) + "):\n";
    std::cin >> lineLen;
    if (lineLen < 3 || lineLen > size) {
        throw std::range_error("Line len is " + std::to_string(lineLen));
    }
    Settings settings;
    settings.size = size;
    settings.lineLen = lineLen;
    first->init(settings, FIRST);
    second->init(settings, SECOND);
    auto grid = makeGrid(size);
    game(std::move(first), std::move(second), grid);
}

std::pair<PlayerPtr, PlayerPtr> init() {
    std::pair<PlayerPtr, PlayerPtr> answer;
    std::cout << "First is human? (y/n)\n";
    std::string response;
    std::cin >> response;
    if (response == "y") {
        answer.first = std::make_unique<HumanPlayer>();
    } else {
        answer.first = std::make_unique<ComputerPlayer>();
    }

    std::cout << "Second is human? (y/n)\n";
    std::cin >> response;
    if (response == "y") {
        answer.second = std::make_unique<HumanPlayer>();
    } else {
        answer.second = std::make_unique<ComputerPlayer>();
    }
    return answer;
}

int main() {
    auto players = init();
    startGame(std::move(players.first), std::move(players.second));
    return 0;
}
