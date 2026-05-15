/*!**************************************************************************
\file    TBRDF.h
\author  Jiri Filip, UTIA AS CR
\date    June 2013
\version 1.00

  The header file for:
   - the BRDF dataset loading from binary file (doubles) 
   - linear values interpolation for the query incoming and ougoing directions
******************************************************************************/

#ifndef TBRDF_c
#define TBRDF_c

class BRDF{
 public:
  char *path;
  char *matName;
  int step;
  float step_t,step_p;
  int nti,ntv,npi,npv,planes; //!\brief BRDF dimensions
  double *Bd;                 //!\brief BRDF data

 public:
  BRDF();
  ~BRDF();

  union {     double f;
    unsigned char c[8];
  } u;

  double readdouble(FILE *f) {
    double v;
    fread((void*)(&v), sizeof(v), 1, f);
    return v;
  }

  void load_brdf(char *fileName);
  void lookup_brdf_val(float theta_i, float phi_i, float theta_v, float phi_v, double RGB[]);
}; //--- BRDF -----------------------------------------------

#endif
