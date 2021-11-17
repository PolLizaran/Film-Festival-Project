#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <limits>
#include <time.h>
#include <utility>

using namespace std;
using MB = vector<vector<bool>>; //Matrix for the Incompatibilities
using fd = pair<int, int>; //film and its day

// GLOBAL VARIABLES
/* ----------------------------------------------------- */
clock_t start = clock(); //initialization of the elapsed time
double duration;

int num_films, num_preferences, num_rooms, shortest_festival = INT_MAX;
vector<fd> best_perm, perm; //global variables were the optimal and partial solutions are stored
vector<bool> used;
vector<string> billboard, cinema_rooms;
map<string, int> filmindex; //dictionatt to store the film and its index in the vector so that we don't have to look across the vector
MB Inc;
/* ----------------------------------------------------- */

void print_projection(const vector<fd>& best_perm2){
    //exectution time stops
    duration = (clock() - start) / (double) CLOCKS_PER_SEC;
    cout << duration << endl
         << shortest_festival << endl;
    for(int j = 0; j < best_perm2.size(); ++j){
        int x = j % num_rooms; //variable that helps us to determine in which days the fils are reproduced
        cout << billboard[best_perm2[j].first] << " " << best_perm2[j].second << " "
             << cinema_rooms[x] << endl;
    }
}

//checks whether the film can be projected in the current cinema room
bool legal_projection(int k, int film, const MB& Inc, const vector<fd>& perm){ //passem per copia pq no volem que els canvis es guardin
    while(k % num_rooms != 0){
        if(Inc[perm[k].first][film]) return false; //they are incompatible
        --k;
    }
    return true;
}

//shortest_festival is the best configutation at the moment, at the end we'll return this value
void optimal_billboard_schedule(int k, const MB& Inc, vector<fd>& perm,
                                vector<bool>& used, int current_festival, int legal_films){
    if(current_festival > shortest_festival) return; // ens hem passat
    //as k is initially 0, a room is addes, which doesn't affect as 0 rooms doesn't make sense
    if(k % num_rooms == 0 or legal_films == 0){
        ++current_festival;
        legal_films = num_films - k;} //day changes) ++current_festival; //all rooms are assigned, so the festival lasts 1 day more
    if(k == perm.size()){ //all films have been placed
        if(current_festival < shortest_festival){
            shortest_festival = current_festival;
            best_perm = perm;
            print_projection(best_perm); //HEM DE REDIRIGIR LA SORTIDA EN COMPTES D'IMPRIMIR-LA
        }
    }

    for(int film = 0; film < perm.size(); ++film){ //loop for each film
        if(not used[film]){
            if(legal_projection(k, film, Inc, perm)){ //hem de tirar tants cops enrere en la permutacio fins que arribem a una divisio modul exacta
                //aixo significara que estem en el mateix dia
                used[film] = true;
                perm[k] = {film, current_festival};
                optimal_billboard_schedule(k + 1, Inc, perm, used, current_festival, legal_films - 1);
                used[film] = false;
            }else{
                optimal_billboard_schedule(k + 1, Inc, perm, used, current_festival, legal_films - 1);
            }
        }
    }
}

void read_films(){
    cin >> num_films;
    billboard = vector<string>(num_films);
    for(int i = 0; i < num_films; ++i){
        cin >> billboard[i];
        filmindex.insert({billboard[i], i});
    }
    //vectors that will be used later during the computation of the solution
    perm = vector<fd>(num_films);
    used = vector<bool>(num_films, false);
}

//pel·licules que no es poden projectar alhora
void read_incompatibilities(){
    cin >> num_preferences;
    Inc = MB(num_films, vector<bool>(num_films, false));
    for(uint i = 0; i < num_preferences; ++i) { //MIRAR SI HEM D'IMPLEMENTAR EN LLISTA O DICC
        string a, b;
        cin >> a >> b; //S'HA DE CANVIAR PER LLEGIR DES D'ARXIU
        Inc[filmindex[a]][filmindex[b]] = Inc[filmindex[b]][filmindex[a]] = true;
    }

}

void read_cinema_rooms(){
    cin >> num_rooms;
    cinema_rooms = vector<string>(num_rooms);
    for(uint i = 0; i < num_rooms; ++i) cin >> cinema_rooms[i];
}

void read_data(){
    read_films();
    read_incompatibilities();
    read_cinema_rooms();
}

int main(){
    read_data();
    optimal_billboard_schedule(0, Inc, perm, used, 0, num_films); //no faria falta passar com a paràmetre alguns atributs
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
