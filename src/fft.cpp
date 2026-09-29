// WARN: THIS MODULE IS SOMEHOW INTRODUCING A
// BUNCH OF SHIT I CAN'T COMPREHEND

#include <assert.h>
#include <cmath>

#define FFT_MAX_N 512
#define W_MAX_SIZE (FFT_MAX_N * 2)

/*
N point Complex fft and inverse fft functions
*/

typedef struct
{
    float real;
    float imag;
} complex_t;

static complex_t w[W_MAX_SIZE]; // W_MAX_SIZE = 512 * 2

void FFT_init(void)
{
    for (int k = 0; k < FFT_MAX_N / 2; ++k)
    {
        w[k].real =  cosf(2.0f * (float)M_PI * k / FFT_MAX_N);
        w[k].imag = -sinf(2.0f * (float)M_PI * k / FFT_MAX_N);
    }
}

void FFT(complex_t *Y, int N) /*input sample array, # of points      */
{
    complex_t temp1, temp2;   /*temporary storage variables          */
    int i, j, k;              /*loop counter variables               */
    int upper_leg, lower_leg; /*index of upper/lower butterfly leg   */
    int leg_diff;             /*difference between upper/lower leg   */
    int num_stages = 0;       /*number of FFT stages, or iterations  */
    int index, step;          /*index and step between twiddle factor*/

    /* log(base 2) of # of points = # of stages  */
    i = 1;
    do
    {
        num_stages += 1;
        i = i * 2;
    } while (i != N);

    /* starting difference between upper and lower butterfly legs*/
    leg_diff = N / 2;
    /* step between values in twiddle factor array twiddle.h     */
    step = FFT_MAX_N / N;
    /* For N-point FFT                                           */

    for (i = 0; i < num_stages; i++)
    {
        index = 0;
        for (j = 0; j < leg_diff; j++)
        {
            for (upper_leg = j; upper_leg < N; upper_leg += (2 * leg_diff))
            {
                lower_leg = upper_leg + leg_diff;
                temp1.real = (Y[upper_leg]).real + (Y[lower_leg]).real;
                temp1.imag = (Y[upper_leg]).imag + (Y[lower_leg]).imag;
                temp2.real = (Y[upper_leg]).real - (Y[lower_leg]).real;
                temp2.imag = (Y[upper_leg]).imag - (Y[lower_leg]).imag;
                (Y[lower_leg]).real = temp2.real * (w[index]).real - temp2.imag * (w[index]).imag;
                (Y[lower_leg]).imag = temp2.real * (w[index]).imag + temp2.imag * (w[index]).real;
                (Y[upper_leg]).real = temp1.real;
                (Y[upper_leg]).imag = temp1.imag;
            }
            index += step;
        }
        leg_diff = leg_diff / 2;
        step *= 2;
    }
    /* bit reversal for resequencing data */
    j = 0;
    for (i = 1; i < (N - 1); i++)
    {
        k = N / 2;
        while (k <= j)
        {
            j = j - k;
            k = k / 2;
        }
        j = j + k;
        if (i < j)
        {
            temp1.real = (Y[j]).real;
            temp1.imag = (Y[j]).imag;
            (Y[j]).real = (Y[i]).real;
            (Y[j]).imag = (Y[i]).imag;
            (Y[i]).real = temp1.real;
            (Y[i]).imag = temp1.imag;
        }
    }
    return;
}

void IFFT(complex_t* Y, int N)
{
    for (int i = 0; i < N; ++i)
    {
        Y[i].imag = -Y[i].imag;
    }
    FFT(Y, N);
    for (int i = 0; i < N; ++i)
    {
        Y[i].real /=  N;
        Y[i].imag  = -Y[i].imag / N;
    }
}

// Compares magnitudes to deduce the most dominant frequency in the spectrum
int argmax_mag(complex_t *X, int size)
{
    int   best_idx = 1;
    float best_mag = X[1].real * X[1].real + X[1].imag * X[1].imag;

    for (int i = 2; i <= size / 2; ++i)
    {
        float m = X[i].real * X[i].real + X[i].imag * X[i].imag;
        if (m > best_mag)
        {
            best_mag = m;
            best_idx = i;
        }
    }
    return best_idx;
}

float get_frequency(void* user_data, int sps, int size)
{
    complex_t user_data_fd[size];
    for (int i = 0; i < size; ++i)
    {
        user_data_fd[i].real = ((float*)user_data)[i];
        user_data_fd[i].imag = 0.0f;
    }
    FFT(user_data_fd, size);

    int k = argmax_mag(user_data_fd, size);
    return (float)k * (float)sps / (float)size;
}
