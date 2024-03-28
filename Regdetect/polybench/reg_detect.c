/**
 * reg_detect.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
/* Default data type is int, default size is 50. */
#include "reg_detect.h"


/* Array initialization. */
static
void init_array(int _PB_MAXGRID,
		DATA_TYPE POLYBENCH_2D(sum_tang,MAXGRID,MAXGRID,_PB_MAXGRID,_PB_MAXGRID),
		DATA_TYPE POLYBENCH_2D(mean,MAXGRID,MAXGRID,_PB_MAXGRID,_PB_MAXGRID),
		DATA_TYPE POLYBENCH_2D(path,MAXGRID,MAXGRID,_PB_MAXGRID,_PB_MAXGRID))
{
  int i, j;

  for (i = 0; i < _PB_MAXGRID; i++)
    for (j = 0; j < _PB_MAXGRID; j++) {
      sum_tang[i][j] = (DATA_TYPE)((i+1)*(j+1));
      mean[i][j] = ((DATA_TYPE) i-j) / _PB_MAXGRID;
      path[i][j] = ((DATA_TYPE) i*(j-1)) / _PB_MAXGRID;
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int _PB_MAXGRID,
		 DATA_TYPE POLYBENCH_2D(path,MAXGRID,MAXGRID,_PB_MAXGRID,_PB_MAXGRID))
{
  int i, j;

  for (i = 0; i < _PB_MAXGRID; i++)
    for (j = 0; j < _PB_MAXGRID; j++) {
      fprintf (stderr, DATA_PRINTF_MODIFIER, path[i][j]);
      if ((i * _PB_MAXGRID + j) % 20 == 0) fprintf (stderr, "\n");
    }
  fprintf (stderr, "\n");
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
/* Source (modified): http://www.cs.uic.edu/~iluican/reg_detect.c */
static
void kernel_reg_detect(int niter, int _PB_MAXGRID, int _PB_LENGTH,
		       DATA_TYPE POLYBENCH_2D(sum_tang,MAXGRID,MAXGRID,_PB_MAXGRID,_PB_MAXGRID),
		       DATA_TYPE POLYBENCH_2D(mean,MAXGRID,MAXGRID,_PB_MAXGRID,_PB_MAXGRID),
		       DATA_TYPE POLYBENCH_2D(path,MAXGRID,MAXGRID,_PB_MAXGRID,_PB_MAXGRID),
		       DATA_TYPE POLYBENCH_3D(diff,MAXGRID,MAXGRID,LENGTH,_PB_MAXGRID,_PB_MAXGRID,_PB_LENGTH),
		       DATA_TYPE POLYBENCH_3D(sum_diff,MAXGRID,MAXGRID,LENGTH,_PB_MAXGRID,_PB_MAXGRID,_PB_LENGTH))
{
/*
  int t, i, j, cnt;
#pragma scop
  for (t = 0; t < _PB_NITER; t++) {
    for (j = 0; j <= _PB_MAXGRID - 1; j++) {
	  for (i = j; i <= _PB_MAXGRID - 1; i++) {
	    for (cnt = 0; cnt <= _PB_LENGTH - 1; cnt++) {
	      diff[j][i][cnt] = sum_tang[j][i];
        }
      }
    }

    for (j = 0; j <= _PB_MAXGRID - 1; j++) {
	  for (i = j; i <= _PB_MAXGRID - 1; i++) {
	    sum_diff[j][i][0] = diff[j][i][0];
	    for (cnt = 1; cnt <= _PB_LENGTH - 1; cnt++) {
	      sum_diff[j][i][cnt] = sum_diff[j][i][cnt - 1] + diff[j][i][cnt];
        }
	    mean[j][i] = sum_diff[j][i][_PB_LENGTH - 1];
      }
    }

    for (i = 0; i <= _PB_MAXGRID - 1; i++) {
      // john: this isn't present in the linked code.. they must've refactored the loops to benefit themselves somehow (it gets rid of an if-else block)
      // john: this suggests their polyhedral tool was not able to detect an if-else prologue, so they rewrote it to make the if-else block a loop
	  path[0][i] = mean[0][i];
    }

    for (j = 1; j <= _PB_MAXGRID - 1; j++) {
	  for (i = j; i <= _PB_MAXGRID - 1; i++) {
	    path[j][i] = path[j - 1][i - 1] + mean[j][i];
      }
    }
  }
#pragma endscop
*/
#pragma scop
  int i,j,cnt;
  /* this task is covered by the input 
  for( j=0; j<=_PB_MAXGRID-1; j++)
  {
    sum_tang[j][j] = tangent[(_PB_MAXGRID+1)*j];
    for( i=j+1; i<=_PB_MAXGRID-1; i++)
    {
      sum_tang[j][i] = sum_tang[j][i-1] + tangent[i+_PB_MAXGRID*j];
    }
  }*/

  for( j=0; j<=_PB_MAXGRID-1; j++)
  {
    for( i=j; i<=_PB_MAXGRID-1; i++)
    {
      for( cnt=0; cnt<=_PB_LENGTH-1; cnt++)
      {
        diff[j][i][cnt] = sum_tang[j][i];
      }
    }
  }

  for( j=0; j<=_PB_MAXGRID-1; j++)
  {
    for( i=j; i<=_PB_MAXGRID-1; i++)
    {
      sum_diff[j][i][0] = diff[j][i][0];
      for( cnt=1; cnt<=_PB_LENGTH-1; cnt++)
      {
        sum_diff[j][i][cnt] = sum_diff[j][i][cnt-1] + diff[j][i][cnt];
      }
	  // mean[j][i] = sum_tang[j][i]*(_PB_LENGTH-1);
      mean[j][i] = sum_diff[j][i][_PB_LENGTH-1];
    }
  }

  for( j=0; j<=_PB_MAXGRID-1; j++)
  {
    for( i=j; i<=_PB_MAXGRID-1; i++)
    {
      if (j>0)
      {
        path[j][i] = path[j-1][i-1]+mean[j][i];
      }
      else
      {
        path[j][i] = mean[j][i];
      }
    }
  }
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int niter = NITER;
  int _PB_MAXGRID = MAXGRID;
  int _PB_LENGTH = LENGTH;

  /* Variable declaration/allocation. */
  POLYBENCH_2D_ARRAY_DECL(sum_tang, DATA_TYPE, MAXGRID, MAXGRID, _PB_MAXGRID, _PB_MAXGRID);
  POLYBENCH_2D_ARRAY_DECL(mean, DATA_TYPE, MAXGRID, MAXGRID, _PB_MAXGRID, _PB_MAXGRID);
  POLYBENCH_2D_ARRAY_DECL(path, DATA_TYPE, MAXGRID, MAXGRID, _PB_MAXGRID, _PB_MAXGRID);
  POLYBENCH_3D_ARRAY_DECL(diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, _PB_MAXGRID, _PB_MAXGRID, _PB_LENGTH);
  POLYBENCH_3D_ARRAY_DECL(sum_diff, DATA_TYPE, MAXGRID, MAXGRID, LENGTH, _PB_MAXGRID, _PB_MAXGRID, _PB_LENGTH);

  /* Initialize array(s). */
  init_array (_PB_MAXGRID,
	      POLYBENCH_ARRAY(sum_tang),
	      POLYBENCH_ARRAY(mean),
	      POLYBENCH_ARRAY(path));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_reg_detect (niter, _PB_MAXGRID, _PB_LENGTH,
		     POLYBENCH_ARRAY(sum_tang),
		     POLYBENCH_ARRAY(mean),
		     POLYBENCH_ARRAY(path),
		     POLYBENCH_ARRAY(diff),
		     POLYBENCH_ARRAY(sum_diff));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(_PB_MAXGRID, POLYBENCH_ARRAY(path)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(sum_tang);
  POLYBENCH_FREE_ARRAY(mean);
  POLYBENCH_FREE_ARRAY(path);
  POLYBENCH_FREE_ARRAY(diff);
  POLYBENCH_FREE_ARRAY(sum_diff);

  return 0;
}
