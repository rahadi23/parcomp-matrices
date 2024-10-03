#include <iostream>
#include <cmath>
#include <fstream>
#include "matrix.h"
#include <mpi.h>
#define PI 3.14159265358979323846

using namespace std;

matrix::matrix(int ix, int iy, int ran, int eps2, int psiz)
{
	a = 0;
	nx = ix;
	ny = iy;
	hx = 2.0 / (nx);
	hy = 1.0 / (ny);
	delta0 = 0;
	delta1 = 0;
	sum_res = 0;
	alpha = 0;
	beta = 0;
	temp_z = 0;
	size = (nx + 1) * (ny + 1);
	// Dynamic memory allocation to ensure golden touch policy
	v = new double[size]();
	res = new double[size]();
	d = new double[size]();
	rhs = new double[size]();
	z = new double[size]();
	temp_hx = 2 * PI * hx;
	temp_hy = 2 * PI * hy;
	k_2 = 4 * PI * PI;
	hx_2 = hx * hx;
	hy_2 = hy * hy;
	temp_pi = k_2 * hx_2 * hy_2;
	recp = 1 / (hx_2 * hy_2);
	psize = psiz;
	myrank = ran;
	eps1 = eps2;
};

// calculation of f(x,y) (RHS)//
void matrix::setrhs()
{
	int i, j;
	double val = 0;
	double temp_j = 0;
	for (j = 0; j <= ny; j++)
	{
		temp_j = temp_hy * j;
		for (i = 0; i <= nx; i++)
		{
			val = temp_pi * sin(temp_hx * i) * sinh(temp_j);
			rhs[i + ((nx + 1) * j)] = val;
		}
	}
};

void matrix::setBoundary()
{
	double temp;
	double pi2 = 2 * PI;
	for (int i = 0; i <= nx; i++)
	{
		set(i, 0, 0); // bottom boundary
		temp = sin(2 * PI * hx * i) * sinh(pi2);
		set(i, ny, temp); // top boundary
	}
	for (int j = 0; j <= ny; j++)
	{
		set(0, j, 0);	 // left boundary
		set(nx, j, 0); // right boundary
	}
};

void matrix::cG(int k)
{
	ndims = 2; // 2D Cartesian topology
	// Slice methodology implemented in Y direction
	dim[0] = psize;
	dim[1] = 1;
	// no periodicity
	periods[0] = 0;
	periods[1] = 0;
	reorder = 1;																// reorder enabled
	int up_rank, down_rank;											/// for sending and receiving
	int up_send, down_send, up_recv, down_recv; // buffer location for send and receive
	double temp_hx2 = (-1) / hx_2;
	double temp_hy2 = (-1) / hy_2;
	double temp_h = temp_hx2 * temp_hy2;

	int work_Q = (ny - 1) / psize; // work allocation
	int work_REM = (ny - 1) % psize;
	double temp_delta0 = 0;
	double temp_delta1 = 0;
	double temp_alpha = 0;
	MPI_Request reqs[6]; // for checking the status of send and receive

	// 2D Cartesian topology for communication within the doamin

	MPI_Cart_create(MPI_COMM_WORLD, ndims, dim, periods, reorder, &comm_cart);
	MPI_Comm_rank(comm_cart, &rank);
	// To fetch co-ordatinates of all processes
	MPI_Cart_coords(comm_cart, rank, 2, mycoord);
	// Work allocation for all processes except last
	if (mycoord[0] != psize - 1)
	{
		j_init = 1 + mycoord[0] * work_Q;
		j_fina = (mycoord[0] + 1) * work_Q;
	}
	// Work allocation for last process
	else
	{
		j_init = 1 + mycoord[0] * work_Q;
		j_fina = (mycoord[0] + 1) * work_Q + work_REM;
	}
	// Calculation of delta0
	double temp = 0;
	for (int j = j_init; j <= j_fina; j++)
	{
		for (int i = 1; i < nx; i++)
		{
			temp = ((temp_hx2 * (get(i - 1, j) - 2 * get(i, j) + get(i + 1, j))) + (temp_hy2 * (get(i, j - 1) - 2 * get(i, j) + get(i, j + 1))) + (k_2 * get(i, j)));
			res[i + (nx + 1) * j] = (temp_h * rhs[i + ((nx + 1) * j)]) - temp;
		}
	}

	for (int j = j_init; j <= j_fina; j++)
	{
		for (int i = 1; i < nx; i++)
		{
			temp_delta0 = temp_delta0 + res[i + (nx + 1) * j] * res[i + (nx + 1) * j];
		}
	}
	MPI_Allreduce(&temp_delta0, &delta0, 1, MPI_DOUBLE, MPI_SUM, comm_cart);
	sum_res = delta0 / ((nx - 1) * (ny - 1));
	sum_res = pow(sum_res, 0.5);

	if (sum_res > eps1)
	{
		// set d//
		for (int i = 0; i < ((nx + 1) * (ny + 1)); i++)
		{
			d[i] = res[i];
		}
		// iteration//
		//  To obtain neighbouring ranks of each processes in dir 0 with disp 1
		//  Up rank is bottom processor in y direction (dir 0)
		//  Down rank is top processor in y direction (dir 0)
		MPI_Cart_shift(comm_cart, 0, 1, &up_rank, &down_rank);
		// buffer locations
		down_send = (nx + 1) * j_fina + 1;
		up_send = (nx + 1) * j_init + 1;
		down_recv = (nx + 1) * (j_fina + 1) + 1;
		up_recv = (nx + 1) * (j_init - 1) + 1;

		for (a = 0; a < k; a++)
		{
			MPI_Isend(&d[up_send], (nx - 1), MPI_DOUBLE, up_rank, 0, comm_cart, &reqs[0]);
			MPI_Isend(&d[down_send], (nx - 1), MPI_DOUBLE, down_rank, 1, comm_cart, &reqs[1]);
			MPI_Irecv(&d[down_recv], (nx - 1), MPI_DOUBLE, down_rank, 0, comm_cart, &reqs[2]);
			MPI_Irecv(&d[up_recv], (nx - 1), MPI_DOUBLE, up_rank, 1, comm_cart, &reqs[3]);

			MPI_Waitall(4, reqs, status);
			// setZ
			for (int j = j_init; j <= j_fina; j++)
			{
				for (int i = 1; i < nx; i++)
				{
					z[i + (nx + 1) * j] = ((temp_hx2 * (getD(i - 1, j) - 2 * getD(i, j) + getD(i + 1, j))) + (temp_hy2 * (getD(i, j - 1) - 2 * getD(i, j) + getD(i, j + 1))) + (k_2 * getD(i, j)));
				}
			}
			// setAlpha
			temp_z = 0;
			for (int j = j_init; j <= j_fina; j++)
			{
				for (int i = 1; i < nx; i++)
				{
					temp_z = temp_z + d[i + (nx + 1) * j] * z[i + (nx + 1) * j];
				}
			}

			MPI_Allreduce(&temp_z, &temp_alpha, 1, MPI_DOUBLE, MPI_SUM, comm_cart);
			alpha = delta0 / temp_alpha;
			// setU & R
			for (int j = j_init; j <= j_fina; j++)
			{
				for (int i = 1; i < nx; i++)
				{
					v[i + (nx + 1) * j] = v[i + (nx + 1) * j] + alpha * d[i + (nx + 1) * j];
					res[i + (nx + 1) * j] = res[i + (nx + 1) * j] - alpha * z[i + (nx + 1) * j];
				}
			}
			temp_delta1 = 0;
			delta1 = 0;
			for (int j = j_init; j <= j_fina; j++)
			{
				for (int i = 1; i < nx; i++)
				{
					temp_delta1 = temp_delta1 + res[i + (nx + 1) * j] * res[i + (nx + 1) * j];
				}
			}
			MPI_Allreduce(&temp_delta1, &delta1, 1, MPI_DOUBLE, MPI_SUM, comm_cart);
			sum_res = delta1 / ((nx - 1) * (ny - 1));
			sum_res = pow(sum_res, 0.5);

			if (sum_res <= eps1)
				break;
			beta = delta1 / delta0;
			for (int j = j_init; j <= j_fina; j++)
			{
				for (int i = 1; i < nx; i++)
				{
					d[i + (nx + 1) * j] = res[i + (nx + 1) * j] + beta * d[i + (nx + 1) * j];
				}
			}
			delta0 = delta1;
		}
	}
	if (rank == 0)
	{
		cout << "[INFO] Iteration number: " << a << endl;
		cout << "[INFO] Residual norm: " << sum_res << endl;
	}
};

double matrix::getD(int i, int j)
{
	return d[i + ((nx + 1) * j)];
};

double matrix::get(int i, int j)
{
	return v[i + ((nx + 1) * j)];
};

void matrix::set(int i, int j, double val)
{
	v[i + ((nx + 1) * j)] = val;
};

void matrix::writetemp()
{
	if (rank == 0)
	{
		ofstream bout;
		bout.open("solution.txt");

		bout << "# x y u(x,y)" << endl;
		int j = 0;
		for (int i = 0; i <= nx; i++)
		{
			bout << i * hx << " " << j * hy << " " << get(i, j) << endl;
		}
		bout << endl;
		bout.close();
	}
	for (int som = 0; som < psize; som++)
	{
		MPI_Barrier(comm_cart);
		if (mycoord[0] == som)
		{
			// cout<<"Writing the results to solution"<<rank<<".txt :"<<endl;
			ofstream fout;
			fout.open("solution.txt", std::ios_base::app);

			// fout<<"# x y u(x,y)"<<endl;
			for (int j = j_init; j <= j_fina; j++)
			{
				for (int i = 0; i <= nx; i++)
				{
					fout << i * hx << " " << j * hy << " " << get(i, j) << endl;
				}
				fout << endl;
			}
			fout.close();
		}
	}
	MPI_Barrier(comm_cart);
	if (rank == 0)
	{
		ofstream aout;
		aout.open("solution.txt", std::ios_base::app);
		int j = ny;
		for (int i = 0; i <= nx; i++)
		{
			aout << i * hx << " " << j * hy << " " << get(i, j) << endl;
		}
		aout << endl;
		aout.close();
	}
};

void matrix::release()
{
	delete[] v;
	delete[] res;
	delete[] rhs;
	delete[] d;
	delete[] z;
};
