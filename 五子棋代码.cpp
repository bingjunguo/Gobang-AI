#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iomanip>

using namespace std;
using namespace cv;
//sro ���� Orz

cv::Mat chessboard, whiteChess, blackChess, tmp, BGS;

int is_red(Vec3b X) {
    // cout << (int)X[1] << ' ' << (int)X[2] << ' ' << (int)X[3] << endl;
    return X[0] < 200 && X[1] < 200 && X[2] > 230;
}

cv::Mat BG;

void imageCopyToBG(cv::Mat chess, int x, int y) {
    x *= 35;
    y *= 35;
    int rows = chess.rows;
    int cols = chess.cols;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (!is_red(chess.at<Vec3b>(i, j))) {
                BG.at<Vec3b>(x + i + 8, y + j + 8) = chess.at<Vec3b>(i, j);
            }
        }
    }
}


/*
ʵ���õĲ���
*/
class CONFIG {
public:
    static const int BOARD_SIZE = 15;
    static const int EMPTY = 0;
    static const int USER_1 = 1;
    static const int USER_2 = 2;
    static const int AI_EMPTY = 0; // ����
    static const int AI_MY = 1; // ��������
    static const int AI_OP = 2; // �Է��ӻ�������
    static const int MAX_NODE = 2;
    static const int MIN_NODE = 1;
    static const int INF = 106666666;
    static const int ERROR_INDEX = -1;
    //����ֵ
    static const int AI_ZERO = 0;
    static const int AI_ONE = 10;
    static const int AI_ONE_S = 1;
    static const int AI_TWO = 100;
    static const int AI_TWO_S = 10;
    static const int AI_THREE = 1000;
    static const int AI_THREE_S = 100;
    static const int AI_FOUR = 10000;
    static const int AI_FOUR_S = 1000;
    static const int AI_FIVE = 100000;
};

/*
���̸���
*/
class Grid :CONFIG {
public:
    int type; //����

    Grid() {
        type = EMPTY;
    }

    Grid(int t) {
        type = t;
    }

    void grid(int t = EMPTY) {
        type = t;
    }

    int isEmpty() {
        return type == EMPTY ? true : false;
    }
};

/*
����
*/
class ChessBoard :CONFIG {
public:
    Grid chessBoard[BOARD_SIZE][BOARD_SIZE];

    ChessBoard() {
        for (int i = 0; i < BOARD_SIZE; ++i)
            for (int j = 0; j < BOARD_SIZE; ++j)
                chessBoard[i][j].grid();
    }

    ChessBoard(const ChessBoard &othr) {
        for (int i = 0; i < BOARD_SIZE; ++i)
            for (int j = 0; j < BOARD_SIZE; ++j)
                chessBoard[i][j].grid(othr.chessBoard[i][j].type);
    }

    /*
    ��������
    ���ط��������Ƿ�ɹ�
    */
    bool placePiece(int x, int y, int type) {
        if (chessBoard[x][y].isEmpty()) {
            chessBoard[x][y].type = type;
            return true;
        }
        return false;
    }
};

/*
ɷ��AI
*/
class Game :CONFIG {
public:
    ChessBoard curState; // ��ǰ����
    bool isStart; // �Ƿ������
    int curUser; // ��ǰ������
    int MAX_DEPTH; // �����������

                   /*
                   ��ʼ���趨�Ѷ�
                   */
    void startGame(int nd = 2) {
        MAX_DEPTH = nd;
        isStart = true;
        curUser = USER_1;
    }

    /*
    ת��������
    */
    void changeUser() {
        curUser = curUser == USER_1 ? USER_2 : USER_1;
    }

    /*
    ���ݸ���type
    A:���ж����ӵ�����
    type:�ҷ����ӵ�����
    ����A�Ǵ��ж����� ������ �Է�����
    */
    int getPieceType(int A, int type) {
        return A == type ? AI_MY : (A == EMPTY ? AI_EMPTY : AI_OP);
    }

    int getPieceType(const ChessBoard &board, int x, int y, int type) {
        if (x < 0 || y < 0 || x >= BOARD_SIZE || y >= BOARD_SIZE)// �����߽簴�Է�������
            return AI_OP;
        else
            return getPieceType(board.chessBoard[x][y].type, type);
    }

    /*
    ��ǰ�����˷�������
    ����ʧ�ܷ���ʧ��
    ���óɹ�
    �����Ϸ�Ƿ����
    ת����Ϸ��ɫ�󷵻سɹ�
    */
    bool placePiece(int x, int y) {
        if (curState.placePiece(x, y, curUser)) {
            // ����������Ƿ�ʤ��
            if (isWin(x, y)) {
                isStart = false; // ��Ϸ����
                                 //  return true;
            }
            changeUser(); // ת����Ϸ��ɫ
            return true;
        }
        return false;
    }

    bool isWin(int x, int y) {
        if (evaluatePiece(curState, x, y, curUser) >= AI_FIVE)
            return true;
        return false;
    }

    /*
    ��center��Ϊ����λ�ý�������һ�����������
    */
    int evaluateLine(int line[], bool ALL) {
        int value = 0; // ��ֵ
        int cnt = 0; // ������
        int blk = 0; // �����
        for (int i = 0; i < BOARD_SIZE; ++i) {
            if (line[i] == AI_MY) { // �ҵ���һ������������
                                    // ��ԭ����
                cnt = 1;
                blk = 0;
                // ������Ƿ���
                if (line[i - 1] == AI_OP)
                    ++blk;
                // ����������
                for (i = i + 1; i < BOARD_SIZE && line[i] == AI_MY; ++i, ++cnt);
                // ���Ҳ��Ƿ���
                if (line[i] == AI_OP)
                    ++blk;
                // ��������ֵ
                value += getValue(cnt, blk);
            }
        }
        return value;
    }

    /*
    ��center��Ϊ����λ�ý�������һ����������ӣ�ǰ��4��Χ�ڣ�
    */
    int evaluateLine(int line[]) {
        int cnt = 1; // ������
        int blk = 0; // �����
                     // ������ɨ
        for (int i = 3; i >= 0; --i) {
            if (line[i] == AI_MY) ++cnt;
            else if (line[i] == AI_OP) {
                ++blk;
                break;
            }
            else
                break;
        }
        for (int i = 5; i < 9; ++i) {
            if (line[i] == AI_MY) ++cnt;
            else if (line[i] == AI_OP) {
                ++blk;
                break;
            }
            else
                break;
        }
        return getValue(cnt, blk);
    }

    /*
    �����������ͷ��������һ������ֵ
    */
    int getValue(int cnt, int blk) {
        if (blk == 0) {// ����
            switch (cnt) {
            case 1:
                return AI_ONE;
            case 2:
                return AI_TWO;
            case 3:
                return AI_THREE;
            case 4:
                return AI_FOUR;
            default:
                return AI_FIVE;
            }
        }
        else if (blk == 1) {// �������
            switch (cnt) {
            case 1:
                return AI_ONE_S;
            case 2:
                return AI_TWO_S;
            case 3:
                return AI_THREE_S;
            case 4:
                return AI_FOUR_S;
            default:
                return AI_FIVE;
            }
        }
        else {// ˫�����
            if (cnt >= 5)
                return AI_FIVE;
            else
                return AI_ZERO;
        }
    }

    /*
    ��һ��״̬��һ��λ�÷���һ�����͵����ӵ����ӽ��й���
    */
    int evaluatePiece(ChessBoard state, int x, int y, int type) {
        int value = 0; // ����ֵ
        int line[17];  //��״̬
        bool flagX[8];// ����߽��־
        flagX[0] = x - 4 < 0;
        flagX[1] = x - 3 < 0;
        flagX[2] = x - 2 < 0;
        flagX[3] = x - 1 < 0;
        flagX[4] = x + 1 > 14;
        flagX[5] = x + 2 > 14;
        flagX[6] = x + 3 > 14;
        flagX[7] = x + 4 > 14;
        bool flagY[8];// ����߽��־
        flagY[0] = y - 4 < 0;
        flagY[1] = y - 3 < 0;
        flagY[2] = y - 2 < 0;
        flagY[3] = y - 1 < 0;
        flagY[4] = y + 1 > 14;
        flagY[5] = y + 2 > 14;
        flagY[6] = y + 3 > 14;
        flagY[7] = y + 4 > 14;

        line[4] = AI_MY; // ��������
                         // ��
        line[0] = flagX[0] ? AI_OP : (getPieceType(state.chessBoard[x - 4][y].type, type));
        line[1] = flagX[1] ? AI_OP : (getPieceType(state.chessBoard[x - 3][y].type, type));
        line[2] = flagX[2] ? AI_OP : (getPieceType(state.chessBoard[x - 2][y].type, type));
        line[3] = flagX[3] ? AI_OP : (getPieceType(state.chessBoard[x - 1][y].type, type));

        line[5] = flagX[4] ? AI_OP : (getPieceType(state.chessBoard[x + 1][y].type, type));
        line[6] = flagX[5] ? AI_OP : (getPieceType(state.chessBoard[x + 2][y].type, type));
        line[7] = flagX[6] ? AI_OP : (getPieceType(state.chessBoard[x + 3][y].type, type));
        line[8] = flagX[7] ? AI_OP : (getPieceType(state.chessBoard[x + 4][y].type, type));

        value += evaluateLine(line);

        // ��
        line[0] = flagY[0] ? AI_OP : getPieceType(state.chessBoard[x][y - 4].type, type);
        line[1] = flagY[1] ? AI_OP : getPieceType(state.chessBoard[x][y - 3].type, type);
        line[2] = flagY[2] ? AI_OP : getPieceType(state.chessBoard[x][y - 2].type, type);
        line[3] = flagY[3] ? AI_OP : getPieceType(state.chessBoard[x][y - 1].type, type);

        line[5] = flagY[4] ? AI_OP : getPieceType(state.chessBoard[x][y + 1].type, type);
        line[6] = flagY[5] ? AI_OP : getPieceType(state.chessBoard[x][y + 2].type, type);
        line[7] = flagY[6] ? AI_OP : getPieceType(state.chessBoard[x][y + 3].type, type);
        line[8] = flagY[7] ? AI_OP : getPieceType(state.chessBoard[x][y + 4].type, type);

        value += evaluateLine(line);

        // ����-����
        line[0] = flagX[0] || flagY[0] ? AI_OP : getPieceType(state.chessBoard[x - 4][y - 4].type, type);
        line[1] = flagX[1] || flagY[1] ? AI_OP : getPieceType(state.chessBoard[x - 3][y - 3].type, type);
        line[2] = flagX[2] || flagY[2] ? AI_OP : getPieceType(state.chessBoard[x - 2][y - 2].type, type);
        line[3] = flagX[3] || flagY[3] ? AI_OP : getPieceType(state.chessBoard[x - 1][y - 1].type, type);

        line[5] = flagX[4] || flagY[4] ? AI_OP : getPieceType(state.chessBoard[x + 1][y + 1].type, type);
        line[6] = flagX[5] || flagY[5] ? AI_OP : getPieceType(state.chessBoard[x + 2][y + 2].type, type);
        line[7] = flagX[6] || flagY[6] ? AI_OP : getPieceType(state.chessBoard[x + 3][y + 3].type, type);
        line[8] = flagX[7] || flagY[7] ? AI_OP : getPieceType(state.chessBoard[x + 4][y + 4].type, type);

        value += evaluateLine(line);

        // ����-����
        line[0] = flagX[7] || flagY[0] ? AI_OP : getPieceType(state.chessBoard[x + 4][y - 4].type, type);
        line[1] = flagX[6] || flagY[1] ? AI_OP : getPieceType(state.chessBoard[x + 3][y - 3].type, type);
        line[2] = flagX[5] || flagY[2] ? AI_OP : getPieceType(state.chessBoard[x + 2][y - 2].type, type);
        line[3] = flagX[4] || flagY[3] ? AI_OP : getPieceType(state.chessBoard[x + 1][y - 1].type, type);

        line[5] = flagX[3] || flagY[4] ? AI_OP : getPieceType(state.chessBoard[x - 1][y + 1].type, type);
        line[6] = flagX[2] || flagY[5] ? AI_OP : getPieceType(state.chessBoard[x - 2][y + 2].type, type);
        line[7] = flagX[1] || flagY[6] ? AI_OP : getPieceType(state.chessBoard[x - 3][y + 3].type, type);
        line[8] = flagX[0] || flagY[7] ? AI_OP : getPieceType(state.chessBoard[x - 4][y + 4].type, type);

        value += evaluateLine(line);

        return value;
    }

    /*
    ����һ�������ϵ�һ��
    */
    int evaluateState(ChessBoard state, int type) {
        int value = 0;
        // �ֽ����״̬
        int line[6][17];
        int lineP;

        for (int p = 0; p < 6; ++p)
            line[p][0] = line[p][16] = AI_OP;

        // ���ĸ��������
        for (int i = 0; i < BOARD_SIZE; ++i) {
            // ������״̬
            lineP = 1;

            for (int j = 0; j < BOARD_SIZE; ++j) {
                line[0][lineP] = getPieceType(state, i, j, type); /* | */
                line[1][lineP] = getPieceType(state, j, i, type); /* - */
                line[2][lineP] = getPieceType(state, i + j, j, type); /* \ */
                line[3][lineP] = getPieceType(state, i - j, j, type); /* / */
                line[4][lineP] = getPieceType(state, j, i + j, type); /* \ */
                line[5][lineP] = getPieceType(state, BOARD_SIZE - j - 1, i + j, type); /* / */
                ++lineP;
            }
            // ����
            int special = i == 0 ? 4 : 6;
            for (int p = 0; p < special; ++p) {
                value += evaluateLine(line[p], true);
            }
        }
        return value;
    }

    /*
    ��x, yλ����Χ1����������������
    */
    bool canSearch(ChessBoard state, int x, int y) {

        int tmpx = x - 1;
        int tmpy = y - 1;
        for (int i = 0; tmpx < BOARD_SIZE && i < 3; ++tmpx, ++i) {
            int ty = tmpy;
            for (int j = 0; ty < BOARD_SIZE && j < 3; ++ty, ++j) {
                if (tmpx >= 0 && ty >= 0 && state.chessBoard[tmpx][ty].type != EMPTY)
                    return true;
                else
                    continue;
            }
        }
        return false;
    }

    /*
    ������̽ڵ������
    */
    int nextType(int type) {
        return type == MAX_NODE ? MIN_NODE : MAX_NODE;
    }

    /*
    state ��ת����״̬
    type  ��ǰ��ı�ǣ�MAX MIN
    depth ��ǰ����
    alpha ����alphaֵ
    beta  ����betaֵ
    */
    int minMax(ChessBoard state, int x, int y, int type, int depth, int alpha, int beta) {
        ChessBoard newState(state);
        newState.placePiece(x, y, nextType(type));

        int weight = 0;
        int max = -INF; // �²�Ȩֵ�Ͻ�
        int min = INF; // �²�Ȩֵ�½�

        if (depth < MAX_DEPTH) {
            // �������ʤ�򲻼�������
            if (evaluatePiece(newState, x, y, nextType(type)) >= AI_FIVE) {
                if (type == MIN_NODE)
                    return AI_FIVE; // �ҷ�ʤ
                else
                    return -AI_FIVE;
            }

            int i, j;
            for (i = 0; i < BOARD_SIZE; ++i) {
                for (j = 0; j < BOARD_SIZE; ++j) {
                    if (newState.chessBoard[i][j].type == EMPTY && canSearch(newState, i, j)) {
                        weight = minMax(newState, i, j, nextType(type), depth + 1, min, max);

                        if (weight > max)
                            max = weight; // �����²��Ͻ�
                        if (weight < min)
                            min = weight; // �����²��½�

                                          // alpha-beta
                        if (type == MAX_NODE) {
                            if (max >= alpha)
                                return max;
                        }
                        else {
                            if (min <= beta)
                                return min;
                        }
                    }
                    else
                        continue;
                }
            }

            if (type == MAX_NODE)
                return max; // ����������ֵ
            else
                return min; // ��С�������Сֵ
        }
        else {
            weight = evaluateState(newState, MAX_NODE); // �����ҷ�����
            weight -= type == MIN_NODE ? evaluateState(newState, MIN_NODE) * 10 : evaluateState(newState, MIN_NODE); // �����Է�����
            return weight; // �������޶�������Ȩֵ
        }
    }


    int cnt[BOARD_SIZE][BOARD_SIZE];
    /*
    AI ����
    */
    bool placePieceAI() {
        int weight;
        int max = -INF; // �����Ȩֵ�Ͻ�
        int x = 0, y = 0;
        memset(cnt, 0, sizeof(cnt));
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if (curState.chessBoard[i][j].type == EMPTY && canSearch(curState, i, j)) {
                    weight = minMax(curState, i, j, nextType(MAX_NODE), 1, -INF, max);
                    cnt[i][j] = weight;
                    if (weight > max) {
                        max = weight; // �����²��Ͻ�
                        x = i;
                        y = j;
                    }
                }
                else
                    continue;
            }
        }
        return placePiece(x, y); // AI���ŵ�
    }

    /*
    ����̨��ӡ������
    */
    void show() {

        chessboard.copyTo(BG);
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if (curState.chessBoard[i][j].type == 1)
                    imageCopyToBG(blackChess, i, j);
                if (curState.chessBoard[i][j].type == 2)
                    imageCopyToBG(whiteChess, i, j);
            }
        }
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if (curState.chessBoard[i][j].type == 0)
                    cout << " -";
                if (curState.chessBoard[i][j].type == 1)
                    cout << " X";
                if (curState.chessBoard[i][j].type == 2)
                    cout << " O";
            }
            cout << endl;
        }
        imshow("gobang", BG);
        cv::waitKey(5);
    }

};

using namespace cv;
using namespace std;

int X, Y = 0;
int optIsOk = 1;
Game G;

static void onMouse(int event, int x, int y, int, void*)
{
    if (event != EVENT_LBUTTONDOWN)
        return;
    if (!optIsOk) return;
    X = x;   Y = y;
    optIsOk = 0;
}

int main(int argc, char** argv)
{
    chessboard = cv::imread("chessboard.bmp");
    tmp = cv::imread("whiteChess.bmp");
    resize(tmp, whiteChess, Size(30, 30), 0, 0, CV_INTER_LINEAR);
    tmp = cv::imread("blackChess.bmp");
    resize(tmp, blackChess, Size(30, 30), 0, 0, CV_INTER_LINEAR);

    namedWindow("gobang", 1);
    setMouseCallback("gobang", onMouse, 0);
    chessboard.copyTo(BG);
    imshow("gobang", BG);
    cv::waitKey(50);

    int flag = 0;
    
    G.startGame(4);
    for (;;)
    {   
     //   if (!optIsOk) {
        /*    int tx = (X - 8) / 35, ty = (Y - 8) / 35;
            cout << tx << ' ' << ty << endl;
            G.placePiece(ty, tx);
            cout << tx << ' ' << ty << endl;*/
            G.placePieceAI();
            G.show();
            G.placePieceAI();
            G.show();
            optIsOk = 1;
     //   }
        cv::waitKey(5);
    }

    return 0;
}
