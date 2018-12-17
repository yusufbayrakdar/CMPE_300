/*
*   Yusuf Sabri Bayrakdar
*   2016400378
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
#define size_matris 6
using namespace std;

vector<int> mySplit(string line){
    vector<int> values;
    while(line.find(" ")!=string::npos){
        string atom=line.substr (0,line.find(" ")); 
        line=line.substr(line.find(" ")+1);
        values.push_back(atoi( atom.c_str() ));
    }
    values.push_back(atoi( line.c_str() ));
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
    MPI::Init();
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();
    int territorySize=sqrt(size_matris*size_matris/(size-1));
    int lineSize=0;
    bool master=false;
    int same_Coloumn=size_matris/territorySize;
    int servant_processes=size-1;
    int process_per_line=sqrt(servant_processes);
    //Master process
    if(rank==0){
        ifstream file("matris.txt");
        int x_axis=0;
        int y_axis=0;
        int input[size_matris][size_matris];
        //fill input list
        for(string line; getline( file, line ); ){
            vector<int> lineValues = mySplit(line);
            for(x_axis=0;x_axis<lineValues.size();x_axis++){
                input[x_axis][y_axis]=lineValues[x_axis];
            }
            lineSize=lineValues.size();
            y_axis++;
        }
        //Define each territory size in one dimention
        cout<<"Territory Size "<<territorySize<<endl;
        cout<<"Line Size "<<lineSize<<endl;
        //Divides the input into equal parts and sends them to servants
        x_axis=0;
        y_axis=0;
        for(int process=1;process<size;process++){
            cout<<"process "<<process<<endl;
            if(x_axis>=lineSize){
                x_axis=0;
                y_axis+=territorySize;
            }
            if(x_axis<lineSize){
                cout<<"x "<<x_axis<<" y "<<y_axis<<endl;
                for(int y=y_axis;y<y_axis+territorySize;y++){
                    for(int x=x_axis;x<x_axis+territorySize;x++){
                        MPI_Send(&input[x][y], 1, MPI_INT, process, 0, MPI_COMM_WORLD);
                        cout<<input[x][y];
                    }
                    cout<<endl;
                }
                x_axis+=territorySize;
            }
        }
        cout<<"Master send"<<endl;
        master=true;
    }
    int msgInput[territorySize+2][territorySize+2];
    //Fill the matrixes with 0 to clear default values
    for(int row = 0; row <territorySize+2; row++){
        for(int column = 0; column < territorySize+2; column++){
            msgInput[row][column]=0;
        }
    }
    //Begin to chat with neighbors
    
    for(int process=1;process<size;process++){
        if(rank==process){
            //Receive input from master process
            for(int j=0;j<territorySize;j++){
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[j+1][i+1]=msgData;
                }
            }
            
            
            if(process*territorySize%6==territorySize){//Leftmost column
                for(int x_axis = 0; x_axis < territorySize; x_axis++)
                {
                    int msgData=msgInput[x_axis+1][territorySize];
                    MPI_Send(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD);
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++)
                {
                    MPI_Recv(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[x_axis+1][territorySize+1]=msgData;
                }
            }
            else if(process*territorySize%6==0){//Rightmost column
                for(int x_axis = 0; x_axis < territorySize; x_axis++){
                    msgData=msgInput[x_axis+1][1];
                    MPI_Send(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD);
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[x_axis+1][0] = msgData;
                }
            }
            else{
                
                for(int x_axis = 0; x_axis < territorySize; x_axis++)
                {
                    int msgData=msgInput[x_axis+1][territorySize];
                    MPI_Send(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD);
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++){
                    msgData=msgInput[x_axis+1][1];
                    MPI_Send(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD);
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++)
                {
                    MPI_Recv(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[x_axis+1][territorySize+1]=msgData;
                }
                for(int x_axis = 0; x_axis < territorySize; x_axis++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[x_axis+1][0] = msgData;
                }

            }
            
            
           
             //Send & Receive upward & bottomward
            if(process<=process_per_line){//First row
                for(int i=0;i<territorySize;i++){
                    msgData=msgInput[territorySize][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[territorySize+1][i+1]=msgData;
                }
            }
            else if(process>servant_processes-process_per_line){//Last Row
                for(int i=0;i<territorySize;i++){
                    msgData=msgInput[1][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[0][i+1]=msgData;
                }

            }
            if(process>process_per_line&&process<=servant_processes-process_per_line){//Between first and last rows

                for(int i=0;i<territorySize;i++){
                    msgData=msgInput[territorySize][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){
                    msgData=msgInput[1][i+1];
                    MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[territorySize+1][i+1]=msgData;
                }
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[0][i+1]=msgData;
                }

            }

            if(process<=process_per_line&&process*territorySize%6==territorySize){//sol yukari
                msgData=msgInput[territorySize][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgInput[territorySize+1][territorySize+1]=msgData;
            }
            if(process<=process_per_line&&process*territorySize%6==0){//sag yukari
                msgData=msgInput[territorySize][1];
                MPI_Send(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process+same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgInput[territorySize+1][0]=msgData;
            }
            if(process>servant_processes-process_per_line&&process*territorySize%6==territorySize){//sol assagi
                msgData=msgInput[1][territorySize];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgInput[0][territorySize+1]=msgData;
            }
            if(process>servant_processes-process_per_line&&process*territorySize%6==0){//sag assagi
                msgData=msgInput[1][1];
                MPI_Send(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&msgData, 1, MPI_INT, process-same_Coloumn-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgInput[0][0]=msgData;
            }
            cout<<"Receiving Process "<<process<<endl;
                for(int row = 0; row <territorySize+2; row++){
					for(int column = 0; column < territorySize+2; column++){
                        cout<<msgInput[row][column];
                    }
                    cout<<endl;
                }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI::Finalize();

    return 0;
}
