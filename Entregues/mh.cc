/* CODE INFORMATION ------------------------------------------------------------------------------- 
The randomness of this metaheuristic is based on the idea of splitting the vector where information
about films is stored. The groups of films with similar number of restrictions are what we call: 
blocks.
Each block has a conditioned probability of being selected to start the metaheuristic from there. 
Once this is done, we have implemented a randomized greedy algorithm as in the GRASP metaheuristic.
------------------------------------------------------------------------------------------------ */

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

using namespace std;
using MI = vector<vector<int>>; //matrix of integers
using ALI = vector<vector<int>>; //adjacency list of integers
using fd = pair<int, int>; //represents a film and the day of its projection

/* GLOBAL VARIABLES ------------------------------------------------------------------------------- */
clock_t start = clock();
double elapsed_time;

const int& UNDEF = -1;

int num_films, num_preferences, num_rooms;
int min_required_lenght, shortest_festival = INT_MAX; //best configutation so far (initially infinite)
int num_films_without_restr = UNDEF;
int num_unequal_restr; 

vector<string> billboard, cinema_rooms;
ALI Inc; // stores the Incompatibilities among films that can't be projected in the same day

map<string, int> filmindex; //stores the name of a film and its position in the "billboard" vector

string output_file;
/* ------------------------------------------------------------------------------------------------ */

//stores information about those films that have the same number of restrictions. The criteria of
//selection during the heuristic will be determined by "num_restr"
struct Score {
    int num_restr;
    int cardinal;
    int first_idx; //position of the first film that fulfills "num_restr" in "films_by_restr" vector
};

//A block is a set of Scores that have similar number of restrictions. Each block will have a different
//probability according to "num_restr", being the first block the most likelihood to be chosen.
struct BlockInfo {
    int starting;
    double cdf; //cummulative distribution function
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

/*
Returns:
    A vector containing information about films with the same "num_restr". It also modifies the global 
    variable that counts those films without restrictions, that are free to be projected anywhere. 
    Films that achieve "num_restr" = 0 are not added in the vector.
*/
vector<Score> agrupate_by_restrictions(const vector<pair<int, int>>& films_by_restr){
    //remind that "films_by_restr" contains (<film, num_restr>) ordered by number of restrictions
    num_films_without_restr = 0;
    vector<Score> films_scores;
    films_scores.push_back({ films_by_restr[0].second, 1, 0}); //(<num_rest, cardinal, first_idx>)
    for (int i = 1; i < num_films; ++i) {
        if(films_by_restr[i].second != 0){ //omit none restricted films 
            if (films_by_restr[i].second == films_scores.back().num_restr) { //"num_restr" already existing
                ++films_scores.back().cardinal;
            } else {
                films_scores.push_back({ films_by_restr[i].second, 1, i});
            }
        }else{
            ++num_films_without_restr;
        }
    }
    return films_scores;
}

/*
Returns:
    A vector containing the starting index and the probability of each block. For this heuristic it was 
    chosen to split the vector films_scores in 5 blocks at maximum, where a block represents a set of Scores,
    that is to say, groups of films having the same num_restr. As there will not always exist 5 films having 
    diferent num_restr, the vector cuttings, could have less than 5 separations of blocks. To associate a 
    probability to each block, a vector of probabilities is defined, which contains decreasing probabilities 
    to be assigned to blocks.
*/
vector<BlockInfo> blocks_probabilities(int fs_size)
{
    vector<BlockInfo> cuttings;
    cuttings.push_back({0, 0.0}); //beginning of the first block
    double limit_block = (fs_size > 5 ? (fs_size / 5.0) : 1.0); 
    //the first block contains fs_size/5.0 elements when there are 5 or more diferent number of restrictions
    
    double denom = 32 - pow(2, (4 - (fs_size > 5 ? 4 : fs_size - 1))); 
    //sum of the powers of 2 conditioned to the number of blocks the vector will have
    vector<double> probs = {16/denom, 8/denom, 4/denom, 2/denom, 1/denom}; 
    //when fs_size < 5, last elements will be omitted

    double prob_accumulated = probs[0]; 
    for (int j = 0, block = 1; j < fs_size; ++j) { //associate a probability to each block
        if (int(cuttings.size()) < 5 and j >= int(limit_block)) { //next block has been reached
            cuttings.push_back({ int(limit_block), prob_accumulated });
            prob_accumulated += probs[block];
            limit_block += fs_size / 5.0; 
            ++block;
        }
    }
    cuttings.push_back({ fs_size, 1 }); //determines the end of the vector. Helpful during the generation
    return cuttings;
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

/*
Returns:
    A semi-random day to project a given film. During the selection process, it generates a vector of all 
    the candidate days, checking up possible forbiddings. At the beggining of the execution of the program, 
    not all days are available for projection in order to narrow down the first festival schedule duration.
    Once the program has been runnning for 15 seconds, all the days on the vector are available to be 
    choosen to introduce even more randomness to the heuristic. 
*/
int choose_day_from_RCL(const MI& prohibitions_per_day, const vector<int>& occupied_rooms, int film, 
                        int& lenght_festival){
    vector<int> candidates;
    for(int day = 1; day <= lenght_festival; ++day){
        //no prohibitions and enough free rooms 
        if(prohibitions_per_day[day][film] == 0 and occupied_rooms[day] < num_rooms) 
            candidates.push_back(day);
    }
    if (candidates.size() == 0){ //none compatible days found to project the films, an extra day is needed
        ++lenght_festival;
        return lenght_festival;
    }else{
        int restricted_days = candidates.size()/2; 
        int elapsed_time2 = (clock() - start) / (double)CLOCKS_PER_SEC;
        if(elapsed_time2 > 15) restricted_days = 0; 
        int random_restricted_candidate = rand() % (candidates.size() - restricted_days);
        return candidates[random_restricted_candidate]; 
    }
}

/*
Args:
    - modifier: takes value 1 for adding prohibitions in the "day" where "film" is projected
                takes value -1 analogously for removing prohibitions
*/
void propagate_restrictions(MI& prohibitions_per_day, int day, int film, int modifier)
{
    for (const int& banned_film : Inc[film])
        prohibitions_per_day[day][banned_film] += modifier;
}

/*
Works on:
    Actualizes both "index_at_start" and "amount_restr" in order to keep generating for the following block.
    As the generation goes from one block to another, accesses to unexisting vector positions are checked.
*/
void idx_increment(const vector<BlockInfo>& cuttings, bool& first_block, int& index_at_start, 
                   int& amount_restr){
    if(first_block){
        index_at_start = 0; //return to beginning
        first_block = false;
    }else{
        ++index_at_start;
        if(cuttings[index_at_start].starting == num_unequal_restr) //end of the vector has been reached 
            index_at_start = 0;
    }
    amount_restr = cuttings[index_at_start].starting;
}

/*
Works on:
    Fills the vector of the planning (perm), with those films that are not forbidden with other films
    and that by consequence can be projected in any room that is free. It also modifies the length_festival
    whenever extra days are required to project the films.
*/
void add_zero_films(const vector<pair<int, int>>& films_by_restr, vector<int>& occupied_rooms, 
                    vector<fd>& perm, int& length_festival) 
{
    int day = 1;
    int fbr_idx_film = int(films_by_restr.size()) - num_films_without_restr; 
    //initially takes the index of first film with "num_restr = 0"

    int zero_film = films_by_restr.size() - 1; //starts from the very last film in the vector "films_by_restr"
    while(fbr_idx_film < int(films_by_restr.size())) { //loop for all zero films
        if (occupied_rooms[day] < num_rooms) { 
            ++fbr_idx_film;
            ++occupied_rooms[day];
            perm.push_back({films_by_restr[zero_film].first, day});
            --zero_film;
        } else ++day; //could not be projected
    }
    if (day > length_festival) length_festival = day;
}

/*
Args:
    - length_festival: current days spent on the festival
Works on:
    Given the index determining the place to start the heuristic, it computes a schedule for the festival. This algorithm 
    is based on a GRASP method, as it is implemented as a randomized greedy, in where the Restricted Candidate List are the 
    available days to project a film. Once the films of the first block are assigned, the algorithm jumps to the beginning 
    of the vector "cuttings" and then keeps iterating for every pending block. The reason why the algorithm needs two loops 
    is because the vector "films_scores" contains information about the films having same "num_restr" but those films are in 
    the vector "films_by_restr". 
    For each existing film, the day in where it will be projected is randomly selected from the available restricted candidates.
*/
void metaheuristic(const vector<pair<int, int>>& films_by_rest, const vector<Score>& films_scores,
                       const vector<BlockInfo>& cuttings,  int index_at_start, int amount_restr, int lenght_festival)
{
    int films_being_projected = 0;
    bool first_block = true; 
    vector<bool> used (num_films);
    vector<fd> perm(num_films - num_films_without_restr); //contains a partial solution (<film, day_of_projection>)
    MI prohibitions_per_day(num_films + 1, vector<int>(num_films, 0));
    //rows simulate days of projection; columns are the films' index; an entry of the matrix contains
    //the amount of retrictions of a film to be projected on that day
    vector<int> occupied_rooms(num_films + 1, 0);
    //occupied_rooms[k] = number of projecting rooms at day 'k'
    //the two elements above skip the first row/position to avoid problems with zero-indexation

    //SOLUTION CONSTRUCTION
    while (films_being_projected < int(perm.size()) and lenght_festival < shortest_festival) {
        //the two loops are linear in regard of the total number of films
        for (int i = amount_restr; i < cuttings[index_at_start + 1].starting; ++i) { //one block
            const Score& group_same_restr = films_scores[i]; 
            for (int k = group_same_restr.first_idx; k < group_same_restr.first_idx + group_same_restr.cardinal; ++k) { 
                if(not used[k]){
                    used[k] = true;
                    const int film = films_by_rest[k].first;
                    int day = choose_day_from_RCL(prohibitions_per_day, occupied_rooms, film, lenght_festival);
                    perm[k] = {film, day};
                    ++occupied_rooms[day];
                    propagate_restrictions(prohibitions_per_day, day, film, 1);
                    ++films_being_projected;
                }
            }
        }
        idx_increment(cuttings, first_block, index_at_start, amount_restr); //prevent ilegal accesses to unexisting blocks
    }
    add_zero_films(films_by_rest, occupied_rooms, perm, lenght_festival);

    //SOLUTION IMPROVEMENT
    if (lenght_festival < shortest_festival) { 
        shortest_festival = lenght_festival;
        print_projection(perm);
    }
}

double set_random_seed()
{
    return (double)rand() / (double)RAND_MAX;
}

/*
Works on:
    Given a random probability "films_seed", updates both "index_at_start" and "amount_restr" variables 
    that will determine where the random probability has fallen and indeed start generating by that block.
    While the first one will be the index of the starting block, the second one is the value taken on 
    that position.
*/
void position_to_start_at(const vector<BlockInfo>& cuttings, double film_seed, int& index_at_start, 
                          int& amount_restr){                       
    int i_block = 1;
    //checking up if the cumulative probability outstrip the random seed
    while(i_block != cuttings.size() and film_seed > cuttings[i_block].cdf){ ++i_block;}
    index_at_start = i_block - 1; 
    amount_restr = cuttings[index_at_start].starting;
}

void optimal_billboard_schedule()
{
    srand(time(NULL)); //set a seed according to the current time
    vector<pair<int, int>> films_by_restr = order_films_by_restr();
    vector<Score> films_scores = agrupate_by_restrictions(films_by_restr); 
    int fs_size = int(films_scores.size()); //the size of "films_scores" is the number of different "num_restr" 
    num_unequal_restr = fs_size;

    vector<BlockInfo> cuttings = blocks_probabilities(fs_size);
    min_required_lenght = ceil(float(num_films) / float(num_rooms));
    
    while(shortest_festival != min_required_lenght){ //finishes when the optimal solution is found
        double film_seed = set_random_seed(); 
        int index_at_start = UNDEF; //block's index to start the heuristic
        int amount_restr = UNDEF; //value taken by the first element of the block (amount of forbiddings)
        position_to_start_at(cuttings, film_seed, index_at_start, amount_restr); 
        metaheuristic(films_by_restr, films_scores, cuttings, index_at_start, amount_restr, 1);
    }
}

int main(int argc, char* argv[])
{
    ifstream input(argv[1]);
    read_data(input);
    input.close();
    output_file = string(argv[2]);
    optimal_billboard_schedule();
}
