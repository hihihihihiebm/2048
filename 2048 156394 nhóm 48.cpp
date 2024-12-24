#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <termios.h>
#include <unistd.h>

using namespace std;

const int SIZE = 4;
int board[SIZE][SIZE];  // Gi? l?i b?ng duy nh?t
int score = 0;
atomic<bool> timeUp(false);

auto start_time = chrono::steady_clock::now(); // Th?i gian b?t d?u

void initializeBoard() {
    score = 0;
    timeUp = false;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            board[i][j] = 0;
        }
    }
    // T?o 2 s? ban d?u
    int x = rand() % SIZE;
    int y = rand() % SIZE;
    board[x][y] = 2;

    x = rand() % SIZE;
    y = rand() % SIZE;
    board[x][y] = 2;
}

void printBoard() {
    // Hi?n th? th?i gian còn l?i ? d?u b?ng
    auto now = chrono::steady_clock::now();
    auto time_left = chrono::duration_cast<chrono::seconds>(chrono::minutes(1) - (now - start_time)).count();
    cout << "\rTh?i gian còn l?i: " << time_left / 60 << " phút " << time_left % 60 << " giây   " << flush;

    // In b?ng và di?m
    cout << "\nÐi?m s?: " << score << endl;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == 0)
                cout << ".\t";
            else
                cout << board[i][j] << "\t";
        }
        cout << endl;
    }
}

bool movePossible() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == 0)
                return true;
            if (i + 1 < SIZE && board[i][j] == board[i + 1][j])
                return true;
            if (j + 1 < SIZE && board[i][j] == board[i][j + 1])
                return true;
        }
    }
    return false;
}

void addNewNumber() {
    vector<pair<int, int>> emptyCells;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == 0)
                emptyCells.push_back({i, j});
        }
    }
    if (!emptyCells.empty()) {
        int index = rand() % emptyCells.size();
        int x = emptyCells[index].first;
        int y = emptyCells[index].second;
        board[x][y] = (rand() % 2 + 1) * 2;
    }
}

void slideLeft() {
    for (int i = 0; i < SIZE; ++i) {
        vector<int> newRow;
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] != 0)
                newRow.push_back(board[i][j]);
        }

        for (int j = 0; j < SIZE; ++j) {
            if (j < newRow.size()) {
                board[i][j] = newRow[j];
            } else {
                board[i][j] = 0;
            }
        }
    }
}

void mergeLeft() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE - 1; ++j) {
            if (board[i][j] != 0 && board[i][j] == board[i][j + 1]) {
                board[i][j] *= 2;
                score += board[i][j];
                board[i][j + 1] = 0;
            }
        }
    }
}

void rotateBoardClockwise() {
    int temp[SIZE][SIZE];
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            temp[j][SIZE - 1 - i] = board[i][j];
        }
    }

    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            board[i][j] = temp[i][j];
        }
    }
}

void moveLeft() {
    slideLeft();
    mergeLeft();
    slideLeft();
}

void moveUp() {
    rotateBoardClockwise();
    rotateBoardClockwise();
    rotateBoardClockwise();
    moveLeft();
    rotateBoardClockwise();
}

void moveRight() {
    rotateBoardClockwise();
    rotateBoardClockwise();
    moveLeft();
    rotateBoardClockwise();
    rotateBoardClockwise();
}

void moveDown() {
    rotateBoardClockwise();
    moveLeft();
    rotateBoardClockwise();
    rotateBoardClockwise();
    rotateBoardClockwise();
}

bool checkWin() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == 2048) {
                return true;
            }
        }
    }
    return false;
}

bool checkGameOver() {
    return !movePossible();
}

void timerThread(int minutes) {
    auto end_time = start_time + chrono::minutes(minutes);

    while (!timeUp) {
        auto now = chrono::steady_clock::now();
        auto time_left = chrono::duration_cast<chrono::seconds>(end_time - now).count();
        if (time_left <= 0) {
            timeUp = true;
            cout << "\nH?t gi?! Trò choi k?t thúc!" << endl;
            break;
        }

        this_thread::sleep_for(chrono::seconds(1));
    }
}

char getArrowKey() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    cin.get(ch);
    if (ch == '\033') { // ESC sequence
        cin.get(ch);    // skip '['
        cin.get(ch);    // get arrow key
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void showInstructions() {
    cout << "\nHU?NG D?N TRÒ CHOI 2048:" << endl;
    cout << "- M?c tiêu: Di chuy?n và h?p nh?t các ô d? d?t du?c s? 2048." << endl;
    cout << "- S? d?ng các phím mui tên d? di?u khi?n:" << endl;
    cout << "  + ?: Di chuy?n lên." << endl;
    cout << "  + ?: Di chuy?n xu?ng." << endl;
    cout << "  + ?: Di chuy?n trái." << endl;
    cout << "  + ?: Di chuy?n ph?i." << endl;
    cout << "- Trò choi kêt thúc khi không còn nu?c di h?p l? ho?c h?t th?i gian." << endl;
    cout << "--------------------------------------" << endl;
    cout << "\nNh?n phím b?t k? d? quay l?i menu!" << endl;
    cin.get(); // Ð?c ký t? phím sau khi hu?ng d?n
    cin.get(); // Ð?c ký t? phím t? ngu?i dùng
}

void playGame() {
    while (true) {
        initializeBoard();
        start_time = chrono::steady_clock::now(); // Ð?t l?i th?i gian b?t d?u

        bool hasWon = false;
        bool hasLost = false;

        while (true) {
            cout << "\033[H\033[J"; // Xóa màn hình tru?c khi in l?i b?ng
            printBoard(); // Hi?n th? b?ng

            // Ki?m tra th?ng/thua
            if (checkWin()) {
                hasWon = true;
                break; // K?t thúc vòng l?p chính n?u th?ng
            }

            if (checkGameOver()) {
                hasLost = true;
                break; // K?t thúc vòng l?p chính n?u thua
            }

            // Ki?m tra th?i gian còn l?i
            auto now = chrono::steady_clock::now();
            auto elapsed_time = chrono::duration_cast<chrono::seconds>(now - start_time).count();
            if (elapsed_time >= 60) { // 60 giây
                timeUp = true;
                break; // K?t thúc vòng l?p chính n?u h?t th?i gian
            }

            // Nh?p và x? lý nu?c di
            cout << "\n(Nh?n 'n' d? thoát ngay l?p t?c)\n";
            char move = getArrowKey();
            if (move == 'n' || move == 'N') { // Ki?m tra n?u ngu?i choi ch?n thoát
                cout << "\nB?n dã ch?n thoát trò choi.\n";
                return; // Thoát kh?i toàn b? trò choi
            }

            switch (move) {
                case 'A': moveUp(); break;    // Arrow Up
                case 'B': moveDown(); break;  // Arrow Down
                case 'C': moveRight(); break; // Arrow Right
                case 'D': moveLeft(); break;  // Arrow Left
                default: continue; // B? qua khi nh?p không h?p l?
            }
            addNewNumber();
        }

        // Hi?n th? b?ng cu?i cùng và k?t qu?
        cout << "\033[H\033[J"; // Xóa màn hình
        printBoard();
        if (hasWon) {
            cout << "\n Chúc m?ng! B?n dã d?t du?c 2048 và chi?n th?ng trò choi! " << endl;
        } else if (hasLost) {
            cout << "\n Game Over! Không còn nu?c di h?p l?. " << endl;
        } else if (timeUp) {
            cout << "\n H?t gi?! Trò choi k?t thúc. "<<endl;
        }

        // H?i ngu?i choi có mu?n choi l?i không
        cout << "\nB?n mu?n choi l?i không? (y/n): ";
        char playAgain;
        cin >> playAgain;
        if (playAgain != 'y' && playAgain != 'Y') {
            break; // Thoát trò choi n?u ngu?i choi không mu?n choi l?i
        }
    }
}


void showMenu() {
    while (true) {
        cout << "\033[H\033[J"; // Xóa màn hình tru?c khi hi?n th? menu
        cout << "\n--- TRÒ CHOI 2048 ---" << endl;
        cout << "1. B?t d?u choi" << endl;
        cout << "2. Xem hu?ng d?n" << endl;
        cout << "3. Thoát trò choi" << endl;
        cout << "Nh?p l?a ch?n c?a b?n (1/2/3): ";
        int choice;
        cin >> choice;


        switch (choice) {
            case 1: playGame(); break;
            case 2: showInstructions(); break;
            case 3: return;
            default: cout << "L?a ch?n không h?p l?! Th? l?i." << endl;
        }
    }
}

int main() {
    srand(time(0)); // Kh?i t?o random seed
    showMenu();  // Hi?n th? menu trò choi
    return 0;
}
