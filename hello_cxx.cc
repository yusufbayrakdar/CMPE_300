//
// Copyright (c) 2004-2006 The Trustees of Indiana University and Indiana
//                         University Research and Technology
//                         Corporation.  All rights reserved.
// Copyright (c) 2006      Cisco Systems, Inc.  All rights reserved.
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
int main(int argc, char **argv)
{
    int rank, size;
    
    int msgData=0;
    MPI::Init();
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();
    int territorySize=sqrt(6*6/(size-1));
    int lineSize=0;
    bool master=false;
    //Master process
    if(rank==0){
        ifstream file("matris.txt");
        int x_axis=0;
        int y_axis=0;
        int input[201][201];
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
    for(int row = 0; row <territorySize+2; row++){
        for(int column = 0; column < territorySize+2; column++){
            msgInput[row][column]=0;
        }
    }
    for(int process=1;process<size;process++){
        if(rank==process){
            //Receive input from master process
            for(int j=0;j<territorySize;j++){//cout<<"P"<<process<<" ";
                for(int i=0;i<territorySize;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[j+1][i+1]=msgData;
                }//cout<<msgInput[1][1]<<endl;
            }
            
            

            if(rank==1){
                for(int x_axis=0;x_axis<territorySize;x_axis++){
                    int msgData=msgInput[x_axis+1][territorySize];
                    MPI_Sendrecv(&msgData,1,MPI_INT,process+1,0,&msgData,1,MPI_INT,process+1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    msgInput[x_axis+1][territorySize+1]=msgData;
                }
                cout<<"If Receiving Process 1"<<endl;
                for(int row = 0; row <territorySize+2; row++){
					for(int column = 0; column < territorySize+2; column++){
                        cout<<msgInput[row][column];
                    }
                    cout<<endl;
                }
            }
            else if(rank==size-1){
                //Send and receive border line
				for(int x_axis = 0; x_axis < territorySize; x_axis++){

                    int msgData = msgInput[1][x_axis+1];
                    MPI_Sendrecv(&msgData, 1, MPI_INT, process-1, 0, &msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[x_axis+1][0] = msgData;

                }
                cout<<"Else if Receiving Process "<<process<<endl;
                for(int row = 0; row <territorySize+2; row++){
					for(int column = 0; column < territorySize+2; column++){
                        cout<<msgInput[row][column];
                    }
                    cout<<endl;
                }
            }
            else{
                
                //Send and Receive rigth border line
                for(int x_axis=0;x_axis<territorySize;x_axis++){
                    int msgData=msgInput[x_axis+1][1];
                    MPI_Sendrecv(&msgData,1,MPI_INT,process-1,0,&msgData,1,MPI_INT,process+1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    msgInput[x_axis+1][2]=msgData;
                }
                //Send and Receive left border line
                for(int x_axis=0;x_axis<territorySize;x_axis++){
                    int msgData=msgInput[x_axis+1][1];
                    MPI_Sendrecv(&msgData,1,MPI_INT,process+1,0,&msgData,1,MPI_INT,process-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    msgInput[x_axis+1][0]=msgData;
                }
                cout<<"Else Receiving Process "<<process<<endl;
                for(int row = 0; row <territorySize+2; row++){
					for(int column = 0; column < territorySize+2; column++){
                        cout<<msgInput[row][column];
                    }
                    cout<<endl;
                }
            }
            
        }
    }
    

    
        
    
    



    // for(int i=0;i<2;i++){
    //     if(rank==0){
    //         msgData=i+1;
    //         MPI_Send(&msgData,1,MPI_INT,1,7,MPI_COMM_WORLD);
    //     }
    // }
    // for(int i=1;i<2;i++){
    //     if(rank==1){
    //         cout<<"Data Before Receive\t"<<msgData<<endl;
    //         MPI_Recv(&msgData,1,MPI_INT,0,7,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    //         cout<<"Data After Receive is\t"<<msgData<<endl;
    //     }
    // }
    MPI::Finalize();

    return 0;
}
