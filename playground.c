#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>

#include"clcg4.h"

#include<pthread.h>

#define ALIVE 1
#define DEAD  0
#define rows 32

char *universe; //universe for each rank
char *unit_universe;
char *ghost_up;      //the ghost row for top row
char *ghost_down;    //the ghost row for buttom row

double threshold = 0.25;
int mpi_myrank = 0;
int mpi_commsize = 4;
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

int loc(int r, int c) { return r*rows + c; }
char CheckComplete(void);
void ResetChecklist(void);
void CreateThread(void);
void* ProcessByThread(void* a);
void OneRound(int t_i);
int ProcessByLine(int row_index);
void PrintMap(void){
	printf("\n");
	for(int i = 0; i < num_row; i++){
		for(int j = 0; j < rows; j++){
			printf("%d ", unit_universe[loc(i, j)]);
		}
		printf("\n");
	}

}

int main(int argc, char** argv){
	

	InitDefault();

	unit_universe = calloc(num_row*rows, sizeof(char));
	ghost_up = calloc(rows, sizeof(char));
	ghost_down = calloc(rows, sizeof(char));


	for(int i = 0; i < rows*num_row; i++){
		unit_universe[i] = 1;
	}

	for(int i = 0; i < rows; i++){
		ghost_up[i] = 1;
		ghost_down[i] = 1;
	}

	PrintMap();

	CreateThread();

	//Ticks
	for(int i = 0; i < 3; i++){


		ticks = i;

		OneRound(0);

		while(!CheckComplete());

	for(int i = 0; i < num_thread; i++){
		printf("%d: %d\t", i, progress_list[i]);
	}
	printf("\n");
	
		printf("%d:", i);
		PrintMap();
		
		ResetChecklist();

	for(int i = 0; i < num_thread; i++){
		printf("%d: %d\t", i, progress_list[i]);
	}
	printf("\n");
		
	}

	stop_flag = 1;

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


