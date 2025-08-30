
/*
IPL-like Tournament System - Simplified Version
- 4 teams, each team has 5 players
- Match: 2 overs, 2 wickets per team
- User input for team creation and match setup
- Ball-by-ball commentary and match summary
- Persistent player statistics tracking
*/

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <memory>
#include <iomanip>
#include <fstream>
#include <random>
#include <chrono>
#include <random>
#include <chrono>

using namespace std;

// Forward declarations
class Player;
class Team;
class Match;
class Tournament;

// Enum for player types
enum class PlayerType {
    BATSMAN,
    BOWLER,
    ALLROUNDER
};

// Enum for match results
enum class MatchResult {
    WIN,
    LOSS,
    TIE,
    NO_RESULT
};

// Enum for ball outcomes
enum class BallOutcome {
    DOT_BALL,
    SINGLE,
    DOUBLE,
    TRIPLE,
    FOUR,
    SIX,
    WICKET
};

// Base Player class
class Player {
protected:
    string name;
    int age;
    PlayerType type;
    int totalCredits;
    int matchCredits;
    
    // Persistent statistics
    int totalRunsScored;
    int totalBallsFaced;
    int totalWicketsTaken;
    int totalBallsBowled;
    int totalRunsConceded;
    
public:
    Player(const string& n, int a, PlayerType t) : name(n), age(a), type(t), 
        totalCredits(0), matchCredits(0), totalRunsScored(0), totalBallsFaced(0),
        totalWicketsTaken(0), totalBallsBowled(0), totalRunsConceded(0) {}
    
    virtual ~Player() = default;
    
    // Getters
    string getName() const { return name; }
    PlayerType getType() const { return type; }
    int getTotalCredits() const { return totalCredits; }
    int getMatchCredits() const { return matchCredits; }
    
    // Statistics getters
    int getTotalRunsScored() const { return totalRunsScored; }
    int getTotalBallsFaced() const { return totalBallsFaced; }
    int getTotalWicketsTaken() const { return totalWicketsTaken; }
    int getTotalBallsBowled() const { return totalBallsBowled; }
    int getTotalRunsConceded() const { return totalRunsConceded; }
    
    // Virtual methods for different player types
    virtual void updateCredits(int runs, int wickets) = 0;
    virtual void resetMatchCredits() { matchCredits = 0; }
    
    // Utility methods
    bool isBatsman() const { return type == PlayerType::BATSMAN || type == PlayerType::ALLROUNDER; }
    bool isBowler() const { return type == PlayerType::BOWLER || type == PlayerType::ALLROUNDER; }
    
    // Statistics update methods
    void addToTotalRuns(int runs) { totalRunsScored += runs; }
    void addToTotalBallsFaced() { totalBallsFaced++; }
    void addToTotalWickets(int wickets) { totalWicketsTaken += wickets; }
    void addToTotalBallsBowled() { totalBallsBowled++; }
    void addToTotalRunsConceded(int runs) { totalRunsConceded += runs; }
    
    // Virtual methods for derived classes
    virtual void addWicket() { addToTotalWickets(1); }
    virtual void addBall() { addToTotalBallsBowled(); }
    virtual void addRuns(int runs) { addToTotalRuns(runs); }
};

// Batsman class
class Batsman : public Player {
private:
    int runsScored;
    int ballsFaced;
    int fours;
    int sixes;
    
public:
    Batsman(const string& name, int age) : Player(name, age, PlayerType::BATSMAN),
        runsScored(0), ballsFaced(0), fours(0), sixes(0) {}
    
    void updateCredits(int runs, int wickets) override {
        matchCredits += runs / 20;  // 20 runs = 1 credit
        totalCredits += runs / 20;
    }
    
    void addRuns(int runs) {
        runsScored += runs;
        addToTotalRuns(runs);
        updateCredits(runs, 0);
        
        if (runs == 4) fours++;
        else if (runs == 6) sixes++;
    }
    
    void addBall() { 
        ballsFaced++; 
        addToTotalBallsFaced();
    }
    
    void addBoundary(bool isSix) {
        if (isSix) sixes++;
        else fours++;
    }
    
    // Getters
    int getRunsScored() const { return runsScored; }
    int getBallsFaced() const { return ballsFaced; }
    double getStrikeRate() const {
        return ballsFaced > 0 ? (double)runsScored * 100 / ballsFaced : 0.0;
    }
    
    void resetMatchStats() {
        runsScored = 0;
        ballsFaced = 0;
        fours = 0;
        sixes = 0;
        resetMatchCredits();
    }
};

// Bowler class
class Bowler : public Player {
private:
    int wicketsTaken;
    int runsConceded;
    int ballsBowled;
    int maidens;
    
public:
    Bowler(const string& name, int age) : Player(name, age, PlayerType::BOWLER),
        wicketsTaken(0), runsConceded(0), ballsBowled(0), maidens(0) {}
    
    void updateCredits(int runs, int wickets) override {
        matchCredits += wickets;  // 1 wicket = 1 credit
        totalCredits += wickets;
    }
    
    void addWicket() { 
        wicketsTaken++; 
        addToTotalWickets(1);
        updateCredits(0, 1);
    }
    
    void addRuns(int runs) { 
        runsConceded += runs; 
        addToTotalRunsConceded(runs);
    }
    
    void addBall() { 
        ballsBowled++; 
        addToTotalBallsBowled();
    }
    
    void addMaiden() { maidens++; }
    
    // Getters
    int getWicketsTaken() const { return wicketsTaken; }
    int getRunsConceded() const { return runsConceded; }
    double getEconomyRate() const {
        return ballsBowled > 0 ? (double)runsConceded * 6 / ballsBowled : 0.0;
    }
    double getAverage() const {
        return wicketsTaken > 0 ? (double)runsConceded / wicketsTaken : 0.0;
    }
    
    void resetMatchStats() {
        wicketsTaken = 0;
        runsConceded = 0;
        ballsBowled = 0;
        maidens = 0;
        resetMatchCredits();
    }
};

// AllRounder class
class AllRounder : public Player {
private:
    Batsman battingStats;
    Bowler bowlingStats;
    
public:
    AllRounder(const string& name, int age) : Player(name, age, PlayerType::ALLROUNDER),
        battingStats(name, age), bowlingStats(name, age) {}
    
    void updateCredits(int runs, int wickets) override {
        matchCredits += runs / 20 + wickets;  // Both batting and bowling credits
        totalCredits += runs / 20 + wickets;
    }
    
    // Delegate methods
    void addBattingRuns(int runs) { battingStats.addRuns(runs); }
    void addBowlingWicket() { bowlingStats.addWicket(); }
    void addBowlingRuns(int runs) { bowlingStats.addRuns(runs); }
    
    void resetMatchStats() {
        battingStats.resetMatchStats();
        bowlingStats.resetMatchStats();
        resetMatchCredits();
    }
};

// Team class
class Team {
private:
    string name;
    string city;
    vector<shared_ptr<Player>> roster;  // 5 players
    vector<shared_ptr<Player>> playing5;  // 5 players
    int points;
    int matchesPlayed;
    int matchesWon;
    int matchesLost;
    int matchesTied;
    
public:
    Team(const string& n, const string& c) : name(n), city(c), points(0), 
        matchesPlayed(0), matchesWon(0), matchesLost(0), matchesTied(0) {}
    
    // Team management
    void addPlayer(shared_ptr<Player> player) {
        roster.push_back(player);
    }
    
    void selectPlaying5() {
        playing5 = roster;  // All 5 players play
    }
    
    bool validatePlaying5() const {
        int bowlers = 0, batsmen = 0;
        for (const auto& player : playing5) {
            if (player->isBowler()) bowlers++;
            if (player->isBatsman()) batsmen++;
        }
        return bowlers >= 2 && batsmen >= 2;
    }
    
    // Match results
    void addPoints(int pts) { points += pts; }
    
    void updateMatchResult(MatchResult result) {
        matchesPlayed++;
        switch (result) {
            case MatchResult::WIN: matchesWon++; break;
            case MatchResult::LOSS: matchesLost++; break;
            case MatchResult::TIE: matchesTied++; break;
            default: break;
        }
    }
    
    // Getters
    string getName() const { return name; }
    vector<shared_ptr<Player>> getPlaying5() const { return playing5; }
    int getPoints() const { return points; }
    double getWinPercentage() const {
        return matchesPlayed > 0 ? (double)matchesWon * 100 / matchesPlayed : 0.0;
    }
    
    // Player search
    shared_ptr<Player> findPlayer(const string& playerName) const {
        for (const auto& player : playing5) {
            if (player->getName() == playerName) return player;
        }
        return nullptr;
    }
};

// Innings class to manage one team's batting
class Innings {
private:
    Team* battingTeam;
    Team* bowlingTeam;
    vector<shared_ptr<Player>> battingOrder;
    vector<shared_ptr<Player>> bowlingOrder;
    
    int currentBatsman1;
    int currentBatsman2;
    int currentBowler;
    int previousBowler;
    
    int totalRuns;
    int totalWickets;
    int totalOvers;
    int totalBalls;
    int currentOverBalls;
    
    map<shared_ptr<Player>, int> playerRuns;
    map<shared_ptr<Player>, int> playerWickets;
    
    // Random ball outcome vector
    vector<int> ballOutcomes = {0, 1, 2, 3, 4, 5, 6};  // 5 = wicket
    
public:
    Innings(Team* batting, Team* bowling) : battingTeam(batting), bowlingTeam(bowling),
        currentBatsman1(0), currentBatsman2(1), currentBowler(0), previousBowler(-1),
        totalRuns(0), totalWickets(0), totalOvers(0), totalBalls(0), currentOverBalls(0) {
        
        battingOrder = batting->getPlaying5();
        bowlingOrder = bowling->getPlaying5();
        
        // Initialize player stats
        for (auto& player : battingOrder) {
            playerRuns[player] = 0;
        }
        for (auto& player : bowlingOrder) {
            playerWickets[player] = 0;
        }
    }
    
    // Setup methods
    void setBatsmen(const string& striker, const string& nonStriker) {
        for (int i = 0; i < battingOrder.size(); i++) {
            if (battingOrder[i]->getName() == striker) currentBatsman1 = i;
            if (battingOrder[i]->getName() == nonStriker) currentBatsman2 = i;
        }
    }
    
    void setBowler(const string& bowlerName) {
        for (int i = 0; i < bowlingOrder.size(); i++) {
            if (bowlingOrder[i]->getName() == bowlerName) currentBowler = i;
        }
    }
    
    // Game logic
    void playBall() {
        if (isInningsComplete()) return;
        
        // Random ball outcome
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, ballOutcomes.size() - 1);
        int outcome = ballOutcomes[dis(gen)];
        
        // Update statistics based on outcome
        if (outcome == 5) {  // Wicket
            totalWickets++;
            playerWickets[bowlingOrder[currentBowler]]++;
            bowlingOrder[currentBowler]->addWicket();
            bowlingOrder[currentBowler]->addBall();
            
            // Change batsman
            changeBatsman();
            
            printCommentary(totalBalls + 1, 0, true);
        } else {  // Runs
            totalRuns += outcome;
            playerRuns[battingOrder[currentBatsman1]] += outcome;
            battingOrder[currentBatsman1]->addRuns(outcome);
            battingOrder[currentBatsman1]->addBall();
            
            // Change strike on odd runs
            if (outcome % 2 == 1) {
                changeStrike();
            }
            
            printCommentary(totalBalls + 1, outcome, false);
        }
        
        // Update ball count
        totalBalls++;
        currentOverBalls++;
        bowlingOrder[currentBowler]->addBall();
        
        // Change bowler every 6 balls
        if (currentOverBalls == 6) {
            changeBowler();
            currentOverBalls = 0;
            totalOvers++;
        }
    }
    
    void changeStrike() {
        swap(currentBatsman1, currentBatsman2);
    }
    
    void changeBatsman() {
        int nextBatsman = max(currentBatsman1, currentBatsman2) + 1;
        if (nextBatsman < battingOrder.size()) {
            if (currentBatsman1 > currentBatsman2) {
                currentBatsman1 = nextBatsman;
            } else {
                currentBatsman2 = nextBatsman;
            }
        }
    }
    
    void changeBowler() {
        previousBowler = currentBowler;
        do {
            currentBowler = (currentBowler + 1) % bowlingOrder.size();
        } while (currentBowler == previousBowler);
    }
    
    bool isInningsComplete() const {
        return totalWickets >= 2 || totalOvers >= 2;
    }
    
    // Getters
    int getTotalRuns() const { return totalRuns; }
    int getTotalWickets() const { return totalWickets; }
    
    shared_ptr<Player> getPlayerOfInnings() const {
        shared_ptr<Player> bestPlayer = nullptr;
        int maxCredits = -1;
        
        for (const auto& player : battingOrder) {
            if (player->getMatchCredits() > maxCredits) {
                maxCredits = player->getMatchCredits();
                bestPlayer = player;
            }
        }
        
        for (const auto& player : bowlingOrder) {
            if (player->getMatchCredits() > maxCredits) {
                maxCredits = player->getMatchCredits();
                bestPlayer = player;
            }
        }
        
        return bestPlayer;
    }
    
    // Commentary
    void printCommentary(int ballNumber, int runs, bool isWicket) {
        string striker = battingOrder[currentBatsman1]->getName();
        string bowler = bowlingOrder[currentBowler]->getName();
        
        cout << "Ball " << ballNumber << ": ";
        
        if (isWicket) {
            cout << "WICKET! " << striker << " is out! Bowled by " << bowler << endl;
        } else if (runs == 0) {
            cout << "Dot ball. " << striker << " defends" << endl;
        } else if (runs == 1) {
            cout << "Single. " << striker << " takes a quick run" << endl;
        } else if (runs == 2) {
            cout << "Two runs. " << striker << " pushes for a couple" << endl;
        } else if (runs == 3) {
            cout << "Three runs. " << striker << " runs hard for three" << endl;
        } else if (runs == 4) {
            cout << "FOUR! " << striker << " hits a boundary!" << endl;
        } else if (runs == 6) {
            cout << "SIX! " << striker << " hits it out of the park!" << endl;
        }
        
        cout << "Score: " << totalRuns << "/" << totalWickets << " (" 
             << totalOvers << "." << currentOverBalls << ")" << endl << endl;
    }
};

// Match class
class Match {
private:
    Team* team1;
    Team* team2;
    unique_ptr<Innings> innings1;
    unique_ptr<Innings> innings2;
    
    MatchResult result;
    shared_ptr<Player> playerOfMatch;
    string venue;
    string date;
    
public:
    Match(Team* t1, Team* t2, const string& v, const string& d) : 
        team1(t1), team2(t2), venue(v), date(d) {
        innings1 = make_unique<Innings>(team1, team2);
        innings2 = make_unique<Innings>(team2, team1);
    }
    
    // Setup methods
    void setupInnings() {
        string striker, nonStriker, bowler;
        
        cout << "\n=== Setting up " << team1->getName() << " innings ===" << endl;
        cout << "Enter striker name: ";
        cin >> striker;
        cout << "Enter non-striker name: ";
        cin >> nonStriker;
        cout << "Enter bowler name: ";
        cin >> bowler;
        
        innings1->setBatsmen(striker, nonStriker);
        innings1->setBowler(bowler);
        
        cout << "\n=== Setting up " << team2->getName() << " innings ===" << endl;
        cout << "Enter striker name: ";
        cin >> striker;
        cout << "Enter non-striker name: ";
        cin >> nonStriker;
        cout << "Enter bowler name: ";
        cin >> bowler;
        
        innings2->setBatsmen(striker, nonStriker);
        innings2->setBowler(bowler);
    }
    
    // Match execution
    void playMatch() {
        cout << "\n=== " << team1->getName() << " vs " << team2->getName() << " ===" << endl;
        cout << "Venue: " << venue << " | Date: " << date << endl << endl;
        
        // First innings
        cout << "=== FIRST INNINGS: " << team1->getName() << " batting ===" << endl;
        while (!innings1->isInningsComplete()) {
            innings1->playBall();
        }
        
        cout << "First innings complete! " << team1->getName() << " scored " 
             << innings1->getTotalRuns() << "/" << innings1->getTotalWickets() << endl << endl;
        
        // Second innings
        cout << "=== SECOND INNINGS: " << team2->getName() << " batting ===" << endl;
        while (!innings2->isInningsComplete()) {
            innings2->playBall();
        }
        
        cout << "Second innings complete! " << team2->getName() << " scored " 
             << innings2->getTotalRuns() << "/" << innings2->getTotalWickets() << endl << endl;
        
        determineResult();
        playerOfMatch = calculatePlayerOfMatch();
        printMatchSummary();
    }
    
    void determineResult() {
        int score1 = innings1->getTotalRuns();
        int score2 = innings2->getTotalRuns();
        
        if (score1 > score2) {
            result = MatchResult::WIN;
            team1->updateMatchResult(MatchResult::WIN);
            team2->updateMatchResult(MatchResult::LOSS);
            team1->addPoints(2);
        } else if (score2 > score1) {
            result = MatchResult::LOSS;
            team1->updateMatchResult(MatchResult::LOSS);
            team2->updateMatchResult(MatchResult::WIN);
            team2->addPoints(2);
        } else {
            result = MatchResult::TIE;
            team1->updateMatchResult(MatchResult::TIE);
            team2->updateMatchResult(MatchResult::TIE);
            team1->addPoints(1);
            team2->addPoints(1);
        }
    }
    
    shared_ptr<Player> calculatePlayerOfMatch() {
        auto player1 = innings1->getPlayerOfInnings();
        auto player2 = innings2->getPlayerOfInnings();
        
        if (player1->getMatchCredits() > player2->getMatchCredits()) {
            return player1;
        } else {
            return player2;
        }
    }
    
    // Getters
    MatchResult getResult() const { return result; }
    shared_ptr<Player> getPlayerOfMatch() const { return playerOfMatch; }
    
    Team* getWinner() const {
        if (result == MatchResult::WIN) return team1;
        else if (result == MatchResult::LOSS) return team2;
        return nullptr;  // Tie or no result
    }
    
    // Summary
    void printMatchSummary() {
        cout << "\n=== MATCH SUMMARY ===" << endl;
        cout << team1->getName() << ": " << innings1->getTotalRuns() << "/" << innings1->getTotalWickets() << endl;
        cout << team2->getName() << ": " << innings2->getTotalRuns() << "/" << innings2->getTotalWickets() << endl;
        
        if (result == MatchResult::WIN) {
            cout << "Result: " << team1->getName() << " won!" << endl;
        } else if (result == MatchResult::LOSS) {
            cout << "Result: " << team2->getName() << " won!" << endl;
        } else {
            cout << "Result: Match tied!" << endl;
        }
        
        cout << "Player of the Match: " << playerOfMatch->getName() << endl;
        cout << "=========================================" << endl << endl;
    }
};

// Tournament class
class Tournament {
private:
    string name;
    vector<shared_ptr<Team>> teams;
    vector<unique_ptr<Match>> matches;
    vector<shared_ptr<Player>> allPlayers;
    
    int currentRound;
    bool isCompleted;
    
public:
    Tournament(const string& n) : name(n), currentRound(0), isCompleted(false) {}
    
    // Tournament management
    void addTeam(shared_ptr<Team> team) {
        teams.push_back(team);
    }
    
    void generateFixtures() {
        // Round-robin: each team plays every other team
        for (int i = 0; i < teams.size(); i++) {
            for (int j = i + 1; j < teams.size(); j++) {
                auto match = make_unique<Match>(teams[i].get(), teams[j].get(), "Home Ground", "Today");
                matches.push_back(move(match));
            }
        }
    }
    
    void playRound() {
        if (currentRound < matches.size()) {
            cout << "\n=== ROUND " << (currentRound + 1) << " ===" << endl;
            matches[currentRound]->setupInnings();
            matches[currentRound]->playMatch();
            currentRound++;
        }
    }
    
    void playTournament() {
        cout << "\n=== TOURNAMENT BEGINS ===" << endl;
        for (int i = 0; i < matches.size(); i++) {
            playRound();
        }
        isCompleted = true;
    }
    
    // User input methods
    void createTeams() {
        string teamNames[] = {"Mumbai Indians", "Chennai Super Kings", "Royal Challengers", "Kolkata Knight Riders"};
        string cities[] = {"Mumbai", "Chennai", "Bangalore", "Kolkata"};
        
        for (int i = 0; i < 4; i++) {
            auto team = make_shared<Team>(teamNames[i], cities[i]);
            addTeam(team);
        }
    }
    
    void createPlayers() {
        // Create players for each team
        for (auto& team : teams) {
            cout << "\nCreating players for " << team->getName() << ":" << endl;
            
            for (int i = 0; i < 5; i++) {
                string name, typeStr;
                int age, typeChoice;
                
                cout << "Player " << (i + 1) << " name: ";
                cin >> name;
                cout << "Player " << (i + 1) << " age: ";
                cin >> age;
                cout << "Player " << (i + 1) << " type (1-Batsman, 2-Bowler, 3-AllRounder): ";
                cin >> typeChoice;
                
                shared_ptr<Player> player;
                switch (typeChoice) {
                    case 1: player = make_shared<Batsman>(name, age); break;
                    case 2: player = make_shared<Bowler>(name, age); break;
                    case 3: player = make_shared<AllRounder>(name, age); break;
                    default: player = make_shared<Batsman>(name, age); break;
                }
                
                team->addPlayer(player);
                allPlayers.push_back(player);
            }
            
            team->selectPlaying5();
        }
    }
    
    // Statistics and results
    vector<shared_ptr<Team>> getPointsTable() const {
        auto sortedTeams = teams;
        sort(sortedTeams.begin(), sortedTeams.end(), 
             [](const auto& a, const auto& b) { return a->getPoints() > b->getPoints(); });
        return sortedTeams;
    }
    
    shared_ptr<Player> getPlayerOfTournament() const {
        shared_ptr<Player> bestPlayer = nullptr;
        int maxCredits = -1;
        
        for (const auto& player : allPlayers) {
            if (player->getTotalCredits() > maxCredits) {
                maxCredits = player->getTotalCredits();
                bestPlayer = player;
            }
        }
        
        return bestPlayer;
    }
    
    shared_ptr<Team> getChampion() const {
        auto pointsTable = getPointsTable();
        return pointsTable.empty() ? nullptr : pointsTable[0];
    }
    
    // Getters
    string getName() const { return name; }
    bool getIsCompleted() const { return isCompleted; }
    
    // Display methods
    void displayTeams() {
        cout << "\n=== TOURNAMENT TEAMS ===" << endl;
        for (const auto& team : teams) {
            cout << team->getName() << " (" << team->getPlaying5().size() << " players)" << endl;
        }
        cout << "=========================" << endl;
    }
    
    void displayPlayerStats() {
        cout << "\n=== FINAL PLAYER STATISTICS ===" << endl;
        cout << setw(20) << "Name" << setw(10) << "Runs" << setw(10) << "Balls" << setw(10) << "Wickets" << setw(10) << "Credits" << endl;
        cout << "------------------------------------------------------------" << endl;
        
        for (const auto& player : allPlayers) {
            cout << setw(20) << player->getName() 
                 << setw(10) << player->getTotalRunsScored()
                 << setw(10) << player->getTotalBallsFaced()
                 << setw(10) << player->getTotalWicketsTaken()
                 << setw(10) << player->getTotalCredits() << endl;
        }
        
        cout << "\n=== FINAL POINTS TABLE ===" << endl;
        auto pointsTable = getPointsTable();
        for (int i = 0; i < pointsTable.size(); i++) {
            cout << (i + 1) << ". " << setw(25) << pointsTable[i]->getName() 
                 << " - " << pointsTable[i]->getPoints() << " points" << endl;
        }
        
        auto champion = getChampion();
        auto playerOfTournament = getPlayerOfTournament();
        
        cout << "\n=== TOURNAMENT AWARDS ===" << endl;
        cout << "Champion: " << champion->getName() << endl;
        cout << "Player of the Tournament: " << playerOfTournament->getName() << endl;
    }
};

// Statistics and utility classes
class Statistics {
public:
    static shared_ptr<Player> getPlayerOfMatch(const vector<shared_ptr<Player>>& players) {
        shared_ptr<Player> bestPlayer = nullptr;
        int maxCredits = -1;
        
        for (const auto& player : players) {
            if (player->getMatchCredits() > maxCredits) {
                maxCredits = player->getMatchCredits();
                bestPlayer = player;
            }
        }
        
        return bestPlayer;
    }
    
    static shared_ptr<Player> getPlayerOfTournament(const vector<shared_ptr<Player>>& players) {
        shared_ptr<Player> bestPlayer = nullptr;
        int maxCredits = -1;
        
        for (const auto& player : players) {
            if (player->getTotalCredits() > maxCredits) {
                maxCredits = player->getTotalCredits();
                bestPlayer = player;
            }
        }
        
        return bestPlayer;
    }
    
    static vector<shared_ptr<Team>> sortTeamsByPoints(const vector<shared_ptr<Team>>& teams) {
        auto sortedTeams = teams;
        sort(sortedTeams.begin(), sortedTeams.end(), 
             [](const auto& a, const auto& b) { return a->getPoints() > b->getPoints(); });
        return sortedTeams;
    }
};

// Main function to demonstrate the system
int main() {
    cout << "=== IPL-like Tournament System (Simplified) ===" << endl;
    cout << "4 teams, 5 players each, 2 overs, 2 wickets" << endl << endl;
    
    // Create tournament
    Tournament tournament("IPL Mini Tournament");
    
    // User creates teams and players
    tournament.createTeams();
    tournament.createPlayers();
    
    // Display created teams
    tournament.displayTeams();
    
    // Generate and play matches
    tournament.generateFixtures();
    tournament.playTournament();
    
    // Display final results
    tournament.displayPlayerStats();
    
    cout << "\nTournament completed successfully!" << endl;
    return 0;
}