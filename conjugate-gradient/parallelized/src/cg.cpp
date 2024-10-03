#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <mpi.h>
#include "matrix.cpp"
#include "Timer.h"

#define PI 3.14159265358979323846

using namespace std;

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	if (argc == 5)
	{
		int nx, ny, c, psiz, ran, eps;

		nx = atoi(argv[1]);
		ny = atoi(argv[2]);
		c = atoi(argv[3]);
		eps = atoi(argv[4]);

		double time = 0;

		siwir::Timer timer;

		timer.reset();

		MPI_Comm_rank(MPI_COMM_WORLD, &ran);
		MPI_Comm_size(MPI_COMM_WORLD, &psiz);

		if (ran == 0)
		{ /* #processes in application */
			cout << "[INFO] N: [" << nx << ", " << ny << "], NP: " << psiz << endl;
		}

		matrix u(nx, ny, ran, eps, psiz);

		u.setrhs();
		u.setBoundary();
		u.cG(c);

		time = timer.elapsed();

		if (ran == 0)
		{
			cout << "[INFO] Sys time: " << setprecision(6) << time << endl;
		}

		// TODO: add filename parameter/argv to write solution
		// u.writetemp();

		u.release();
	}

	MPI_Finalize();

	return 0;
}
