#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <math.h>
#include <stdlib.h> //from random
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

const int& UNDEF = -1; //l'usarem per la última posició de Block Info
int num_films, num_preferences, num_rooms;
int shortest_festival = INT_MAX; //simulates infinite value
int num_films_without_restr = 0; //initially 0
int min_required_lenght, last_cut;

vector<string> billboard, cinema_rooms;
ALI Inc; // stores the Incompatibilities among films that can't be projected in the same day

map<string, int> filmindex; //stores the name of a film and its position in the "billboard" vector

string output_file;
/* ----------------------------------------------------- */

struct Score { 
    int num_restr;
    int cardinal;
    int first_idx; //determina l'index de la last_cutrimera peli del vector de restriccions que té aquest num_restr
};

struct BlockInfo {
    int starting;
    double cdf;
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
vector<pair<int, int>> order_films_by_restr()
{
    vector<pair<int, int>> films_by_restr;
    films_by_restr = vector<pair<int, int>>(num_films);
    for (int i = 0; i < Inc.size(); ++i) {
        films_by_restr[i] = { i, Inc[i].size() };
    }
    sort(films_by_restr.begin(), films_by_restr.end(), compare_by_restr);
    return films_by_restr;
}

void print_projection(const vector<fd>& perm)
{
    ofstream output(output_file);
    output << setprecision(1) << fixed;
    elapsed_time = (clock() - start) / (double)CLOCKS_PER_SEC;

    output << elapsed_time << endl
           << shortest_festival << endl;
    cout << shortest_festival << endl;

    // looks across the vector "perm" to print those films that match with the day. Cinema rooms
    // are choosen arbitrarily
    for (int day = 1; day <= num_films; ++day) {
        int r = 0; //room in where a film will be projected
        for (int f = 0; f < num_films; ++f) { //f = films
            if (perm[f].second == day) {
                output << billboard[perm[f].first] << " " << perm[f].second
                       << " " << cinema_rooms[r] << endl;
                ++r;
            }
        }
    }
    output.close();
}

//define vector of scores and modifies global variable that counts those films without restrictions that are free to be projected anywhere
vector<Score> agrupate_by_restrictions(const vector<pair<int, int>>& films_by_restr){
    vector<Score> films_scores;
    films_scores.push_back({ films_by_restr[0].second, 1}); //afegim nombre pelis amb que està prohibida, quantes pelis d'aquest tipus
    for (int j = 1; j < num_films; ++j) {
        if(films_by_restr[j].second != 0){ //discriminem els que no tenen restriccions
            if (films_by_restr[j].second == films_scores.back().num_restr) { //ja estava afegit un element amb k restriccions
                ++films_scores.back().cardinal;
            } else {
                films_scores.push_back({ films_by_restr[j].second, 1, j}); //guardem la posició també
            }
        }else{
            ++num_films_without_restr;
        }
    }
    return films_scores;
}

vector<BlockInfo> blocks_probabilities(const vector<pair<int, int>>& films_by_restr, int s)
{
    vector<BlockInfo> cuttings;
    cuttings.push_back({0, 0.0}); //representa el 1r bloc, com a mínim sempre hi haurà un
    double limit_block = (s > 5 ? (s / 5) : 1); //el límit del primer block comença en s/5
    double denom = 32 - pow(2, (4 - (s > 5 ? 4 : s - 1)));//
    vector<double> probs = {16/denom, 8/denom, 4/denom, 2/denom, 1/denom}; //en cas que hi hagin menys de 5 blocs, els últims tindran una probabilitat però no els usarem, sinó PROB > 1
    double prob_accumulated = probs[0]; //ini
    for (int j = 0, block = 1; j < s; ++j) { //associem una probabilitat als 5 blocs del vector
        if (int(cuttings.size()) < 5 and j >= int(limit_block)) { //hem passat al següent block, restringim < 4 pq si el bloc coincideix al final, afegeix 5 separadors
            cuttings.push_back({ int(limit_block), prob_accumulated }); //guardem iterador a la posició on hi ha un break. Ha d'haver 4 o menys sempre, ja que volem que com a màxim hi hagi 5 blocs diferents
            prob_accumulated += probs[block];
            limit_block += s / 5; //si s/5 és 0 vol dir que tenim menys de 5 elements i cada block estarà format per un sol element
            ++block;
        }
    }
    cuttings.push_back({ s, 1 });
    return cuttings;
}


double set_random_seed()
{
    return (double)rand() / (double)RAND_MAX;
}

//donat una probabilitat aleatòria retorna l'iterador en el vector que determina quin és el primer element per generar a partir d'allà
int position_to_start_at(const vector<BlockInfo>& v, const vector<Score>& films_scores, double film_seed, int& index_at_start){
    int i_block = 1;
    while(i_block != v.size() and film_seed > v[i_block].cdf){  //la posició i_block = 0 no ens fa falta
        ++i_block;  
    }
    index_at_start = i_block - 1;
    return v[index_at_start].starting;
}

int choose_day_from_RCL(const MI& prohibitions_per_day, const vector<int>& occupied_rooms, int film, int& lenght_festival){
    vector<int> candidates;
    for(int day = 1; day <= lenght_festival; ++day){
        if(prohibitions_per_day[day][film] == 0 and occupied_rooms[day] < num_rooms) candidates.push_back(day);
    }
    if (candidates.size() == 0){ 
        ++lenght_festival;
        return lenght_festival;
    }else{
        int restricted_days = candidates.size()/2;
        int elapsed_time2 = (clock() - start) / (double)CLOCKS_PER_SEC; 
        if(elapsed_time2 > 15) restricted_days = 0; //Si han passat 15 segons, generem el dia random sense tenir en compte la meitat millors
        int random_restricted_candidate = rand() % (candidates.size() - restricted_days);
        return candidates[random_restricted_candidate]; //retorna una dia aleatori en què posar la peli sobre els que teníem disponibles
    }
}

/*
Args:
    - modifier = takes value 1 for adding prohibitions in the "day" where "film" is projected
                 takes value -1 analogously for removing prohibitions
*/
void propagate_restrictions(int day, MI& prohibitions_per_day, int film, int modifier)
{
    for (const int& banned_film : Inc[film])
        prohibitions_per_day[day][banned_film] += modifier;
}

void idx_increment(const vector<BlockInfo>& cuttings, bool& first_block, int& index_at_start, int& start_point){
    if(first_block){
            index_at_start = 0;
            first_block = false;
    }else{
        index_at_start = index_at_start + 1;
        if(cuttings[index_at_start].starting == last_cut) index_at_start = 0;//hem arribat al final del vector
    }        
    start_point = cuttings[index_at_start].starting;
}

//referenciem legth_festival per si al final el canviem 
void add_zero_films(const vector<pair<int, int>>& films_by_rest, vector<int>& occupied_rooms, vector<fd>& perm, 
                    int& length_festival) //afegir les pelis que no tenen restriccions i que per tant poden anar en qualsevol dia
{
    int fbr_idx_film = int(films_by_rest.size()) - num_films_without_restr;
    int day = 1;
    int i = films_by_rest.size() - 1;
    while(fbr_idx_film < int(films_by_rest.size())) {
        if (occupied_rooms[day] < num_rooms) {
            ++fbr_idx_film;
            ++occupied_rooms[day];
            perm.push_back({films_by_rest[i].first, day});
            --i; //Mirar si es pot fer sense declarar una nova var.
        } else {
            ++day;
        }
    }
    if (day > length_festival) length_festival = day;
}

//fer un greedy normal, però
void randomized_greedy(const vector<pair<int, int>>& films_by_rest, const vector<Score>& films_scores, 
                       const vector<BlockInfo>& cuttings,  int index_at_start, int lenght_festival, int start_point)
{
    //mirar si surt a compte declarar vector perm dins
    int films_being_projected = 0;
    bool first_block = true;
    vector<bool> used (num_films);
    vector<fd> perm(num_films - num_films_without_restr); //contains a partial solution
    MI prohibitions_per_day(num_films + 1, vector<int>(num_films, 0));
    //rows simulate days of projection; columns are the films' index; an entry of the matrix contains
    //the amount of retrictions of a film to be projected on that day
    vector<int> occupied_rooms(num_films + 1, 0);
    //occupied_rooms[k] = number of projecting rooms at day 'k'
    //the two elements above skip the first row/position to avoid problems with zero-indexation

    while (films_being_projected < num_films - num_films_without_restr and lenght_festival < shortest_festival){
        for (int i = start_point; i < cuttings[index_at_start + 1].starting; ++i) { //lineal respecte nombre de pelis
            const Score& R = films_scores[i];
            for (int k = R.first_idx; k < R.first_idx + R.cardinal; ++k) {
                if(not used[k]){
                    used[k] = true;
                    const int film = films_by_rest[k].first;
                    int day = choose_day_from_RCL(prohibitions_per_day, occupied_rooms, film, lenght_festival);
                    perm[k] = {film, day}; 
                    ++occupied_rooms[day];
                    propagate_restrictions(day, prohibitions_per_day, film, 1);
                    ++films_being_projected;
                }
            } 
        }
        idx_increment(cuttings, first_block, index_at_start, start_point);
    }
    add_zero_films(films_by_rest, occupied_rooms, perm, lenght_festival);
    if (lenght_festival < shortest_festival) { //solution improvement
        shortest_festival = lenght_festival;
        print_projection(perm);
    }
}

void optimal_billboard_schedule()
{
    srand(time(NULL)); //set a seed according to the current time
    //primer element = nombre de restriccions, segon element = nombre d'elements que tenen el nombre de restriccions igual a la clau
    vector<pair<int, int>> films_by_restr = order_films_by_restr();
    vector<Score> films_scores = agrupate_by_restrictions(films_by_restr); //vector mida = diccionari = nombre de blocs de restriccions diferents que hi ha
    int s = int(films_scores.size());
    last_cut = s;
    vector<BlockInfo> cuttings = blocks_probabilities(films_by_restr, s);
    min_required_lenght = ceil(float(num_films) / float(num_rooms));
    while(shortest_festival != min_required_lenght){ //funciona com un while True, l'únic que en cas de trobar la òptima para
        double film_seed = set_random_seed(); //cout << "La llavor és: " << film_seed << endl;
        int index_at_start = UNDEF;
        int start_point = position_to_start_at(cuttings, films_scores, film_seed, index_at_start); //és l'índex del vector films_scores on comença el bloc on començarem a generar
        randomized_greedy(films_by_restr, films_scores, cuttings, index_at_start, 1, start_point);
    }
}

int main(int argc, char* argv[])
{
    ifstream input(argv[1]); //read from file given
    read_data(input);
    input.close();
    output_file = string(argv[2]);
    optimal_billboard_schedule();
}
