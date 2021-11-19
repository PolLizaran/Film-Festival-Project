#include <iostream>
#include <limits>
#include <map>
//#include <math.h>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

using namespace std;
using MB = vector<vector<bool>>; //Matrix for the Incompatibilities
using fd = pair<int, int>; //film and its day

// GLOBAL VARIABLES
/* ----------------------------------------------------- */
clock_t start = clock(); //initialization of the elapsed time
double duration;
const int& UNDEF = -1;

int num_films, num_preferences, num_rooms, shortest_festival = INT_MAX;
vector<fd> best_perm; //global variables were the optimal and partial solutions are stored
//vector<bool> used;
vector<string> billboard, cinema_rooms;
map<string, int> filmindex; //dictionatt to store the film and its index in the vector so that we don't have to look across the vector
MB Inc;
/* ----------------------------------------------------- */

void print_projection()
{
    //exectution time stops
    duration = (clock() - start) / (double)CLOCKS_PER_SEC;
    cout << duration << endl
         << shortest_festival << endl;
    for (int i = 0; i < best_perm.size(); ++i)
        cout << best_perm[i].first << " ";
    cout << endl;
    for (int j = 0; j < best_perm.size(); ++j) {
        int x = j % num_rooms; //variable that helps us to determine in which days the fils are reproduced
        cout << billboard[best_perm[j].first] << " " << best_perm[j].second << " "
             << cinema_rooms[x] << endl;
    }
}

//checks whether the film can be projected in the current cinema room
bool legal_projection(int k, int current_film, const MB& Inc,
                      const vector<fd>& perm)
{ //passem per copia pq no volem que els canvis es guardin
    //if(perm[k].second != current_film) return true;
    //return not reproduced_together[k];
}

//shortest_festival is the best configutation at the moment, at the end we'll return this value
void optimal_billboard_schedule(int current_film, int k, const MB& Inc, vector<fd>& perm,
    vector<bool>& used, int current_festival)
{
    //if(k > 0 and not legal_projection(k - 1, current_film, Inc, perm)) //hem de tirar tants cops enrere en la permutacio fins que arribem a una divisio modul exacta
    //aixo significara que estem en el mateix dia
    //return;
    if (k % num_rooms == 0){
        ++current_festival;
    }
    //day changes) ++current_festival; //all rooms are assigned, so the festival lasts 1 day more
    //if(current_festival > shortest_festival) return; // ens hem passat
    //as k is initially 0, a room is addes, which doesn't affect as 0 rooms doesn't make sense

    //if(current_festival > num_films / num_rooms) return;
    if (k == perm.size()) { //all films have been placed
        if (current_festival < shortest_festival) {
            shortest_festival = current_festival;
            best_perm = perm;
            //print_projection(perm);
            //HEM DE REDIRIGIR LA SORTIDA EN COMPTES D'IMPRIMIR-LA
        }
    } else if (current_festival < shortest_festival) {
        for (int film = 0; film < num_films; ++film) { //loop for each film
            if (not used[film] and (perm[k].first == UNDEF or perm[current_film].second != current_film)) {
                used[film] = true;
                perm[k] = { film, current_festival };
                optimal_billboard_schedule(k, film, Inc, perm, used, current_festival);
                used[film] = false;
            }
        }
    }
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
    Inc = MB(num_films);
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
    vector<fd> perm(num_films, {UNDEF, UNDEF});
    vector<bool> used(num_films, false);
    optimal_billboard_schedule(0, 0, Inc, perm, used, 0); //no faria falta passar com a paràmetre alguns atributs
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
