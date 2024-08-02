#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <chrono>
#include <fstream>
#include <termios.h>
#include <unistd.h>

const int BASE_WIDTH = 16;
const int BASE_HEIGHT = 16;

enum Cell {
    WALL,
    PATH,
    START,
    END,
    PLAYER,
    SOLUTION
};

struct Position {
    int x, y;
    Position() : x(0), y(0) {}
    Position(int x, int y) : x(x), y(y) {}
};

bool isValidMove(int x, int y, const std::vector<std::vector<Cell> >& maze) {
    if (x <= 0 || x >= maze[0].size() - 1 || y <= 0 || y >= maze.size() - 1) return false;
    return maze[y][x] == WALL;
}

std::vector<Position> getNeighbors(const Position& pos) {
    std::vector<Position> neighbors;
    neighbors.push_back(Position(pos.x + 2, pos.y));
    neighbors.push_back(Position(pos.x - 2, pos.y));
    neighbors.push_back(Position(pos.x, pos.y + 2));
    neighbors.push_back(Position(pos.x, pos.y - 2));
    return neighbors;
}

void recursiveBacktrack(std::vector<std::vector<Cell> >& maze, Position current) {
    std::vector<Position> neighbors = getNeighbors(current);
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(neighbors.begin(), neighbors.end(), g);

    for (const auto& next : neighbors) {
        int midX = (current.x + next.x) / 2;
        int midY = (current.y + next.y) / 2;
        if (isValidMove(next.x, next.y, maze) && maze[next.y][next.x] == WALL) {
            maze[midY][midX] = PATH;
            maze[next.y][next.x] = PATH;
            recursiveBacktrack(maze, next);
        }
    }
}

void generateMaze(std::vector<std::vector<Cell> >& maze, int level, int passageWidth) {
    int width = BASE_WIDTH;
    int height = BASE_HEIGHT;
    if (level == 2) {
        width = BASE_WIDTH + 10;
        height = BASE_HEIGHT + 10;
    } else if (level == 3) {
        width = BASE_WIDTH + 20;
        height = BASE_HEIGHT + 20;
    }

    maze = std::vector<std::vector<Cell> >(height, std::vector<Cell>(width, WALL));

    Position start(1, 1);
    Position end(width - 2, height - 2);
    maze[start.y][start.x] = START;
    maze[end.y][end.x] = END;

    recursiveBacktrack(maze, start);

    std::vector<Position> goalNeighbors;
    goalNeighbors.push_back(Position(end.x - 1, end.y));
    goalNeighbors.push_back(Position(end.x + 1, end.y));
    goalNeighbors.push_back(Position(end.x, end.y - 1));
    goalNeighbors.push_back(Position(end.x, end.y + 1));

    for (const auto& pos : goalNeighbors) {
        if (pos.x > 0 && pos.x < width - 1 && pos.y > 0 && pos.y < height - 1) {
            maze[pos.y][pos.x] = PATH;
        }
    }
}

void printMaze(const std::vector<std::vector<Cell> >& maze, const Position& player) {
    system("clear");
    for (auto row = maze.begin(); row != maze.end(); ++row) {
        for (auto cell = row->begin(); cell != row->end(); ++cell) {
            if (player.x == std::distance(row->begin(), cell) && player.y == std::distance(maze.begin(), row)) {
                std::cout << 'P'; // プレイヤーの位置
            } else {
                switch (*cell) {
                    case WALL: std::cout << '#'; break;
                    case PATH: std::cout << ' '; break;
                    case START: std::cout << 'S'; break;
                    case END: std::cout << 'E'; break;
                    case SOLUTION: std::cout << '*'; break;
                    default: std::cout << ' '; break;
                }
            }
        }
        std::cout << std::endl;
    }
}

char getInput() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void saveScore(float time) {
    std::ofstream scoreFile("scores.txt", std::ios::app);
    scoreFile << time << std::endl;
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    int level;
    std::cout << "Enter maze level (1, 2, 3): ";
    std::cin >> level;

    int passageWidth = 2;
    std::vector<std::vector<Cell> > maze;
    generateMaze(maze, level, passageWidth);

    Position player(1, 1);

    auto startTime = std::chrono::high_resolution_clock::now();

    while (true) {
        printMaze(maze, player);

        char input = getInput();
        int newX = player.x;
        int newY = player.y;

        switch (input) {
            case 'w': newY--; break;
            case 's': newY++; break;
            case 'a': newX--; break;
            case 'd': newX++; break;
            default: break;
        }

        if (newX > 0 && newX < maze[0].size() && newY > 0 && newY < maze.size() && maze[newY][newX] != WALL) {
            player.x = newX;
            player.y = newY;
        }

        if (maze[player.y][player.x] == END) {
            printMaze(maze, player);
            auto endTime = std::chrono::high_resolution_clock::now(); 
            std::chrono::duration<float> duration = endTime - startTime;
            std::cout << "Congratulations! You've reached the end of the maze!" << std::endl;
            std::cout << "Time: " << duration.count() << " seconds" << std::endl;
            saveScore(duration.count());
            break;
        }
    }

    return 0;
}
