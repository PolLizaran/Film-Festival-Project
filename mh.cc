#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
//#include <math.h>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

using namespace std;
using MI = vector<vector<int>>; //matrix of integers
using ALI = vector<vector<int>>; //adjacency list of integers
using fd = pair<int, int>; //represents a film and the day of its projection

// GLOBAL VARIABLES
/* ----------------------------------------------------- */
clock_t start = clock();
double elapsed_time;

int num_films, num_preferences, num_rooms;
int shortest_festival = INT_MAX; //simulates infinite value

vector<string> billboard, cinema_rooms;
ALI Inc; // stores the Incompatibilities among films that can't be projected in the same day

map<string, int> filmindex; //stores the name of a film and its position in the "billboard" vector

string output_file;
/* ----------------------------------------------------- */

struct Score {
    int num_restr;
    int cardinal;
    double probability;
};

void read_films(ifstream& input)
{
    input >> num_films;
    billboard = vector<string>(num_films);
    for (int i = 0; i < num_films; ++i) {
        input >> billboard[i];
        filmindex.insert({ billboard[i], i });
    }
}

/*
Works on:
    Fills "Inc"
*/
void read_incompatibilities(ifstream& input)
{
    Inc = ALI(num_films);
    input >> num_preferences;
    for (uint i = 0; i < num_preferences; ++i) {
        string a, b;
        input >> a >> b;
        Inc[filmindex[a]].push_back(filmindex[b]);
        Inc[filmindex[b]].push_back(filmindex[a]);
    }
}

void read_cinema_rooms(ifstream& input)
{
    input >> num_rooms;
    cinema_rooms = vector<string>(num_rooms);
    for (uint i = 0; i < num_rooms; ++i)
        input >> cinema_rooms[i];
}

void read_data(ifstream& input)
{
    read_films(input);
    read_incompatibilities(input);
    read_cinema_rooms(input);
}

/*
Args:
    A, B: contain the index of a film and the amount of films incompatible

Returns:
    The film that is more restricted
*/
bool compare_by_restr(const pair<int, int>& A, const pair<int, int>& B)
{
    if (A.second == B.second)
        return A.first < B.first; //arbitary selection
    return A.second > B.second;
}

/*
Returns:
    A vector of pairs containing the index of a film and the number of films with which is forbidden
    to be projected in the same day. Its elements are in descending order.
*/
vector<pair<int, int>> order_films_by_restr(map<int, int>& different_cardinal, vector<Score>& films_scores)
{
    vector<pair<int, int>> films_by_restr(num_films);
    for (int i = 0; i < Inc.size(); ++i) {
        films_by_restr[i] = { i, Inc[i].size() };
        ++different_cardinal[Inc[i].size()];
    }
    sort(films_by_restr.begin(), films_by_restr.end(), compare_by_restr);

    films_scores.push_back({ films_by_restr[0].second, 1, 0.0 });
    for (int j = 1; j < num_films; ++j) {
        if (films_by_restr[j].second == films_scores.back().num_restr) {
            ++films_scores.back().cardinal;
        } else {
            films_scores.push_back({ films_by_restr[j].first, 1, 0.0 });
        }
    }

    int s = (films_scores.back().num_restr == 0 ? films_scores.size() - 1 : films_scores.size());
    double limit_block = (s > 5 ? (s / 5) : 1);
    double major_prob = (s > 5 ? (1 / (5 / 2.0)) : (1 / ((s + 1) / 2.0)));
    cout << major_prob << endl;
    //prob = 1 / ((s·(s+1)/2)/s) = 1 / (3/3 + 2/3 + 1/3) - no posem s + 1 perquè no considerem el bloc amb 0 restriccions
    double block = 5;
    for (int j = 0; j < s; ++j) {
        if (j > int(limit_block)) {
            --block;
            limit_block += s / 5;
        }
        films_scores[j].probability = major_prob * (block / 5);
    }

    return films_by_restr;
}

void print_projection(const vector<fd>& perm)
{
    ofstream output(output_file);
    output << setprecision(1) << fixed;
    elapsed_time = (clock() - start) / (double)CLOCKS_PER_SEC;

    output << elapsed_time << endl
           << shortest_festival << endl;
    cout << shortest_festival << endl;

    // looks across the vector "perm" to print those films that match with the day. Cinema rooms
    // are choosen arbitarily
    for (int day = 1; day < num_films; ++day) {
        int r = 0; //room in where a film will be projected
        for (int f = 0; f < num_films; ++f) {
            if (perm[f].second == day) {
                output << billboard[perm[f].first] << " " << perm[f].second
                       << " " << cinema_rooms[r] << endl;
                ++r;
            }
        }
    }
    output.close();
}

void optimal_billboard_schedule()
{
    vector<fd> perm(num_films); //contains a partial solution
    vector<bool> used(num_films, false);
    map<int, int> different_cardinal; //primer element = nombre de restriccions, segon element = nombre d'elements que tenen el nombre de restriccions igual a la clau
    vector<Score> films_scores; //vector mida = diccionari = nombre de blocs de restriccions diferents que hi ha
    vector<pair<int, int>> films_by_restr = order_films_by_restr(different_cardinal, films_scores);

    cout << "scores: " << endl;
    for (auto e : films_scores) {
        cout << "block: " << e.num_restr << " cardinal: " << e.cardinal << " probability: " << e.probability << endl;
    }

    /*cout << "diccionari:" << endl;
    int count = 0;
    int num_f = 0;
    for (auto e : different_cardinal) {
        cout << e.first << " " << e.second << endl;
        count += e.first * e.second;
        num_f += e.second;
    }
    cout << "total: " << count << endl;
    cout << "total films: " << num_f << endl;*/

    MI prohibitions_per_day(num_films + 1, vector<int>(num_films, 0));
    //row simulate days of projection; columns are the films' index; an entry of the matrix contains
    //the amount of retrictions of a film to be projected on that day
    vector<int> occupied_rooms(num_films + 1, 0);
    //occupied_rooms[k] = number of projecting room at day 'k'
    //the two elements above skip the first row/position to avoid problems with zero-indexation

    //generate_schedule(0, perm, used, films_by_restr, prohibitions_per_day, occupied_rooms, 1);
}

int main(int argc, char* argv[])
{
    ifstream input(argv[1]); //read from file given
    read_data(input);
    input.close();
    output_file = string(argv[2]);
    optimal_billboard_schedule();
}
