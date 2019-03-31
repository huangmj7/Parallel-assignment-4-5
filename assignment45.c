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
unsigned long long g_start_cycles = 0;
unsigned long long g_end_cycles = 0;

// You define these

//int *universe; //All the cell universe
char *universe; //universe for each rank
char *unit_universe;
char *ghost_up;      //the ghost row for top row
char *ghost_down;    //the ghost row for buttom row

double threshold = 0.25;
int mpi_myrank = -1;
int mpi_commsize = -1;
int num_row;
int num_thread = 8;
int NumOfRow_Thread;

int ticks;
char stop_flag = 0;
char* progress_list;
int** ti_list;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	
enum direction{
	left = 0,
	right,
	up,
	down,
	upright,
	upleft,
	downright,
	downleft
};

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

// You define these

int loc(int r, int c) { return r*rows + c; }
char CheckComplete(void);
void ResetChecklist(void);
void CreateThread(void);
void* ProcessByThread(void* a);
void OneRound(int t_i);
int ProcessByLine(int row_index);


int communicate(char *ghost_up, char *ghost_down, MPI_Comm comm)   //communication between rows 
{

	/*get mpi info*/
	//MPI_Comm_size(comm,&mpi_commsize);
	//MPI_Comm_rank(comm,&mpi_myrank);
	if (mpi_commsize == -1 || mpi_myrank == -1) {
		perror("ERROR: MPI failed\n");
		return -1;
	}

	/*calculate address*/

	MPI_Request send_require, recv_require;
	MPI_Status recv_status;
	int up_address = mpi_myrank - 1; //the address for send the top row and receive the ghost_up layer
	int down_address = mpi_myrank + 1;  //the address for send the buttom row and receive the ghost_down layer

	if (mpi_myrank == 0) {

		up_address = mpi_commsize - 1;
		down_address = mpi_myrank + 1;
	}

	else if (mpi_myrank == mpi_commsize - 1) {

		up_address = mpi_myrank - 1;
		down_address = 0;
	}

	//printf("%d %d %d\n",mpi_myrank,up_address,down_address);
	/*Send*/

	//Send to up row
	MPI_Isend(ghost_up, rows, MPI_CHAR, up_address, 1, MPI_COMM_WORLD, &send_require);
	MPI_Barrier(comm);

	//send to down row
	MPI_Isend(ghost_down, rows, MPI_CHAR, down_address, 1, MPI_COMM_WORLD, &send_require);
	MPI_Barrier(comm);

	/*Receive*/

	//Receive from down neighbour
	MPI_Irecv(ghost_up, rows, MPI_CHAR, up_address, 1, MPI_COMM_WORLD, &recv_require);
	MPI_Wait(&recv_require, &recv_status);


	//Receive from up neighbour
	MPI_Irecv(ghost_down, rows, MPI_CHAR, down_address, 1, MPI_COMM_WORLD, &recv_require);
	MPI_Wait(&recv_require, &recv_status);

	return 0;
}


int main(int argc, char *argv[])
{

	// Example MPI startup and using CLCG4 RN
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_commsize);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_myrank);

	// Init 32,768 RNG streams - each rank has an independent stream
	num_row = rows / mpi_commsize;

	ticks = 0;
	
	InitDefault();

	MPI_Comm comm = MPI_COMM_WORLD;

	unit_universe = calloc(num_row*rows, sizeof(char));
	ghost_up = calloc(rows, sizeof(char));
	ghost_down = calloc(rows, sizeof(char));

	if (ghost_up == NULL || ghost_down == NULL || unit_universe == NULL) {
		perror("ERROR,Unable to form array");
		return -1;
	}

	if (mpi_myrank == 0) {

		universe = calloc(rows*rows, sizeof(char));
		if (universe == NULL) {
			perror("ERROR:unable to form array\n");
			return -1;
		}

		for (int i = 0; i<(rows*rows); i++) { universe[i] = 1; }  //all cell alive in the beginning 
	}

	if (mpi_myrank == 0){
		g_start_cycles = GetTimeBase();
	}

	//Deprecated----------------------------------------------------------------------------------------
	//scatter 

	MPI_Barrier(MPI_COMM_WORLD);
	int pass = -1;
	pass = MPI_Scatter(universe, 
			((rows*rows) / mpi_commsize), 
			MPI_CHAR, 
			unit_universe, 
			((rows*rows) / mpi_commsize), 
			MPI_CHAR,
			0, 
			MPI_COMM_WORLD);

	if (pass == -1) {
		perror("ERROR:scatter failed\n");
		return -1;
	}
	MPI_Barrier(comm);

	//====================================================================================================
	
	//Init locally

	/*
	printf("[%d] ",mpi_myrank);
	for(int i=0; i<rows; i++){printf("%d ",unit_universe[i]);}
	printf("\n");
	*/


	//start generate
	
	CreateThread();
	
	for (int t = 0; t < 15; t++) {

		//update ghost row
		for (int i = 0; i<rows; i++) {
			ghost_up[i] = unit_universe[loc(0, i)];
			ghost_down[i] = unit_universe[loc(num_row, i)];
		}


		//communicate

		pass = communicate(ghost_up, ghost_down, comm);
		if (pass == -1) {
			perror("ERROR: communication failed, program end..\n");
			return -1;
		}

		
		ticks = i;

		OneRound(0);

		while(!CheckComplete());

		ResetChecklist();
	}

	stop_flag = 1;

	g_end_cycles = GetTimeBase();

	//Deprecated----------------------------------------------------------------------------------
	MPI_Gather(unit_universe, num_row*rows, MPI_CHAR, universe, num_row*rows, MPI_CHAR, 0, comm);

	if (mpi_myrank == 0) {
		for (int i = 0; i<rows; i++) {
			printf("[%d] ", i);
			for (int j = 0; j<rows; j++) { printf("%d ", universe[loc(i, j)]); }
			printf("\n");
		}
	}
	//--------------------------------------------------------------------------------------------
	
	//Use Parallel IO instead
	
	g_start_cycles = GetTimeBase();


	g_end_cycles = GetTimeBase();

	//------------------------------------------------

	free(unit_universe);
	free(ghost_up);
	free(ghost_down);
	if (universe != NULL) { free(universe); }
	// END -Perform a barrier and then leave MPI
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}

char CheckComplete(void){
	for(int i = 0; i < num_thread; i++){
		if(progress_list[i] != 1){
			return 0;
		}
	}
	return 1;
}

void ResetChecklist(void){
	pthread_mutex_lock(&lock);

	for(int i = 0; i < num_thread; i++){
		progress_list[i] = 0;
	}

	pthread_mutex_unlock(&lock);
}

void CreateThread(void){
	
	progress_list = calloc(num_thread, sizeof(char));

	ti_list = calloc(num_thread, sizeof(int*));

	NumOfRow_Thread = num_row / num_thread;
	
	for(int i = 1; i < num_thread; i++){
		
		pthread_t tid;

		ti_list[i] = calloc(1, sizeof(int));
		*ti_list[i] = i;

		pthread_create(&tid, NULL, ProcessByThread, ti_list[i]);
		pthread_detach(tid);
	}

}

void* ProcessByThread(void* a){

	int t_i = *(int *) a;

	free(a);

	printf("Create %d\n", t_i);

	int counter = 0;
	
	while(!stop_flag){
		while(counter <= ticks){
			OneRound(t_i);
			counter++;
		}
	}
}

void OneRound(int t_i){
	for(int i = 0; i < NumOfRow_Thread; i++){
		ProcessByLine(t_i * NumOfRow_Thread + i);
	}

	progress_list[t_i] = 1;
}

int ProcessByLine(int row_index){

	int key = row_index + mpi_myrank * rows;

	double rng = GenVal(key);

	char neighbor[8];
	int l, r;
	int lives;

	if(rng < threshold){

	//	printf("%d: random\n", row_index);

		for(int col = 0; col < rows; col++){
			double random = GenVal(key);

			if(random > 0.5){
				unit_universe[loc(row_index, col)] = 1;
			}

			else{
				unit_universe[loc(row_index, col)] = 0;
			}
		}

	}

	else{
		
	//	printf("%d: rule\n", row_index);
		
		for(int col = 0; col < rows; col++){
			l = col - 1;
			if(l < 0) { l = rows - 1; }

			r = col + 1;
			if(r >= rows) { r = 0; }

			//Left
			neighbor[left] = unit_universe[loc(row_index, l)];
			//Right
			neighbor[right] = unit_universe[loc(row_index, r)];


			if (row_index == 0) {
				neighbor[up] = ghost_up[col];
				neighbor[upleft] = ghost_up[l];
				neighbor[upright] = ghost_up[r];
			}
			else {
				neighbor[up] = unit_universe[loc(row_index - 1, col)];
				neighbor[upleft] = unit_universe[loc(row_index - 1, l)];
				neighbor[upright] = unit_universe[loc(row_index, r)];
			}


			if (row_index == num_row - 1) {
				neighbor[down] = ghost_down[col];
				neighbor[downleft] = ghost_down[l];
				neighbor[downright] = ghost_down[r];
			}
			
			else {
				neighbor[down] = unit_universe[loc(row_index + 1, col)];
				neighbor[downleft] = unit_universe[loc(row_index + 1, l)];
				neighbor[downright] = unit_universe[loc(row_index + 1, r)];
			}
		
			int state = 0;

			for(int i = 0; i < 8; i++){
				state += neighbor[i];
			}

			if(unit_universe[loc(row_index, col)] == 1){
				if(state < 2 || state > 3){
					unit_universe[loc(row_index, col)] = 0;
				}
			}

			else{
				if(state == 3){
					unit_universe[loc(row_index, col)] = 1;
					lives++;
				}
			}
		}
	}

	//unit_universe[loc(row_index, ticks)] = 9;
		
	return lives;
}


