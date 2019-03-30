/***************************************************************************/
/* Template for Asssignment 4/5 ********************************************/
/* Team Names Here              **(*****************************************/
/***************************************************************************/

/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>

#include<clcg4.h>

#include<mpi.h>
#include<pthread.h>

// #define BGQ 1 // when running BG/Q, comment out when testing on mastiff

#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
#else
#define GetTimeBase MPI_Wtime            
#endif

/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

#define ALIVE 1
#define DEAD  0
#define rows 32

/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/

double g_time_in_secs = 0;
double g_processor_frequency = 1600000000.0; // processing speed for BG/Q
unsigned long long g_start_cycles=0;
unsigned long long g_end_cycles=0;

// You define these

//int *universe; //All the cell universe

int **universe; //universe for each rank
int *line_universe;
int *ghost_up;      //the ghost row for top row
int *ghost_down;    //the ghost row for buttom row

double threshold = 0.25;
int mpi_myrank = -1;
int mpi_commsize = -1;
/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

// You define these

void index(r,c){return r*rows+c;}

int generate(int row_index,int num_row,int key)
{
	//int key = row_index+mpi_myrank*(rows/mpi_commsize)
	
	//get RNG
	double rng =  GenVal(key);
        //printf("%d\n == %lf",row_index,rng);
	
	//get state
        int left, right, up, down,upright,upleft,downright,downleft;
	int l,r;
        int lives = 0;

        
	//random
        
        if(rng < threshold){

                for(int col=0; col<rows; col++){
                        double random = GenVal(key);
                        if(random > 0.5){
                                universe[row_index][col] = 1;
                                lives ++;
                        }
                        else{universe[row_index][col] = 0;}
                }

                return lives;
        }

	//rule
	
	for(int col=0; col<rows; col++){
		
                l = col-1;
		if(l < 0){l = rows-1;}
		r = col+1;
		if(r>= rows){r = 0;}

                left = universe[row_index][l];
		right = universe[row_index][r];

		
		if(row_index == 0){

			up = ghost_up[col];
			upleft = ghost_up[l];
			upright = ghost_up[r];
		}
		else{
			up = universe[row_index-1][col];
                        upleft = universe[row_index-1][l];
                        upright = universe[row_index-1][r];

		}

		

		if(row_index == num_row-1){

                        down = ghost_down[col];
                        downleft = ghost_down[l];
                        downright = ghost_down[r];
                }
                else{
                        down = universe[row_index+1][col];
                        downleft = universe[row_index+1][l];
                        downright = universe[row_index+1][r];

                }

          
                int state = up + down + left + right + upright + upleft + downright +  downleft;	
		
                if(universe[row_index][col] == 1){                                                                 

                       if(state < 2 || state > 3){universe[row_index][col] = 0;} //underpopulate & overpopulate
                       else{lives ++;}
		       
                }
		
                
                
                else{
                        if(state == 3){
                                universe[row_index][col] = 1;
                                lives ++;
                        } //reporduction
                }
		


        }
	
	
	
        //printf("\n");
        return lives;

}   
//update to each row


int communicate(int *ghost_up, int *ghost_down,MPI_Comm comm)   //communication between rows 
{
	
	/*get mpi info*/
        //MPI_Comm_size(comm,&mpi_commsize);
        //MPI_Comm_rank(comm,&mpi_myrank);
        if(mpi_commsize == -1|| mpi_myrank == -1){
                perror("ERROR: MPI failed\n");
                return -1;
        }

	/*calculate address*/

	 MPI_Request send_require,recv_require;
	 MPI_Status recv_status;
	 int up_address =  mpi_myrank -1; //the address for send the top row and receive the ghost_up layer
	 int down_address = mpi_myrank +1;  //the address for send the buttom row and receive the ghost_down layer
	 
	 if(mpi_myrank == 0){
		 
		 up_address =  mpi_commsize -1;
		 down_address = mpi_myrank +1;
    	}

    	else if(mpi_myrank == mpi_commsize -1 ){

            	up_address = mpi_myrank -1;
            	down_address = 0;
    	}


    	//printf("%d %d %d\n",mpi_myrank,up_address,down_address);





       /*Send*/

    	//Send to up row
    	MPI_Isend(ghost_up,rows,MPI_INT,up_address,1,MPI_COMM_WORLD,&send_require);
    	MPI_Barrier(comm);
	
	//send to down row
	MPI_Isend(ghost_down,rows,MPI_INT,down_address,1,MPI_COMM_WORLD,&send_require);
	MPI_Barrier(comm);

	/*Receive*/

	//Receive from down neighbour
	MPI_Irecv(ghost_up,rows,MPI_INT,up_address,1,MPI_COMM_WORLD,&recv_require);
	MPI_Wait(&recv_require,&recv_status);


  	 //Receive from up neighbour
	 MPI_Irecv(ghost_down,rows,MPI_INT,down_address,1,MPI_COMM_WORLD,&recv_require);
	 MPI_Wait(&recv_require,&recv_status);

         return 0;




}

/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
//    int i = 0;
// Example MPI startup and using CLCG4 RNG
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);
    
// Init 32,768 RNG streams - each rank has an independent stream
    InitDefault();
    

// Note, used the mpi_myrank to select which RNG stream to use.
// You must replace mpi_myrank with the right row being used.
// This just show you how to call the RNG.    
    //printf("Rank %d of %d has been started and a first Random Value of %lf\n", 
	   //mpi_myrank, mpi_commsize, GenVal(mpi_myrank));
    
    //MPI_Barrier( MPI_COMM_WORLD );
    
// Insert your code
    
    //Initalization, start timer
    //if(mpi_myrank == 0){}
    //MPI_Scatter();
    //


    int num_row = rows/mpi_commsize;
    universe = calloc(num_row,sizeof(int*));

    if(universe == NULL){
	    perror("ERROR:Unable to form universe\n");
	    return -1;
    }

    for(int i=0; i<num_row; i++){
	    
	    universe[i] = calloc(rows,sizeof(int));
	    if(universe == NULL){
		    perror("ERROR:Unable to form universe\n");
		    return -1;
	    }

    }

    ghost_up = calloc(rows,sizeof(int));
    ghost_down = calloc(rows,sizeof(int));

    if(ghost_up == NULL || ghost_down == NULL){
            perror("ERROR,Unable to form array");
            return -1;
    }
   
    int tick = 10;
    for(int i=0; i<num_row; i++){
                    for(int j=0;j<rows; j++){universe[i][j] = 1;}
            }


    
    for(int t=0; t<tick; t++){

   	    for(int i=0; i<rows; i++){ghost_up[i] = universe[0][i];}
    	    for(int i=0; i< rows; i++){ghost_down[i] = universe[num_row-1][i];}
            //printf("%d %d\n",num_row,mpi_myrank);
           communicate(ghost_up, ghost_down,MPI_COMM_WORLD);
           //printf("[%d] [%s] [%d]\n",mpi_myrank,"u",ghost_up[1]);
           //printf("[%d] [%s] [%d]\n",mpi_myrank,"d",ghost_down[1]);
	   
	   for(int i=0; i<num_row; i++){generate(i,num_row,(i+mpi_myrank*num_row));}
    }

    
    for(int i=0; i<num_row; i++){
            printf("[%d] ",i+mpi_myrank*num_row);
	    for(int j=0; j<rows; j++){
		   printf("%d ",universe[i][j]);
	    }
	    printf("\n");
    }
   

    for(int i=0; i<num_row; i++){free(universe[i]);}
    free(universe);



// END -Perform a barrier and then leave MPI
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}

/***************************************************************************/
/* Other Functions - You write as part of the assignment********************/
/***************************************************************************/

