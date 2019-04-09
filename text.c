#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>

#define rows 32768
#define lineoffset 65536

int my_mpi_size = -1;
int my_mpi_rank = -1;

int unitsize = 0;


char** unit_universe;
int* heatmap;
int* recvheatmap;

int loc(int x, int y){return 1024 * x + y;}

int main(int argc, char** argv){
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &my_mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_mpi_rank);

	unitsize = rows / my_mpi_size;

	unit_universe = calloc(unitsize, sizeof(char*));
	for(int i = 0; i < unitsize; i++){
		unit_universe[i] = calloc(rows, sizeof(char));
	}	

	heatmap = calloc(1024 * 1024, sizeof(int));
	if(my_mpi_rank == 0) recvheatmap = calloc(1024 * 1024, sizeof(int));

	char* buffer = calloc(100000, sizeof(char));

	MPI_Status status;
	MPI_File fh;
	MPI_Offset offset;

	MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

	for(int i = 0; i < unitsize; i++){

		offset = my_mpi_rank * (unitsize * lineoffset) + i * lineoffset;
		offset *= sizeof(char);
		MPI_File_read_at(fh, offset, buffer, lineoffset, MPI_CHAR, &status);

		for(int j = 0; j < rows; j++){
			unit_universe[i][j] = buffer[2*j];
		}
	}

	for(int i = 0; i < unitsize / 32; i++){
		for(int j = 0; j < rows / 32; j++){
			
			int sum = 0;

			for(int x = 0; x < 32; x++){
				for(int y = 0; y < 32; y++){
					sum += unit_universe[32* i + x][ 32* j + y];
					sum -= '0';
				}
			}

			heatmap[loc(i, j)] = sum;

//			if(my_mpi_rank == 0) printf("%d ",heatmap[loc(i, j)]);
		}
//		 if(my_mpi_rank == 0) printf("\n");
	}

	MPI_Barrier(MPI_COMM_WORLD);

	long localsize = (unitsize * rows / 1024);

	if(my_mpi_rank == 0){
		MPI_Gather(heatmap, localsize, MPI_INT, recvheatmap, localsize, MPI_INT, 0, MPI_COMM_WORLD);
	}

	else{
		MPI_Gather(heatmap, localsize, MPI_INT, NULL, localsize, MPI_INT, 0, MPI_COMM_WORLD);
	}

	if(my_mpi_rank == 0){
	for(int i = 0; i < 1024; i++){
		for(int j = 0; j < 1024; j++){
			 printf("%d ", recvheatmap[loc(i, j)]);
		}
		
		printf("\n");
	}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	return EXIT_SUCCESS;
}
