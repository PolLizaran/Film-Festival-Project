#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <map>
#include <math.h>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

using namespace std;
using MI = vector<vector<int>>; //Matrix for the Incompatibilities
using MB = vector<vector<bool>>;
using fd = pair<int, int>; //film and its day

// GLOBAL VARIABLES
/* ----------------------------------------------------- */
clock_t start = clock();
double elapsed_time;

string output_file;

int num_films, num_preferences, num_rooms, shortest_festival = INT_MAX;
vector<string> billboard, cinema_rooms;
map<string, int> filmindex; //dictionary to store the film and its index in the vector so that we don't have to look across the vector
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
    "Symetric"
*/
void read_incompatibilities(MI& Inc, ifstream& input)
{
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

MI read_data(ifstream& input)
{
    read_films(input);
    MI Inc(num_films);
    read_incompatibilities(Inc, input);
    read_cinema_rooms(input);
    return Inc;
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

void set_films_by_rest(vector<pair<int, int>>& v, const MI& Inc)
{
    for (int i = 0; i < Inc.size(); ++i) {
        v[i] = { i, Inc[i].size() }; //we store the dimension of the array of restrictions
    }

    sort(v.begin(), v.end(), compare_by_restr);

}

void print_projection(const vector<fd>& perm)
{
    ofstream output(output_file);
    output << setprecision(1) << fixed;
    elapsed_time = (clock() - start) / (double)CLOCKS_PER_SEC;

    output << elapsed_time << endl
           << shortest_festival << endl;
    cout << shortest_festival << endl;
    for (int i = 0; i < num_films; ++i) {
        int r = 0;
        for (int j = 0; j < num_films; ++j) {
            if (perm[j].second == i) {
                //int x = j % num_rooms; //variable that helps us to determine in which days the fils are reproduced
                output << billboard[perm[j].first] << " " << perm[j].second << " "
                       << cinema_rooms[r] << endl;
                ++r;
            }
        }
    }
}

//checks whether the film can be projected in the current cinema room
bool can_be_fit(const MI& reproduced_together, const vector<int>& occupied_rooms, int film, int day)
{
    return reproduced_together[day][film] == 0 and occupied_rooms[day] < num_rooms;
}

/*
Args:
    modifier: takes value 1 for adding prohibitions in the "day" where "film" is projected
              takes value -1 analogously for removing prohibitions
*/
void propagate_restrictions(int day, const MI& Inc, MI& reproduced_together, int film, int modifier)
{

    for (const int& x : Inc[film]) reproduced_together[day][x] += modifier;
    reproduced_together[day][film] += modifier;
}



//shortest_festival is the best configutation at the moment, at the end we'll return this value
/* VARIABLES:
Inc = Llista Adj de Incompatibilities
perm = permutació actual fins a k (pot no estar acabada no ser la òptima)
used = vector que determina si hem usat la i-èssima peli
films_by_rest = vector de pelis ordenades per nombre de restriccions, el primer és el que té més
reproduced_together = matriu de restriccions que diu en el j-èssim dia quines pelis estan prohibides
occupied_rooms = nombre de sales ocupades en el k-èssim dia
*/
void optimal_billboard_schedule(int k, const MI& Inc, vector<fd>& perm,
    vector<bool>& used, const vector<pair<int, int>>& films_by_rest, MI& reproduced_together,
    vector<int>& occupied_rooms, int lenght_festival)
{
    if (shortest_festival == ceil(float(num_films) / float(num_rooms))) return;
    if (k == num_films) { //all films have been placed
        if (lenght_festival < shortest_festival) {
            shortest_festival = lenght_festival;
            print_projection(perm);
        }
        return; //CREC QUE ES POT ELIMINAR
    } else {
        for (int day = 1; day <= lenght_festival; ++day) { //loop for each film
            const int film = films_by_rest[k].first;
            bool film_not_added = true;
            if (not used[film] and lenght_festival < shortest_festival){
                if(can_be_fit(reproduced_together, occupied_rooms, film, day)){
                    used[film] = true;
                    film_not_added = true;
                    ++occupied_rooms[day];
                    perm[k] = { film, day };
                    propagate_restrictions(day, Inc, reproduced_together, film, 1);
                    optimal_billboard_schedule(k + 1, Inc, perm, used, films_by_rest, reproduced_together,
                        occupied_rooms, lenght_festival);
                    propagate_restrictions(day, Inc, reproduced_together, film, -1);
                    film_not_added = false;
                    --occupied_rooms[day];
                    used[film] = false;
                }else if (film_not_added and day == lenght_festival) {
                    ++lenght_festival;
                }
            }
        }
    }
}


int main(int argc, char* argv[])
{
    ifstream input(argv[1]); //read from file given
    MI Inc = read_data(input);
    input.close();
    output_file = string(argv[2]);
    vector<fd> perm(num_films);
    vector<bool> used(num_films, false);
    vector<pair<int, int>> films_by_rest(num_films); //ordered by the number of restrictions, being the firts element the one with the most number of restrictions
    set_films_by_rest(films_by_rest, Inc);
    MI reproduced_together(num_films + 1, vector<int>(num_films, 0)); //we waste space because maybe we won't need a day for each film to be projected, first row will be avoided
    vector<int> occupied_rooms(num_films + 1, 0); // el 0 no utilitzem
    optimal_billboard_schedule(0, Inc, perm, used, films_by_rest, reproduced_together, occupied_rooms, 1); //no faria falta passar com a paràmetre alguns atributs
    //print_projection();

}

/*
Lectura d'un arxiu:

ifstream in(file);

int P;
in >> P;
I2P.resize(P);
for (int k = 0; k < P; ++k) {
  in >> I2P[k];
  P2I[ I2P[k] ] = k;
}


Redireccio de dades:

ofstream f("noms.txt");
f << "Joan" << endl;
f << "Pere" << endl;
f.close();





    for(int i = 0; i < num_films; ++i) cout << billboard[i] << " ";
    cout << endl;;
    for(int i = 0; i < num_films; ++i){
        for(int j = 0; j < num_films; ++j){
            cout << Inc[i][j] << ' ';
        }
        cout << endl;
    }
    for(int i = 0; i < num_rooms; ++i) cout << cinema_rooms[i] << ' ';
    cout << endl;



-print_projection()
    cout << "SIZE : " << perm.size() << endl;
    for (int i = 0; i < perm.size(); ++i)
        cout << perm[i].first << " " << perm[i].second << "    ";

    cout << endl << endl;
    cout << "Pepe : " << perm.size() << endl;
    sort(perm.begin(), perm.end(), compare_by_first_projected);


-set_films_by_rest()
    cout << "vector : ";
    for(pair<int, int> u : v) cout << u.first << ' ' << u.second << "  ";
    cout << endl << endl << endl << endl;

    for(int j = 0; j < Inc.size(); ++j){
        cout << "film " << j << ':' << endl;
        for(int k = 0; k < Inc[j].size(); ++k){
            cout << Inc[j][k] << ' ';
        }
        cout << endl;
    }
    cout << "ordered vector : ";
    for(pair<int, int> x : v) cout << x.first << ' ' << x.second << "  ";
    cout << endl << endl << endl << endl;



*/
