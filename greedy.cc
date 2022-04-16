#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <time.h>
#include <vector>

using namespace std;
using MI = vector<vector<int>>; //matrix of integers
using ALI = vector<vector<int>>; //adjacency list of integers

/* GLOBAL VARIABLES ------------------------------------------------------------------------------- */
clock_t start = clock();
double elapsed_time;

int num_films, num_preferences, num_rooms;

vector<string> billboard, cinema_rooms;
ALI Inc; // stores the Incompatibilities among films that can't be projected in the same day

map<string, int> filmindex; //stores the name of a film and its position in the "billboard" vector

string output_file;
/* ------------------------------------------------------------------------------------------------ */

struct Projection{
    int film_indx_p; 
    int day_proj;
};

struct Orderings{ //used when sorting the films
    int film_indx_o; 
    int num_of_restr;
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
    Fills the global adjacency list variable "Incompatibilities". Those films that are forbidden to be 
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
bool compare_by_restr(const Orderings& A, const Orderings& B)
{
    if (A.num_of_restr == B.num_of_restr)
        return A.film_indx_o < B.film_indx_o; //arbitary selection
    return A.num_of_restr > B.num_of_restr;
}

/*
Returns:
    A sorted vector of structs containing the index of a film and the number of films with which is 
    forbidden to be projected in the same day. Its elements are in descending order.
*/
vector<Orderings> order_films_by_rest()
{
    vector<Orderings> films_by_rest(num_films);
    for (int i = 0; i < Inc.size(); ++i) {
        films_by_rest[i] = { i, int(Inc[i].size()) }; //film 'i' has Inc[i].size() restrictions
    }
    sort(films_by_rest.begin(), films_by_rest.end(), compare_by_restr);
    return films_by_rest;
}

void print_projection(const vector<Projection>& perm, int length_festival)
{
    ofstream output(output_file);
    output << setprecision(1) << fixed;
    elapsed_time = (clock() - start) / (double)CLOCKS_PER_SEC;

    output << elapsed_time << endl
           << length_festival << endl;

    // looks across the vector "perm" to print those films that match with the day of projection. 
    // Cinema rooms are choosen arbitrarily.
    for (int day = 1; day <= num_films; ++day) {
        int r = 0; //room in where a film will be projected
        for (int f = 0; f < num_films; ++f) { //f = film
            if (perm[f].day_proj == day) {
                output << billboard[perm[f].film_indx_p] << " " << perm[f].day_proj
                       << " " << cinema_rooms[r] << endl;
                ++r;
            }
        }
    }
    output.close();
}

/*
Returns:
    Whether a film can legaly be assigned to a day checking its restrictions and the emptyness of the rooms.
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
    for (const int& banned_film : Inc[film]) prohibitions_per_day[day][banned_film] += modifier;
}

/* 
Works on:
    Searches a configuration of the cinema festival, that enables all films to be projected, taking 
    into account the restrictions among others and following a greedy strategy. This means it will 
    not always be an optimal solution. It iterates throughout the vector of films ordered by their 
    number of restrictions and assign them in the first day where can be projected. In case a film 
    could not be assigned to a day, festival's lenght is increased as the film will need an extra day.
*/
void generate_greedy_schedule(const vector<Orderings>& films_by_rest, vector<Projection>& perm, 
                              vector<int>& occupied_rooms, MI& prohibitions_per_day)
{
    int lenght_festival = 1; //current days spent on the festival
    for (int k = 0; k < num_films; ++k) {
        const int film = films_by_rest[k].film_indx_o; //generation from more to less restricted
        bool film_is_projected = false; //boolean for each k-th film 
        for(int day = 1; day <= lenght_festival and not film_is_projected; ++day){ 
            if (can_be_projected(prohibitions_per_day, occupied_rooms, film, day)) {
                film_is_projected = true;
                ++occupied_rooms[day];
                perm[k] = { film, day };
                propagate_restrictions(prohibitions_per_day, day, film, 1);
            } else if (day == lenght_festival) { //the film could not yet be projected
                ++lenght_festival;
                perm[k] = { film, lenght_festival }; //assign the film to that extra day
            }
        }
    }
    print_projection(perm, lenght_festival);
}

void optimal_billboard_schedule()
{
    vector<Projection> perm(num_films); //contains a partial solution 
    vector<Orderings> films_by_rest = order_films_by_rest();

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
    ifstream input(argv[1]);
    read_data(input);
    input.close();
    output_file = string(argv[2]);
    optimal_billboard_schedule();
}
