#include <algorithm>
#include <fstream>
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

int shortest_festival = INT_MAX;

int num_films, num_preferences, num_rooms;
//vector<bool> used;
vector<string> billboard, cinema_rooms;
map<string, int> filmindex; //dictionatt to store the film and its index in the vector so that we don't have to look across the vector
/* ----------------------------------------------------- */

//first element is the film, and the number of restrictions
bool compare_by_restr(const pair<int, int>& A, const pair<int, int>& B)
{
    if (A.second == B.second)
        return A.first < B.first;
    return A.second > B.second; //order by day of projection
}

void print_projection(const vector<vector<int>>& perm, ofstream& output)
{
    //exectution time stops
    duration = (clock() - start) / (double)CLOCKS_PER_SEC;
    output << duration << endl
           << shortest_festival << endl;

    for (int i = 0; i < perm.size(); ++i) {
        cout << "perm.size " << perm.size() << endl;
        for (int j = 0; j < perm[i].size(); ++j) {
            cout << "perm[i].size " << perm[i].size() << "  j  " << j << endl;
            cout << billboard[perm[i][j]] << " " << i << " "
                   << cinema_rooms[j] << endl;
        }
    }
}

//checks whether the film can be projected in the current cinema room
//int projection(int current_film, MB& reproduced_together){}

void propagate_restrictions(int day, const MI& Inc, MB& reproduced_together, int film, bool insert)
{
    for (int x : Inc[film])
        reproduced_together[day][x] = insert;
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
void optimal_billboard_schedule(int k, const MI& Inc, vector<vector<int>>& perm,
    vector<bool>& used, const vector<pair<int, int>>& films_by_rest, MB& reproduced_together,
    vector<int>& occupied_rooms, int lenght_festival, ofstream& output)
{

    //day changes) ++lenght_festival; //all rooms are assigned, so the festival lasts 1 day more
    //as k is initially 0, a room is addes, which doesn't affect as 0 rooms doesn't make sense

    if (shortest_festival == ceil(float(num_films) / float(num_rooms)))
        return;
    if (k == perm.size()) { //all films have been placed
        if (lenght_festival < shortest_festival) {
            shortest_festival = lenght_festival;
            //best_perm = perm;
            print_projection(perm, output);
            /*for (int u = 0; u <= num_films; ++u) {
                for (int v = 0; v < num_films; ++v) {
                    cout << reproduced_together[u][v] << ' ';
                }
                cout << endl;
            }*/
            //HEM DE REDIRIGIR LA SORTIDA EN COMPTES D'IMPRIMIR-LA
        }
    } else {
        for (int u = 0; u < num_films; ++u) { //loop for each film
            const int& film = films_by_rest[u].first;
            if (not used[film] and lenght_festival < shortest_festival) {
                int day = search_day_to_be_fit(reproduced_together, occupied_rooms, film, lenght_festival);
                if (day == -1) {
                    ++lenght_festival;
                    day = lenght_festival;
                }
                used[film] = true;
                ++occupied_rooms[day];
                propagate_restrictions(day, Inc, reproduced_together, film, true);
                perm[day - 1].push_back(film); //0 indexed
                optimal_billboard_schedule(k + 1, Inc, perm, used, films_by_rest, reproduced_together,
                    occupied_rooms, lenght_festival, output);
                propagate_restrictions(day, Inc, reproduced_together, film, false);
                --occupied_rooms[day];
                used[film] = false;
            }
        }
    }
    return;
}

void set_films_by_rest(vector<pair<int, int>>& v, const MI& Inc)
{
    for (int i = 0; i < Inc.size(); ++i) {
        v[i] = { i, Inc[i].size() }; //we store the dimension of the array of restrictions
    }
    sort(v.begin(), v.end(), compare_by_restr);
    /*
    for(int j = 0; j < Inc.size(); ++j){
        cout << "film " << j << ':' << endl;
        for(int k = 0; k < Inc[j].size(); ++k){
            cout << Inc[j][k] << ' ';
        }
        cout << endl;
    }
    cout << "ordered vector : ";
    for(pair<int, int> u : v) cout << u.first << ' ' << u.second << "  ";
    cout << endl;*/
}

void read_films(ifstream& input)
{
    input >> num_films;
    billboard = vector<string>(num_films);
    for (int i = 0; i < num_films; ++i) {
        input >> billboard[i];
        filmindex.insert({ billboard[i], i });
    }
    //vectors that will be used later during the computation of the solution
}

//pel·licules que no es poden projectar alhora
void read_incompatibilities(MI& Inc, ifstream& input)
{
    input >> num_preferences;
    for (uint i = 0; i < num_preferences; ++i) { //MIRAR SI HEM D'IMPLEMENTAR EN LLISTA O DICC
        string a, b;
        input >> a >> b; //S'HA DE CANVIAR PER LLEGIR DES D'ARXIU
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

int main()
{
    cout.setf(ios::fixed);
    cout.precision(1);
    string file;
    cin >> file;
    ifstream input(file);
    MI Inc = read_data(input);
    start = clock();
    vector<vector<int>> perm(num_films);
    vector<bool> used(num_films, false);
    vector<pair<int, int>> films_by_rest(num_films); //ordered by the number of restrictions, being the firts element the one with the most number of restrictions
    set_films_by_rest(films_by_rest, Inc);
    MB reproduced_together(num_films + 1, vector<bool>(num_films, false)); //we waste space because maybe we won't need a day for each film to be projected, first row will be avoided
    vector<int> occupied_rooms(num_films + 1, 0); // el 0 no utilitzem
    ofstream output("sortida1.txt");
    optimal_billboard_schedule(0, Inc, perm, used, films_by_rest, reproduced_together, occupied_rooms, 0, output); //no faria falta passar com a paràmetre alguns atributs
    //print_projection();
    input.close();
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
