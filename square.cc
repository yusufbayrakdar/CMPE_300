/*
*   Yusuf Sabri Bayrakdar
*   201size_matris400378
*   Algorithm Analaysis Project
*
*   To compile => mpic++ -o hello_cxx hello_cxx.cc
*/

#include "mpi.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <cmath>
#define size_matris 200
#define Times 10000
using namespace std;

vector<int> mySplit(string line){
    vector<int> values;
    while(line.find(" ")!=string::npos){
        string atom=line.substr (0,line.find(" ")); 
        line=line.substr(line.find(" ")+1);
        values.push_back(atoi( atom.c_str() ));
    }
    // values.push_back(atoi( line.c_str() ));
    return values;
}
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
    sscanf(argv[4],"%lf",&pi);
    double Y=log((1-pi)/pi)/2;
    double B;
    sscanf(argv[3],"%lf",&B);
    
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
        //Divides the input into equal parts and sends them to servants
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
    int msgInput[territorySize+2][territorySize+2];
    int msgCopy[territorySize+2][territorySize+2];
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
            for(int i=0;i<Times;i++){
            if(process*territorySize%size_matris==territorySize){//Leftmost column
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
            else if(process*territorySize%size_matris==0){//Rightmost column
                for(int x_axis = 0; x_axis < territorySize; x_axis++){
                    msgData=msgCopy[x_axis+1][1];
                    MPI_Send(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD);
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[x_axis+1][0] = msgData;
                }
            }
            else{
                
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
            if(process<=process_per_line){//First row
                for(int i=0;i<territorySize;i++){
                    msgData=msgCopy[territorySize][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[territorySize+1][i+1]=msgData;
                }
            }
            else if(process>servant_processes-process_per_line){//Last Row
                for(int i=0;i<territorySize;i++){
                    msgData=msgCopy[1][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[0][i+1]=msgData;
                }

            }
            if(process>process_per_line&&process<=servant_processes-process_per_line){//Between first and last rows

                for(int i=0;i<territorySize;i++){
                    msgData=msgCopy[territorySize][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){
                    msgData=msgCopy[1][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[territorySize+1][i+1]=msgData;
                }
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgCopy[0][i+1]=msgData;
                }

            }

            if(process<=process_per_line&&process*territorySize%size_matris==territorySize){//sol yukari
                msgData=msgCopy[territorySize][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][territorySize+1]=msgData;
            }
            else if(process<=process_per_line&&process*territorySize%size_matris==0){//sag yukari
                msgData=msgCopy[territorySize][1];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][0]=msgData;
            }
            else if(process>servant_processes-process_per_line&&process*territorySize%size_matris==territorySize){//sol assagi
                msgData=msgCopy[1][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][territorySize+1]=msgData;
            }
            else if(process>servant_processes-process_per_line&&process*territorySize%size_matris==0){//sag assagi
                msgData=msgCopy[1][1];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][0]=msgData;
            }
            else if (process<=process_per_line) {
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
            if (process>servant_processes-process_per_line) {
                msgData=msgCopy[1][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][territorySize+1]=msgData;

                msgData=msgCopy[1][1];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][0]=msgData;
            }
            else 
            if (process*territorySize%size_matris==territorySize) {
                msgData=msgCopy[territorySize][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][territorySize+1]=msgData;

                msgData=msgCopy[1][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][territorySize+1]=msgData;
            }
            else 
            if (process*territorySize%size_matris==0) {
                msgData=msgCopy[territorySize][1];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[territorySize+1][0]=msgData;

                msgData=msgCopy[1][1];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgCopy[0][0]=msgData;
            }
            
            else
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
            //Calculating & Denoising
            int random=rand()%pixels;
            int y=random%size_matris+1;
            int x=random/size_matris+1;
            int up=msgCopy[x-1][y];
            int down=msgCopy[x+1][y];
            int left=msgCopy[x][y-1];
            int right=msgCopy[x][y+1];
            int left_top=msgCopy[x-1][y-1];
            int right_top=msgCopy[x-1][y+1];
            int left_bottom=msgCopy[x+1][y-1];
            int right_bottom=msgCopy[x+1][y+1];

            int env;
            env=up+down+left+right+left_top+right_top+left_bottom+right_bottom;
            double ex=monte_carlo(msgInput[x][y],msgCopy[x][y],env,B,1.0);
            if(ex>1)ex=1;
            double treshold=(rand()%9+1)*0.1;
            
            if (treshold<ex) {
                msgCopy[x][y]=-1*msgCopy[x][y];
            }
            }
            // cout<<"Process "<<process<<endl;
            for(int i = 0; i <territorySize; i++)
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
