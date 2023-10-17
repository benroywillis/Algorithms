
//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//#include <math.h>

#ifndef TRACING
#define TRACING 0
#endif

#define PRECISION 	float
#define SIZE 		64

void GEMV(PRECISION (*in0)[SIZE], PRECISION (*in1), PRECISION (*out)[SIZE])
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            out[j] += in0[i][j] * in1[j];
        }
    }
}

int main()
{
    PRECISION (*in0)[SIZE] = (PRECISION (*)[SIZE])malloc(SIZE * sizeof(PRECISION[SIZE][SIZE]));
    PRECISION (*in1) =       (PRECISION (*))malloc(SIZE * sizeof(PRECISION[SIZE]));
    PRECISION (*out) = (PRECISION (*))malloc(SIZE * sizeof(PRECISION[SIZE]));
    //PRECISION in0[SIZE][SIZE];
    //PRECISION in1[SIZE];
    //PRECISION out[SIZE][SIZE];

    for (int i = 0; i < SIZE; i++)
    {
        in1[i] = rand();
        out[i] = 0;
        for (int j = 0; j < SIZE; j++)
        {
            in0[i][j] = rand();
        }
    }

	GEMV(in0, in1, out);

    return 0;
}
