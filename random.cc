#include <iostream>
#include <random>

using namespace std;

int main(){
    srand(time(NULL)); //set a seed according to the current time
    for(int i = 0; i < 100; ++i){
    cout << rand() % 10;
    }
    double x;
    x = 7 / 3.0;
    cout << endl << endl << x; 
}