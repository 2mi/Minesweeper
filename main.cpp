#include <random>
#include <queue>
#include "main.h"

#pragma warning(disable: 4102)

Minesweeper::Minesweeper(uint8_t col, uint8_t row, uint8_t numMines)
 : N(col), 
 M(row), 
 K(numMines), 
 numRests(col * row - numMines), 
 gameStatus(GameStatus::Processing), 
 seed(std::random_device{}())
{
    board.assign(col, std::vector<BlockType>(M, BlockType::Normal));
}

void Minesweeper::start(uint8_t i, uint8_t j)
{
    std::default_random_engine rng(seed);
    std::uniform_int_distribution<uint16_t> genMinePos(0, static_cast<uint16_t>(N * M - 1));
        
    board[i][j] = BlockType::Blank;

    for (auto numMines = K ; numMines ; )
    {
        int r = genMinePos(rng);
        auto &mineBlock = board[r / M][r % M];
        if ((mineBlock & 0x0f) == BlockType::Normal) {
               --numMines;
            mineBlock = static_cast<BlockType>(board[i][j] & ~0x0f | BlockType::Mine);
        }
    }

    board[i][j] = BlockType::Normal;
    gameStatus = static_cast<GameStatus>(GameStatus::Processing | GameStatus::Prepared);
}

void Minesweeper::restore()
{
    numRests = N * M - K;
    for (auto &line : board)
    {
        for (auto &block : line)
        {
            block = static_cast<BlockType>(block & ~(block & 0xf0));
            
            if (block == BlockType::Explode) {
                block = BlockType::Mine;
            } else if (block != BlockType::Mine) {
                block = BlockType::Normal;
            }
        }
    }
    gameStatus = static_cast<GameStatus>(GameStatus::Processing | GameStatus::Prepared);
}

void Minesweeper::expose(const lpfnResultRecv &fnUpdate)
{
    for (uint8_t i = 0 ; i != N ; ++i)
    {
        for (uint8_t j = 0 ; j != M ; ++j)
        {
            if ((board[i][j] & 0x0f) == BlockType::Mine) {
                fnUpdate(i, j, BlockType::Explode);
            }
        }
    }
}

bool Minesweeper::isValidBlock(uint8_t i, uint8_t j)
{   //i != 0 && i != N + 1 && j != 0 && j != M + 1
    if (i < N && j < M) {
        return true;
    }
    return false;
}

void Minesweeper::setFlag(uint8_t i, uint8_t j, const lpfnResultRecv &fnUpdate)
{
    if (!isValidBlock(i, j) || board[i][j] < BlockType::Normal) {
        return;
    }

    BlockType newFlag = BlockType::Origin;

    if ((board[i][j] & 0xf0) == BlockType::Origin) {
        newFlag = BlockType::Mystery;
    } else if ((board[i][j] & 0xf0) == BlockType::Mystery) {
        newFlag = BlockType::Flag;
    }

    board[i][j] = static_cast<BlockType>((board[i][j] & ~0xf0) | newFlag);
    fnUpdate(i, j, board[i][j]);
}

void Minesweeper::click(uint8_t i, uint8_t j, const lpfnResultRecv &fnUpdate)
{
    if (!isValidBlock(i, j) || BlockType::Flag == (board[i][j] & 0xf0)) {
        return;
    }

    if (GameStatus::Prepared != (gameStatus & 0x0f)) {
        start(i, j);
    }

    if (BlockType::Mine == (board[i][j] & 0x0f)) {
        fnUpdate(i, j, BlockType::Explode);
        gameStatus = static_cast<GameStatus>(GameStatus::None | GameStatus::Lost);
    } else {
        bfs(i, j, std::forward<decltype(fnUpdate)>(fnUpdate));
        if (0 == numRests) gameStatus = static_cast<GameStatus>(GameStatus::None | GameStatus::Won); //
    }
__leave_:;
}

void Minesweeper::doubleClick(uint8_t i, uint8_t j, const lpfnResultRecv &fnUpdate)
{
    std::vector<std::pair<uint8_t, uint8_t> > blockPos;
    int8_t numMines;

    if (!isValidBlock(i, j) || !(board[i][j] < BlockType::Digital + 8)) {
        return;
    }
    numMines = board[i][j] - 0;

    for (auto negh_i = std::max<uint8_t>(i, 1) ; negh_i != std::min<uint8_t>(i + 2, N) + 1 ; ++negh_i)
    {
        for (auto negh_j = std::max<uint8_t>(j, 1) ; negh_j != std::min<uint8_t>(j + 2, M) + 1 ; ++negh_j)
        {
            if ((board[negh_i - 1][negh_j - 1] & 0xf0) == BlockType::Flag) {
                --numMines;
            } else if (true) { //(board[negh_i - 1][negh_j - 1] & 0x0f) == BlockType::Normal
                blockPos.emplace_back(static_cast<uint8_t>(negh_i - 1), static_cast<uint8_t>(negh_j - 1));
            }
        }
    }

    if (numMines > 0) {
        return;
    }
    
    for (const auto &pr : blockPos) 
    {
        std::tie(i, j) = pr;
        click(i, j, std::forward<decltype(fnUpdate)>(fnUpdate));

        if ((gameStatus & 0xf0) != GameStatus::Processing) {
            break;
        }
    }
}

void Minesweeper::bfs(uint8_t i, uint8_t j, const lpfnResultRecv &fnUpdate)
{
    using blockPos = std::pair<uint8_t, uint8_t>;

    std::deque<blockPos> blocks;
    uint16_t numblocks;

    for ( ; ; )
    {
        if ((board[i][j] & 0x0f) != BlockType::Normal) {
            goto __next_;
        }

        uint8_t mineCnt = 0;
        numblocks = static_cast<uint16_t>(blocks.size());
        for (auto negh_i = std::max<uint8_t>(i, 1) ; negh_i != std::min<uint8_t>(i + 2, N) + 1 ; ++negh_i)
        {
            for (auto negh_j = std::max<uint8_t>(j, 1) ; negh_j != std::min<uint8_t>(j + 2, M) + 1 ; ++negh_j) 
            {/* 
                if (negh_i - 1 == i && negh_j - 1 == j) {   // optiomal
                    continue;   
                }
            */
                if ((board[negh_i - 1][negh_j - 1] & 0x0f) == BlockType::Mine) {
                    ++mineCnt;
                } else if (!mineCnt && (board[negh_i - 1][negh_j - 1] & 0xf0) != BlockType::Flag && (board[negh_i - 1][negh_j - 1] & 0x0f) == BlockType::Normal) {
                    blocks.emplace_back(static_cast<uint8_t>(negh_i - 1), static_cast<uint8_t>(negh_j - 1));
                }
            }
        }

        if (mineCnt) {
            board[i][j] = static_cast<BlockType>(BlockType::Digital + mineCnt - 1);
            if (numblocks != blocks.size()) {
                blocks.erase(blocks.begin() + numblocks, blocks.end());
            }
        } else {
            board[i][j] = BlockType::Blank;
        }
        --numRests;
        fnUpdate(i, j, board[i][j]);
    __next_:
        if (blocks.empty()) {
            break;
        }
        std::tie(i, j) = blocks.front();
        blocks.pop_front();
    }
}
Minesweeper minesweeper(0, 0, 0);