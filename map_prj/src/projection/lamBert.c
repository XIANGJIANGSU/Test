
#include <math.h>
#include "lamBert.h"

/***************************************************************************/
/*
*                              DEFINES
*/
#define PI (3.14159265358979)
#define FLT_EPSILON_DXJ 1.192092896E-07F

/***************************************************************************/
/*                               DEFINES
*
*/

#define PI_OVER_2  (PI / 2.0)
#define PI_OVER_4  (PI / 4.0)
#define LAMBERT_MAX_LAT    (( PI * 89.0) / 180.0)  /* 89 degrees in radians */
#define TWO_PI     (2.0 * PI)
#define LAMBERT_m(clat,essin)                  (clat / sqrt(1.0 - essin * essin))
#define LAMBERT_t(lat,essin)                   tan(PI_OVER_4 -lat / 2) / pow((1.0 - essin) / (1.0 + essin), Lambert_es_OVER_2)
#define ES_SIN(sinlat)                         (Lambert_es * sinlat)

/**************************************************************************/
/*                               GLOBAL DECLARATIONS
*
*/



/* Ellipsoid Parameters, default to WGS 84  */
static double Lambert_a = 6378137.0;               /* Semi-major axis of ellipsoid in meters */
static double Lambert_b = 6356752.3142;            /* Semi-minor axis of ellipsoid in meters */
static double Lambert_es = 0.08181919084262188000;         /* Eccentricity of ellipsoid */
static double Lambert_es2 = 0.0066943799901413800;         /* Eccentricity squared */
static double Lambert_es_OVER_2 = .040909595421311;       /* Eccentricity / 2.0 */
static double Lambert_n = .70802074516367;              /* Ratio of angle between meridians */
static double Lambert_F = 1.8538379832459;              /* Flattening of ellipsoid */
static double Lambert_rho0 = 6356377.2707128;           /* Height above ellipsoid */
static double Lambert_aF = 11824032.632946;     /* Lambert_a * Lambert_F */

/* Lambert_Conformal_Conic projection Parameters */
static double Lambert_Std_Parallel_1 = (40 * PI / 180); /* Lower std. parallel in radians */
static double Lambert_Std_Parallel_2 = (50 * PI / 180); /* Upper std. parallel in radians */
static double Lambert_Origin_Lat = (45 * PI / 180);     /* Latitude of origin in radians */
static double Lambert_Origin_Long = 0.0;                /* Longitude of origin in radians */
static double Lambert_False_Northing = 0.0;             /* False northing in meters */
static double Lambert_False_Easting = 0.0;              /* False easting in meters */


/* Maximum variance for easting and northing values for WGS 84. */
static double Lambert_Delta_Easting = 40000000.0;
static double Lambert_Delta_Northing = 40000000.0;

/*
/////////////////////////////////////////////////////////////////////////////
// CLCCProjection
*/
unsigned int Set_Lambert_Parameters(double a,
						   double b,
						   double Origin_Latitude,
						   double Central_Meridian,
						   double Std_Parallel_1,
						   double Std_Parallel_2,
						   double False_Easting,
						   double False_Northing)
						   
{ /* BEGIN Set_Lambert_Parameters */
  /*
  * The function Set_Lambert_Parameters receives the ellipsoid parameters and
  * Lambert Conformal Conic projection parameters as inputs, and sets the
  * corresponding state variables.  If any errors occur, the error code(s)
  * are returned by the function, otherwise LAMBERT_NO_ERROR is returned.
  *
  *   a                   : Semi-major axis of ellipsoid, in meters   (input)
  *   b                   : Semi-minor axis of ellipsoid, in meters   (input)
  *   Origin_Latitude     : Latitude of origin in radians             (input)
  *   Central_Meridian    : Longitude of origin in radians            (input)
  *   Std_Parallel_1      : First standard parallel                   (input)
  *   Std_Parallel_2      : Second standard parallel                  (input)
  *   False_Easting       : False easting in meters                   (input)
  *   False_Northing      : False northing in meters                  (input)
	*/
	
	double a2, b2;
	double slat, slat1, clat;
	double es_sin;
	double t0, t1, t2;
	double m1, m2;
	unsigned int Error_Code = LAMBERT_NO_ERROR;
	
	if (a <= 0.0)
	{ /* Semi-major axis must be greater than zero */
		Error_Code |= LAMBERT_A_ERROR;
	}
	if (b <= 0.0)
	{ /* Semi-minor axis must be greater than zero */
		Error_Code |= LAMBERT_B_ERROR;
	}
	if (a < b)
	{ /* Semi-major axis can not be less than Semi-minor axis */
		Error_Code |= LAMBERT_A_LESS_B_ERROR;
	}
	if ((Origin_Latitude < -PI_OVER_2) || (Origin_Latitude > PI_OVER_2))
	{ /* Origin Latitude out of range */
		Error_Code |= LAMBERT_ORIGIN_LAT_ERROR;
	}
	if ((Std_Parallel_1 < -LAMBERT_MAX_LAT) || (Std_Parallel_1 > LAMBERT_MAX_LAT))
	{ /* First Standard Parallel out of range */
		Error_Code |= LAMBERT_FIRST_STDP_ERROR;
	}
	if ((Std_Parallel_2 < -LAMBERT_MAX_LAT) || (Std_Parallel_2 > LAMBERT_MAX_LAT))
	{ /* Second Standard Parallel out of range */
		Error_Code |= LAMBERT_SECOND_STDP_ERROR;
	}
	/*if((Std_Parallel_1 == 0) && (Std_Parallel_2 == 0))*/  /*2013-12-10 11:50 DXJ modify*/
	if ((fabs(Std_Parallel_1) < FLT_EPSILON_DXJ) && (fabs(Std_Parallel_2) < FLT_EPSILON_DXJ))
	{ /* First & Second Standard Parallels are both 0 */
		Error_Code |= LAMBERT_FIRST_SECOND_ERROR;
	}
	/*if (Std_Parallel_1 == -Std_Parallel_2)*/
	if ( (Std_Parallel_1 + Std_Parallel_2) < FLT_EPSILON_DXJ)
	{ /* Parallels are the negation of each other */
		Error_Code |= LAMBERT_HEMISPHERE_ERROR;
	}
	if ((Central_Meridian < -PI) || (Central_Meridian > TWO_PI))
	{ /* Origin Longitude out of range */
		Error_Code |= LAMBERT_CENT_MER_ERROR;
	}
	
	if (LAMBERT_NO_ERROR == Error_Code)
	{ /* no errors */
		
		Lambert_a = a;
		Lambert_b = b;
		Lambert_Origin_Lat = Origin_Latitude;
		Lambert_Std_Parallel_1 = Std_Parallel_1;
		Lambert_Std_Parallel_2 = Std_Parallel_2;
		if (Central_Meridian > PI)
			Central_Meridian -= TWO_PI;
		Lambert_Origin_Long = Central_Meridian;
		Lambert_False_Easting = False_Easting;
		Lambert_False_Northing = False_Northing;
		
		
		a2 = a * a;
		b2 = b * b;
		Lambert_es2 = (a2 - b2) / a2;
		Lambert_es = sqrt(Lambert_es2);
		Lambert_es_OVER_2 = Lambert_es / 2.0;
		
		slat = sin(Lambert_Origin_Lat);
		es_sin = ES_SIN(slat);
		t0 = LAMBERT_t(Lambert_Origin_Lat, es_sin);
		
		slat1 = sin(Lambert_Std_Parallel_1);
		clat = cos(Lambert_Std_Parallel_1);
		es_sin = ES_SIN(slat1);
		m1 = LAMBERT_m(clat, es_sin);
		t1 = LAMBERT_t(Lambert_Std_Parallel_1, es_sin);
		
		
		if (fabs(Lambert_Std_Parallel_1 - Lambert_Std_Parallel_2) > 1.0e-10)
		{
			slat = sin(Lambert_Std_Parallel_2);
			clat = cos(Lambert_Std_Parallel_2);
			es_sin = ES_SIN(slat);
			m2 = LAMBERT_m(clat, es_sin);
			t2 = LAMBERT_t(Lambert_Std_Parallel_2, es_sin);
			Lambert_n = log(m1 / m2) / log(t1 / t2);
		}
		else
			Lambert_n = slat1;
		Lambert_F = m1 / (Lambert_n * pow(t1, Lambert_n));
		Lambert_aF = Lambert_a * Lambert_F;
		Lambert_rho0 = Lambert_aF * pow(t0, Lambert_n);
		
	}
	return (Error_Code);
} /* END OF Set_Lambert_Parameters */

/*Ϊ�˱�֤��ģ�鹦�ܵ������ԣ�Ӧ���д˹��ܣ�����*/
void Get_Lambert_Parameters(double *a,
							double *b,
							double *Origin_Latitude,
							double *Central_Meridian,
							double *Std_Parallel_1,
							double *Std_Parallel_2,
							double *False_Easting,
							double *False_Northing)
							
{ /* BEGIN Get_Lambert_Parameters */
  /*                         
  * The function Get_Lambert_Parameters returns the current ellipsoid
  * parameters and Lambert Conformal Conic projection parameters.
  *
  *   a                   : Semi-major axis of ellipsoid, in meters   (output)
  *   b                   : Semi-minor axis of ellipsoid, in meters   (output)
  *   Origin_Latitude     : Latitude of origin, in radians            (output)
  *   Central_Meridian    : Longitude of origin, in radians           (output)
  *   Std_Parallel_1      : First standard parallel                   (output)
  *   Std_Parallel_2      : Second standard parallel                  (output)
  *   False_Easting       : False easting, in meters                  (output)
  *   False_Northing      : False northing, in meters                 (output)
	*/
	
	if(0 == a || 0 == b || 0 == Origin_Latitude ||
	   0 == Central_Meridian || 0 == Std_Parallel_1 || 
	   0 == Std_Parallel_2 || 0 == False_Easting || 0 == False_Northing)
		return;
	*a = Lambert_a;
	*b = Lambert_b;
	*Std_Parallel_1 = Lambert_Std_Parallel_1;
	*Std_Parallel_2 = Lambert_Std_Parallel_2;
	*Origin_Latitude = Lambert_Origin_Lat;
	*Central_Meridian = Lambert_Origin_Long;
	*False_Easting = Lambert_False_Easting;
	*False_Northing = Lambert_False_Northing;
	return;
} /* END OF Get_Lambert_Parameters */


unsigned int Convert_Geodetic_To_Lambert(double Latitude,
								double Longitude,
								double *Easting,
								double *Northing)
								
{ /* BEGIN Convert_Geodetic_To_Lambert */
  /*
  * The function Convert_Geodetic_To_Lambert converts Geodetic (latitude and
  * longitude) coordinates to Lambert Conformal Conic projection (easting
  * and northing) coordinates, according to the current ellipsoid and
  * Lambert Conformal Conic projection parameters.  If any errors occur, the
  * error code(s) are returned by the function, otherwise LAMBERT_NO_ERROR is
  * returned.
  *
  *    Latitude         : Latitude in radians                         (input)
  *    Longitude        : Longitude in radians                        (input)
  *    Easting          : Easting (X) in meters                       (output)
  *    Northing         : Northing (Y) in meters                      (output)
	*/
	
	double slat;
	double es_sin;
	double t;
	double rho;
	double dlam;
	double theta;
	unsigned int  Error_Code = LAMBERT_NO_ERROR;
	
	if(0 == Easting || 0 == Northing)
	{ 
		Error_Code |= LAMBERT_OUTPUT_ERROR;
	}
	if ((Latitude < -PI_OVER_2) || (Latitude > PI_OVER_2))
	{  /* Latitude out of range */
		Error_Code|= LAMBERT_LAT_ERROR;
	}
	if ((Longitude < -PI) || (Longitude > TWO_PI))
	{  /* Longitude out of range */
		Error_Code|= LAMBERT_LON_ERROR;
	}
	
	if (LAMBERT_NO_ERROR == Error_Code)
	{ /* no errors */
		
		if (fabs(fabs(Latitude) - PI_OVER_2) > 1.0e-10)
		{
			slat = sin(Latitude);
			es_sin = ES_SIN(slat);
			t = LAMBERT_t(Latitude, es_sin);
			rho = Lambert_aF * pow(t, Lambert_n);
		}
		else
		{
			if ((Latitude * Lambert_n) <= 0)
			{ /* Point can not be projected */
				Error_Code |= LAMBERT_LAT_ERROR;
				return (Error_Code);
			}
			rho = 0.0;
		}
		
		dlam = Longitude - Lambert_Origin_Long;
		if (dlam > PI)
		{
			dlam -= TWO_PI;
		}
		if (dlam < -PI)
		{
			dlam += TWO_PI;
		}
		theta = Lambert_n * dlam;
		
		*Easting = rho * sin(theta) + Lambert_False_Easting;
		*Northing = Lambert_rho0 - rho * cos(theta) + Lambert_False_Northing;
		
	}
	return (Error_Code);
} /* END OF Convert_Geodetic_To_Lambert */



unsigned int Convert_Lambert_To_Geodetic(double Easting,
								double Northing,
								double *Latitude,
								double *Longitude)
								
{ /* BEGIN Convert_Lambert_To_Geodetic */
  /*
  * The function Convert_Lambert_To_Geodetic converts Lambert Conformal
  * Conic projection (easting and northing) coordinates to Geodetic
  * (latitude and longitude) coordinates, according to the current ellipsoid
  * and Lambert Conformal Conic projection parameters.  If any errors occur,
  * the error code(s) are returned by the function, otherwise LAMBERT_NO_ERROR
  * is returned.
  *
  *    Easting          : Easting (X) in meters                       (input)
  *    Northing         : Northing (Y) in meters                      (input)
  *    Latitude         : Latitude in radians                         (output)
  *    Longitude        : Longitude in radians                        (output)
	*/
	
	
	double dy, dx;
	double rho, rho0_MINUS_dy;
	double t;
	double PHI;
	double tempPHI = 0.0;
	double sin_PHI;
	double es_sin;
	double theta = 0.0;
	double tolerance = 4.85e-10;
	unsigned int Error_Code = LAMBERT_NO_ERROR;
	
	if(0 == Latitude || 0 == Longitude)
	{ 
		Error_Code |= LAMBERT_OUTPUT_ERROR;
	}
	if ((Easting < (Lambert_False_Easting - Lambert_Delta_Easting))
		||(Easting > (Lambert_False_Easting + Lambert_Delta_Easting)))
	{ /* Easting out of range  */
		Error_Code |= LAMBERT_EASTING_ERROR;
	}
	if ((Northing < (Lambert_False_Northing - Lambert_Delta_Northing))
		|| (Northing > (Lambert_False_Northing + Lambert_Delta_Northing)))
	{ /* Northing out of range */
		Error_Code |= LAMBERT_NORTHING_ERROR;
	}
	
	if (LAMBERT_NO_ERROR == Error_Code)
	{ /* no errors */
		
		dy = Northing - Lambert_False_Northing;
		dx = Easting - Lambert_False_Easting;
		rho0_MINUS_dy = Lambert_rho0 - dy;
		rho = sqrt(dx * dx + (rho0_MINUS_dy) * (rho0_MINUS_dy));
		
		if (Lambert_n < 0.0)
		{
			rho *= -1.0;
			dy *= -1.0;
			dx *= -1.0;
			rho0_MINUS_dy *= -1.0;
		}
		
		/*if(rho != 0.0)*/  /*2013-12-10 11:50 DXJ*/
		if ( fabs(rho) >= FLT_EPSILON_DXJ)
		{
			theta = atan2(dx, rho0_MINUS_dy);
			t = pow(rho / (Lambert_aF) , 1.0 / Lambert_n);
			PHI = PI_OVER_2 - 2.0 * atan(t);
			while (fabs(PHI - tempPHI) > tolerance)
			{
				tempPHI = PHI;
				sin_PHI = sin(PHI);
				es_sin = ES_SIN(sin_PHI);
				PHI = PI_OVER_2 - 2.0 * atan(t * pow((1.0 - es_sin) / (1.0 + es_sin), Lambert_es_OVER_2));
			}
			*Latitude = PHI;
			*Longitude = theta / Lambert_n + Lambert_Origin_Long;
			
			if (*Latitude > PI_OVER_2)  /* force distorted values to 90, -90 degrees */
				*Latitude = PI_OVER_2;
			else if (*Latitude < -PI_OVER_2)
				*Latitude = -PI_OVER_2;
			
			if (*Longitude > PI)
				*Longitude -= TWO_PI;
			if (*Longitude < -PI)
				*Longitude += TWO_PI;
			
			if (*Longitude > PI)  /* force distorted values to 180, -180 degrees */
				*Longitude = PI;
			else if (*Longitude < -PI)
				*Longitude = -PI;
		}
		else
		{
			if (Lambert_n > 0.0)
				*Latitude = PI_OVER_2;
			else
				*Latitude = -PI_OVER_2;
			*Longitude = Lambert_Origin_Long;
			
		}
	}
	return (Error_Code);
} /* END OF Convert_Lambert_To_Geodetic */
