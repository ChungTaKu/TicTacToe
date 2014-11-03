
#include <vector>
#include <iostream>
using namespace std;

/***************************
 *         Player          *
 ***************************/

class Player {
public:
    int type;
#define TYPE_MAN    0
#define TYPE_PC     1
    int mark;
#define MARK_E      0    // empty
#define MARK_O     -1    // o
#define MARK_X      1    // x
};


/***************************
 *         Board           *
 ***************************/

class Board {
    void draw(char (Board::*func)(int));
    char slot2char(int n) { return (slot[n]==MARK_E?' ':((slot[n]==MARK_O)?'o':'x')); }
    char slot2slot(int n) { return (n + '0'); }
public:
    int slot[9];
    Board() { for (int i=0; i<9; i++) slot[i] = MARK_E; }
    void selectSlotWithMark(int n, int mark) { slot[n] = mark; };
    void drawBoard(void) { draw(&Board::slot2char); };
    void drawBoardInstruction(void) { draw(&Board::slot2slot); };
};

void Board::draw(char (Board::*func)(int)) {
    printf("\n");
    printf("        %c | %c | %c \n", (this->*func)(0), (this->*func)(1), (this->*func)(2));
    printf("       ---+---+---   \n");
    printf("        %c | %c | %c \n", (this->*func)(3), (this->*func)(4), (this->*func)(5));
    printf("       ---+---+---   \n");
    printf("        %c | %c | %c \n", (this->*func)(6), (this->*func)(7), (this->*func)(8));
    printf("\n");
}


/***************************
 *         SubGame         *
 ***************************/

// Class definition
class SubGame {
public:
    int role;
#define ROLE_PLAYER   0
#define ROLE_OPPONENT 1
    int mark;
    int depth;
    int slot[9];
    int score;
    int choice;
    int gameOver;
    SubGame(int role, int mark, int depth, int *slot) {
        this->role = role;
        this->mark = mark;
        this->depth = depth;
        score = 0;
        choice = -1;
        gameOver = 0;
        memcpy(this->slot, slot, 9*sizeof(int));
    }
};


/***************************
 *      Intellegence       *
 ***************************/

// Class definition
int lines[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};

class Intellegence {
    int score(SubGame *game);
    int hasWon(int mark, int *slot);
    void miniMaxAlgo(SubGame *game);
public:
    Intellegence() {};
    int computeBestSlot(int mark, Board *b);
    int whichMarkIsWinner(Board *b);
};

// Public methods
int Intellegence::computeBestSlot(int mark, Board *b) {
    SubGame game = SubGame(ROLE_PLAYER, mark, 0, b->slot);
    miniMaxAlgo(&game);
    return game.choice;
}

int Intellegence::whichMarkIsWinner(Board *b) {
    if (hasWon(MARK_O, b->slot))
        return MARK_O;
    else if (hasWon(MARK_X, b->slot))
        return MARK_X;
    else
        return MARK_E;
}

// Private methods
int Intellegence::score(SubGame *game) {
    if (hasWon(game->mark*(-1), game->slot)) {
        if ((!game->role) == ROLE_PLAYER)
            return (10 - game->depth);
        else
            return (game->depth - 10);
    } else {
        return 0;
    }
}

int Intellegence::hasWon(int mark, int *slot) {
    for (int i=0; i<8; i++) {
        int m1 = slot[lines[i][0]];
        int m2 = slot[lines[i][1]];
        int m3 = slot[lines[i][2]];
        if (m1 + m2 + m3 == 3*mark)
            return true;
    }
    return false;
}

void Intellegence::miniMaxAlgo(SubGame *game) {

    int game_score = score(game);
    if (game_score != 0) {
        // someone win
        game->score = game_score;
        return;
    }

    vector<int> scores;
    int *indexTable = (int *)calloc(9, sizeof(int));

    for (int i=0; i<9; i++) {
        // generate all possible sub games
        if (game->slot[i] != MARK_E) continue;
        int *possibleSlots = (int *)calloc(9, sizeof(int));
        memcpy(possibleSlots, game->slot, 9*sizeof(int));
        possibleSlots[i] = game->mark;
        SubGame possibleSubGame = SubGame(!game->role, game->mark*(-1), game->depth+1, possibleSlots);

        // run minMaxAlgo & get score
        indexTable[scores.size()] = i;
        miniMaxAlgo(&possibleSubGame);
        scores.push_back(possibleSubGame.score);
    }
    
    // tie game
    if (scores.size() == 0)
        return;

    // select slot satisfying min/max rules
    int maxScore, minScore, choice;
    maxScore = minScore = scores[0];
    choice = 0;
    if (game->role == ROLE_PLAYER) {
        for (int i=0; i<(int)scores.size(); i++) {
            if (scores[i] > maxScore) {
                maxScore = scores[i];
                choice = i;
            }
        }
        game->score = maxScore;
    } else {
        for (int i=0; i<(int)scores.size(); i++) {
            if (scores[i] < minScore) {
                minScore = scores[i];
                choice = i;
            }
        }
        game->score = minScore;
    }
    game->choice = indexTable[choice];
}


/***************************
 *        TTTGame          *
 ***************************/

// Class definition
class TTTGame {
    Player player[2];
    Board board;
    Intellegence intellegence;
    int emptyCount;
    int nextExpectedPlayer;
public:
    TTTGame(int p0_Type, int p1_Type);
    bool isOccupied(int n);
    void drawBoard(void) { board.drawBoard(); }
    void drawBoardInstruction(void) { board.drawBoardInstruction(); }
    void selectSlotForPlayer(int playerId, int *n);
    int winner;
#define WINNER_UNKNOWN  -1
#define WINNER_PLAYER0  0
#define WINNER_PLAYER1  1
#define WINNER_TIE      2
};

// Public methods
TTTGame::TTTGame(int typeP0, int typeP1) {
    player[0].type = typeP0;
    player[1].type = typeP1;
    player[0].mark = MARK_O;
    player[1].mark = MARK_X;
    emptyCount = 9;
    winner = WINNER_UNKNOWN;
    nextExpectedPlayer = 0;
}

bool TTTGame::isOccupied(int n) {
    return (board.slot[n] != MARK_E);
}

void TTTGame::selectSlotForPlayer(int pid, int *n) {
    if (pid != nextExpectedPlayer) return;
    if (player[pid].type == TYPE_PC) {
        *n = intellegence.computeBestSlot(player[pid].mark, &board);
    }
    board.selectSlotWithMark(*n, player[pid].mark);
    emptyCount --;

    nextExpectedPlayer = !nextExpectedPlayer;
    int winnerMark = intellegence.whichMarkIsWinner(&board);
    if (winnerMark == MARK_E && emptyCount == 0) 
        winner = WINNER_TIE;
    else if (winnerMark != MARK_E)
        winner = (winnerMark == player[0].mark) ? WINNER_PLAYER0 : WINNER_PLAYER1;
}


/***************************
 *        Main             *
 ***************************/

void main(void) {

_START_THE_GAME:
    system("CLS");
    cout << "**************************************" << endl;
    cout << "*      Name:   Tic-Tac-Toe Game      *" << endl;
    cout << "*      Author: Chung-Ta Ku           *" << endl;
    cout << "*      Date:   11/03/14              *" << endl;
    cout << "**************************************" << endl;

    cout << "Do you want to play first ? (Yes:1 No:0) ";
    char c = ' ';
    do { cin >> c; } while (!isdigit(c) || (c != '0' && c != '1'));
    int manPlayFirst = c - '0';

    int player[2];
    player[0] = manPlayFirst ? TYPE_MAN : TYPE_PC;
    player[1] = manPlayFirst ? TYPE_PC : TYPE_MAN;
    TTTGame game(player[0], player[1]);
    
    int curPlayerId = 0;
    while (game.winner == WINNER_UNKNOWN) {
        int slot = -1;
        if (player[curPlayerId] == TYPE_MAN) {
            // man's turn
            cout << "--------------------------------------" << endl;
            game.drawBoardInstruction();
            cout << "PLAYER " << curPlayerId << ": Enter your slot num (0~8): ";
            do { cin >> c; } while (!isdigit(c) || (c-'0') < 0 || (c-'8') > 0 || game.isOccupied(c-'0'));
            slot = c - '0';
            system("CLS");
        }
        // man's or pc's turn
        game.selectSlotForPlayer(curPlayerId, &slot);
        cout << "PLAYER " << curPlayerId << " select slot " << slot << "." << endl;
        game.drawBoard();
        curPlayerId = !curPlayerId;
    }

    if (game.winner == WINNER_TIE) {
        cout << "!!!!!!!  It's a tie game! !!!!!!!" << endl;
    } else {
        cout << "!!!!!!! Player " << game.winner << " (" << ((player[game.winner]==TYPE_MAN)?"MAN":"PC") << ") won !!!!!!!" << endl << endl;;
    }

    cout << "Replay ? (Yes:1 No:0) ";
    do { cin >> c; } while (!isdigit(c) || (c != '0' && c != '1'));
    int replay = c - '0';

    if (replay)
        goto _START_THE_GAME;
}
