/*
 * Random number generator -
 * adapted from the FORTRAN version 
 * in "Software Manual for the Elementary Functions"
 * by W.J. Cody, Jr and William Waite.
 */
double ran()
{
	static long int iy = 100001;
	
	iy *= 125;
	iy -= (iy/2796203) * 2796203;
	return (double) iy/ 2796203.0;
}

double randl(x)
double x;
{
	double exp();

	return exp(x*ran());
}
