#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
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
    for (unsigned int i = 0; i < num_preferences; ++i) {
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
    for (unsigned int i = 0; i < num_rooms; ++i)
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

void print_projection(const vector<fd>& perm, int lenght_festival)
{
    ofstream output(output_file);
    output << setprecision(1) << fixed;
    elapsed_time = (clock() - start) / (double)CLOCKS_PER_SEC;

    output << elapsed_time << endl
           << lenght_festival << endl;

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
    Whether a film can be projected in a day checking its restrictions and the emptyness of the rooms.
*/
bool can_be_fit(const MI& prohibitions_per_day, const vector<int>& occupied_rooms, int film, int day)
{
    return prohibitions_per_day[day][film] == 0 and occupied_rooms[day] < num_rooms;
}

/*
Args:
    - modifier: takes value 1 for adding prohibitions in the "day" where "film" is projected
                takes value -1 analogously for removing prohibitions
*/
void propagate_restrictions(int day, MI& prohibitions_per_day, int film, int modifier)
{
    for (const int& banned_film : Inc[film])
        prohibitions_per_day[day][banned_film] += modifier;
}

/* 
Works on:
    Searches a configuration of the cinema festival, that enables all films to be projected, taking 
    into account the restrictions among others and following a greedy strategy. This means it will 
    not always be an optimal solution. It iterates througout the vector of films ordered by their 
    number of restrictions and assign them in the first day where can be projected. In case a film 
    could not be assigned to a day, festival's lenght is increased as the film will need an extra day.
*/
void generate_greedy_schedule(const vector<pair<int, int>>& films_by_rest, vector<fd>& perm, 
                              vector<int>& occupied_rooms, MI& prohibitions_per_day)
{
    int lenght_festival = 1; //current days spent on the festival
    for (int k = 0; k < num_films; ++k) {
        const int film = films_by_rest[k].first;
        bool film_is_projected = false; //boolean for each k-th film 
        for(int day = 1; day <= lenght_festival and not film_is_projected; ++day){ 
            if (can_be_fit(prohibitions_per_day, occupied_rooms, film, day)) {
                film_is_projected = true;
                ++occupied_rooms[day];
                perm[k] = { film, day };
                propagate_restrictions(day, prohibitions_per_day, film, 1);
            } else if (day == lenght_festival) { //the film could not be projected yet
                ++lenght_festival;
                perm[k] = { film, lenght_festival }; //assign the film to that extra day
            }
        }
    }
    print_projection(perm, lenght_festival);
}

void optimal_billboard_schedule()
{
    vector<fd> perm(num_films); //contains a partial solution (<film, day_of_projection>)
    vector<pair<int, int>> films_by_rest = order_films_by_rest();

    MI prohibitions_per_day(num_films + 1, vector<int>(num_films, 0));
    //rows simulate days of projection; columns are the films' index; an entry of the matrix contains
    //the amount of retrictions of a film to be projected on that day
    vector<int> occupied_rooms(num_films + 1, 0);
    //occupied_rooms[k] = number of projecting rooms at day 'k'
    //the two elements above skip the first row/position to avoid problems with zero-indexation

    generate_greedy_schedule(films_by_rest, perm, occupied_rooms, prohibitions_per_day);
}

int main(int argc, char* argv[])
{
    ifstream input(argv[1]); //read from file given
    read_data(input);
    input.close();
    output_file = string(argv[2]);
    optimal_billboard_schedule();
}
