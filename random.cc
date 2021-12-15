#include <iostream>
#include <random>

using namespace std;

int main(){
    srand(time(NULL)); //set a seed according to the current time
    for(int i = 0; i < 100; ++i){
    cout << rand() % 10  << endl;
    }
}