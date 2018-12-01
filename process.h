#include<iostream>
#include <fstream>
#include <string>

using namespace std;
class process {
public:
    int x_start;
    int x_end;
    int y_start;
    int y_end;
    int lineSize=x_end-x_start;
    int matris[lineSize][lineSize];
    int border_top[lineSize];
    int border_rigth[lineSize];
    int border_left[lineSize];
    int border_bottom[lineSize];
};
