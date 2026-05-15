/*!**************************************************************************
\file    TBRDF.cpp
\author  Jiri Filip, UTIA AS CR
\date    June 2013
\version 1.00

  The main file for:
   - the BRDF dataset loading from binary file (doubles) 
   - linear values interpolation for the query incoming and ougoing directions
******************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "TBRDF.h"

#define PI 3.14159265358979323846

BRDF::BRDF()
{
  this->step_t = 15;
  this->step_p = 7.5;
  this->nti = 6;
  this->ntv = 6;
  this->npi = (int)(360.f/step_p);
  this->npv = (int)(360.f/step_p);
  this->planes = 3;

  Bd = new double [planes*nti*npi*ntv*npv];

}//--- BRDF ----------------------------------------------

BRDF::~BRDF()
{
    delete [] Bd;
    Bd = 0;
}//--- BRDF -----------------------------------------------------------

void
BRDF::load_brdf(char *fileName)
{
  FILE *fpr;
  if ((fpr=fopen(fileName,"rb"))==0)
    {
      printf("Error opening file '%s' !!!\nExiting...\n",fileName);
      exit(0);
    }

  int count = 0;
  for(int isp=0;isp<planes;isp++)
    {
      for(int ni=0;ni<nti*npi;ni++)
        for(int nv=0;nv<ntv*npv;nv++)
          Bd[count++] = readdouble(fpr);
    }
  fclose(fpr);
  printf("Reading BRDF from file %s ...done\n",fileName);

  return;
}//--- load_brdf -----------------------------------------------------

void
BRDF::lookup_brdf_val(float theta_i, float phi_i, float theta_v, float phi_v, double RGB[])
{
  float PI2 = PI*0.5;
  if(theta_i>PI2 || theta_v>PI2) {
    RGB[0] = 0.f;
    RGB[1] = 0.f;
    RGB[2] = 0.f;
    return;
  }

  float d2r = 180.f/PI;
  theta_i *= d2r;
  theta_v *= d2r;
  phi_i *= d2r;
  phi_v *= d2r;
  if(phi_i>=360.f)
    phi_i = 0.f;
  if(phi_v>=360.f)
    phi_v = 0.f;

  int iti[2],itv[2],ipi[2],ipv[2];
  iti[0] = (int)(floor(theta_i/step_t));
  iti[1] = iti[0]+1;
  if(iti[0]>nti-2)
    {
      iti[0] = nti-2;
      iti[1] = nti-1;
    }
  itv[0] = (int)(floor(theta_v/step_t));
  itv[1] = itv[0]+1;
  if(itv[0]>ntv-2)
    {
      itv[0] = ntv-2;
      itv[1] = ntv-1;
    }

  ipi[0] = (int)(floor(phi_i/step_p));
  ipi[1] = ipi[0]+1;
  ipv[0] = (int)(floor(phi_v/step_p));
  ipv[1] = ipv[0]+1;

  float sum;
  float wti[2],wtv[2],wpi[2],wpv[2];
  wti[1] = theta_i - (float)(step_t*iti[0]);
  wti[0] = (float)(step_t*iti[1]) - theta_i;
  sum = wti[0]+wti[1];
  wti[0] /= sum;
  wti[1] /= sum;
  wtv[1] = theta_v - (float)(step_t*itv[0]);
  wtv[0] = (float)(step_t*itv[1]) - theta_v;
  sum = wtv[0]+wtv[1];
  wtv[0] /= sum;
  wtv[1] /= sum;

  wpi[1] = phi_i - (float)(step_p*ipi[0]);
  wpi[0] = (float)(step_p*ipi[1]) - phi_i;
  sum = wpi[0]+wpi[1];
  wpi[0] /= sum;
  wpi[1] /= sum;
  wpv[1] = phi_v - (float)(step_p*ipv[0]);
  wpv[0] = (float)(step_p*ipv[1]) - phi_v;
  sum = wpv[0]+wpv[1];
  wpv[0] /= sum;
  wpv[1] /= sum;

  if(ipi[1]==npi) 
    ipi[1] = 0;
  if(ipv[1]==npv) 
    ipv[1] = 0;

  int nc = npv*ntv;
  int nr = npi*nti;
  for(int isp=0;isp<planes;isp++)
    {
      RGB[isp] = 0.f;
      for(int i=0;i<2;i++)
        for(int j=0;j<2;j++)
          for(int k=0;k<2;k++)
            for(int l=0;l<2;l++)
              RGB[isp] += Bd[isp*nr*nc + nc*(npi*iti[i]+ipi[k]) + npv*itv[j]+ipv[l]] * wti[i] * wtv[j] * wpi[k] * wpv[l];

      //      RGB[isp] *= cos(theta_i/d2r);
    }
  return;
}//--- lookup_brdf_val -----------------------------------------------------

