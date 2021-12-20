#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <math.h>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

using namespace std;
using MI = vector<vector<int>>; //matrix of integers
using ALI = vector<vector<int>>; //adjacency list of integers
using fd = pair<int, int>; //represents a film and the day of its projection

/* GLOBAL VARIABLES ------------------------------------------------------------------------------- */
clock_t start = clock();
double elapsed_time;

int num_films, num_preferences, num_rooms;
int shortest_festival = INT_MAX; //best configutation so far (initially infinite)
int min_required_lenght;

vector<string> billboard, cinema_rooms;
ALI Inc; // stores the Incompatibilities among films that can't be projected in the same day

map<string, int> filmindex; //stores the name of a film and its position in the "billboard" vector

string output_file;
/* ------------------------------------------------------------------------------------------------ */

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
    Fills the global adjacency list variable "Incompatibilities". Those films that are forbiden to be
    projected together are added in both films' lists.
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
    - A, B: contain the index of a film and the amount of films incompatible

Returns:
    The film that is more restricted.
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
vector<pair<int, int>> order_films_by_rest()
{
    vector<pair<int, int>> films_by_rest(num_films);
    for (int i = 0; i < Inc.size(); ++i) {
        films_by_rest[i] = { i, Inc[i].size() };
    }
    sort(films_by_rest.begin(), films_by_rest.end(), compare_by_restr);
    return films_by_rest;
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
    // are choosen arbitrarily
    for (int day = 1; day <= num_films; ++day) {
        int r = 0; //room in where a film will be projected
        for (int f = 0; f < num_films; ++f) { //f = film
            if (perm[f].second == day) {
                output << billboard[perm[f].first] << " " << perm[f].second
                       << " " << cinema_rooms[r] << endl;
                ++r;
            }
        }
    }
    output.close();
}

/*
Returns:
    Whether a film can legaly be assigned to a day checking at restrictions and the emptyness of the rooms.
*/
bool can_be_projected(const MI& prohibitions_per_day, const vector<int>& occupied_rooms, int film, int day)
{
    return prohibitions_per_day[day][film] == 0 and occupied_rooms[day] < num_rooms;
}

/*
Args:
    - modifier: takes value 1 for adding prohibitions in the "day" where "film" is projected
                takes value -1 analogously for removing prohibitions
*/
void propagate_restrictions(MI& prohibitions_per_day, int day, int film, int modifier)
{
    for (const int& banned_film : Inc[film])
        prohibitions_per_day[day][banned_film] += modifier;
}

/*
Args:
    - k: index that determines the limit between the invariant part already computed and the current
         film being studied
    - length_festival: current days spent on the festival

Works on:
    Computes all the possible shcedules of the films projections regarding their restricions. It
    iterates throughtout all the current days and, in case a film can be projected, adds it according
    to its day of projection. While iterating, those threads that overtake "shortest_festival" are
    omitted.
    Whenever is required, it increases the festival's length allowing new
    combinations to be generated. As solutions may not be optimal, once a thread ends, changes made
    must be undone.
*/
void generate_schedule(int k, const vector<pair<int, int>>& films_by_rest, vector<fd>& perm,
                       vector<int>& occupied_rooms, MI& prohibitions_per_day, vector<bool>& used,
                       int lenght_festival)
{
    if (shortest_festival == min_required_lenght) return; //partial solution is already optimal
    if (k == num_films) {
        if (lenght_festival < shortest_festival) { //better solution found
            shortest_festival = lenght_festival;
            print_projection(perm);
        }
    } else {
        for (int day = 1; day <= lenght_festival; ++day) {
            const int film = films_by_rest[k].first;
            if (not used[film] and lenght_festival < shortest_festival) {
                if (can_be_projected(prohibitions_per_day, occupied_rooms, film, day)) {
                    used[film] = true;
                    ++occupied_rooms[day];
                    perm[k] = { film, day };
                    propagate_restrictions(prohibitions_per_day, day, film, 1);
                    generate_schedule(k + 1, films_by_rest, perm, occupied_rooms, prohibitions_per_day, 
                                      used, lenght_festival);
                    propagate_restrictions(prohibitions_per_day, day, film, -1);
                    used[film] = false;
                    --occupied_rooms[day];
                } else if (not used[film] and day == lenght_festival) { //the film could not yet be projected
                    ++lenght_festival;
                }
            }
        }
    }
}

void optimal_billboard_schedule()
{
    vector<fd> perm(num_films); //contains a partial solution (<film, day_of_projection>)
    vector<bool> used(num_films, false);
    vector<pair<int, int>> films_by_rest = order_films_by_rest();

    MI prohibitions_per_day(num_films + 1, vector<int>(num_films, 0));
    //rows simulate days of projection; columns are the films' index; an entry of the matrix contains
    //the amount of retrictions of a film to be projected on that day
    vector<int> occupied_rooms(num_films + 1, 0);
    //occupied_rooms[k] = number of projecting rooms at day 'k'
    //the two elements above skip the first row/position to avoid problems with zero-indexation

    min_required_lenght = ceil(float(num_films) / float(num_rooms));
    generate_schedule(0, films_by_rest, perm, occupied_rooms, prohibitions_per_day, used, 1);
}

int main(int argc, char* argv[])
{
    ifstream input(argv[1]);
    read_data(input);
    input.close();
    output_file = string(argv[2]);
    optimal_billboard_schedule();
}
