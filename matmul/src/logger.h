#include <iostream>
#include <iomanip>
#include <string>

#include <mpi.h>

#define PRECISION 8

class Logger
{
private:
  std::iostream *stream;
  double cts = MPI_Wtime();

  double lapse()
  {
    double elapsed = MPI_Wtime() - cts;
    cts = MPI_Wtime();
    return elapsed;
  }

public:
  Logger(std::iostream *tl) : stream(tl) {}

  void log(double *time_agg)
  {
    double elapsed = lapse();
    *time_agg += elapsed;
    *stream << std::setprecision(PRECISION) << elapsed << ',';
  }

  void log(double *time_agg, std::string tag)
  {
    double elapsed = lapse();
    *time_agg += elapsed;
    *stream << tag << ':' << std::setprecision(PRECISION) << elapsed << ',';
  }
};