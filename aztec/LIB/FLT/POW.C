#include "math.h"
#include "errno.h"

double pow(a,b)
double a,b;
{
	double loga;
	extern int errno;
	
	if (a<=0.0) {
		if (a<0.0 || a==0.0 && b<=0.0) {
			errno = EDOM;
			return -HUGE_VAL;
		}
		else return 0.0;
	}
	loga = log(a);
	loga *= b;
	if (loga > LOGHUGE) {
		errno = ERANGE;
		return HUGE_VAL;
	}
	if (loga < LOGTINY) {
		errno = ERANGE;
		return 0.0;
	}
	return exp(loga);
}
