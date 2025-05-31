#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <fstream>
using namespace std;


struct rating {
    int userID;
    int movieID;
    double rating;
};

void loadTrainData(vector<rating>& ratings){
    string line;

    ifstream trainFile("train.txt");
    trainFile.seekg(0, ios::beg);

    if(!trainFile.is_open()){
        cerr << "[ERROR] : File can not opened!" << endl;
        cout << "program ending..." << endl;
        exit(1);
    }

    while(getline(trainFile, line)){
    rating currRating;

    istringstream ss(line);
    ss >> currRating.userID >> currRating.movieID >> currRating.rating;

    ratings.push_back(currRating);
    }


    trainFile.close();
}


void buildUserRating(vector<rating>& trainData, unordered_map<int, unordered_map<int, double>>& usersData){
    for(const auto rating : trainData){
        usersData[rating.userID][rating.movieID] = rating.rating;
    }
}


double calculateUserAvarage(const unordered_map<int, double>& userRatings){
    
    if (userRatings.empty()) return 3.0;

    double sum{};
    int count{};

    for(const pair<int,double>& rating : userRatings){
        if( rating.second >= 1.0 && rating.second <= 5.0){
            sum += rating.second;
            count++;
        }
    }
    return count > 0 ? sum/count : 3.0;
}


double calculateMovieAvarage(int movieID, const unordered_map<int, unordered_map<int, double>>& usersRatings){

    double sum{};
    int count{};

    for(const pair<int, unordered_map<int, double>>& userRatingPair : usersRatings){

        const unordered_map<int, double>& userRating = userRatingPair.second;

        if(userRating.count(movieID)){
            sum += userRating.at(movieID);
            count++;
        }
    }

    return count > 0 ? sum/count : 3.0;
}


double calculateSim(const unordered_map<int, double>& ratings1, const unordered_map<int, double>& rating2, double avg1, double avg2){

    vector<pair<double, double>> commonRatings;

    for(const pair<int, double>& pair : ratings1){
        int movieID = pair.first;
        if (rating2.count(movieID)){
            commonRatings.push_back({(pair.second - avg1),(rating2.at(movieID) - avg2)});
        }
    }

    if(commonRatings.size() < 3) return 0.0;


    double numerator{}, sum1_squared{}, sum2_squared{};

    for(const auto& points : commonRatings){
        numerator += points.first * points.second;
        sum1_squared += points.first * points.first;
        sum2_squared += points.second * points.second;
    }

    if (sum1_squared == 0.0 || sum2_squared == 0.0) return 0.0;

    double similarity = numerator / (sqrt(sum1_squared) * sqrt(sum2_squared));

    double weight = min(1.0, commonRatings.size() / 5.0);
    similarity *= weight;
    
    return similarity;
}


double predictRatingUBCF(int userID, int movieID, const unordered_map<int, unordered_map<int, double>>& usersRatings){

    //if user have not evaluate any film, return movies avarage rating
    if(usersRatings.find(userID) == usersRatings.end()){
        return calculateMovieAvarage(movieID, usersRatings);
    }

    //calculate users avarage rating
    unordered_map<int,double> userRatings = usersRatings.at(userID);
    double userAvg = calculateUserAvarage(userRatings);
    //calculate movies avarge rating
    double movieAvg = calculateMovieAvarage(movieID, usersRatings);


    vector<pair<double, double>> userScores;

    //find similar users and get their movie rating.
    for(const pair<int, unordered_map<int, double>>& userPair : usersRatings){

        int otherUserID = userPair.first;
        unordered_map<int, double> otherRatings = userPair.second;

        if(otherUserID != userID && otherRatings.count(movieID)){

            double otherAvg = calculateUserAvarage(otherRatings);
            double similarity = calculateSim(userRatings, otherRatings, userAvg, otherAvg);

            if (similarity > 0.1){
                double normalizedRating = otherRatings.at(movieID) - otherAvg;
                userScores.push_back({similarity, normalizedRating});
            }
        }
    }


    if (userScores.empty()){
        return 0.6 * userAvg + 0.4 * movieAvg;
    }


    sort(userScores.begin(), userScores.end(), [](const pair<double, double>& a, const pair<double, double>& b){return a.first > b.first;});

    const int K = min(5, static_cast<int>(userScores.size()));
    
    double weightedSum = 0.0;
    double weightSum = 0.0;
    
    for (int i = 0; i < K; i++) {
        double weight = pow(userScores[i].first, 2);
        weightedSum += weight * userScores[i].second;
        weightSum += weight;
    }
    
    double prediction;
    if (weightSum > 0) {
        prediction = userAvg + (weightedSum / weightSum);
    } else {
        prediction = userAvg;
    }
    
    double confidence = min(1.0, weightSum / K);

    prediction = confidence * prediction + (1.0 - confidence) * (0.6 * userAvg + 0.4 * movieAvg);
    
    if (prediction < 1.0) prediction = 1.0;
    if (prediction > 5.0) prediction = 5.0;
    
    return prediction;
}


int main() {
    vector<rating> trainData;
    vector<pair<int, int>> testData;
    
    loadTrainData(trainData);

    unordered_map<int, unordered_map<int, double>> usersRatings;

    buildUserRating(trainData, usersRatings);

    bool exit = false;
    int userID{}, movieID{};
    cout << "Write '0' for ending program" << endl;

    while(!exit){
        cout << "Enter userID: ";
        cin >> userID;
        if(userID == 0) break;
        cout << "Enter movieID: ";
        cin >> movieID;
        if(movieID == 0) break;

        double predictedRatingUBCF = predictRatingUBCF(userID, movieID, usersRatings);
        cout << predictedRatingUBCF << endl;
    }

    return 0;
}