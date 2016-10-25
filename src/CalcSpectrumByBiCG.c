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
#ifdef MPI
#include "komega/pkomega_bicg.h"
#else
#include "komega/komega_bicg.h"
#endif

/**
 * @file   CalcSpectrumByBiCG.c
 * @version 1.1
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 * 
 * @brief  File for givinvg functions of calculating spectrum by Lanczos
 * 
 * 
 */

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
  unsigned long int i, i_max;
  FILE *fp;
  int iret;
  unsigned long int liLanczosStp = X->Bind.Def.Lanczos_max;
  unsigned long int liLanczosStp_vec = 0;
  unsigned long int i_max_tmp;
  double complex *v12, *v14, *alphaCG, *betaCG, *res_save, z_seed, res_proj[1];
  int stp, one, status[3];
#ifdef MPI
  MPI_Comm comm;
  comm = MPI_COMM_WORLD;
#endif

  one = 1;

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
    fread(&liLanczosStp_vec, sizeof(liLanczosStp_vec), 1, fp);
    fread(&i_max, sizeof(long int), 1, fp);
    if (i_max != X->Bind.Check.idim_max) {
      fprintf(stderr, "Error: The size of the input vector is incorrect.\n");
      exitMPI(-1);
    }
    fread(v2, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fread(v12, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fread(v4, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fread(v14, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fclose(fp);
    fprintf(stdoutMPI, "  End:   Input vectors for recalculation.\n");
    TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_InputSpectrumRecalcvecEnd, "a");
  }
  /*
  
  */
  if (X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents ||
      X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents_VEC ||
      X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC)
  {
    iret = ReadTMComponents_BiCG(X, alphaCG, betaCG, &z_seed, res_save, &liLanczosStp);
    if (!iret == TRUE) {
      fprintf(stdoutMPI, "  Error: Fail to read TMcomponents\n");
      return FALSE;
    }

    if (X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents) {
        X->Bind.Def.Lanczos_restart = 0;
    }
    else if (X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC ||
             X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents_VEC) {
      if (liLanczosStp_vec != liLanczosStp) {
        fprintf(stdoutMPI, "  Error: Input files for vector and TMcomponents are incoorect. \n");
        fprintf(stdoutMPI, "  Error: Input vector %ld th stps, TMcomponents  %ld th stps.\n", liLanczosStp_vec, liLanczosStp);
        return FALSE;
      }
      X->Bind.Def.Lanczos_restart = liLanczosStp;
      liLanczosStp = liLanczosStp + X->Bind.Def.Lanczos_max;
    }
#ifdef MPI
    pkomega_BiCG_restart(&i_max, &one, &Nomega, dcSpectrum, dcomega, &liLanczosStp, &eps_Lanczos, &comm, status,
      &liLanczosStp, &v2[1], &v12[1], &v4[1], &v14[1], alphaCG, betaCG, &z_seed, res_save);
#else
    komega_BiCG_restart(&i_max, &one, &Nomega, dcSpectrum, dcomega, &liLanczosStp, &eps_Lanczos, status,
      &liLanczosStp, &v2[1], &v12[1], &v4[1], &v14[1], alphaCG, betaCG, &z_seed, res_save);
#endif
  }
  else {
#ifdef MPI
    pkomega_BiCG_init(&i_max, &one, &Nomega, dcSpectrum, dcomega, &liLanczosStp, &eps_Lanczos, &comm);
#else
    komega_BiCG_init(&i_max, &one, &Nomega, dcSpectrum, dcomega, &liLanczosStp, &eps_Lanczos);
#endif
  }

  // calculate ai, bi
  if (X->Bind.Def.iFlgCalcSpec == RECALC_NOT ||
      X->Bind.Def.iFlgCalcSpec == RECALC_OUTPUT_TMComponents_VEC ||
      X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents_VEC ||
      X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC
    )
  {
    fprintf(stdoutMPI, "    Start: Calculate tridiagonal matrix components.\n");
    TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_GetTridiagonalStart, "a");

    /*
    Set Maximum number of loop to the dimension of the Wavefunction
    */
    i_max_tmp = SumMPI_li(i_max);
    if (i_max_tmp < liLanczosStp) {
      liLanczosStp = i_max_tmp;
    }
    if (i_max_tmp < X->Bind.Def.LanczosTarget) {
      liLanczosStp = i_max_tmp;
    }
    /*
     BiCG loop
    */
    for (stp = X->Bind.Def.Lanczos_restart + 1; stp <= liLanczosStp; stp++) {

      mltply(X, v12, v2);
      mltply(X, v14, v4);

      pkomega_BiCG_update(&v12[1], &v2[1], &v14[1], &v4[1], dcSpectrum, res_proj, status);

      fprintf(stdoutMPI, "%d %d %d %25.15e\n", status[0], status[1], status[2], creal(v12[1]));
      if (status[0] < 0) break;
    }

    fprintf(stdoutMPI, "    End:   Calculate tridiagonal matrix components.\n\n");
    TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_GetTridiagonalEnd, "a");

    komega_BiCG_mp_komega_BiCG_getcoef();
    OutputTMComponents_BiCG(X, alphaCG, betaCG, z_seed, res_save, liLanczosStp);

  }//X->Bind.Def.iFlgCalcSpec == RECALC_NOT || RECALC_FROM_TMComponents_VEC

  //output vectors for recalculation
  if (X->Bind.Def.iFlgCalcSpec == RECALC_OUTPUT_TMComponents_VEC ||
      X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC) {

    fprintf(stdoutMPI, "    Start: Output vectors for recalculation.\n");
    TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_OutputSpectrumRecalcvecStart, "a");

    komega_BiCG_mp_komega_BiCG_getvec();

    sprintf(sdt, cFileNameOutputRestartVec, X->Bind.Def.CDataFileHead, myrank);
    if (childfopenALL(sdt, "wb", &fp) != 0) {
      exitMPI(-1);
    }
    fwrite(&liLanczosStp, sizeof(liLanczosStp), 1, fp);
    fwrite(&X->Bind.Check.idim_max, sizeof(X->Bind.Check.idim_max), 1, fp);
    fwrite(v2, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fwrite(v12, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fwrite(v4, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fwrite(v14, sizeof(complex double), X->Bind.Check.idim_max + 1, fp);
    fclose(fp);

    fprintf(stdoutMPI, "    End:   Output vectors for recalculation.\n");
    TimeKeeper(&(X->Bind), cFileNameTimeKeep, c_OutputSpectrumRecalcvecEnd, "a");
  }

  return TRUE;
}

int ReadTMComponents_BiCG(
  struct EDMainCalStruct *X,
  double complex *alphaCG,
  double complex *betaCG,
  double complex *z_seed,
  double complex *res_save,
  unsigned long int *_i_max
) {
  char sdt[D_FileNameMax];
  char ctmp[256];

  unsigned long int idx;
  unsigned long int i_max;
  double z_seed_r, z_seed_i, alpha_r, alpha_i, beta_r, beta_i, res_r, res_i;
  FILE *fp;

  idx = 0;
  sprintf(sdt, cFileNameTridiagonalMatrixComponents, X->Bind.Def.CDataFileHead);
  childfopenMPI(sdt, "r", &fp);

  fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp);
  sscanf(ctmp, "%ld \n", &i_max);
  if (X->Bind.Def.iFlgCalcSpec == RECALC_INOUT_TMComponents_VEC ||
    X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents_VEC) {
    alphaCG = (double complex*)realloc(alphaCG, sizeof(double complex)*(i_max + X->Bind.Def.Lanczos_max));
    betaCG = (double complex*)realloc(betaCG, sizeof(double complex)*(i_max + X->Bind.Def.Lanczos_max));
  }
  else if (X->Bind.Def.iFlgCalcSpec == RECALC_FROM_TMComponents) {
    alphaCG = (double complex*)realloc(alphaCG, sizeof(double complex)*i_max);
    betaCG = (double complex*)realloc(betaCG, sizeof(double complex)*i_max);
  }
  fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp);
  sscanf(ctmp, "%lf %lf\n", &z_seed_r, &z_seed_i);
  *z_seed = z_seed_r + I * z_seed_i;
  while (fgetsMPI(ctmp, sizeof(ctmp) / sizeof(char), fp) != NULL) {
    sscanf(ctmp, "%lf %lf %lf %lf %lf %lf\n",
      &alpha_r, &alpha_i, &beta_r, &beta_i, &res_r, &res_i);
    alphaCG[idx] = alpha_r + I * alpha_i;
    betaCG[idx] = beta_r + I * beta_i;
    res_save[idx] = res_r + I * res_i;
    idx += 1;
  }
  fclose(fp);
  *_i_max = i_max;
  return TRUE;
}


int OutputTMComponents_BiCG(
  struct EDMainCalStruct *X,
  double complex *alphaCG,
  double complex *betaCG,
  double complex z_seed,
  double complex *res_save,
  int liLanczosStp
)
{
  char sdt[D_FileNameMax];
  unsigned long int i;
  FILE *fp;

  sprintf(sdt, cFileNameTridiagonalMatrixComponents, X->Bind.Def.CDataFileHead);
  childfopenMPI(sdt, "w", &fp);
  fprintf(fp, "%d \n", liLanczosStp);
  fprintf(fp, "%.10lf \n", z_seed);
  for (i = 0; i < liLanczosStp; i++) {
    fprintf(fp, "%25.16le %25.16le %25.16le %25.16le %25.16le %25.16le\n",
      creal(alphaCG[i]), cimag(alphaCG[i]),
      creal(betaCG[i]), cimag(betaCG[i]),
      creal(res_save[i]), cimag(res_save[i]));
  }
  fclose(fp);
  return TRUE;
}
