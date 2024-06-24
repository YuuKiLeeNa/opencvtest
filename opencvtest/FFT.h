/****************************************************
		Pitch/Time Change using Sinusoidal Model
	    C code for floating point implementation
					Version 1.0
						1999
				  Author Luo Xin
	  University of Science and Technology of China
****************************************************/

/****************************************************
	File : fft.c

	Description:
		process the FFT and Inverse FFT.

	Function:
	global:
		fft():	process the FFT and Inverse FFT.
****************************************************/
#include <math.h>
/*#include <const.h>*/

/****************************************************
	Function:	fft()
	
	Description:process the FFT and Inverse FFT.

	Inputs:
		FLOAT * datam1:	Data to be used in FFT or IFFT
		int	nn:	the FFT SIZE
		int isign:	the sign index of FFT or IFFT
					-1:-----	FFT
					1 :-----	IFFT

	Outputs:
		FLOAT * datam1:	Data after FFT or IFFT transform

	Return Value:
		none

	Algorithm Description:
	Subroutine FFT: Fast Fourier Transform 		
* Replaces data by its DFT, if isign is 1, or replaces data   *
* by inverse DFT times nn if isign is -1.  data is a complex  *
* array of length nn, input as a real array of length 2*nn.   *
* nn MUST be an integer power of two.  This is not checked    *
* The real part of the number should be in the zeroeth        *
* of data , and the imaginary part should be in the next      *
* element.  Hence all the real parts should have even indeces *
* and the imaginary parts, odd indeces.			      *

* Data is passed in an array starting in position 0, but the  *
* code is copied from Fortran so uses an internal pointer     *
* which accesses position 0 as position 1, etc.		      *

	Improtance:!!!
* This code uses e+jwt sign convention, so isign should be    *
* reversed for e-jwt.                                         *
****************************************************/

#define	SWAP(a,b) tempr = (a);(a) = (b); (b) = tempr

void fft(float *datam1,int nn,int isign)

{
	int	n,mmax,m,j,istep,i;
	double	wtemp,wr,wpr,wpi,wi,theta;
	float register	tempr,tempi;
	float	*data;

	/*  Use pointer indexed from 1 instead of 0	*/
	data = &datam1[-1];

	n = nn << 1;
	j = 1;
	for( i = 1; i < n; i+=2 ) {
	  if ( j > i) {
		SWAP(data[j],data[i]);
		SWAP(data[j+1],data[i+1]);
	  }
	  m = n >> 1;
	  while ( m >= 2 && j > m ) {
		j -= m;
		m >>= 1;
	  }
	  j += m;
	}
	mmax = 2;
	while ( n > mmax) {
	  istep = 2 * mmax;
	  theta = 6.28318530717959/(isign*mmax);
	  wtemp = sin(0.5*theta);
	  wpr   = -2.0*wtemp*wtemp;
	  wpi   = sin(theta);
	  wr = 1.0;
	  wi = 0.0;
	  for ( m = 1; m < mmax;m+=2) {
	    for ( i = m; i <= n; i += istep) {
   	      	j = i + mmax;
		tempr = wr * data[j] - wi * data[j+1];
	      	tempi = wr * data[j+1] + wi * data[j];
	   	data[j] = data[i] - tempr;
		data[j+1] = data[i+1] - tempi;
		data[i] += tempr;
		data[i+1] += tempi;
	    }
	    wr = (wtemp=wr)*wpr-wi*wpi+wr;
	    wi = wi*wpr+wtemp*wpi+wi;
	  }
	  mmax = istep;
	}
}
