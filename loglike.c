#include "loglike.h"

double loglike(const int N, double *x, double *y, double *yerr)
{
    int n;
    double result = 0.00;
    double  diff  = 0.00;

    for (n = 0; n < N; n++) 
    {
        diff = (y[n] - (1.00 * x[n] + 0.00)) / yerr[n];
        result += diff * diff;
    }

    return result;
}