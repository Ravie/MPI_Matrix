#include "C:\Program Files (x86)\Microsoft SDKs\MPI\Include\mpi.h"
#include <iostream>	// I/O functions
#include <cstdlib>	// srand(),rand()
#include <ctime>	// time()

#define _MAXPROC 20
#define _SIZE 10
#define _MAXELEMENT 32767
using namespace std;

void filling_matrix(int array[][_SIZE])
{
	srand((unsigned int)time(NULL));
	for (int i = 0; i < _SIZE; i++)
		for (int j = 0; j < _SIZE; j++)
			array[i][j] = rand();
}

void convert_to_linear(int array[][_SIZE], int *conv_arr)
{
	int k = 0;
	for (int i = 0; i < _SIZE; i++)
		for (int j = 0; j < _SIZE; j++)
		{
			conv_arr[k] = array[i][j];
			k++;
		}
}

void show_matrix(int array[][_SIZE])
{
	for (int i = 0; i < _SIZE; i++)
	{
		cout << endl;
		for (int j = 0; j < _SIZE; j++)
		{
			cout << "\t|" << array[i][j] << "|";
		}
		cout << "\n";
	}
}

int MPI_find_min_element(int *array, int piece, int num_of_pieces)
{
	int result = _MAXELEMENT;
	for (int i = 0; i < (_SIZE * _SIZE / (num_of_pieces)); i++)
		if (array[i] < result)
			result = array[i];
	//cout << "Process[" << piece << "] Minimal Element: " << result << endl;
	return result;
}

int seq_find_min_element(int array[][_SIZE])
{
	int result = _MAXELEMENT;
	for (int i = 0; i < _SIZE; i++)
		for (int j = 0; j < _SIZE; j++)
			if (array[i][j] < result)
				result = array[i][j];
	return result;
}

int find_min_of_min(int *array, int num)
{
	int result = _MAXELEMENT;
	for (int i = 1; i < num; i++)
	{
		if (array[i] < result)
			result = array[i];
	}
	return result;
}

int main(int argc, char *argv[])
{
	int matrix[_SIZE][_SIZE], lin_arr[_SIZE * _SIZE], seq_res, recv_matrix[_MAXPROC][_SIZE*_SIZE], MinInProcess[_MAXPROC], TotalMin;
	double seq_start_time, par_start_time, parallel_dt, sequence_dt;

	int ProcNum;	// Количество процессов
	int ProcRank;	// Ранг процесса
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	
	if (ProcRank == 0)
	{
		if (_SIZE % (ProcNum - 1) != 0)
		{
			cout << "Trebyets9 kratnoe" << _SIZE << endl;
			return -1;
		}
		cout << "Matrix is filling" << endl;
		filling_matrix(matrix);
		convert_to_linear(matrix, lin_arr);
		cout << "Matrix:" << endl;
		show_matrix(matrix);

		// Последовательное выполнение
		seq_start_time = MPI_Wtime();
		seq_res = seq_find_min_element(matrix);
		cout << "Minimal Element(sequence version): " << seq_res << endl;
		sequence_dt = MPI_Wtime() - seq_start_time;
		cout << "Time exceed(sequence version):" << sequence_dt << endl;
	}
	par_start_time = MPI_Wtime();
	MPI_Scatter(&lin_arr[(ProcRank) * (_SIZE * _SIZE / (ProcNum))], (_SIZE * _SIZE / (ProcNum)), MPI_INT,
				&recv_matrix[ProcRank],								(_SIZE * _SIZE / (ProcNum)), MPI_INT, 0, MPI_COMM_WORLD);
	MinInProcess[ProcRank] = MPI_find_min_element(recv_matrix[ProcRank], ProcRank, ProcNum);

	// Отображение минимальных элементов в частях матрицы
	//cout << "Minimal Element in " << ProcRank << " process: ";
	//cout << MinInProcess[ProcRank] << endl;
	MPI_Reduce(&MinInProcess[ProcRank], &TotalMin, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
	if (ProcRank == 0)
	{
		parallel_dt = MPI_Wtime() - par_start_time;
		cout << "Minimal Element(parallel version): " << TotalMin << endl;
		cout << "Time exceed(parallel version):" << parallel_dt << endl;
	}
	MPI_Finalize();

	return 0;
}
