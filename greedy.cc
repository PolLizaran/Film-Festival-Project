#include <algorithm>
#include <iostream>
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

// GLOBAL VARIABLES
/* ----------------------------------------------------- */
clock_t start; //initialization of the elapsed time
double duration;

int num_films, num_preferences, num_rooms;
//vector<bool> used;
vector<string> billboard, cinema_rooms;
map<string, int> filmindex; //dictionatt to store the film and its index in the vector so that we don't have to look across the vector
/* ----------------------------------------------------- */

bool compare_by_restr(const pair<int, int>& A, const pair<int, int>& B)
{
    if (A.second == B.second)
        return A.first < B.first;
    return A.second > B.second; //order by day of projection
}

void print_projection(const vector<vector<int>>& perm, int lenght_festival)
{
    //exectution time stops
    duration = (clock() - start) / (double)CLOCKS_PER_SEC;
    cout << duration << endl
         << lenght_festival << endl;
    for (int i = 0; i < perm.size(); ++i) {
        for (int j = 0; j < perm[i].size(); ++j) {
            cout << billboard[perm[i][j]] << " " << i << " "
                 << cinema_rooms[j] << endl;
        }
    }
}

//checks whether the film can be projected in the current cinema room
//int projection(int current_film, MB& reproduced_together){}

void propagate_restrictions(int day, const MI& Inc, MB& reproduced_together, int film)
{
    for (int x : Inc[film])
        reproduced_together[day][x] = true;
}

int search_day_to_be_fit(const MB& reproduced_together, const vector<int>& occupied_rooms, int film, int lenght_festival)
{
    for (int day = 1; day <= lenght_festival; ++day) {
        if (not reproduced_together[day][film] and occupied_rooms[day] < num_rooms)
            return day;
        //podem en cas que estigui prohibida o no quedin sales disponibles aquell dia
    }
    return -1;
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
void optimal_billboard_schedule(const MI& Inc, vector<vector<int>>& perm, vector<pair<int, int>>& films_by_rest, MB& reproduced_together,
    vector<int>& occupied_rooms, int lenght_festival)
{
    for (int i = 0; i < num_films; ++i) { //loop for each film
        const int& film = films_by_rest[i].first;
        int day = search_day_to_be_fit(reproduced_together, occupied_rooms, film, lenght_festival);
        if (day == -1) {
            ++lenght_festival;
            day = lenght_festival;
        }
        ++occupied_rooms[day];
        propagate_restrictions(day, Inc, reproduced_together, film);
        perm[day].push_back(film);
    }
    print_projection(perm, lenght_festival);
}

void set_films_by_rest(vector<pair<int, int>>& v, const MI& Inc)
{
    for (int i = 0; i < Inc.size(); ++i) {
        v[i] = { i, Inc[i].size() }; //we store the dimension of the array of restrictions
    }
    sort(v.begin(), v.end(), compare_by_restr);
}

void read_films()
{
    cin >> num_films;
    billboard = vector<string>(num_films);
    for (int i = 0; i < num_films; ++i) {
        cin >> billboard[i];
        filmindex.insert({ billboard[i], i });
    }
    //vectors that will be used later during the computation of the solution
}

//pel·licules que no es poden projectar alhora
void read_incompatibilities(MI& Inc)
{
    cin >> num_preferences;
    for (uint i = 0; i < num_preferences; ++i) { //MIRAR SI HEM D'IMPLEMENTAR EN LLISTA O DICC
        string a, b;
        cin >> a >> b; //S'HA DE CANVIAR PER LLEGIR DES D'ARXIU
        Inc[filmindex[a]].push_back(filmindex[b]);
        Inc[filmindex[b]].push_back(filmindex[a]);
    }
}

void read_cinema_rooms()
{
    cin >> num_rooms;
    cinema_rooms = vector<string>(num_rooms);
    for (uint i = 0; i < num_rooms; ++i)
        cin >> cinema_rooms[i];
}

MI read_data()
{
    read_films();
    MI Inc(num_films);
    read_incompatibilities(Inc);
    read_cinema_rooms();
    return Inc;
}

int main()
{

    MI Inc = read_data();
    start = clock();
    vector<vector<int>> perm(num_films + 1);
    vector<pair<int, int>> films_by_rest(num_films); //ordered by the number of restrictions, being the firts element the one with the most number of restrictions
    set_films_by_rest(films_by_rest, Inc);
    MB reproduced_together(num_films + 1, vector<bool>(num_films, false)); //we waste space because maybe we won't need a day for each film to be projected, first row will be avoided
    vector<int> occupied_rooms(num_films + 1, 0); // el 0 no utilitzem
    optimal_billboard_schedule(Inc, perm, films_by_rest, reproduced_together, occupied_rooms, 0); //no faria falta passar com a paràmetre alguns atributs
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




*/
