#include <bits/stdc++.h>

using namespace std;

// Used to calculate the run time
double calc_time(struct timespec start, struct timespec end) {
  double start_sec =
      (double)start.tv_sec * 1000000000.0 + (double)start.tv_nsec;
  double end_sec = (double)end.tv_sec * 1000000000.0 + (double)end.tv_nsec;

  if (end_sec < start_sec) {
    return 0;
  } else {
    return end_sec - start_sec;
  }
}

// 3b) For each point, use the calculated number of raindrops that will
// trickle to the lowest neighbor(s) to update the number of raindrops at
// each lowest neighbor, if applicable.
void calcTrickle(int i, int j, vector<vector<float>> &rain,
                 const vector<vector<int>> &elevation,
                 vector<vector<float>> &trickle,
                 vector<vector<vector<vector<int>>>> &neighborsToTrickle,
                 vector<vector<float>> &tempTrickle) {
  if (trickle[i][j] == 0 || neighborsToTrickle[i][j].size() == 0) {
    return;
  }
  vector<vector<int>> direct = {
      {0, -1}, {-1, 0}, {0, 1}, {1, 0}}; // Left, up, right, down

  float share = trickle[i][j] / neighborsToTrickle[i][j].size();
  rain[i][j] -= trickle[i][j];
  for (auto n : neighborsToTrickle[i][j]) {
    tempTrickle[i + direct[n[1]][0]][j + direct[n[1]][1]] += share;
  }
}

// 1) Receive a new raindrop (if it is still raining) for each point.
// 2) If there are raindrops on a point, absorb water into the point
// 3a) Calculate the number of raindrops that will next trickle to the
// lowest neighbor(s)
void rainAbsorbTrickle(vector<vector<float>> &rain,
                       vector<vector<float>> &absorb,
                       vector<vector<float>> &trickle, int &timeSteps,
                       const float &absRate, float &isDrain,
                       vector<vector<float>> &nextTrickle,
                       const vector<vector<int>> &elevation,
                       vector<vector<vector<vector<int>>>> &neighborsToTrickle,
                       vector<vector<float>> &tempTrickle) {
  for (int i = 0; i < rain.size(); ++i) {
    for (int j = 0; j < rain[0].size(); ++j) {
      // Add trickle from the previous step
      rain[i][j] += nextTrickle[i][j];
      // Reset the nextTrickle array
      nextTrickle[i][j] = 0;
      if (timeSteps > 0) {
        // 1) Receive a new raindrop (if it is still raining) for each point.
        ++rain[i][j];
      }
      if (rain[i][j] == 0) {
        continue;
      }
      // 2) If there are raindrops on a point, absorb water into the point
      if (rain[i][j] >= absRate) {
        rain[i][j] -= absRate;
        absorb[i][j] += absRate;
      } else if (rain[i][j] > 0) {
        absorb[i][j] += rain[i][j];
        rain[i][j] = 0;
        continue;
      }
      // 3a) Calculate the number of raindrops that will next trickle to the
      // lowest neighbor(s)
      trickle[i][j] = 0; // Reset the trickle array
      if (rain[i][j] >= 1) {
        trickle[i][j] = 1;
      } else if (rain[i][j] > 0) {
        trickle[i][j] = rain[i][j];
      }
      isDrain += trickle[i][j];
      // 3b) For each point, use the calculated number of raindrops that will
      // trickle to the lowest neighbor(s) to update the number of raindrops
      // at each lowest neighbor, if applicable.
      // calcTrickle(rain, elevation, trickle, neighborsToTrickle, nextTrickle);
      calcTrickle(i, j, rain, elevation, trickle, neighborsToTrickle,
                  tempTrickle);
    }
  }
}

// Sort and determin which neighbor(s) to trickle
vector<vector<int>> countNeighbor(int i, int j,
                                  const vector<vector<int>> &elevation) {
  int N = elevation.size();
  vector<vector<int>> neighToTrickle; // <elevation, number>, number: Left: 0,
                                      // up: 1,right: 2, down:3
  vector<vector<int>> allNeigh;       // All neighbors that have less elevation
  if (j - 1 >= 0 && elevation[i][j - 1] < elevation[i][j]) { // left
    allNeigh.push_back({elevation[i][j - 1], 0});
  }
  if (i - 1 >= 0 && elevation[i - 1][j] < elevation[i][j]) { // up
    allNeigh.push_back({elevation[i - 1][j], 1});
  }
  if (j + 1 < N && elevation[i][j + 1] < elevation[i][j]) { // right
    allNeigh.push_back({elevation[i][j + 1], 2});
  }
  if (i + 1 < N && elevation[i + 1][j] < elevation[i][j]) { // down
    allNeigh.push_back({elevation[i + 1][j], 3});
  }
  sort(allNeigh.begin(), allNeigh.end());
  if (allNeigh.size() == 1) {
    neighToTrickle.push_back(allNeigh[0]);
  }
  for (int i = 0; allNeigh.size() != 0 && i < allNeigh.size() - 1; ++i) {
    if (allNeigh[i][0] == allNeigh[i + 1][0] && i == allNeigh.size() - 2) {
      neighToTrickle.push_back(allNeigh[i]);
      neighToTrickle.push_back(allNeigh[i + 1]);
      break;
    } else if (allNeigh[i][0] == allNeigh[i + 1][0]) {
      neighToTrickle.push_back(allNeigh[i]);
    } else {
      neighToTrickle.push_back(allNeigh[i]);
      break;
    }
  }
  return neighToTrickle;
}

// Calculate the whole time steps needed to drain
int calcRain(const vector<vector<int>> &elevation,
             vector<vector<float>> &absorb, int timeSteps, float absRate,
             struct timespec &start_time, struct timespec &end_time) {
  int N = elevation.size();
  vector<vector<float>> rain(
      N, vector<float>(N, 0)); // Store the current rain on the ground
  vector<vector<float>> trickle(
      N, vector<float>(N, 0)); // Store the trickle of each step
  vector<vector<float>> nextTrickle(
      N,
      vector<float>(N, 0)); // Store the trickle result to be added next round
  vector<vector<float>> tempTrickle(
      N,
      vector<float>(N, 0)); // Store the trickle result to be added next round
  vector<vector<float>> resetTrickle(
      N, vector<float>(N, 0)); // Used for resetting the tempTrickle
  vector<vector<vector<vector<int>>>> neighborsToTrickle(
      N, vector<vector<vector<int>>>(N, vector<vector<int>>()));
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      vector<vector<int>> neighToTrickle = countNeighbor(i, j, elevation);
      neighborsToTrickle[i][j] = neighToTrickle;
    }
  }
  int wholeSteps = 0;

  float isDrain = 1;

  clock_gettime(CLOCK_MONOTONIC, &start_time);

  while (isDrain != 0) {
    isDrain = 0;

    // 1) Receive a new raindrop (if it is still raining) for each point.
    // 2) If there are raindrops on a point, absorb water into the point
    // 3a) Calculate the number of raindrops that will next trickle to the
    // lowest neighbor(s)
    rainAbsorbTrickle(rain, absorb, trickle, timeSteps, absRate, isDrain,
                      nextTrickle, elevation, neighborsToTrickle, tempTrickle);
    nextTrickle = tempTrickle;
    tempTrickle = resetTrickle;
    --timeSteps;
    ++wholeSteps;
  }

  clock_gettime(CLOCK_MONOTONIC, &end_time);

  return wholeSteps;
}
