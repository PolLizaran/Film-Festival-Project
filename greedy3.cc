#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
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

// GLOBAL VARIABLES
/* ----------------------------------------------------- */
clock_t start = clock();
double elapsed_time;

int num_films, num_preferences, num_rooms;
int min_required_lenght = ceil(float(num_films) / float(num_rooms));

vector<string> billboard, cinema_rooms;
ALI Inc; // stores the Incompatibilities among films that can't be projected in the same day

map<string, int> filmindex; //stores the name of a film and its position in the "billboard" vector

string output_file;
/* ----------------------------------------------------- */

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
           << perm.back().second << endl;
    cout << perm.back().second << endl;

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

int search_best_film(const vector<bool>& used, MI& prohibitions_per_day, int day){
    cout << "day " << day << endl;
    int max_common_restr = -1, current_common_retr = 0, choosen_film = -1;
    for(int film = 0; film < num_films; ++film){
        current_common_retr = 0;
        if(not used[film]){
            for(int j = 0; j < Inc[film].size(); ++j){
                int x = Inc[film][j];
                for(int k = 0; k < Inc[x].size(); ++k){
                    if(prohibitions_per_day[day][x] == 0 and
                       Inc[film][j] == Inc[x][k]) ++current_common_retr;
                }
            }
        }
        if(current_common_retr > max_common_restr){
               max_common_restr = current_common_retr;
               choosen_film = film;
        }
    }
    cout << "choosen " << choosen_film << endl;
    return choosen_film;
}

/*
Args:
    - modifier = takes value 1 for adding prohibitions in the "day" where "film" is projected
                 takes value -1 analogously for removing prohibitions
*/
void propagate_restrictions(int day, MI& prohibitions_per_day, int film)
{
    for (const int& x : Inc[film])
        prohibitions_per_day[day][x] = 1; //fa la funció d'un booleà
}

//shortest_festival is the best configutation at the moment, at the end we'll return this value
/* VARIABLES:
Inc = Llista Adj de Incompatibilities
perm = permutació actual fins a k (pot no estar acabada no ser la òptima)
used = vector que determina si hem usat la i-èssima peli
films_by_rest = vector de pelis ordenades per nombre de restriccions, el primer és el que té més
prohibitions_per_day = matriu de restriccions que diu en el j-èssim dia quines pelis estan prohibides
occupied_rooms = nombre de sales ocupades en el k-èssim dia
*/
void generate_schedule(int k, vector<fd>& perm, vector<bool>& used,
    const vector<pair<int, int>>& films_by_rest, MI& prohibitions_per_day,
    vector<int>& occupied_rooms)
{
    int day = 1;
    for (int k = 0; k < num_films; ++k) {
        int film;
        cout<< "iteració " << k << endl;
        if(occupied_rooms[day] == num_rooms){
            ++day;
            int iterator = 0;
            while(used[films_by_rest[iterator].first])iterator++;
            film = films_by_rest[iterator].first; //si no hem pogut posar cap a l'últim dia, posem la que té més restriccions al dia següent
        }else{
            film = search_best_film(used, prohibitions_per_day, day);
        }
        cout<< "peli " << film << endl;
        used[film] = true;
        ++occupied_rooms[day];
        perm[k] = { film, day };
        propagate_restrictions(day, prohibitions_per_day, film);
        /*
        for(int i = 0; i < num_films + 1; ++i){
            for(int j = 0; j < num_films; ++j){
                cout << prohibitions_per_day[i][j] << ' ';
            }
            cout << endl;
        }*/
        cout << endl;
    }
    print_projection(perm);
}

void optimal_billboard_schedule()
{
    vector<fd> perm(num_films); //contains a partial solution
    vector<bool> used(num_films, false);
    vector<pair<int, int>> films_by_rest = order_films_by_rest();

    MI prohibitions_per_day(num_films + 1, vector<int>(num_films, 0));
    //row simulate days of projection; columns are the films' index; an entry of the matrix contains
    //the amount of retrictions of a film to be projected on that day
    vector<int> occupied_rooms(num_films + 1, 0);
    //occupied_rooms[k] = number of projecting room at day 'k'
    //the two elements above skip the first row/position to avoid problems with zero-indexation
    generate_schedule(0, perm, used, films_by_rest, prohibitions_per_day, occupied_rooms);
}

int main(int argc, char* argv[])
{
    ifstream input(argv[1]); //read from file given
    read_data(input);
    input.close();
    output_file = string(argv[2]);
    optimal_billboard_schedule();
}
