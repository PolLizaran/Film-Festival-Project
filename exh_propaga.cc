#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
//#include <math.h>
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
clock_t start = clock(); //initialization of the elapsed time
double duration;

int num_films, num_preferences, num_rooms;
vector<fd> best_perm; //global variables were the optimal and partial solutions are stored
//vector<bool> used;
vector<string> billboard, cinema_rooms;
map<string, int> filmindex; //dictionatt to store the film and its index in the vector so that we don't have to look across the vector
/* ----------------------------------------------------- */

bool compare_by_restr(const pair<int, int>& A, const pair<int, int>& B){
    if(A.second == B.second) return A.first < B.first;
    return A.second > B.second; //order by day of projection
}

void print_projection(const vector<fd>& perm, int shortest_festival)
{
    //exectution time stops
    duration = (clock() - start) / (double)CLOCKS_PER_SEC;
    cout << duration << endl
         << shortest_festival << endl;
    for (int i = 0; i < perm.size(); ++i)
        cout << perm[i].first << " ";
    cout << endl;
    for (int j = 0; j < perm.size(); ++j) {
        int x = j % num_rooms; //variable that helps us to determine in which days the fils are reproduced
        cout << billboard[perm[j].first] << " " << perm[j].second << " "
             << cinema_rooms[x] << endl;
    }
}

//checks whether the film can be projected in the current cinema room
//int projection(int current_film, MB& reproduced_together){}

void propagate_restrictions(int k, const MI& Inc, MB& reproduced_together, const vector<bool>& used, int film, bool insert){
    for(int x : Inc[film]) reproduced_together[k][x] = insert;
}


//shortest_festival is the best configutation at the moment, at the end we'll return this value
void optimal_billboard_schedule(int k, const MI& Inc, vector<fd>& perm,
    vector<bool>& used, const vector<pair<int, int>>& films_by_rest, MB& reproduced_together,
    int current_festival, int shortest_festival)
{
    //if (k % num_rooms == 0){
    //    ++current_festival;}
    //day changes) ++current_festival; //all rooms are assigned, so the festival lasts 1 day more
    //as k is initially 0, a room is addes, which doesn't affect as 0 rooms doesn't make sense

    //if(current_festival > num_films / num_rooms) return;
    if (k == perm.size()) { //all films have been placed
        if (current_festival < shortest_festival) {
            //shortest_festival = current_festival;
            //best_perm = perm;
            print_projection(perm, 0);
            for (int u = 0; u < num_films; ++u) {
                for (int v = 0; v < num_films; ++v) {
                    cout << reproduced_together[u][v] << ' ';
                }
                cout << endl;
            }
            //HEM DE REDIRIGIR LA SORTIDA EN COMPTES D'IMPRIMIR-LA
        }
    } else if (current_festival < shortest_festival) {
        for (int u = 0; u < num_films; ++u) { //loop for each film
            if (not used[films_by_rest[u].first]) {
                used[films_by_rest[u].first] = true;
                propagate_restrictions(k, Inc, reproduced_together, used, films_by_rest[u].first, true);
                perm[k] = {films_by_rest[u].first, k};
                optimal_billboard_schedule(k + 1, Inc, perm, used, films_by_rest, reproduced_together, current_festival, shortest_festival);
                propagate_restrictions(k, Inc, reproduced_together, used, films_by_rest[u].first, false);
                used[films_by_rest[u].first] = false;
            }
        }
    } else return;
}

void set_films_by_rest(vector<pair<int, int>>& v, const MI& Inc){
    for(int i = 0; i < Inc.size(); ++i){
        v[i] = { i, Inc[i].size()}; //we store the dimension of the array of restrictions
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
    vector<fd> perm(num_films);
    vector<bool> used(num_films, false);
    vector<pair<int, int>> films_by_rest(num_films); //ordered by the number of restrictions, being the firts element the one with the most number of restrictions
    set_films_by_rest(films_by_rest, Inc);
    MB reproduced_together(num_films, vector<bool>(num_films, false)); //we waste space because maybe we won't need a day for each film to be projected
    optimal_billboard_schedule(0, Inc, perm, used, films_by_rest, reproduced_together, 0, INT_MAX); //no faria falta passar com a paràmetre alguns atributs

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
