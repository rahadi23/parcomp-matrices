#ifndef MATRIX_H
#define MATRIX_H
#include <iostream>
#include <cmath>
#include <mpi.h>
#include <fstream>
#include <sstream>
#include "logger.h"

#define PI 3.14159265358979323846

using namespace std;
class matrix
{
private:
  double *v = NULL;
  double *rhs = NULL;
  double *d = NULL;
  double *z = NULL;
  int nx, ny, a;
  double delta0;
  double delta1;
  double alpha;
  double beta;
  double sum_res;
  double hx, hy;
  double temp_hx;
  double temp_z;
  double temp_hy;
  double hx_2, hy_2;
  double temp_pi;
  double k_2;
  double *res = NULL;
  int size;
  double recp;
  int eps1;

  // FOR MPI//
  int mycoord[2];
  int j_init;
  int j_fina;
  int psize, myrank, rank;
  int work, ndims, dim[2], periods[2], reorder;
  MPI_Status status[4];
  MPI_Comm comm_cart;

  // mpi-timer
  Logger *pLogger;
  double *pCompTime;
  double *pMpiCartTime;
  double *pMpiAllreduceTime;
  double *pMpiIsendTime;
  double *pMpiIrecvTime;
  double *pMpiWaitallTime;
  double *pMpiBarrierTime;

public:
  matrix(int ix,
         int iy,
         int ran,
         int eps2,
         int psiz,
         Logger *logger,
         double *compTime,
         double *mpiCartTime,
         double *mpiAllreduceTime,
         double *mpiIsendTime,
         double *mpiIrecvTime,
         double *mpiWaitallTime,
         double *mpiBarrierTime);

  double get(int i, int j);

  void set(int i, int j, double val);

  void setrhs();

  void setBoundary();

  void release();

  void cG(int k);

  double getD(int i, int j);

  void writetemp();
};

#endif
