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
/*<-Libriries*/
#include "mpi.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <cmath>
/*Libriries->*/

/*<-Definitions*/
#define size_matris 200
#define T 500000
using namespace std;
/*Definitions->*/

vector<int> mySplit(string line){
    vector<int> values;
    while(line.find(" ")!=string::npos){
        string atom=line.substr (0,line.find(" ")); 
        line=line.substr(line.find(" ")+1);
        values.push_back(atoi( atom.c_str() ));
    }
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
    bool isBorder=true;
    
    MPI::Init();
    rank = MPI::COMM_WORLD.Get_rank();
    size = MPI::COMM_WORLD.Get_size();

    int servant_processes=size-1;
    int territorySize=size_matris/(servant_processes);
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
            y_axis++;
        }
        
        //Define each territory size in one dimention
        //Divides the input into equal parts and sends them to servants
        x_axis=0;y_axis=0;
        for(int process=1;process<size;process++){
            for(int col=x_axis;col<x_axis+territorySize;col++){
                for(int row=0;row<size_matris;row++){
                    MPI_Send(&input[row][col], 1, MPI_INT, process, 0, MPI_COMM_WORLD);
                }
            }
            x_axis+=territorySize;
        }
        //Receive from slaves
        x_axis=0;
        for(int process=1;process<size;process++){
            for(int col=x_axis;col<x_axis+territorySize;col++){
                for(int row=0;row<size_matris;row++){
                    MPI_Recv(&msgData, 1, MPI_INT, process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    output[row][col]=msgData;
                }
            }
            x_axis+=territorySize;
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
    int msgInput[territorySize+2][size_matris];
    int msgCopy[territorySize+2][size_matris];
    //Fill the matrixes with 0 to clear default values
    for(int row = 0; row <territorySize+2; row++){
        for(int column = 0; column < size_matris; column++){
            msgInput[row][column]=0;
        }
    }
    //Begin to chat with neighbors
    int pixels=territorySize*size_matris;
    
    for(int process=1;process<size;process++){
        //Receive pixels from master process
        
        if(rank==process){
        for(int i=0;i<territorySize;i++){
            for(int j=0;j<size_matris;j++){
                MPI_Recv(&msgData, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                msgInput[i+1][j]=msgData;
            }
        }
        //msgInput 2D array is our old values & msgCopy 2D array is our new values
        for(int row = 0; row <territorySize+2; row++){//Z=X.copy()
            for(int column = 0; column < size_matris; column++){
                msgCopy[row][column]=msgInput[row][column];
            }
        }
        for(int i=0;i<T/servant_processes;i++){
            
            int random=rand()%pixels;//Pick a random number to find a random pixel
            int y=random%size_matris;//Cnvert the random number to y coordinate of the random pixel
            int x=random/size_matris+1;//Cnvert the random number to x coordinate of the random pixel
            
            if (x==1||x==territorySize) {//If the random pixel is a border pixel then let slaves send/receive mesagge to each other
                isBorder=true;
            }
            else//Otherwise, if the random is not a border pixel then dont let the slaves to send/receive messages to each other
                isBorder=false;

            if (isBorder) {
                if(rank==1){//If the process is the first process
                    for(int i=0;i<size_matris;i++){
                        msgData=msgCopy[territorySize][i];
                        MPI_Send(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD);
                    }
                    for(int i=0;i<size_matris;i++){
                        MPI_Recv(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        msgCopy[territorySize+1][i]=msgData;
                    }
                }
                else if(rank==servant_processes){//If the process is the last process
                    for(int i=0;i<size_matris;i++){
                        msgData=msgCopy[1][i];
                        MPI_Send(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD);
                    }
                    for(int i=0;i<size_matris;i++){
                        MPI_Recv(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        msgCopy[0][i]=msgData;
                    }
                }
                else{//If the process is between the first & last processes
                    for(int i=0;i<size_matris;i++){
                        msgData=msgCopy[territorySize][i];
                        MPI_Send(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD);
                    }
                    for(int i=0;i<size_matris;i++){
                        MPI_Recv(&msgData, 1, MPI_INT, process+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        msgCopy[territorySize+1][i]=msgData;
                    }
                    for(int i=0;i<size_matris;i++){
                        msgData=msgCopy[1][i];
                        MPI_Send(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD);
                    }
                    for(int i=0;i<size_matris;i++){
                        MPI_Recv(&msgData, 1, MPI_INT, process-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        msgCopy[0][i]=msgData;
                    }
                }
            }
            
            
            //Calculation & Denoising 
            
            
            // cout<<"process "<<process<<" random "<<random<<" x "<<x<<" y "<<y<<endl;
            int up=msgCopy[x-1][y];
            int down=msgCopy[x+1][y];
            int left=msgCopy[x][y-1];
            int right=msgCopy[x][y+1];
            int left_top=msgCopy[x-1][y-1];
            int right_top=msgCopy[x-1][y+1];
            int left_bottom=msgCopy[x+1][y-1];
            int right_bottom=msgCopy[x+1][y+1];

            /*What I am doing here is that 
            *If pixel is the right then env value is the summation of top, bottom, right, left, top-left corner, top-bottom corner values
            *if pixel is the top then env value is the summation of left, right, bottom, left-bottom, right-bottom values
            *And so on, according to where the pixel is calculate the env value
            */
            int env;
            if(y==0)
                env=up+down+right+right_bottom+right_top;
            else if(y==size_matris-1)
                env=up+down+left+left_bottom+left_top;
            else
                env=up+down+left+right+left_top+right_top+left_bottom+right_bottom;
            double ex=monte_carlo(msgInput[x][y],msgCopy[x][y],env,B,Y);//Call the monte_carlo function to calculate exp value
            // cout<<ex<<endl;
            if(ex>1)ex=1;//if the exp value is greater then 1 then equal it to 1
            double treshold=(rand()%9+1)*0.1;//Pick a random treshold number
            if(treshold<ex){//If the exp value is greater than treshold then change the value
                msgCopy[x][y]=-msgCopy[x][y];
            }               
        }
            for(int i=0;i<territorySize;i++){
                for(int j=0;j<size_matris;j++){
                    msgData=msgCopy[i+1][j];
                    MPI_Send(&msgData, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
            }
        }
    }
    MPI::Finalize();
    return 0;
}