# Movie Recommendation System

This project implements a User-Based Collaborative Filtering (UBCF) movie recommendation system. The system predicts movie ratings for users based on their historical ratings and similar users' preferences.

### Features
- User-based collaborative filtering algorithm
- Rating prediction for movies
- Handles cold-start problems
- Weighted similarity calculations
- Confidence-based predictions

### Requirements
- C++11 or higher
- Standard C++ libraries

### How to Use
1. Make sure you have a `train.txt` file in the same directory as the executable
2. Compile the program:
```bash
g++ main.cpp -o movie_recommender
```
3. Run the program:
```bash
./movie_recommender
```
4. Enter a userID and movieID when prompted
5. The program will predict the rating for the given user-movie pair

### Input Format
The `train.txt` file should contain training data in the following format:
```
userID movieID rating
```
Each line represents a user's rating for a movie, where:
- userID: Integer identifier for the user
- movieID: Integer identifier for the movie
- rating: Double value between 1.0 and 5.0