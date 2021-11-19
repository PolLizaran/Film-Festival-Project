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

int num_films, num_preferences, num_rooms, shortest_festival = INT_MAX;
vector<fd> best_perm; //global variables were the optimal and partial solutions are stored
//vector<bool> used;
vector<string> billboard, cinema_rooms;
map<string, int> filmindex; //dictionatt to store the film and its index in the vector so that we don't have to look across the vector
MI Inc;
/* ----------------------------------------------------- */

bool compare(const fd& A, const fd& B){
    return A.second < B.second; //order by day of projection
}

void print_projection()
{
    //exectution time stops
    duration = (clock() - start) / (double)CLOCKS_PER_SEC;
    cout << duration << endl
         << shortest_festival << endl;
    for (int i = 0; i < best_perm.size(); ++i)
        cout << best_perm[i].first << " ";
    cout << endl;
    sort(best_perm.begin(), best_perm.end(), compare);
    for (int j = 0; j < best_perm.size(); ++j) {
        int x = j % num_rooms; //variable that helps us to determine in which days the fils are reproduced
        cout << billboard[best_perm[j].first] << " " << best_perm[j].second << " "
             << cinema_rooms[x] << endl;
    }
}


int projection(int current_film, vector<vector<bool>>& reproduced_together, vector<int>& used_rooms)
{
    for(int day = 1; day < reproduced_together.size(); ++day){
        if(used_rooms[day] < num_rooms and
           not reproduced_together[day][current_film]) return day;
    }
    return -1;
}

void propagate_restrictions(const int& day_of_projection, vector<vector<bool>>& reproduced_together, int film){
    for (const int& f : Inc[film]) reproduced_together[day_of_projection][f] = true;
    //for (bool x : reproduced_together[day_of_projection]) cout << x << ' ';
    //cout << endl << endl;
}

//shortest_festival is the best configutation at the moment, at the end we'll return this value
void optimal_billboard_schedule(int k, const MI& Inc, vector<fd>& perm,
     vector<bool>& used, vector<vector<bool>>& reproduced_together, vector<int>& used_rooms,
     int current_festival)
{
    if (k == perm.size()) { //all films have been placed
        if (current_festival < shortest_festival) {
            shortest_festival = current_festival;
            best_perm = perm;
            //print_projection(perm);
            //HEM DE REDIRIGIR LA SORTIDA EN COMPTES D'IMPRIMIR-LA
        }
    } else if (current_festival < shortest_festival) {
        for (int film = 0; film < num_films; ++film) { //loop for each film
            if (not used[film]) {
                int day_of_projection = projection(film, reproduced_together, used_rooms);
                if(day_of_projection < 0){
                    ++current_festival;
                    day_of_projection = current_festival; //the following day
                    reproduced_together.push_back(vector<bool>(num_films, false));
                    used_rooms.push_back(0);

                }
                //cout << "used_rooms: ";
                //for(int i = 0; i < used_rooms.size(); ++i) cout << used_rooms[i] << ' ';
                //cout << endl;
                propagate_restrictions(day_of_projection, reproduced_together, film);//won't allow future films to be reproduced in the same day in case they are incompatible
                used[film] = true;
                used_rooms[day_of_projection] += 1;
                perm[k] = { film, day_of_projection };
                optimal_billboard_schedule(k + 1, Inc, perm, used, reproduced_together,
                                           used_rooms, current_festival);
                used_rooms[day_of_projection] -= 1;
                used[film] = false;
            }
        }
    } else return;
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
void read_incompatibilities()
{
    cin >> num_preferences;
    Inc = MI(num_films);
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

void read_data()
{
    read_films();
    read_incompatibilities();
    read_cinema_rooms();
}

int main()
{
    read_data();
    vector<fd> perm(num_films);
    vector<bool> used(num_films, false);
    vector<vector<bool>> reproduced_together;
    reproduced_together.push_back(vector<bool>(num_films, false)); //we'll avoid this line of the vector

    vector<int> used_rooms;
    used_rooms.push_back(0);
    optimal_billboard_schedule(0, Inc, perm, used, reproduced_together, used_rooms, 0); //no faria falta passar com a paràmetre alguns atributs
    print_projection();
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
