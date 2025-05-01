#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <algorithm>
#include <string>

const int WINDOW_SIZE = 600;
const int GRID_SIZE = 3;
const int CELL_SIZE = WINDOW_SIZE / GRID_SIZE;

enum Player { NONE, X, O };

struct User {
    std::string email;
    std::string password;
};

bool emailExists(const std::string& email) {
    std::ifstream file("users.txt");
    std::string line;
    while (getline(file, line)) {
        if (line.find(email + " ") == 0)
            return true;
    }
    return false;
}

bool authenticate(const std::string& email, const std::string& password) {
    std::ifstream file("users.txt");
    std::string line;
    while (getline(file, line)) {
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            std::string fileEmail = line.substr(0, spacePos);
            std::string filePass = line.substr(spacePos + 1);
            if (fileEmail == email && filePass == password)
                return true;
        }
    }
    return false;
}

void signup() {
    std::string email, password;
    std::cout << "Sign Up\nEmail: ";
    std::cin >> email;
    if (emailExists(email)) {
        std::cout << "Email already exists. Try signing in.\n";
        return;
    }
    std::cout << "Password: ";
    std::cin >> password;

    std::ofstream file("users.txt", std::ios::app);
    file << email << " " << password << "\n";
    std::cout << "Sign up successful. You can now log in.\n";
}

bool login() {
    std::string email, password;
    std::cout << "Sign In\nEmail: ";
    std::cin >> email;
    std::cout << "Password: ";
    std::cin >> password;
    if (authenticate(email, password)) {
        std::cout << "Login successful. Welcome, " << email << "!\n";
        return true;
    }
    else {
        std::cout << "Invalid credentials.\n";
        return false;
    }
}

struct Button {
    sf::RectangleShape shape;
    sf::Text text;

    bool isHovered(const sf::Vector2f& mousePos) const {
        return shape.getGlobalBounds().contains(mousePos);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        window.draw(text);
    }
};

bool checkWin(const std::vector<std::vector<Player>>& board, Player player, sf::VertexArray& strikeLine) {
    strikeLine.clear();
    for (int i = 0; i < GRID_SIZE; ++i) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player) {
            strikeLine.append(sf::Vertex(sf::Vector2f(0, i * CELL_SIZE + CELL_SIZE / 2), sf::Color::Green));
            strikeLine.append(sf::Vertex(sf::Vector2f(WINDOW_SIZE, i * CELL_SIZE + CELL_SIZE / 2), sf::Color::Green));
            return true;
        }
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player) {
            strikeLine.append(sf::Vertex(sf::Vector2f(i * CELL_SIZE + CELL_SIZE / 2, 0), sf::Color::Green));
            strikeLine.append(sf::Vertex(sf::Vector2f(i * CELL_SIZE + CELL_SIZE / 2, WINDOW_SIZE), sf::Color::Green));
            return true;
        }
    }
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player) {
        strikeLine.append(sf::Vertex(sf::Vector2f(0, 0), sf::Color::Green));
        strikeLine.append(sf::Vertex(sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE), sf::Color::Green));
        return true;
    }
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player) {
        strikeLine.append(sf::Vertex(sf::Vector2f(WINDOW_SIZE, 0), sf::Color::Green));
        strikeLine.append(sf::Vertex(sf::Vector2f(0, WINDOW_SIZE), sf::Color::Green));
        return true;
    }
    return false;
}

bool isBoardFull(const std::vector<std::vector<Player>>& board) {
    for (auto& row : board)
        for (Player p : row)
            if (p == NONE) return false;
    return true;
}

int evaluate(const std::vector<std::vector<Player>>& board) {
    sf::VertexArray dummy(sf::Lines);
    if (checkWin(board, O, dummy)) return 1;
    if (checkWin(board, X, dummy)) return -1;
    return 0;
}

int minimax(std::vector<std::vector<Player>>& board, bool isMax) {
    int score = evaluate(board);
    if (score == 1 || score == -1 || isBoardFull(board)) return score;
    if (isMax) {
        int best = std::numeric_limits<int>::min();
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                if (board[i][j] == NONE) {
                    board[i][j] = O;
                    best = std::max(best, minimax(board, false));
                    board[i][j] = NONE;
                }
        return best;
    }
    else {
        int best = std::numeric_limits<int>::max();
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                if (board[i][j] == NONE) {
                    board[i][j] = X;
                    best = std::min(best, minimax(board, true));
                    board[i][j] = NONE;
                }
        return best;
    }
}

sf::Vector2i findBestMove(std::vector<std::vector<Player>>& board) {
    int bestVal = std::numeric_limits<int>::min();
    sf::Vector2i bestMove = { -1, -1 };
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (board[i][j] == NONE) {
                board[i][j] = O;
                int moveVal = minimax(board, false);
                board[i][j] = NONE;
                if (moveVal > bestVal) {
                    bestMove = { j, i };
                    bestVal = moveVal;
                }
            }
    return bestMove;
}

void drawCenteredText(sf::RenderWindow& window, const std::string& str, sf::Font& font, unsigned int size, float y, sf::Color color = sf::Color::Black, bool bold = false) {
    sf::Text text(str, font, size);
    text.setFillColor(color);
    if (bold) text.setStyle(sf::Text::Bold);
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.width / 2, textRect.height / 2);
    text.setPosition(WINDOW_SIZE / 2, y);
    window.draw(text);
}

int main() {
    std::cout << "1. Sign Up\n2. Sign In\nChoose (1/2): ";
    int choice;
    std::cin >> choice;
    if (choice == 1) {
        signup();
        return 0;
    }
    else if (choice == 2) {
        if (!login()) return 0;
    }
    else {
        std::cout << "Invalid choice.\n";
        return 0;
    }

    // GAME STARTS
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Tic Tac Toe");
    sf::Font font;
    font.loadFromFile("arial.ttf");

    Button singlePlayerBtn, multiPlayerBtn, restartBtn, exitBtn;
    singlePlayerBtn.shape.setSize({ 200, 50 });
    singlePlayerBtn.shape.setFillColor(sf::Color(100, 250, 100));
    singlePlayerBtn.shape.setPosition(200, 250);
    singlePlayerBtn.text.setFont(font);
    singlePlayerBtn.text.setString("Single Player");
    singlePlayerBtn.text.setCharacterSize(20);
    singlePlayerBtn.text.setFillColor(sf::Color::Black);
    singlePlayerBtn.text.setPosition(220, 260);

    multiPlayerBtn.shape = singlePlayerBtn.shape;
    multiPlayerBtn.shape.setPosition(200, 320);
    multiPlayerBtn.text = singlePlayerBtn.text;
    multiPlayerBtn.text.setString("Multiplayer");
    multiPlayerBtn.text.setPosition(230, 330);

    restartBtn.shape.setSize({ 180, 40 });
    restartBtn.shape.setFillColor(sf::Color(150, 150, 250));
    restartBtn.shape.setPosition(210, 420);
    restartBtn.text.setFont(font);
    restartBtn.text.setString("Play Again");
    restartBtn.text.setCharacterSize(22);
    restartBtn.text.setStyle(sf::Text::Bold);
    restartBtn.text.setFillColor(sf::Color::Black);
    restartBtn.text.setPosition(225, 425);

    exitBtn.shape = restartBtn.shape;
    exitBtn.shape.setPosition(210, 470);
    exitBtn.text = restartBtn.text;
    exitBtn.text.setString("Exit");
    exitBtn.text.setPosition(260, 475);

    bool menu = true, gameOver = false, singlePlayer = false;
    Player currentPlayer = X;
    std::vector<std::vector<Player>> board(3, std::vector<Player>(3, NONE));
    sf::VertexArray strikeLine(sf::Lines);
    strikeLine.append(sf::Vertex(sf::Vector2f(), sf::Color::Green));
    strikeLine.append(sf::Vertex(sf::Vector2f(), sf::Color::Green));

    bool aiThinking = false;
    sf::Clock aiClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
            if (menu && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (singlePlayerBtn.isHovered(mousePos)) {
                    singlePlayer = true;
                    menu = false;
                }
                if (multiPlayerBtn.isHovered(mousePos)) {
                    singlePlayer = false;
                    menu = false;
                }
            }
            else if (gameOver && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (restartBtn.isHovered(mousePos)) {
                    board.assign(3, std::vector<Player>(3, NONE));
                    strikeLine.clear();
                    currentPlayer = X;
                    gameOver = false;
                    menu = true;
                }
                else if (exitBtn.isHovered(mousePos)) {
                    window.close();
                }
            }
            else if (!menu && !gameOver && !aiThinking && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int x = event.mouseButton.x / CELL_SIZE;
                int y = event.mouseButton.y / CELL_SIZE;
                if (board[y][x] == NONE) {
                    board[y][x] = currentPlayer;
                    if (checkWin(board, currentPlayer, strikeLine) || isBoardFull(board)) {
                        gameOver = true;
                    }
                    else {
                        currentPlayer = (currentPlayer == X) ? O : X;
                        if (singlePlayer && currentPlayer == O) {
                            aiThinking = true;
                            aiClock.restart();
                        }
                    }
                }
            }
        }

        if (aiThinking && aiClock.getElapsedTime().asMilliseconds() > 200) {
            sf::Vector2i aiMove = findBestMove(board);
            if (aiMove.x != -1) {
                board[aiMove.y][aiMove.x] = O;
                if (checkWin(board, O, strikeLine) || isBoardFull(board)) gameOver = true;
                else currentPlayer = X;
            }
            aiThinking = false;
        }

        window.clear(sf::Color::White);
        if (menu) {
            drawCenteredText(window, "TIC TAC TOE", font, 40, 80, sf::Color::Blue, true);
            drawCenteredText(window, "Do you want to play single or multiplayer?", font, 22, 140, sf::Color::Black, true);
            singlePlayerBtn.draw(window);
            multiPlayerBtn.draw(window);
        }
        else {
            for (int i = 0; i < GRID_SIZE; ++i)
                for (int j = 0; j < GRID_SIZE; ++j) {
                    sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 5, CELL_SIZE - 5));
                    cell.setPosition(j * CELL_SIZE + 2.5f, i * CELL_SIZE + 2.5f);
                    cell.setFillColor(sf::Color(230, 230, 255));
                    window.draw(cell);
                    if (board[i][j] != NONE) {
                        sf::Text text(board[i][j] == X ? "X" : "O", font, 100);
                        text.setFillColor(board[i][j] == X ? sf::Color::Red : sf::Color::Blue);
                        text.setStyle(sf::Text::Bold);
                        sf::FloatRect textRect = text.getLocalBounds();
                        text.setOrigin(textRect.width / 2, textRect.height / 2);
                        text.setPosition(j * CELL_SIZE + CELL_SIZE / 2, i * CELL_SIZE + CELL_SIZE / 2);
                        window.draw(text);
                    }
                }

            if (aiThinking) drawCenteredText(window, "AI is thinking...", font, 20, WINDOW_SIZE - 30, sf::Color::Black, true);
            if (strikeLine.getVertexCount() == 2) {
                sf::Vector2f p1 = strikeLine[0].position;
                sf::Vector2f p2 = strikeLine[1].position;
                sf::RectangleShape thickLine;
                thickLine.setFillColor(sf::Color::Green);
                thickLine.setSize({ static_cast<float>(sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2))), 20.f });
                thickLine.setPosition(p1);
                thickLine.setRotation(atan2(p2.y - p1.y, p2.x - p1.x) * 180 / 3.14159f);
                window.draw(thickLine);
            }

            if (gameOver) {
                std::string resultText = "";
                sf::Color boxColor = sf::Color::Red;
                if (checkWin(board, O, strikeLine)) resultText = "O wins!";
                else if (checkWin(board, X, strikeLine)) resultText = "X wins!";
                else {
                    resultText = "It's a draw!";
                    boxColor = sf::Color(128, 128, 128);
                }
                sf::RectangleShape resultBox;
                resultBox.setSize({ 360, 70 });
                resultBox.setFillColor(boxColor);
                resultBox.setOutlineThickness(5);
                resultBox.setOutlineColor(sf::Color::Black);
                resultBox.setPosition((WINDOW_SIZE - 360) / 2, 20);
                window.draw(resultBox);
                drawCenteredText(window, resultText, font, 36, 55, sf::Color::White, true);
                restartBtn.draw(window);
                exitBtn.draw(window);
            }
        }
        window.display();
    }
    return 0;
}