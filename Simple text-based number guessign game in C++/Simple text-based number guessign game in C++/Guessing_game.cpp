#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <chrono>
#include <limits>
#include <string>
#include <sstream>
#include <cstdio>  // for remove()

using namespace std;

// Constants for maximum attempts per difficulty
const int MAX_ATTEMPTS_EASY = 15;
const int MAX_ATTEMPTS_MEDIUM = 10;
const int MAX_ATTEMPTS_HARD = 5;

// Global game statistics (for single-player mode)
int totalGamesPlayed = 0;
int totalAttemptsMade = 0;

// Helper function: Get a valid integer input from the user (using getline for robust input)
int getValidInt(const string& prompt, int minVal = numeric_limits<int>::min(), int maxVal = numeric_limits<int>::max()) {
    int value;
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        try {
            value = stoi(input);
            if (value < minVal || value > maxVal) {
                cout << "Please enter a number between " << minVal << " and " << maxVal << ".\n";
                continue;
            }
            return value;
        }
        catch (...) {
            cout << "Invalid input. Please enter a valid number.\n";
        }
    }
}

// Helper function: Get a valid string input (e.g., for player names)
string getValidString(const string& prompt) {
    string input;
    cout << prompt;
    getline(cin, input);
    return input;
}

// Check if a file exists
bool fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

// For single-player, we keep best score in files by difficulty.
// We'll use: "best_score_easy.txt", "best_score_medium.txt", "best_score_hard.txt"
string getBestScoreFilename(int difficulty) {
    if (difficulty == 1) return "best_score_easy.txt";
    else if (difficulty == 2) return "best_score_medium.txt";
    else return "best_score_hard.txt";
}

int loadBestScore(int difficulty) {
    int best_score = 0;
    string filename = getBestScoreFilename(difficulty);
    ifstream file(filename);
    if (file) {
        file >> best_score;
    }
    file.close();
    return best_score;
}

void saveBestScore(int difficulty, int score) {
    string filename = getBestScoreFilename(difficulty);
    ofstream file(filename);
    file << score;
    file.close();
}

// Leaderboard file for single-player (append best scores as achievements)
void saveToLeaderboard(int difficulty, int score) {
    // We'll record difficulty text as well.
    string diffStr;
    if (difficulty == 1) diffStr = "Easy";
    else if (difficulty == 2) diffStr = "Medium";
    else diffStr = "Hard";

    ofstream file("leaderboard.txt", ios::app);
    file << diffStr << " mode: " << score << " attempts" << endl;
    file.close();
}

// Display the leaderboard (top 5 entries)
void displayLeaderboard() {
    ifstream file("leaderboard.txt");
    string line;
    cout << "\nLeaderboard (Top 5 entries):\n";
    if (file) {
        int count = 0;
        while (getline(file, line) && count < 5) {
            cout << line << endl;
            count++;
        }
    }
    else {
        cout << "No leaderboard data available.\n";
    }
    file.close();
}

// Reset best score files (for all difficulties) and leaderboard
void resetBestScores() {
    ofstream file1("best_score_easy.txt");
    file1 << 0;
    file1.close();
    ofstream file2("best_score_medium.txt");
    file2 << 0;
    file2.close();
    ofstream file3("best_score_hard.txt");
    file3 << 0;
    file3.close();
    ofstream lb("leaderboard.txt");
    lb.close(); // Clear leaderboard
    cout << "Best scores and leaderboard have been reset!" << endl;
}

// Single-player game function
void playSinglePlayer() {
    cout << "\n--- Single-Player Mode ---\n";
    // Choose difficulty
    int difficulty = getValidInt("Select difficulty level (1 = Easy, 2 = Medium, 3 = Hard): ", 1, 3);

    // Set maximum attempts based on difficulty
    int max_attempts;
    if (difficulty == 1)
        max_attempts = MAX_ATTEMPTS_EASY;
    else if (difficulty == 2)
        max_attempts = MAX_ATTEMPTS_MEDIUM;
    else
        max_attempts = MAX_ATTEMPTS_HARD;

    // Ask if user wants to use a custom upper bound for the secret number
    int upperBound = 100;
    cout << "Default upper bound is 100. Do you want to set a custom upper bound? (y/n): ";
    string custom;
    getline(cin, custom);
    if (!custom.empty() && (custom[0] == 'y' || custom[0] == 'Y')) {
        upperBound = getValidInt("Enter the new upper bound (greater than 1): ", 2);
    }

    // Generate the secret number
    int number_to_guess = rand() % upperBound + 1;
    int attempts = 0;

    // Load current best score for this difficulty
    int best_score = loadBestScore(difficulty);
    if (best_score == 0)
        cout << "No best score yet. Try to beat the default score!" << endl;
    else
        cout << "Current best score for this difficulty: " << best_score << " attempts." << endl;

    cout << "\nGame started! You have " << max_attempts << " attempts." << endl;

    auto startTime = chrono::steady_clock::now();
    while (true) {
        int guess = getValidInt("Enter your guess: ");
        attempts++;

        // Provide a hint: range narrowing (after 3 attempts, reveal a 20% range)
        if (attempts == 3) {
            int lowHint = number_to_guess - upperBound / 10;
            int highHint = number_to_guess + upperBound / 10;
            if (lowHint < 1) lowHint = 1;
            if (highHint > upperBound) highHint = upperBound;
            cout << "Hint: The number is between " << lowHint << " and " << highHint << "." << endl;
        }

        if (guess > number_to_guess)
            cout << "Too high!" << endl;
        else if (guess < number_to_guess)
            cout << "Too low!" << endl;
        else {
            cout << "Congratulations! You guessed the correct number." << endl;
            cout << "Attempts: " << attempts << endl;
            auto endTime = chrono::steady_clock::now();
            auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime);
            cout << "Time taken: " << duration.count() << " seconds." << endl;

            // Achievement: guess in 3 attempts or less
            if (attempts <= 3)
                cout << "Achievement unlocked: Master Guesser!" << endl;

            // Update best score if needed
            if (best_score == 0 || attempts < best_score) {
                best_score = attempts;
                saveBestScore(difficulty, best_score);
                saveToLeaderboard(difficulty, best_score);
                cout << "New best score for this difficulty! (Saved)" << endl;
            }
            else {
                cout << "Your best score for this difficulty remains " << best_score << " attempts." << endl;
            }

            // Update global stats
            totalGamesPlayed++;
            totalAttemptsMade += attempts;
            break;
        }

        if (attempts >= max_attempts) {
            cout << "Sorry, you've exceeded the maximum attempts. The number was " << number_to_guess << "." << endl;
            totalGamesPlayed++;
            totalAttemptsMade += attempts;
            break;
        }
    }
}

// Multiplayer game function (two players take turns)
void playMultiplayer() {
    cout << "\n--- Multiplayer Mode ---\n";
    string player1 = getValidString("Enter name for Player 1: ");
    string player2 = getValidString("Enter name for Player 2: ");

    // Use a fixed upper bound for simplicity (could be customized)
    int upperBound = 100;
    cout << "Playing with an upper bound of " << upperBound << "." << endl;
    int number_to_guess = rand() % upperBound + 1;

    // Set maximum attempts (total for both players)
    int max_attempts = MAX_ATTEMPTS_MEDIUM;  // You can adjust this or ask players

    int attempts = 0;
    bool player1Turn = true;
    auto startTime = chrono::steady_clock::now();

    while (attempts < max_attempts) {
        string currentPlayer = player1Turn ? player1 : player2;
        cout << "\n" << currentPlayer << "'s turn." << endl;
        int guess = getValidInt("Enter your guess: ");
        attempts++;

        if (guess > number_to_guess)
            cout << "Too high!" << endl;
        else if (guess < number_to_guess)
            cout << "Too low!" << endl;
        else {
            cout << "Congratulations, " << currentPlayer << "! You guessed the correct number." << endl;
            cout << "Total attempts: " << attempts << endl;
            auto endTime = chrono::steady_clock::now();
            auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime);
            cout << "Time taken: " << duration.count() << " seconds." << endl;
            break;
        }

        player1Turn = !player1Turn;

        if (attempts >= max_attempts) {
            cout << "\nMaximum attempts reached. The correct number was " << number_to_guess << "." << endl;
            break;
        }
    }
}

// Function to display game statistics for single-player mode
void displayStats() {
    if (totalGamesPlayed == 0) {
        cout << "\nNo games played yet." << endl;
    }
    else {
        double avgAttempts = static_cast<double>(totalAttemptsMade) / totalGamesPlayed;
        cout << "\n--- Game Statistics ---" << endl;
        cout << "Total games played: " << totalGamesPlayed << endl;
        cout << "Total attempts made: " << totalAttemptsMade << endl;
        cout << "Average attempts per game: " << avgAttempts << endl;
    }
}

int main() {
    srand(static_cast<unsigned>(time(0))); // Seed random number generator

    while (true) {
        cout << "\n----- Main Menu -----\n";
        cout << "1. Single-Player Mode\n";
        cout << "2. Multiplayer Mode\n";
        cout << "3. View Leaderboard\n";
        cout << "4. Reset Best Scores & Leaderboard\n";
        cout << "5. View Game Statistics (Single-Player)\n";
        cout << "6. Exit\n";

        int choice = getValidInt("Enter your choice (1-6): ", 1, 6);

        if (choice == 1)
            playSinglePlayer();
        else if (choice == 2)
            playMultiplayer();
        else if (choice == 3)
            displayLeaderboard();
        else if (choice == 4)
            resetBestScores();
        else if (choice == 5)
            displayStats();
        else if (choice == 6) {
            cout << "Exiting the game. Goodbye!" << endl;
            break;
        }
    }

    return 0;
}
