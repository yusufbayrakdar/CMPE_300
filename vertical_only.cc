//
// Copyright (c) 2004-200size_matris The Trustees of Indiana University and Indiana
//                         University Research and Technology
//                         Corporation.  All rights reserved.
// Copyright (c) 200size_matris      Cisco Systems, Inc.  All rights reserved.
//
// Sample MPI "hello world" application in C++
//
//To compile
//mpic++ -o hello_cxx hello_cxx.cc

#include "mpi.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <stdlib.h>
#include<cmath>
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

double monte_carlo(int x,int y,int value,int env,double B,double Y){
    double calculate=-2*Y*x*y-2*B*value*env;
    double ex=exp(calculate);
    return ex;
}

int main(int argc, char **argv)
{
    int rank, size;
    int msgData=0;
    double pi ;
    sscanf(argv[4],"%lf",&pi);
    double Y=log((1-pi)/pi)/2;
    double B;
    sscanf(argv[3],"%lf",&B);
    MPI::Init();
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();
    //cout<<"Monte Carlo:"<<monte_carlo(1,1,1,-2,0.4,1.0)<<endl;
    


    int territorySize=size_matris/(size-1);
    int lineSize=0;
    bool master=false;
    int same_Coloumn=size_matris/territorySize;
    int servant_processes=size-1;
    int process_per_line=sqrt(servant_processes);
    //Master process
    if(rank==0){
        ifstream file(argv[1]);
        int x_axis=0;
        int y_axis=0;
        int input[size_matris][size_matris];
        //fill input list
        
        for(string line; getline( file, line ); ){
            vector<int> lineValues = mySplit(line);
            for(x_axis=0;x_axis<lineValues.size();x_axis++){
                input[x_axis][y_axis]=lineValues[x_axis];
            }
            y_axis++;
        }
        
        //Define each territory size in one dimention
        cout<<"Territory Size "<<territorySize<<endl;
        cout<<"Line Size "<<lineSize<<endl;
        
        
        //Divides the input into equal parts and sends them to servants
        x_axis=0;y_axis=0;
        for(int process=1;process<size;process++){
            cout<<"process "<<process<<endl;
            for(int col=x_axis;col<x_axis+territorySize;col++){
                for(int row=0;row<size_matris;row++){
                    //cout<<row<<" "<<col<<" "<<input[row][col]<<endl;
                    MPI_Send(&input[row][col], 1, MPI_INT, process, 0, MPI_COMM_WORLD);
                }
            }
            x_axis+=territorySize;
        }
        cout<<"Master send"<<endl;
    }
    int msgInput[territorySize+2][size_matris];
    //Fill the matrixes with 0 to clear default values
    for(int row = 0; row <territorySize+2; row++){
        for(int column = 0; column < size_matris; column++){
            msgInput[row][column]=0;
        }
    }
    //Begin to chat with neighbors
    MPI_Barrier(MPI_COMM_WORLD);
    for(int process=1;process<size;process++){
        if(rank==process){//Receive pixels from master process

        for(int i=0;i<territorySize;i++){
            for(int j=0;j<size_matris;j++){
                MPI_Recv(&msgData, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgInput[i+1][j]=msgData;
            }
        }
            if(rank==1){
                for(int i=0;i<size_matris;i++){
                    msgData=msgInput[territorySize][i];
                    MPI_Sendrecv(&msgData, 1, MPI_INT, process+1, 0, &msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[territorySize+1][i]=msgData;
                }
            }
            else if(rank==size-1){
                for(int i=0;i<size_matris;i++){
                    msgData=msgInput[1][i];
                    MPI_Sendrecv(&msgData, 1, MPI_INT, process-1, 0, &msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[0][i]=msgData;
                }
            }
            else{
                for(int i=0;i<size_matris;i++){
                    msgData=msgInput[1][i];
                    MPI_Sendrecv(&msgData, 1, MPI_INT, process-1, 0, &msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[0][i]=msgData;  

                    msgData=msgInput[territorySize][i];
                    MPI_Sendrecv(&msgData, 1, MPI_INT, process+1, 0, &msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[territorySize+1][i]=msgData; 
                }  
            }
            //Calculation
            int pixels=territorySize*size_matris;
            vector<int> history; 
            for(int i=0;i<pixels;i++){
                int random=rand()%pixels+1;
                
                while(true){
                    bool jail=true;
                    for(int j=0;j<history.size();j++){
                        if(history[j]==random){
                            jail=false;
                            break;
                        }
                    }
                    if(jail==true){
                        
                        int x=random%size_matris;
                        int y=random/size_matris;
                        cout<<"Process "<<process<<" Random "<<i+1<<" "<<random<<" x "<<x+1<<" y "<<y<<" msgInputValue "<<msgInput[x+1][y]<<endl;
                        history.push_back(random);
                        break;
                    }
                    else{
                        if(i==pixels-1){
                            int x=random%size_matris;
                            int y=random/size_matris;
                            cout<<"Process "<<process<<" Random "<<i+1<<" "<<random<<" x "<<x+1<<" y "<<y<<" msgInputValue "<<msgInput[x+1][y]<<endl;
                            break;
                        }
                        random=rand()%pixels+1;
                    }
                }
                
            }
            cout<<endl;
            cout<<"Receiving Process "<<process<<endl;
                for(int row = 0; row <territorySize+2; row++){
					for(int column = 0; column <size_matris ; column++){
                        cout<<msgInput[row][column];
                    }
                    cout<<endl;
                }
        }}
    
    MPI::Finalize();

    return 0;
}
