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
#include<cmath>
#define size_matris 201
#define T 5000000
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
    double pi ;
    sscanf(argv[4],"%lf",&pi);
    double Y=log((1-pi)/pi)/2;
    double B;
    sscanf(argv[3],"%lf",&B);
    MPI::Init();
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();
    

    
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
        int output[size_matris][size_matris];
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
            for(int col=x_axis;col<x_axis+territorySize;col++){
                for(int row=0;row<size_matris;row++){
                    //cout<<row<<" "<<col<<" "<<input[row][col]<<endl;
                    MPI_Send(&input[row][col], 1, MPI_INT, process, 0, MPI_COMM_WORLD);
                }
            }
            x_axis+=territorySize;
        }
        
        cout<<"Master send"<<endl;
        //Receive from slaves
        x_axis=0;
        
        for(int process=1;process<size;process++){
            for(int col=x_axis;col<x_axis+territorySize;col++){
                for(int row=0;row<size_matris;row++){
                    //cout<<"wait process "<<i<<endl;
                    MPI_Recv(&msgData, 1, MPI_INT, process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    output[row][col]=msgData;
                    //cout<<"process "<<process<<" x "<<row<<" y "<<col<<" "<<output[row][col]<<endl;
                }
                //cout<<endl;
            }
            x_axis+=territorySize;
        }



        ofstream out (argv[2]);
        for(int col=0;col<size_matris;col++){
            for(int row=0;row<size_matris;row++){
                out<<output[row][col]<<" ";
                cout<<output[row][col]<<" ";
            }
            out<<endl;
            cout<<endl;
        }


    }
    int msgInput[territorySize+2][size_matris];
    int msgCopy[territorySize+2][size_matris];
    //Fill the matrixes with 0 to clear default values
    for(int row = 0; row <territorySize+2; row++){
        for(int column = 0; column < size_matris; column++){
            msgInput[row][column]=0;
        }
    }
    //Begin to chat with neighbors
    //MPI_Barrier(MPI_COMM_WORLD);
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
                    MPI_Send(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<size_matris;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[territorySize+1][i]=msgData;
                }
            }
            else if(rank==size-1){
                for(int i=0;i<size_matris;i++){
                    msgData=msgInput[1][i];
                    MPI_Send(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<size_matris;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[0][i]=msgData;
                }
            }
            else{
                for(int i=0;i<size_matris;i++){
                    msgData=msgInput[territorySize][i];
                    MPI_Send(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<size_matris;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[territorySize+1][i]=msgData;
                }
                for(int i=0;i<size_matris;i++){
                    msgData=msgInput[1][i];
                    MPI_Send(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD);
                }
                for(int i=0;i<size_matris;i++){
                    MPI_Recv(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    msgInput[0][i]=msgData;
                }
            }
            //Calculation
            for(int row = 0; row <territorySize+2; row++){//Z=X.copy()
                for(int column = 0; column < size_matris; column++){
                    msgCopy[row][column]=msgInput[row][column];
                }
            }
            
            
            // denoising
            int pixels=territorySize*size_matris;
            vector<int> history; 
            for(int i=0;i<pixels;i++){
                int random=rand()%pixels;
                
                while(true){
                    bool jail=true;
                    for(int j=0;j<history.size();j++){
                        if(history[j]==random){
                            jail=false;
                            break;
                        }
                    }
                    if(jail==true){
                        
                        int y=random%size_matris;
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
                        if(y==0)
                            env=up+down+right+right_bottom+right_top;
                        else if(y==size_matris-1)
                            env=up+down+left+left_bottom+left_top;
                        else
                            env=up+down+left+right+left_top+right_top+left_bottom+right_bottom;
                        //cout<<"Total env:"<<env<<endl;
                        double ex=monte_carlo(msgInput[x][y],msgCopy[x][y],env,B,1.0);
                        if(ex>1)ex=1;
                        // cout<<"Exp:"<<ex<<endl;
                        if(ex>0.5){
                            msgCopy[x][y]=-msgInput[x][y];
                            // cout<<"old value:"<<msgInput[x][y]<<" new value:"<<msgCopy[x][y]<<endl;
                        }
                        history.push_back(random);
                        break;
                    }
                    else{
                        int x=random%size_matris;
                        int y=random/size_matris;
                        random=rand()%pixels;
                    }
                }
                
            }
            cout<<"Sending Process "<<rank<<endl;
            for(int i=0;i<territorySize;i++){
                for(int j=0;j<size_matris;j++){
                    msgData=msgCopy[i+1][j];
                    //cout<<msgData;
                    MPI_Send(&msgData, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
                //cout<<endl;
            }

            // cout<<endl;
            // cout<<"Receiving Process New "<<process<<endl;
            // for(int row = 0; row <territorySize+2; row++){
            //     for(int column = 0; column <size_matris ; column++){
            //         cout<<msgCopy[row][column];
            //     }
            //     cout<<endl;
            // }
            // cout<<endl;
            // cout<<"Receiving Process Old "<<process<<endl;
            // for(int row = 0; row <territorySize+2; row++){
            //     for(int column = 0; column <size_matris ; column++){
            //         cout<<msgInput[row][column];
            //     }
            //     cout<<endl;
            // }
        }}
    
    MPI::Finalize();

    return 0;
}
