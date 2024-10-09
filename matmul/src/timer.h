#include <iostream>
#include <iomanip>
#include <mpi.h>
#include <string>
#include <sstream>

const int PRECISION = 8;

class Timer
{
private:
  std::stringstream *tl;
  double cts = MPI_Wtime();
  double *comp_time, *mpi_time;

  double lapse()
  {
    double elapsed = MPI_Wtime() - cts;
    cts = MPI_Wtime();
    return elapsed;
  }

public:
  Timer(std::stringstream *tl, double *comp_time, double *mpi_time) : tl(tl), comp_time(comp_time), mpi_time(mpi_time) {}

  void log_comp(std::string tag = "comp")
  {
    double elapsed = lapse();
    *comp_time += elapsed;
    *tl << tag << ':' << std::setprecision(PRECISION) << elapsed << ',';
  }

  void log_mpi(std::string tag = "mpi")
  {
    double elapsed = lapse();
    *mpi_time += elapsed;
    *tl << tag << ':' << std::setprecision(PRECISION) << elapsed << ',';
  }
};