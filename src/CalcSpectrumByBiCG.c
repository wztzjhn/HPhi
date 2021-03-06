/* HPhi  -  Quantum Lattice Model Simulator */
/* Copyright (C) 2015 Takahiro Misawa, Kazuyoshi Yoshimi, Mitsuaki Kawamura, Youhei Yamaji, Synge Todo, Naoki Kawashima */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#include "CalcSpectrumByLanczos.h"
#include "Lanczos_EigenValue.h"
#include "FileIO.h"
#include "wrapperMPI.h"
#include "mfmemory.h"
#if defined(MPI)
#include "komega/pkomega_bicg.h"
#else
#include "komega/komega_bicg.h"
#endif
#include "mltply.h"

/**
 * @file   CalcSpectrumByBiCG.c
 * @author Mitsuaki Kawamura (The University of Tokyo)
 * 
 * @brief  File for givinvg functions of calculating spectrum by Lanczos
 * 
 * 
 */


int ReadTMComponents_BiCG(
  struct EDMainCalStruct *X,
  double complex *v2,
  double complex *v4,
  double complex *v12,
  double complex *v14,
  int Nomega,
  double complex *dcSpectrum,
  double complex *dcomega
) {
  char sdt[D_FileNameMax];
  char ctmp[256];

  int one = 1, status[3], idim_max2int, liLanczosStp2int, max_step;
  unsigned long int idx;
  unsigned long int liLanczosStp;
  double complex *alphaCG, *betaCG, *res_save, z_seed;
  double z_seed_r, z_seed_i, alpha_r, alpha_i, beta_r, beta_i, res_r, res_i;
  FILE *fp;
#if defined(MPI)
  int comm;
  comm = MPI_Comm_c2f(MPI_COMM_WORLD);
#endif

  idx = 0;
  sprintf(sdt, cFileNameTridiagonalMatrixComponents, X->Bind.Def.CDataFileHead);
  childfopenMPI(sdt, "r", &fp);

  fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp);
  sscanf(ctmp, "%ld \n", &liLanczosStp);
  if (X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC ||
    X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents_VEC) {
    alphaCG = (double complex*)malloc((liLanczosStp + X->Bind.Def.Lanczos_max) * sizeof(double complex));
    betaCG = (double complex*)malloc((liLanczosStp + X->Bind.Def.Lanczos_max) * sizeof(double complex));
    res_save = (double complex*)malloc((liLanczosStp + X->Bind.Def.Lanczos_max) * sizeof(double complex));
  }
  else if (X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents) {
    alphaCG = (double complex*)malloc(liLanczosStp * sizeof(double complex));
    betaCG = (double complex*)malloc(liLanczosStp * sizeof(double complex));
    res_save = (double complex*)malloc(liLanczosStp * sizeof(double complex));
  }
  fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp);
  sscanf(ctmp, "%lf %lf\n", &z_seed_r, &z_seed_i);
  z_seed = z_seed_r + I * z_seed_i;
  while (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) != NULL) {
    sscanf(ctmp, "%lf %lf %lf %lf %lf %lf\n",
      &alpha_r, &alpha_i, &beta_r, &beta_i, &res_r, &res_i);
    alphaCG[idx] = alpha_r + I * alpha_i;
    betaCG[idx] = beta_r + I * beta_i;
    res_save[idx] = res_r + I * res_i;
    idx += 1;
  }
  fclose(fp);

  if (X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents) {
    X->Bind.Def.Lanczos_restart = 0;
    max_step = (int)liLanczosStp;
  }
  else if (X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC ||
    X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents_VEC) {
    X->Bind.Def.Lanczos_restart = liLanczosStp;
    max_step = (int)(liLanczosStp + X->Bind.Def.Lanczos_max);
  }

  idim_max2int = (int)X->Bind.Check.idim_max;
  liLanczosStp2int = (int)liLanczosStp;
#if defined(MPI)
  pkomega_bicg_restart(&idim_max2int, &one, &Nomega, dcSpectrum, dcomega, &max_step, &eps_Lanczos, &comm, status,
    &liLanczosStp2int, &v2[1], &v12[1], &v4[1], &v14[1], alphaCG, betaCG, &z_seed, res_save);
#else
  komega_bicg_restart(&idim_max2int, &one, &Nomega, dcSpectrum, dcomega, &max_step, &eps_Lanczos, status,
    &liLanczosStp2int, &v2[1], &v12[1], &v4[1], &v14[1], alphaCG, betaCG, &z_seed, res_save);
#endif
  free(alphaCG);
  free(betaCG);
  free(res_save);

  return TRUE;
}/*int ReadTMComponents_BiCG*/


int OutputTMComponents_BiCG(
  struct EDMainCalStruct *X,
  int liLanczosStp
)
{
  char sdt[D_FileNameMax];
  unsigned long int stp;
  FILE *fp;
  double complex *alphaCG, *betaCG, *res_save, z_seed;

  alphaCG = (double complex*)malloc(liLanczosStp * sizeof(double complex));
  betaCG = (double complex*)malloc(liLanczosStp * sizeof(double complex));
  res_save = (double complex*)malloc(liLanczosStp * sizeof(double complex));
#if defined(MPI)
  pkomega_bicg_getcoef(alphaCG, betaCG, &z_seed, res_save);
#else
  komega_bicg_getcoef(alphaCG, betaCG, &z_seed, res_save);
#endif

  sprintf(sdt, cFileNameTridiagonalMatrixComponents, X->Bind.Def.CDataFileHead);
  childfopenMPI(sdt, "w", &fp);
  fprintf(fp, "%d \n", liLanczosStp);
  fprintf(fp, "%.10lf %.10lf\n", creal(z_seed), cimag(z_seed));
  for (stp = 0; stp < liLanczosStp; stp++) {
    fprintf(fp, "%25.16le %25.16le %25.16le %25.16le %25.16le %25.16le\n",
      creal(alphaCG[stp]), cimag(alphaCG[stp]),
      creal(betaCG[stp]), cimag(betaCG[stp]),
      creal(res_save[stp]), cimag(res_save[stp]));
  }
  fclose(fp);
  free(alphaCG);
  free(betaCG);
  free(res_save);

  return TRUE;
}/*int OutputTMComponents_BiCG*/
/*
 Initialize Shadow Residual as a vrandom vector
*/
void InitShadowRes(
  struct BindStruct *X, double complex *v4)
{
  long int idim, iv;
  int mythread;
  double dnorm;
  /*
  For DSFMT
  */
  long unsigned int u_long_i;
  dsfmt_t dsfmt;

  iv = X->Def.initial_iv;
#pragma omp parallel default(none) private(idim, u_long_i, mythread, dsfmt) \
              shared(v4, iv, X, nthreads, myrank)
  {
    /*
     Initialise MT
    */
#ifdef _OPENMP
    mythread = omp_get_thread_num();
#else
    mythread = 0;
#endif
    u_long_i = 123432 + labs(iv) + mythread + nthreads * myrank;
    dsfmt_init_gen_rand(&dsfmt, u_long_i);

#pragma omp for
    for (idim = 1; idim <= X->Check.idim_max; idim++)
      v4[idim] = 2.0*(dsfmt_genrand_close_open(&dsfmt) - 0.5)
               + 2.0*(dsfmt_genrand_close_open(&dsfmt) - 0.5)*I;
  }/*#pragma omp parallel*/

  dnorm = sqrt(creal(VecProdMPI(X->Check.idim_max, v4, v4)));
#pragma omp parallel for default(none) shared(X,v4,dnorm) private(idim)
  for (idim = 1; idim <= X->Check.idim_max; idim++) v4[idim] /= dnorm;

}/*void InitShadowRes*/
/** 
 * @brief A main function to calculate spectrum by BiCG method
 * 
 * @param[in,out] X CalcStruct list for getting and pushing calculation information 
 * @retval 0 normally finished
 * @retval -1 error
 *
 * @author Mitsuaki Kawamura (The University of Tokyo)
 * 
 */
int CalcSpectrumByBiCG(		 
  struct EDMainCalStruct *X,
  double complex *vrhs,
  double complex *v2,
  double complex *v4,
  int Nomega,
  double complex *dcSpectrum,
  double complex *dcomega
)
{
  char sdt[D_FileNameMax];
  unsigned long int idim, i_max;
  FILE *fp;
  size_t byte_size;
  int iret, i_max2int, max_step;
  unsigned long int liLanczosStp_vec = 0;
  double complex *v12, *v14, res_proj;
  int stp, one = 1, status[3], iomega;
  double *resz;
#if defined(MPI)
  int comm;
  comm = MPI_Comm_c2f(MPI_COMM_WORLD);
#endif

  fprintf(stdoutMPI, "#####  Spectrum calculation with BiCG  #####\n\n");
  
  v12 = (double complex*)malloc((X->Bind.Check.idim_max + 1) * sizeof(double complex));
  v14 = (double complex*)malloc((X->Bind.Check.idim_max + 1) * sizeof(double complex));
  resz = (double*)malloc(Nomega * sizeof(double));

  /*
    Read residual vectors
  */
  if (X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents_VEC ||
      X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC) {
    fprintf(stdoutMPI, "  Start: Input vectors for recalculation.\n");
    TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_InputSpectrumRecalcvecStart, "a");

    sprintf(sdt, cFileNameOutputRestartVec, X->Bind.Def.CDataFileHead, myrank);
    if (childfopenALL(sdt, "rb", &fp) != 0) {
      exitMPI(-1);
    }
    byte_size = fread(&liLanczosStp_vec, sizeof(liLanczosStp_vec), 1, fp);
    byte_size = fread(&i_max, sizeof(long int), 1, fp);
    if (i_max != X->Bind.Check.idim_max) {
      fprintf(stderr, "Error: The size of the input vector is incorrect.\n");
      exitMPI(-1);
    }
    byte_size = fread(v2, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    byte_size = fread(v12, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    byte_size = fread(v4, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    byte_size = fread(v14, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fclose(fp);
    fprintf(stdoutMPI, "  End:   Input vectors for recalculation.\n");
    TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_InputSpectrumRecalcvecEnd, "a");
    if(byte_size == 0) printf("byte_size : %d\n", (int)byte_size);
  }
  else {
    i_max = X->Bind.Check.idim_max;
#pragma omp parallel for default(none) shared(i_max,v2,v4,vrhs) private(idim)
    for (idim = 1; idim <= i_max; idim++) {
      v2[idim] = vrhs[idim];
      v4[idim] = vrhs[idim];
    }
    //InitShadowRes(&(X->Bind), v4);
  }
  /*
    Input alpha, beta, projected residual, or start from scratch
  */
  if (X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents ||
      X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents_VEC ||
      X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC)
  {
    iret = ReadTMComponents_BiCG(X,v2,v4,v12,v14,Nomega,dcSpectrum,dcomega);
    if (!iret == TRUE) {
      fprintf(stdoutMPI, "  Error: Fail to read TMcomponents\n");
      return FALSE;
    }
  }
  else {
    i_max2int = (int)i_max;
    max_step = (int)X->Bind.Def.Lanczos_max;
#if defined(MPI)
    pkomega_bicg_init(&i_max2int, &one, &Nomega, dcSpectrum, dcomega, &max_step, &eps_Lanczos, &comm);
#else
    komega_bicg_init(&i_max2int, &one, &Nomega, dcSpectrum, dcomega, &max_step, &eps_Lanczos);
#endif
  }

  /*
  BiCG loop
  */
  fprintf(stdoutMPI, "    Start: Calculate tridiagonal matrix components.\n");
  TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_GetTridiagonalStart, "a");
  fprintf(stdoutMPI, "\n  Iteration     Status     Seed     Residual-2-Norm\n");
  childfopenMPI("residual.dat", "w", &fp);

  for (stp = 0; stp <= X->Bind.Def.Lanczos_max; stp++) {

#pragma omp parallel for default(none) shared(i_max,v12,v14) private(idim)
    for (idim = 1; idim <= i_max; idim++) {
      v12[idim] = 0.0;
      v14[idim] = 0.0;
    }

    iret = mltply(&X->Bind, v12, v2);
    iret = mltply(&X->Bind, v14, v4);

    res_proj = VecProdMPI(i_max, vrhs, v2);

#if defined(MPI)
    pkomega_bicg_update(&v12[1], &v2[1], &v14[1], &v4[1], dcSpectrum, &res_proj, status);
#else
    komega_bicg_update(&v12[1], &v2[1], &v14[1], &v4[1], dcSpectrum, &res_proj, status);
#endif

    if (stp % 10 == 0) {
#if defined(MPI)
      pkomega_bicg_getresidual(resz);
#else
      komega_bicg_getresidual(resz);
#endif
      for (iomega = 0; iomega < Nomega; iomega++) {
        fprintf(fp, "%7i %20.10e %20.10e %20.10e %20.10e\n", 
          stp, creal(dcomega[iomega]), 
          creal(dcSpectrum[iomega]), cimag(dcSpectrum[iomega]),
          resz[iomega]);
      }
      fprintf(fp, "\n");
    }

    fprintf(stdoutMPI, "  %9d  %9d %8d %25.15e\n", abs(status[0]), status[1], status[2], creal(v12[1]));
    if (status[0] < 0) break;
  }/*for (stp = 0; stp <= X->Bind.Def.Lanczos_max; stp++)*/
  fclose(fp);

  fprintf(stdoutMPI, "    End:   Calculate tridiagonal matrix components.\n\n");
  TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_GetTridiagonalEnd, "a");
  /*
   Save alpha, beta, projected residual
  */
  if (X->Bind.Def.iFlgCalcSpec != RECALC_FROM_TMComponents)
    OutputTMComponents_BiCG(X, abs(status[0]));
  /*
    output vectors for recalculation
  */
  if (X->Bind.Def.iFlgCalcSpec == RECALC_OUTPUT_TMComponents_VEC ||
      X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC) {

    fprintf(stdoutMPI, "    Start: Output vectors for recalculation.\n");
    TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_OutputSpectrumRecalcvecStart, "a");

#if defined(MPI)
    pkomega_bicg_getvec(&v12[1], &v14[1]);
#else
    komega_bicg_getvec(&v12[1], &v14[1]);
#endif

    sprintf(sdt, cFileNameOutputRestartVec, X->Bind.Def.CDataFileHead, myrank);
    if (childfopenALL(sdt, "wb", &fp) != 0) {
      exitMPI(-1);
    }
    byte_size = fwrite(&status[0], sizeof(status[0]), 1, fp);
    byte_size = fwrite(&X->Bind.Check.idim_max, sizeof(X->Bind.Check.idim_max), 1, fp);
    byte_size = fwrite(v2, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    byte_size = fwrite(v12, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    byte_size = fwrite(v4, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    byte_size = fwrite(v14, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fclose(fp);

    fprintf(stdoutMPI, "    End:   Output vectors for recalculation.\n");
    TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_OutputSpectrumRecalcvecEnd, "a");
  }

#if defined(MPI)
  pkomega_bicg_finalize();
#else
  komega_bicg_finalize();
#endif

  free(resz);
  free(v12);
  free(v14);

  return TRUE;
}/*int CalcSpectrumByBiCG*/
