#pragma once

#include <vector>
#include <functional>
/*
#pragma pack(1)
template<typename _Ty, size_t _N>
struct UnionType
{
    template<typename _Ty2>
    inline void set(_Ty2) {
        ;
    }
    template<typename _Ty2>
    inline bool test(_Ty2 ty) const {
        return val >> _N * count<_Ty2>() & ty == ty;
    }
    inline operator _Ty() const {
        return val;
    }
    inline UnionType(_Ty _val) : val(_val) {

    }
    bool operator ==(const UnionType &) = delete;
    UnionType& operator =(const UnionType &) = delete;
private:
    _Ty val;

    template<typename _Ty2, uint8_t _N2 = sizeof(_Ty2) - 1>
    constexpr uint8_t count() {
        return ;
    }
};
#pragma pack()

struct BlockType : UnionType<uint8_t, 4>
{
    typedef enum : uint8_t { Digital = 8, Blank, Normal, Mine, Explode } _field1;
    typedef enum : uint16_t { Mystery, Flag } _field2;
    inline BlockType(uint8_t _val) : UnionType(_val) {}
};

struct GameStatus : UnionType<uint8_t, 7>
{
    typedef enum : uint8_t { Prepared, Lost, Won } _field1;
    typedef enum : uint16_t { Processing, None } _field2;
    inline GameStatus(uint8_t _val) : UnionType(_val) {}
};*/

typedef enum : uint8_t { Origin = 0x00, Mystery = 0x10, Flag = 0x20, Digital = 1, Blank = 9, Explode, Normal, Mine } BlockType;
typedef enum : uint8_t { None = 0x00, Processing = 0x80, Prepared = 1, Lost = 2, Won = 3 } GameStatus;

class Minesweeper
{
public:
    using lpfnResultRecv = std::function<void(uint8_t, uint8_t, BlockType)>;
    GameStatus gameStatus;

    Minesweeper(uint8_t col, uint8_t row, uint8_t numMines);
    void restore();
    void click(uint8_t i, uint8_t j, const lpfnResultRecv &);
    void doubleClick(uint8_t i, uint8_t j, const lpfnResultRecv &);
    void setFlag(uint8_t i, uint8_t j, const lpfnResultRecv &);
    void expose(const lpfnResultRecv &);
    
    inline const auto& getCol() {
        return N;
    }
    inline const auto& getRow() {
        return M;
    }
    inline const auto& getNumMines() {
        return K;
    }
private:
    unsigned seed;
    const uint8_t N, M, K;
    uint16_t numRests;
    std::vector<std::vector<BlockType>> board;

    inline bool isValidBlock(uint8_t i, uint8_t j);
    void bfs(uint8_t i, uint8_t j, const lpfnResultRecv &);
protected:
    void start(uint8_t i, uint8_t j);
};
extern Minesweeper minesweeper;