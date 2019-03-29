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
char *universe; //universe for each rank
char *unit_universe;
char *ghost_up;      //the ghost row for top row
char *ghost_down;    //the ghost row for buttom row

double threshold = 0.25;
int mpi_myrank = -1;
int mpi_commsize = -1;
/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

// You define these

int loc(int r,int c){return r*rows+c;}


int communicate(char *ghost_up, char *ghost_down,MPI_Comm comm)   //communication between rows 
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
    	MPI_Isend(ghost_up,rows,MPI_CHAR,up_address,1,MPI_COMM_WORLD,&send_require);
    	MPI_Barrier(comm);
	
	//send to down row
	MPI_Isend(ghost_down,rows,MPI_CHAR,down_address,1,MPI_COMM_WORLD,&send_require);
	MPI_Barrier(comm);

	/*Receive*/

	//Receive from down neighbour
	MPI_Irecv(ghost_up,rows,MPI_CHAR,up_address,1,MPI_COMM_WORLD,&recv_require);
	MPI_Wait(&recv_require,&recv_status);


  	 //Receive from up neighbour
	 MPI_Irecv(ghost_down,rows,MPI_CHAR,down_address,1,MPI_COMM_WORLD,&recv_require);
	 MPI_Wait(&recv_require,&recv_status);

         return 0;




}


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
			//printf("%d ",loc(row_index,col));
			
                        if(random > 0.5){
                                unit_universe[loc(row_index,col)] = 1;
                                lives ++;
                        }
                        else{unit_universe[loc(row_index,col)] = 0;}
			
                }
		
		//printf("\n");

                return lives;
        }

	//rule

	
	for(int col=0; col<rows; col++){

                l = col-1;
		if(l < 0){l = rows-1;}
		r = col+1;
		if(r>= rows){r = 0;}

                left = unit_universe[loc(row_index,l)];
		right = unit_universe[loc(row_index,r)];


		if(row_index == 0){

			up = ghost_up[col];
			upleft = ghost_up[l];
			upright = ghost_up[r];
		}
		else{
			up = unit_universe[loc(row_index-1,col)];
                        upleft = unit_universe[loc(row_index-1,l)];
                        upright = unit_universe[loc(row_index,r)];

		}



		if(row_index == num_row-1){

                        down = ghost_down[col];
                        downleft = ghost_down[l];
                        downright = ghost_down[r];
                }
                else{
                        down = unit_universe[loc(row_index+1,col)];
                        downleft = unit_universe [loc(row_index+1,l)];
                        downright = unit_universe[loc(row_index+1,r)];

                }


                int state = up + down + left + right + upright + upleft + downright +  downleft;

                if(unit_universe[loc(row_index,col)] == 1){

                       if(state < 2 || state > 3){unit_universe[loc(row_index,col)] = 0;} //underpopulate & overpopulate
                       else{lives ++;}

                }



                else{
                        if(state == 3){
                                unit_universe[loc(row_index,col)] = 1;
                                lives ++;
                        } //reporduction
                }
        }
	



        //printf("\n");
        return lives;

}


int main(int argc, char *argv[])
{	

	// Example MPI startup and using CLCG4 RN
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
    MPI_Comm comm = MPI_COMM_WORLD;

    unit_universe = calloc(num_row*rows,sizeof(char));
    ghost_up = calloc(rows,sizeof(char));
    ghost_down = calloc(rows,sizeof(char));

    if(ghost_up == NULL || ghost_down == NULL || unit_universe == NULL){
            perror("ERROR,Unable to form array");
            return -1;
    }

    if(mpi_myrank == 0){

	    universe = calloc(rows*rows, sizeof(char));
	    if(universe == NULL){
		    perror("ERROR:unable to form array\n");
		    return -1;
	    }

	    for(int i=0; i<(rows*rows); i++){universe[i] = 1;}  //all cell alive in the beginning 
    }

    //scatter 

    MPI_Barrier(MPI_COMM_WORLD);
    int pass = -1;
    pass = MPI_Scatter(universe,((rows*rows)/mpi_commsize),MPI_CHAR,unit_universe,((rows*rows)/mpi_commsize),MPI_CHAR,0,MPI_COMM_WORLD);
    if(pass == -1){
	    perror("ERROR:scatter failed\n");
	    return -1;
    }
    MPI_Barrier(comm);
    
    /*
    printf("[%d] ",mpi_myrank);
    for(int i=0; i<rows; i++){printf("%d ",unit_universe[i]);}
    printf("\n");
    */
    

    //start generate

    int tick = 15;
    for(int t=0;t<tick;t++){

	    //update ghost row
	    for(int i=0; i<rows; i++){
		    ghost_up[i] = unit_universe[loc(0,i)];
		    ghost_down[i] = unit_universe[loc(num_row,i)];
	    }


	    //communicate

	    pass = communicate(ghost_up,ghost_down,comm);
	    if(pass == -1){
		    perror("ERROR: communication failed, program end..\n");
		    return -1;
	    }

	    //generate -- thread part
            for(int row_index=0; row_index < num_row; row_index++){
		        //generate -- thread part
                    int key = row_index+mpi_myrank*rows;
		    generate(row_index,num_row,key);
	    }
    }
    
    MPI_Gather(unit_universe,num_row*rows,MPI_CHAR,universe,num_row*rows,MPI_CHAR,0,comm);

    if(mpi_myrank == 0){
	    for(int i=0; i<rows; i++){
		    printf("[%d] ",i);
                    for(int j=0;j<rows; j++){printf("%d ",universe[loc(i,j)]);}
                    printf("\n");
                    }

     }






    // END -Perform a barrier and then leave MPI
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();

    return 0;
}
