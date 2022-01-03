#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <algorithm>

using namespace std;

using AdjacencyList = vector<int>;
using Graph = vector<AdjacencyList>;
using Matrix = vector<vector<int>>;

struct Instance {
  chrono::steady_clock::time_point initial_time;
  int num_films;
  vector<string> film_titles;
  int num_incompatibilities;
  Graph incompatibility_graph;  // films are nodes and incompatibilities are edges
  int num_cinemas;
  vector<string> cinema_names;
};

struct ScreeningData { // each film is assigned a screening (a day and a cinema)
  int day;
  string cinema;
};

struct Solution {
  double elapsed_time;
  int last_day;
  map<string, ScreeningData> screenings;
  bool optimal;
};
Instance read_instance(const char* input_filename) {
  Instance ins;

  ins.initial_time = chrono::steady_clock::now();

  ifstream input(input_filename);

  input >> ins.num_films;
  ins.film_titles.resize(ins.num_films);
  map<string, int> film_to_id; // used to build the incompatibily graph
  for (int id = 0; id < ins.num_films; ++id) {
    input >> ins.film_titles[id];
    film_to_id[ins.film_titles[id]] = id;
  }

  input >> ins.num_incompatibilities;
  ins.incompatibility_graph.resize(ins.num_films);
  for (int i = 0; i < ins.num_incompatibilities; ++i) {
    string film1, film2;
    input >> film1 >> film2;
    int id1 = film_to_id[film1], id2 = film_to_id[film2];
    ins.incompatibility_graph[id1].push_back(id2); // incompatibilities are bidirectional
    ins.incompatibility_graph[id2].push_back(id1);
  }

  input >> ins.num_cinemas;
  ins.cinema_names.resize(ins.num_cinemas);
  for (string& cinema : ins.cinema_names)
    input >> cinema;

  return ins;
}

void write_solution(const Solution& sol, const char* output_filename) {
  ofstream output(output_filename);
  output << sol.elapsed_time << endl;
  output << sol.last_day + 1 << endl;  // days are 0-indexed
  for (const auto& [film, screening_data] : sol.screenings)
    output << film << ' ' <<  screening_data.day + 1 << ' ' << screening_data.cinema << endl;
}

void assign_screenings(const Instance& ins, const vector<int>& festival, Solution& sol) {
  vector<int> used_cinemas(ins.num_films);
  for (int film = 0; film < ins.num_films; ++film) {
    int day = festival[film];
    string cinema = ins.cinema_names[used_cinemas[day]];  // cinemas are arbitrary
    sol.screenings[ins.film_titles[film]] = {day, cinema};
    ++used_cinemas[day];
  }
}

int ceil_division(int numerator, int denominator) {  // returns ceil(numerator / denominator)
  return numerator / denominator + (numerator % denominator != 0);
}

void compute_festival_recursive(const Instance& ins, const vector<int>& film_order,
                                int num_assigned_films, int last_day,
                                vector<int>& festival, vector<int>& used_cinemas, Matrix& incompatibilities,
                                Solution& sol, const char* output_filename) {
  if (num_assigned_films == ins.num_films) {
    if (last_day < sol.last_day) {
      sol.last_day = last_day;
      assign_screenings(ins, festival, sol);
      sol.elapsed_time = chrono::duration<double>(chrono::steady_clock::now() - ins.initial_time).count();
      write_solution(sol, output_filename);

      if (ceil_division(ins.num_films, ins.num_cinemas) == sol.last_day + 1) sol.optimal = true;
    }
  } else if (last_day < sol.last_day and not sol.optimal) {
    int film_to_assign = film_order[num_assigned_films];
    for (int day = 0; day < sol.last_day; ++day) {
      if (used_cinemas[day] < ins.num_cinemas and incompatibilities[day][film_to_assign] == 0) {
        festival[film_to_assign] = day;

        ++used_cinemas[day];
        for (const int& incompatible_film : ins.incompatibility_graph[film_to_assign])
          ++incompatibilities[day][incompatible_film];

        compute_festival_recursive(ins, film_order,
                                   num_assigned_films + 1, max(last_day, day),
                                   festival, used_cinemas, incompatibilities,
                                   sol, output_filename);

        --used_cinemas[day];
        for (const int& incompatible_film : ins.incompatibility_graph[film_to_assign])
          --incompatibilities[day][incompatible_film];
      }
    }
  }
}

// films are ordered by decreasing number of incompatibilities (greedy strategy)
vector<int> compute_film_order(const Instance& ins) {
  vector<pair<int, int>> film_order_temp(ins.num_films);  // auxiliary vector for sorting
  
  for (int film = 0; film < ins.num_films; ++film)
    // the number of incompatibilities of each film is negative to sort by decreasing order
    film_order_temp[film] = {-int(ins.incompatibility_graph[film].size()), film};  
  
  sort(film_order_temp.begin(), film_order_temp.end());
  
  vector<int> film_order(ins.num_films);  // once sorted, only the film ids are needed
  for (int film = 0; film < ins.num_films; ++film)
    film_order[film] = film_order_temp[film].second;

  return film_order;
}

void compute_festival(const Instance& ins, const char* output_filename) {
  Solution sol;
  sol.last_day = ins.num_films - 1;  // starts at its maximum value to be minimized
  sol.optimal = false;
  
  // each cell represents a film, and its value is the day when it is projected, starting at 0
  vector<int> festival(ins.num_films);

  // each cell represents a day, and its value is the number of used cinemas
  // the size is the number of films as the festival cannot have more days than films
  vector<int> used_cinemas(ins.num_films);
  
  // each cell (i, j) counts the number of incompatible films with film j that are projected in day i
  Matrix incompatibilities(ins.num_films, vector<int>(ins.num_films));
  
  // the order in which the films are assigned to be able to prune earlier
  vector<int> film_order = compute_film_order(ins);

  // recursive backtracking
  compute_festival_recursive(ins, film_order,
                             0, 0,
                             festival, used_cinemas, incompatibilities,
                             sol, output_filename);
}

int main(int argc, char** argv) {
  if (argc == 3) {
    Instance ins = read_instance(argv[1]);
    compute_festival(ins, argv[2]);
  } else cout << "Usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE" << endl;
}
