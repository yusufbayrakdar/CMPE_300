/*
*   Algorithm Analaysis Project
*   Student Name: Yusuf Sabri Bayrakdar
*   Student Number: 2016400378
*   Compile Status: Compiling
*   Program Status: Working
*   To compile  =>  mpic++ -o  square square.cc
*   To run      =>  mpiexec -n  5 ./square lena200_noisy.txt output.txt 0.4 0.15
*   To see the image    => python text_to_image.py output.txt output.jpg
*/

#include "mpi.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <cmath>
#define size_matris 200 //Matrix size
#define Times 500000 //Iteration number
using namespace std;
//mySplit function take a line as parameter and split it in 1 or -1 integer value.It stores these digits in a vector and return this vector.I call this function for every line.
vector<int> mySplit(string line){
    vector<int> values;//This vector will be stored integers (1 or -1) converted string to integer line by line
    while(line.find(" ")!=string::npos){//In this loop I convert the string to int and push it in values vector.
        string atom=line.substr (0,line.find(" ")); 
        line=line.substr(line.find(" ")+1);
        values.push_back(atoi( atom.c_str() ));
    }
    return values;//Return the vector stores pixel values as 1 or -1 
}

/*monte_carlo function takes for parameter
*   oldValue=first read value of random pixel
*   newValue=last value of random pixel
*   env     =the sum of environment pixe
*   B       =beta value
*   Y       =gama value
*/
double monte_carlo(int oldValue,int newValue,int env,double B,double Y){
    double calculate=-2*Y*oldValue*newValue-2*B*newValue*env;
    double ex=exp(calculate);
    return ex;
}

int main(int argc, char **argv)
{
    int rank, size;
    int msgData=0;
    double pi;
    sscanf(argv[4],"%lf",&pi);//receive the value of pi from user
    double Y=log((1-pi)/pi)/2;//calculate pi value
    double B;
    sscanf(argv[3],"%lf",&B);//receive beta value
    bool isBorder=true;
    
    MPI::Init();
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();
    int servant_processes=size-1;
    int territorySize=sqrt(size_matris*size_matris/servant_processes);//Define each territory size in one dimention
    int lineSize=0;
    int same_Coloumn=size_matris/territorySize;
    int process_per_line=sqrt(servant_processes);
    /******************** Master Process *********************/
    if(rank==0){
        ifstream file(argv[1]);
        int x_axis=0;
        int y_axis=0;
        int input[size_matris][size_matris];
        int output[size_matris][size_matris];
        //fill input list
        for(string line; getline( file, line ); ){
            vector<int> lineValues = mySplit(line);
            for(x_axis=0;x_axis<lineValues.size();x_axis++){
                input[x_axis][y_axis]=lineValues[x_axis];
            }
            lineSize=lineValues.size();
            y_axis++;
        }
        //Divides the input into equal parts as square and sends them to servants
        x_axis=0;y_axis=0;
        for(int process=1;process<size;process++){
            if(x_axis>=lineSize){
                x_axis=0;
                y_axis+=territorySize;
            }
            if(x_axis<lineSize){
                for(int y=y_axis;y<y_axis+territorySize;y++){
                    for(int x=x_axis;x<x_axis+territorySize;x++){
                        MPI_Send(&input[x][y], 1, MPI_INT, process, 0, MPI_COMM_WORLD);
                    }
                }
                x_axis+=territorySize;
            }
        }
        //Master receive from slaves
        for(int process=1;process<size;process++){
            if(x_axis>=lineSize){
                x_axis=0;
                y_axis+=territorySize;
            }
            if(x_axis<lineSize){
                for(int y=y_axis;y<y_axis+territorySize;y++){
                    for(int x=x_axis;x<x_axis+territorySize;x++){
                        MPI_Recv(&msgData, 1, MPI_INT, process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        output[x][y]=msgData;
                    }
                }
                x_axis+=territorySize;
            }
        }
        //Master writes values to the output.txt
        ofstream out (argv[2]);
        for(int col=0;col<size_matris;col++){
            for(int row=0;row<size_matris;row++){
                if(output[row][col]!=0){
                    out<<output[row][col]<<" ";
                }
            }
            out<<endl;
        }
    }
    /*********************** Master Process End *************************/
    int msgInput[territorySize+2][territorySize+2];//This array is for storing the first values of pixels
    int msgCopy[territorySize+2][territorySize+2];//This array is for storing the latest
    //Fill the matrixes with 0 to clear default values
    for(int row = 0; row <territorySize+2; row++){
        for(int column = 0; column < territorySize+2; column++){
            msgInput[row][column]=0;
        }
    }
    //Begin to chat with neighbors
    int pixels=territorySize*size_matris;
    for(int process=1;process<size;process++){
        if(rank==process){
            //Receive input from master process
            for(int j=0;j<territorySize;j++){
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[j+1][i+1]=msgData;
                }
            }
            //msgInput 2D array is our old values & msgCopy 2D array is our new values
            for(int row = 0; row <territorySize+2; row++){//Z=X.copy()
                for(int column = 0; column < size_matris; column++){
                    msgCopy[row][column]=msgInput[row][column];
                }
            }
            for(int i=0;i<Times/servant_processes;i++){//Iterate "Times/number of servant processes" times
            
            int random=rand()%pixels;//Choose a random number to find a random pixel
            int y=random%size_matris+1;//Cnvert the random number to y coordinate of the random pixel
            int x=random/size_matris+1;//Cnvert the random number to x coordinate of the random pixel
            if (x==1||y==1||x==territorySize||y==territorySize) {//If the random pixel is a border pixel then let slaves send/receive mesagge to each other
                isBorder=true;
            }
            else//Otherwise, if the random is not a border pixel then dont let the slaves to send/receive messages to each other
                isBorder=false;
            if (isBorder) {
                if(process*territorySize%size_matris==territorySize){//If process is in leftmost column
                for(int x_axis = 0; x_axis < territorySize; x_axis++)
                {
                    int msgData=msgCopy[x_axis+1][territorySize];
                    MPI_Send(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD);
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++)
                {
                    MPI_Recv(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[x_axis+1][territorySize+1]=msgData;
                }
            }
            else if(process*territorySize%size_matris==0){//If process is in rightmost column
                for(int x_axis = 0; x_axis < territorySize; x_axis++){//send left
                    msgData=msgCopy[x_axis+1][1];
                    MPI_Send(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD);
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++){//receive left
                    MPI_Recv(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[x_axis+1][0] = msgData;
                }
            }
            else{//If process is neither left most nor rightmost
                
                for(int x_axis = 0; x_axis < territorySize; x_axis++)
                {
                    int msgData=msgCopy[x_axis+1][territorySize];
                    MPI_Send(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD);
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++){
                    msgData=msgCopy[x_axis+1][1];
                    MPI_Send(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD);
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++)
                {
                    MPI_Recv(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[x_axis+1][territorySize+1]=msgData;
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[x_axis+1][0] = msgData;
                }

            }
            //Send & Receive upward & bottomward
            if(process<=process_per_line){//If process is in first row
                for(int i=0;i<territorySize;i++){//send bottom
                    msgData=msgCopy[territorySize][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){//receive bottom
                    MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[territorySize+1][i+1]=msgData;
                }
            }
            else if(process>servant_processes-process_per_line){//If process is in last Row
                for(int i=0;i<territorySize;i++){//Send top
                    msgData=msgCopy[1][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){//Receive top
                    MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[0][i+1]=msgData;
                }

            }
            if(process>process_per_line&&process<=servant_processes-process_per_line){//If process is in between first and last rows

                for(int i=0;i<territorySize;i++){//Send bottom
                    msgData=msgCopy[territorySize][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){//Send top
                    msgData=msgCopy[1][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){//Receive bottom
                    MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[territorySize+1][i+1]=msgData;
                }
                for(int i=0;i<territorySize;i++){//Receive top
                    MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[0][i+1]=msgData;
                }

            }

            if(process<=process_per_line&&process*territorySize%size_matris==territorySize){//If process is in the left-top corner
                msgData=msgCopy[territorySize][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][territorySize+1]=msgData;
            }
            else if(process<=process_per_line&&process*territorySize%size_matris==0){//If process is in the right-top corner
                msgData=msgCopy[territorySize][1];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][0]=msgData;
            }
            else if(process>servant_processes-process_per_line&&process*territorySize%size_matris==territorySize){//If process is in left-bottom
                msgData=msgCopy[1][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][territorySize+1]=msgData;
            }
            else if(process>servant_processes-process_per_line&&process*territorySize%size_matris==0){//If process is in right-bottom
                msgData=msgCopy[1][1];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][0]=msgData;
            }
            else if (process<=process_per_line) {//If process is first row but neither left-top corner nor right-top corner then just send/receive left-bottom&right-bottom corner
                msgData=msgCopy[territorySize][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][territorySize+1]=msgData;

                msgData=msgCopy[territorySize][1];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][0]=msgData;
            }
            else 
            if (process>servant_processes-process_per_line) {//If process is last row but neither left-bottom corner nor right-bottom corner then just send/receive left-top&right-top corner
                msgData=msgCopy[1][territorySize];//right-top corner
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][territorySize+1]=msgData;

                msgData=msgCopy[1][1];//left-top corner
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][0]=msgData;
            }
            else 
            if (process*territorySize%size_matris==territorySize) {//If process is leftmost column but neither left-top corner nor left-bottom corner then just send/receive right-bottom&right-top corner
                msgData=msgCopy[territorySize][territorySize];//right-top
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][territorySize+1]=msgData;

                msgData=msgCopy[1][territorySize];//right-bottom
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][territorySize+1]=msgData;
            }
            else 
            if (process*territorySize%size_matris==0) {//If process is rightmost column but neither right-top corner nor right-bottom corner then just send/receive left-bottom&left-top corner
                msgData=msgCopy[territorySize][1];//left-top
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][0]=msgData;

                msgData=msgCopy[1][1];//left-bottom
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][0]=msgData;
            }
            
            else//If process is in the middle and it has neighbours at every side
            {
                msgData=msgCopy[territorySize][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][territorySize+1]=msgData;

                msgData=msgCopy[territorySize][1];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][0]=msgData;

                msgData=msgCopy[1][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][territorySize+1]=msgData;

                msgData=msgCopy[1][1];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][0]=msgData;
            }
            }
            
            
            
            //Calculating & Denoising
            int up=msgCopy[x-1][y];//value of top pixel
            int down=msgCopy[x+1][y];//value of bottom pixel
            int left=msgCopy[x][y-1];//value of left pixel 
            int right=msgCopy[x][y+1];//value of right pixel
            int left_top=msgCopy[x-1][y-1];//value of left-top pixel
            int right_top=msgCopy[x-1][y+1];//value of right-top pixel 
            int left_bottom=msgCopy[x+1][y-1];//value of left-bottom pixel
            int right_bottom=msgCopy[x+1][y+1];//value of right-bottom pixel
            
            int env;
            env=up+down+left+right+left_top+right_top+left_bottom+right_bottom;//sum of environment of random pixel
            double ex=monte_carlo(msgInput[x][y],msgCopy[x][y],env,B,Y);//calculate the posterior distribution
            if(ex>1)ex=1;//If the posterior distribution is greater than 1 equal it 1
            double treshold=(rand()%9+1)*0.1;//choose random threshold number between 0 and 1
            
            if (treshold<ex) {//If threshold less than the posterior distribution than flip
                msgCopy[x][y]=-1*msgCopy[x][y];
            }
            }
            for(int i = 0; i <territorySize; i++)//Send all values to master
            {   
                for(int j = 0; j < territorySize; j++)
                {
                    msgData=msgCopy[i+1][j+1];
                    MPI_Send(&msgData,1,MPI_INT,0,0,MPI_COMM_WORLD);
                }
            }
        }
    }
    MPI::Finalize();

    return 0;
}
