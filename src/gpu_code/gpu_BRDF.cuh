/**
 * @file gpu_BRDF.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_BRDF class
 */

#pragma once
#include <cuda_runtime_api.h>
#include "../include/TBRDF.h"
#include <cmath>

#define PI 3.14159265358979323846

/**
 * @class gpu_BRDF
 *
 * GPU compatible BRDF class
 */
class gpu_BRDF {
public:
    int step;
    float step_t,step_p;
    int nti,ntv,npi,npv,planes; //!\brief BRDF dimensions
    double *Bd;                 //!\brief BRDF data

    /**
     * Constructor from a CPU BRDF
     *
     * @param[in] brdf     CPU BRDF
     */
    __host__
    gpu_BRDF(const BRDF& brdf): step(brdf.step), step_t(brdf.step_t), step_p(brdf.step_p), nti(brdf.nti), ntv(brdf.ntv), npi(brdf.npi), npv(brdf.npv), planes(brdf.planes), Bd(nullptr) {
        cudaMallocManaged(reinterpret_cast<void **>(&Bd), planes*nti*npi*ntv*npv*sizeof(double));
        memcpy(Bd, brdf.Bd, planes*nti*npi*ntv*npv*sizeof(double));
    }

    /**
     * Simple destructor that frees the CUDA memory
     */
    __host__ __device__
    ~gpu_BRDF() {
        if (Bd != nullptr) cudaFree(Bd);
    }

    /**
     * Samples the BRDF based on the view and light elevation and azimuth
     *
     * @param[in] theta_i   light elevation
     * @param[in] phi_i     light azimuth
     * @param[in] theta_v   view elevation
     * @param[in] phi_v     view azimuth
     * @param[out] RGB      output RGB value
     */
    __device__
    void lookup_brdf_val(float theta_i, float phi_i, float theta_v, float phi_v, double RGB[]) const {
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
    }
};
