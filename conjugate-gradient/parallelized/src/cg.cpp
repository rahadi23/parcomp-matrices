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

  if (argc == 5)
  {
    MPI_Init(&argc, &argv);
    int nx, ny, c, psiz, ran;
    double eps;

    nx = atoi(argv[1]);
    ny = atoi(argv[2]);
    c = atoi(argv[3]);
    eps = atof(argv[4]);

    double time = 0;

    siwir::Timer timer;

    timer.reset();

    MPI_Comm_rank(MPI_COMM_WORLD, &ran);
    MPI_Comm_size(MPI_COMM_WORLD, &psiz);

    double compTime = 0.0;
    double mpiCartTime = 0.0;
    double mpiAllReduceTime = 0.0;
    double mpiIsendTime = 0.0;
    double mpiIrecvTime = 0.0;
    double mpiWaitallTime = 0.0;
    double mpiBarrierTime = 0.0;

    stringstream tl;
    tl << std::setw(2) << ran << ": [INFO] Timeline: ";

    Logger logger(&tl);

    if (ran == 0)
    { /* #processes in application */
      cout << "[INFO] N: [" << nx << ", " << ny << "], NP: " << psiz << endl;
    }

    matrix u(
        nx,
        ny,
        ran,
        eps,
        psiz,
        &logger,
        &compTime,
        &mpiCartTime,
        &mpiAllReduceTime,
        &mpiIsendTime,
        &mpiIrecvTime,
        &mpiWaitallTime,
        &mpiBarrierTime);

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

    MPI_Finalize();

    double mpiTime = mpiCartTime + mpiAllReduceTime + mpiIsendTime + mpiIrecvTime + mpiWaitallTime + mpiBarrierTime;
    double totalTime = compTime + mpiTime;

    cout << setw(2) << ran << ": [INFO] Cart. time: " << setprecision(6) << mpiCartTime << endl;
    cout << setw(2) << ran << ": [INFO] ARed. time: " << setprecision(6) << mpiAllReduceTime << endl;
    cout << setw(2) << ran << ": [INFO] Isend time: " << setprecision(6) << mpiIsendTime << endl;
    cout << setw(2) << ran << ": [INFO] Irecv time: " << setprecision(6) << mpiIrecvTime << endl;
    cout << setw(2) << ran << ": [INFO] WAll. time: " << setprecision(6) << mpiWaitallTime << endl;
    cout << setw(2) << ran << ": [INFO] Barr. time: " << setprecision(6) << mpiBarrierTime << endl;
    cout << setw(2) << ran << ": [INFO] COMM. TIME: " << setprecision(6) << mpiTime << endl;
    cout << setw(2) << ran << ": [INFO] COMP. TIME: " << setprecision(6) << compTime << endl;
    cout << setw(2) << ran << ": [INFO] TOTAL TIME: " << setprecision(6) << totalTime << endl;
    // cout << tl.str() << endl;
  }

  return 0;
}
