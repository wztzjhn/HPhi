/* HPhi  -  Quantum Lattice Model Simulator */
/* Copyright (C) 2015 The University of Tokyo */

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

//Define Mode for mltply
// complex version

#ifdef MPI
#include "mpi.h"
#endif
#include "Common.h"
#include "mltplyCommon.h"
#include "mltplyHubbardCore.h"
#include "mltplyMPIHubbard.h"
#include "mltplyMPIHubbardCore.h"
#include "bitcalc.h"
#include "wrapperMPI.h"

int CheckPE(
	    int org_isite,
	    struct BindStruct *X
	    )
{
  if(org_isite+1 > X->Def.Nsite){
    return TRUE;
  }
  else{
    return FALSE;
  }
}

int CheckBit_Cis(
		 long unsigned int is1_spin,
		 long unsigned int orgbit,
		 long unsigned int *offbit
){
  long unsigned int ibit_tmp;
  ibit_tmp = orgbit & is1_spin;
  if(ibit_tmp == 0){
    *offbit = orgbit+is1_spin;
    return TRUE;
  }
  *offbit=0;
  return FALSE;
}

int CheckBit_Ajt(
		 long unsigned int is1_spin,
		 long unsigned int orgbit,
		 long unsigned int *offbit
){
  long unsigned int ibit_tmp;
  ibit_tmp = orgbit & is1_spin;
  if(ibit_tmp != 0){
    *offbit = orgbit-is1_spin;
    return TRUE;
  }
  *offbit=0;
  return FALSE;
}

int CheckBit_InterAllPE(
			int org_isite1,
			int org_isigma1,
			int org_isite2,
			int org_isigma2,
			int org_isite3,
			int org_isigma3,
			int org_isite4,
			int org_isigma4,
			struct BindStruct *X,
			long unsigned int orgbit,
			long unsigned int *offbit
			)
{
  long unsigned int tmp_ispin;
  long unsigned int tmp_org, tmp_off;
  int iflgBitExist = TRUE;
  tmp_org=orgbit;
  
  if(CheckPE(org_isite1, X)==TRUE){
    tmp_ispin = X->Def.Tpow[2*org_isite1+org_isigma1];
    if(CheckBit_Ajt(tmp_ispin, tmp_org, &tmp_off) != TRUE){
      iflgBitExist=FALSE;
    }
    tmp_org = tmp_off;
  }

  if(CheckPE(org_isite2, X)==TRUE){
    tmp_ispin = X->Def.Tpow[2*org_isite2+org_isigma2];
    if(CheckBit_Cis(tmp_ispin, tmp_org, &tmp_off) != TRUE){
      iflgBitExist=FALSE;
    }
    tmp_org = tmp_off;
  }
  
  if(CheckPE(org_isite3, X)==TRUE){
    tmp_ispin = X->Def.Tpow[2*org_isite3+org_isigma3];
    if(CheckBit_Ajt(tmp_ispin, tmp_org, &tmp_off) != TRUE){
      iflgBitExist=FALSE;
    }
    tmp_org = tmp_off;
  }

  if(CheckPE(org_isite4, X)==TRUE){
    tmp_ispin = X->Def.Tpow[2*org_isite4+org_isigma4];
    if(CheckBit_Cis(tmp_ispin, tmp_org, &tmp_off) != TRUE){
      iflgBitExist=FALSE;
    }
    tmp_org = tmp_off;
  }

  if(iflgBitExist != TRUE){
    *offbit=0;
    return FALSE;
  }
  
  *offbit=tmp_org;
  return TRUE;
}

int CheckBit_PairPE(
		    int org_isite1,
		    int org_isigma1,
		    int org_isite3,
		    int org_isigma3,
		    struct BindStruct *X,
		    long unsigned int orgbit
		    )
{
  long unsigned int tmp_ispin;
  long unsigned int tmp_org, tmp_off;
  int iflgBitExist = TRUE;
  tmp_org=orgbit;
  
  if(CheckPE(org_isite1, X)==TRUE){
    tmp_ispin = X->Def.Tpow[2*org_isite1+org_isigma1];
    if(CheckBit_Ajt(tmp_ispin, tmp_org, &tmp_off) != TRUE){
      iflgBitExist=FALSE;
    }
  }
  
  if(CheckPE(org_isite3, X)==TRUE){
    tmp_ispin = X->Def.Tpow[2*org_isite3+org_isigma3];
    if(CheckBit_Ajt(tmp_ispin, tmp_org, &tmp_off) != TRUE){
      iflgBitExist=FALSE;
    }
  }
  
  if(iflgBitExist != TRUE){
    return FALSE;
  }

  return TRUE;
}

int GetSgnInterAll(
		   int isite1,
		   int isite2,
		   int isite3,
		   int isite4,
		   int *Fsgn,
		   struct BindStruct *X,
		   unsigned long int orgbit,
		   unsigned long int *offbit
		   )
{
  long unsigned int diffA;
  long unsigned int tmp_off;
  long unsigned int tmp_ispin1, tmp_ispin2;
  int tmp_sgn=0;

  tmp_ispin1=isite1;
  tmp_ispin2=isite2;
 
  if(tmp_ispin1 == tmp_ispin2){
    if( (orgbit & tmp_ispin1) == 0){
      *offbit =0;
      *Fsgn = tmp_sgn;
      return FALSE;
    }
    tmp_sgn=1;
    tmp_off =orgbit;
  }
  else{
    
    if(tmp_ispin2 > tmp_ispin1) diffA = tmp_ispin2 - tmp_ispin1*2;
    else diffA = tmp_ispin1-tmp_ispin2*2;  
    
    tmp_sgn=X_GC_CisAjt(orgbit, X, tmp_ispin1, tmp_ispin2, tmp_ispin1+tmp_ispin2, diffA, &tmp_off);
    
    if(tmp_sgn ==0){
      *offbit =0;
      *Fsgn = 0;
      return FALSE;
    }
  }

  tmp_ispin1 = isite3;
  tmp_ispin2 = isite4;
  if(tmp_ispin1 == tmp_ispin2){
    if( (tmp_off & tmp_ispin1) == 0){
      *offbit =0;
      *Fsgn = 0;
      return FALSE;
    }
    *offbit=tmp_off;
  }
  else{
    if(tmp_ispin2 > tmp_ispin1) diffA = tmp_ispin2 - tmp_ispin1*2;
    else diffA = tmp_ispin1-tmp_ispin2*2;  
    
    tmp_sgn *=X_GC_CisAjt(tmp_off, X, tmp_ispin1, tmp_ispin2, tmp_ispin1+tmp_ispin2, diffA, offbit);
    
    if(tmp_sgn ==0){
      *offbit =0;
      *Fsgn = 0;
      return FALSE;
    }
  }
  
  *Fsgn =tmp_sgn;
  *offbit = *offbit%X->Def.OrgTpow[2*X->Def.Nsite];
  
  // exitMPI(-1);
  return TRUE;
}

double complex X_GC_child_CisAisCjtAjt_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 int org_isite3,
 int org_ispin3,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 )
{
#ifdef MPI
  double complex dam_pr=0.0;
  int iCheck;
  unsigned long int tmp_ispin1;
  unsigned long int i_max = X->Check.idim_max;
  unsigned long int tmp_off, j;
  double complex dmv;
//  MPI_Status statusMPI;

  iCheck=CheckBit_PairPE(org_isite1, org_ispin1, org_isite3, org_ispin3, X, (long unsigned int) myrank);
  if(iCheck != TRUE){
    return 0.0;
  }
  if(org_isite1+1 > X->Def.Nsite && org_isite3+1 > X->Def.Nsite) {
      if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X) private(dmv, j, tmp_off)
          for (j = 1; j <= i_max; j++) {
              dmv = tmp_v1[j] * tmp_V;
              tmp_v0[j] += dmv;
              dam_pr += conj(tmp_v1[j]) * dmv;
          }
      }
      else {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X) private(dmv, j, tmp_off)
          for (j = 1; j <= i_max; j++) {
              dmv = tmp_v1[j] * tmp_V;
              dam_pr += conj(tmp_v1[j]) * dmv;
          }
      }
  }
  else if (org_isite1+1 > X->Def.Nsite || org_isite3+1 > X->Def.Nsite) {
      if (org_isite1 > org_isite3) {
          tmp_ispin1 = X->Def.Tpow[2 * org_isite3 + org_ispin3];
      }
      else {
          tmp_ispin1 = X->Def.Tpow[2 * org_isite1 + org_ispin1];
      }
      if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) { // for multply
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X, tmp_ispin1) private(dmv, j, tmp_off)
          for (j = 1; j <= i_max; j++) {
              if (CheckBit_Ajt(tmp_ispin1, j - 1, &tmp_off) == TRUE) {
                  dmv = tmp_v1[j] * tmp_V;
                  tmp_v0[j] += dmv;
                  dam_pr += conj(tmp_v1[j]) * dmv;

              }
          }
      }
      else {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X, tmp_ispin1) private(dmv, j, tmp_off)
          for (j = 1; j <= i_max; j++) {
              if (CheckBit_Ajt(tmp_ispin1, j - 1, &tmp_off) == TRUE) {
                  dmv = tmp_v1[j] * tmp_V;
                  dam_pr += conj(tmp_v1[j]) * dmv;
              }
          }
      }
  }
  return dam_pr;
#endif
}

double complex X_GC_child_CisAjtCkuAku_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 int org_isite2,
 int org_ispin2,
 int org_isite3,
 int org_ispin3,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 ) {
#ifdef MPI
    double complex dam_pr = 0.0;
    unsigned long int i_max = X->Check.idim_max;
    unsigned long int idim_max_buf;
    int iCheck, ierr, Fsgn;
    unsigned long int isite1, isite2, isite3;
    unsigned long int tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4;
    unsigned long int j, Asum, Adiff;
    double complex dmv;
    unsigned long int origin, tmp_off;
    unsigned long int org_rankbit;
    MPI_Status statusMPI;

    iCheck = CheckBit_InterAllPE(org_isite1, org_ispin1, org_isite2, org_ispin2, org_isite3, org_ispin3, org_isite3,
                                 org_ispin3, X, (long unsigned int) myrank, &origin);
    isite1 = X->Def.Tpow[2 * org_isite1 + org_ispin1];
    isite2 = X->Def.Tpow[2 * org_isite2 + org_ispin2];
    isite3 = X->Def.Tpow[2 * org_isite3 + org_ispin3];

    if (iCheck == TRUE) {
        tmp_isite1 = X->Def.OrgTpow[2 * org_isite1 + org_ispin1];
        tmp_isite2 = X->Def.OrgTpow[2 * org_isite2 + org_ispin2];
        tmp_isite3 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
        tmp_isite4 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
        Asum = tmp_isite1 + tmp_isite2;
        if (tmp_isite2 > tmp_isite1) Adiff = tmp_isite2 - tmp_isite1 * 2;
        else Adiff = tmp_isite1 - tmp_isite2 * 2;
    }
    else {
        iCheck = CheckBit_InterAllPE(org_isite3, org_ispin3, org_isite3, org_ispin3, org_isite2, org_ispin2, org_isite1,
                                     org_ispin1, X, (long unsigned int) myrank, &origin);
        if (iCheck == TRUE) {
            tmp_V = conj(tmp_V);
            tmp_isite4 = X->Def.OrgTpow[2 * org_isite1 + org_ispin1];
            tmp_isite3 = X->Def.OrgTpow[2 * org_isite2 + org_ispin2];
            tmp_isite2 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
            tmp_isite1 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
            Asum = tmp_isite3 + tmp_isite4;
            if (tmp_isite4 > tmp_isite3) Adiff = tmp_isite4 - tmp_isite3 * 2;
            else Adiff = tmp_isite3 - tmp_isite4 * 2;
            if (X->Large.mode == M_CORR|| X->Large.mode == M_CALCSPEC) {
                tmp_V = 0;
            }
        }
        else {
            return 0.0;
        }
    }

    if (myrank == origin) {// only k is in PE

        if (CheckBit_Ajt(isite3, myrank, &tmp_off) == FALSE) return 0.0;

#pragma omp parallel for default(none) reduction(+:dam_pr) firstprivate(i_max,X,Asum,Adiff,isite1,isite2, tmp_V) private(j,tmp_off) shared(tmp_v0, tmp_v1)
        for (j = 1; j <= i_max; j++) {
            dam_pr += GC_CisAjt(j, tmp_v0, tmp_v1, X, isite2, isite1, Asum, Adiff, tmp_V, &tmp_off);
        }

        if (X->Large.mode != M_CORR) {
#pragma omp parallel for default(none) reduction(+:dam_pr) firstprivate(i_max,X,Asum,Adiff,isite1,isite2, tmp_V) private(j,tmp_off) shared(tmp_v0, tmp_v1)
            for (j = 1; j <= i_max; j++) {
                dam_pr += GC_CisAjt(j, tmp_v0, tmp_v1, X, isite1, isite2, Asum, Adiff, tmp_V, &tmp_off);
            }
        }
        return dam_pr;
    }//myrank =origin
    else {
        ierr = MPI_Sendrecv(&X->Check.idim_max, 1, MPI_UNSIGNED_LONG, origin, 0,
                            &idim_max_buf, 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);
        ierr = MPI_Sendrecv(tmp_v1, X->Check.idim_max + 1, MPI_DOUBLE_COMPLEX, origin, 0,
                            v1buf, idim_max_buf + 1, MPI_DOUBLE_COMPLEX, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);

        if (org_isite1 + 1 > X->Def.Nsite && org_isite2 + 1 > X->Def.Nsite) {
            if (isite2 > isite1) Adiff = isite2 - isite1 * 2;
            else Adiff = isite1 - isite2 * 2;
            SgnBit(((long unsigned int) myrank & Adiff), &Fsgn);
            tmp_V *= Fsgn;

            if (org_isite3 + 1 > X->Def.Nsite) {
                if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv) firstprivate(idim_max_buf, tmp_V, X) shared(v1buf, tmp_v1, tmp_v0)
                    for (j = 1; j <= idim_max_buf; j++) {
                        dmv = tmp_V * v1buf[j];
                        tmp_v0[j] += dmv;
                        dam_pr += conj(tmp_v1[j]) * dmv;
                    }


                }
                else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv) firstprivate(idim_max_buf, tmp_V, X) shared(v1buf, tmp_v1, tmp_v0)
                    for (j = 1; j <= idim_max_buf; j++) {
                        dmv = tmp_V * v1buf[j];
                        dam_pr += conj(tmp_v1[j]) * dmv;
                    }
                }
            }
            else { //org_isite3 <= X->Def.Nsite
                if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, tmp_off) firstprivate(idim_max_buf, tmp_V, X, isite3) shared(v1buf, tmp_v1, tmp_v0)
                    for (j = 1; j <= idim_max_buf; j++) {
                        if (CheckBit_Ajt(isite3, j - 1, &tmp_off) == TRUE) {
                            dmv = tmp_V * v1buf[j];
                            tmp_v0[j] += dmv;
                            dam_pr += conj(tmp_v1[j]) * dmv;
                        }
                    }
                }
                else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, tmp_off) firstprivate(idim_max_buf, tmp_V, X, isite3) shared(v1buf, tmp_v1, tmp_v0)
                    for (j = 1; j <= idim_max_buf; j++) {
                        if (CheckBit_Ajt(isite3, j - 1, &tmp_off) == TRUE) {
                            dmv = tmp_V * v1buf[j];
                            dam_pr += conj(tmp_v1[j]) * dmv;
                        }
                    }
                }
            }
        }
        else {
            org_rankbit = X->Def.OrgTpow[2 * X->Def.Nsite] * origin;
            if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {

#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, tmp_off, Fsgn) firstprivate(idim_max_buf, tmp_V, X, tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4, org_rankbit) shared(v1buf, tmp_v1, tmp_v0)
                for (j = 1; j <= idim_max_buf; j++) {
                    /*
                    if(GetSgnInterAll(tmp_isite3, tmp_isite4, tmp_isite1, tmp_isite2, &Fsgn, X, (j-1)+org_rankbit, &tmp_off)==TRUE){
                    */
                    if (GetSgnInterAll(tmp_isite4, tmp_isite3, tmp_isite2, tmp_isite1, &Fsgn, X, (j - 1) + org_rankbit,
                                       &tmp_off) == TRUE) {
                        dmv = tmp_V * v1buf[j] * Fsgn;
                        tmp_v0[tmp_off + 1] += dmv;
                        dam_pr += conj(tmp_v1[tmp_off + 1]) * dmv;

                    }
                }
            }
            else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, tmp_off, Fsgn) firstprivate(idim_max_buf, tmp_V, X, tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4, org_rankbit) shared(v1buf, tmp_v1, tmp_v0)
                for (j = 1; j <= idim_max_buf; j++) {
                    /*
                    if(GetSgnInterAll(tmp_isite3, tmp_isite4, tmp_isite1, tmp_isite2, &Fsgn, X, (j-1)+org_rankbit, &tmp_off)==TRUE){
                    */
                    if (GetSgnInterAll(tmp_isite4, tmp_isite3, tmp_isite2, tmp_isite1, &Fsgn, X, (j - 1) + org_rankbit,
                                       &tmp_off) == TRUE) {
                        dmv = tmp_V * v1buf[j] * Fsgn;
                        dam_pr += conj(tmp_v1[tmp_off + 1]) * dmv;

                    }
                }
            }
        }
    }
    return dam_pr;
#endif
}


double complex X_GC_child_CisAisCjtAku_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 int org_isite3,
 int org_ispin3,
 int org_isite4,
 int org_ispin4,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 )
{
#ifdef MPI
  double complex dam_pr=0;
  dam_pr=X_GC_child_CisAjtCkuAku_Hubbard_MPI
    (
     org_isite4, org_ispin4, org_isite3, org_ispin3,
     org_isite1, org_ispin1, conj(tmp_V), X, tmp_v0, tmp_v1
     );

  return conj(dam_pr);
#endif
}


double complex X_GC_child_CisAjtCkuAlv_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 int org_isite2,
 int org_ispin2,
 int org_isite3,
 int org_ispin3,
 int org_isite4,
 int org_ispin4,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 ) {
#ifdef MPI
    double complex dam_pr = 0;
    unsigned long int i_max = X->Check.idim_max;
    unsigned long int idim_max_buf;
    int iCheck, ierr, Fsgn;
    unsigned long int isite1, isite2, isite3, isite4;
    unsigned long int tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4;
    unsigned long int j, Adiff, Bdiff;
    double complex dmv;
    unsigned long int origin, tmp_off, tmp_off2;
    unsigned long int org_rankbit;
    int iFlgHermite = FALSE;
    MPI_Status statusMPI;

    iCheck = CheckBit_InterAllPE(org_isite1, org_ispin1, org_isite2, org_ispin2,
                                 org_isite3, org_ispin3, org_isite4, org_ispin4,
                                 X, (long unsigned int) myrank, &origin);
    isite1 = X->Def.Tpow[2 * org_isite1 + org_ispin1];
    isite2 = X->Def.Tpow[2 * org_isite2 + org_ispin2];
    isite3 = X->Def.Tpow[2 * org_isite3 + org_ispin3];
    isite4 = X->Def.Tpow[2 * org_isite4 + org_ispin4];

    if (iCheck == TRUE) {
        tmp_isite1 = X->Def.OrgTpow[2 * org_isite1 + org_ispin1];
        tmp_isite2 = X->Def.OrgTpow[2 * org_isite2 + org_ispin2];
        tmp_isite3 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
        tmp_isite4 = X->Def.OrgTpow[2 * org_isite4 + org_ispin4];
    }
    else {
        iCheck = CheckBit_InterAllPE(org_isite4, org_ispin4, org_isite3, org_ispin3,
                                     org_isite2, org_ispin2, org_isite1, org_ispin1,
                                     X, (long unsigned int) myrank, &origin);
        if (iCheck == TRUE) {
            tmp_V = conj(tmp_V);
            tmp_isite4 = X->Def.OrgTpow[2 * org_isite1 + org_ispin1];
            tmp_isite3 = X->Def.OrgTpow[2 * org_isite2 + org_ispin2];
            tmp_isite2 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
            tmp_isite1 = X->Def.OrgTpow[2 * org_isite4 + org_ispin4];
            iFlgHermite = TRUE;
            if (X->Large.mode == M_CORR|| X->Large.mode == M_CALCSPEC) {
                tmp_V = 0;
            }
        }
        else {
            return 0.0;
        }
    }

    if (myrank == origin) {
        if (isite1 == isite4 && isite2 == isite3) { // CisAjvCjvAis =Cis(1-njv)Ais=nis-nisnjv
            //calc nis
            dam_pr = X_GC_child_CisAis_Hubbard_MPI(org_isite1, org_ispin1, tmp_V, X, tmp_v0, tmp_v1);
            //calc -nisniv
            dam_pr -= X_GC_child_CisAisCjtAjt_Hubbard_MPI(org_isite1, org_ispin1, org_isite3, org_ispin3, tmp_V, X,
                                                          tmp_v0, tmp_v1);
        }
        else if (isite2 == isite3) { // CisAjvCjvAku= Cis(1-njv)Aku=-CisAkunjv+CisAku: j is in PE
            //calc CisAku
            if (isite4 > isite1) Adiff = isite4 - isite1 * 2;
            else Adiff = isite1 - isite4 * 2;

#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_off) firstprivate(i_max, tmp_V, X, isite1, isite4, Adiff) shared(tmp_v1, tmp_v0)
            for (j = 1; j <= i_max; j++) {
                dam_pr += GC_CisAjt(j - 1, tmp_v0, tmp_v1, X, isite1, isite4, (isite1 + isite4), Adiff, tmp_V,
                                    &tmp_off);
            }
            //calc -CisAku njv
            dam_pr -= X_GC_child_CisAjtCkuAku_Hubbard_MPI(org_isite1, org_ispin1, org_isite4, org_ispin4, org_isite2,
                                                          org_ispin2, tmp_V, X, tmp_v0, tmp_v1);
            if (X->Large.mode != M_CORR) { //for hermite
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_off) firstprivate(i_max, tmp_V, X, isite1, isite4, Adiff) shared(tmp_v1, tmp_v0)
                for (j = 1; j <= i_max; j++) {
                    dam_pr += GC_CisAjt(j - 1, tmp_v0, tmp_v1, X, isite4, isite1, (isite1 + isite4), Adiff, tmp_V,
                                        &tmp_off);
                }
                //calc -njvCkuAis
                dam_pr -= X_GC_child_CisAisCjtAku_Hubbard_MPI(org_isite2, org_ispin2, org_isite4, org_ispin4,
                                                              org_isite1, org_ispin1, tmp_V, X, tmp_v0, tmp_v1);

            }
        }
        else {// CisAjtCkuAis = -CisAisCkuAjt: i is in PE

            dam_pr = -X_GC_child_CisAisCjtAku_Hubbard_MPI(org_isite1, org_ispin1, org_isite3, org_ispin3, org_isite2,
                                                          org_ispin2, tmp_V, X, tmp_v0, tmp_v1);
            if (X->Large.mode != M_CORR) { //for hermite

                dam_pr += -X_GC_child_CisAisCjtAku_Hubbard_MPI(org_isite1, org_ispin1, org_isite2, org_ispin2,
                                                               org_isite3, org_ispin3, tmp_V, X, tmp_v0, tmp_v1);
            }
        }
        return dam_pr;
    }//myrank =origin
    else {

        ierr = MPI_Sendrecv(&X->Check.idim_max, 1, MPI_UNSIGNED_LONG, origin, 0,
                            &idim_max_buf, 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);
        ierr = MPI_Sendrecv(tmp_v1, X->Check.idim_max + 1, MPI_DOUBLE_COMPLEX, origin, 0,
                            v1buf, idim_max_buf + 1, MPI_DOUBLE_COMPLEX, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);

        if (org_isite1 + 1 > X->Def.Nsite && org_isite2 + 1 > X->Def.Nsite
            && org_isite3 + 1 > X->Def.Nsite && org_isite4 + 1 > X->Def.Nsite) {

            if (isite2 > isite1) Adiff = isite2 - isite1 * 2;
            else Adiff = isite1 - isite2 * 2;
            if (isite4 > isite3) Bdiff = isite4 - isite3 * 2;
            else Bdiff = isite3 - isite4 * 2;

            if (iFlgHermite == FALSE) {
                Fsgn = X_GC_CisAjt((long unsigned int) myrank, X, isite2, isite1, (isite1 + isite2), Adiff, &tmp_off2);
                Fsgn *= X_GC_CisAjt(tmp_off2, X, isite4, isite3, (isite3 + isite4), Bdiff, &tmp_off);
                tmp_V *= Fsgn;
            }
            else {
                Fsgn = X_GC_CisAjt((long unsigned int) myrank, X, isite3, isite4, (isite3 + isite4), Bdiff, &tmp_off2);
                Fsgn *= X_GC_CisAjt(tmp_off2, X, isite1, isite2, (isite1 + isite2), Adiff, &tmp_off);
                tmp_V *= Fsgn;
            }
            if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {

#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv) firstprivate(idim_max_buf, tmp_V, X) shared(v1buf, tmp_v1, tmp_v0)
                for (j = 1; j <= idim_max_buf; j++) {
                    dmv = tmp_V * v1buf[j];
                    tmp_v0[j] += dmv;
                    dam_pr += conj(tmp_v1[j]) * dmv;
                }
            }
            else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv) firstprivate(idim_max_buf, tmp_V, X) shared(v1buf, tmp_v1, tmp_v0)
                for (j = 1; j <= idim_max_buf; j++) {
                    dmv = tmp_V * v1buf[j];
                    dam_pr += conj(tmp_v1[j]) * dmv;
                }
            }
        }
        else {
            org_rankbit = X->Def.OrgTpow[2 * X->Def.Nsite] * origin;
            if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, tmp_off, Fsgn) firstprivate(idim_max_buf, tmp_V, X, tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4, org_rankbit) shared(v1buf, tmp_v1, tmp_v0)
                for (j = 1; j <= idim_max_buf; j++) {

                    if (GetSgnInterAll(tmp_isite3, tmp_isite4, tmp_isite1, tmp_isite2, &Fsgn, X, (j - 1) + org_rankbit,
                                       &tmp_off) == TRUE) {
                        dmv = tmp_V * v1buf[j] * Fsgn;
                        tmp_v0[tmp_off + 1] += dmv;
                        dam_pr += conj(tmp_v1[tmp_off + 1]) * dmv;
                    }
                }
            }
            else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, tmp_off, Fsgn) firstprivate(idim_max_buf, tmp_V, X, tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4, org_rankbit) shared(v1buf, tmp_v1, tmp_v0)
                for (j = 1; j <= idim_max_buf; j++) {

                    if (GetSgnInterAll(tmp_isite3, tmp_isite4, tmp_isite1, tmp_isite2, &Fsgn, X, (j - 1) + org_rankbit,
                                       &tmp_off) == TRUE) {
                        dmv = tmp_V * v1buf[j] * Fsgn;
                        dam_pr += conj(tmp_v1[tmp_off + 1]) * dmv;
                    }
                }
            }
        }
    }

    return dam_pr;
#endif
}

double complex X_GC_child_CisAis_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 ) {
#ifdef MPI
    double complex dam_pr = 0.0;
    unsigned long int i_max = X->Check.idim_max;
    unsigned long int j, isite1, tmp_off;
    double complex dmv;
//  MPI_Status statusMPI;

    isite1 = X->Def.Tpow[2 * org_isite1 + org_ispin1];
    if (org_isite1 + 1 > X->Def.Nsite) {
        if (CheckBit_Ajt(isite1, (unsigned long int) myrank, &tmp_off) == FALSE) return 0.0;

        if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) { // for multply

#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                dmv = tmp_v1[j] * tmp_V;
                tmp_v0[j] += dmv;
                dam_pr += conj(tmp_v1[j]) * dmv;
            }
        }
        else {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                dmv = tmp_v1[j] * tmp_V;
                dam_pr += conj(tmp_v1[j]) * dmv;
            }
        }
    }
    else {
        if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) { // for multply

#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X, isite1) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                if (CheckBit_Ajt(isite1, j - 1, &tmp_off) == TRUE) {
                    dmv = tmp_v1[j] * tmp_V;
                    tmp_v0[j] += dmv;
                    dam_pr += conj(tmp_v1[j]) * dmv;
                }
            }
        }
        else {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X, isite1) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                if (CheckBit_Ajt(isite1, j - 1, &tmp_off) == TRUE) {
                    dmv = tmp_v1[j] * tmp_V;
                    dam_pr += conj(tmp_v1[j]) * dmv;
                }
            }

        }
    }

    return dam_pr;
#endif
}

double complex X_GC_child_CisAjt_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 int org_isite2,
 int org_ispin2,
 double complex tmp_trans,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 ){
  #ifdef MPI
  double complex dam_pr=0.0;
//  MPI_Status statusMPI;

  if(org_isite1 + 1 > X->Def.Nsite && org_isite2+1 > X->Def.Nsite){
    dam_pr = X_GC_child_general_hopp_MPIdouble(org_isite1, org_ispin1, org_isite2, org_ispin2, tmp_trans, X, tmp_v0, tmp_v1);    
  }
  else if(org_isite1 +1 > X->Def.Nsite || org_isite2+1>X->Def.Nsite){
    dam_pr = X_GC_child_general_hopp_MPIsingle(org_isite1, org_ispin1, org_isite2, org_ispin2, tmp_trans, X, tmp_v0, tmp_v1);    
  }
  else{
    //error message will be added.
    exitMPI(-1);
  }  
  return dam_pr;
  #endif
}

double complex X_child_CisAisCjtAjt_Hubbard_MPI
(int org_isite1,
 int org_ispin1,
 int org_isite3,
 int org_ispin3,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1) {
#ifdef MPI
    double complex dam_pr = 0.0;
    int iCheck;
    unsigned long int tmp_ispin1;
    unsigned long int i_max = X->Check.idim_max;
    unsigned long int tmp_off, j;
    double complex dmv;
//  MPI_Status statusMPI;

    iCheck = CheckBit_PairPE(org_isite1, org_ispin1, org_isite3, org_ispin3, X, (long unsigned int) myrank);
    if (iCheck != TRUE) {
        return 0.0;
    }
    if (org_isite1 + 1 > X->Def.Nsite && org_isite3 + 1 > X->Def.Nsite) {
        if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) { // for multply
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                dmv = tmp_v1[j] * tmp_V;
                tmp_v0[j] += dmv;
                dam_pr += conj(tmp_v1[j]) * dmv;
            }

        }
        else {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                dmv = tmp_v1[j] * tmp_V;
                dam_pr += conj(tmp_v1[j]) * dmv;
            }
        }
    }
    else if (org_isite1 + 1 > X->Def.Nsite || org_isite3 + 1 > X->Def.Nsite) {
        if (org_isite1 > org_isite3) {
            tmp_ispin1 = X->Def.Tpow[2 * org_isite3 + org_ispin3];
        }
        else {
            tmp_ispin1 = X->Def.Tpow[2 * org_isite1 + org_ispin1];
        }

        if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1, list_1) \
  firstprivate(i_max, tmp_V, X, tmp_ispin1) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                if (CheckBit_Ajt(tmp_ispin1, list_1[j], &tmp_off) == TRUE) {
                    dmv = tmp_v1[j] * tmp_V;
                    tmp_v0[j] += dmv;
                    dam_pr += conj(tmp_v1[j]) * dmv;
                }
            }
        }
        else {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1, list_1) \
  firstprivate(i_max, tmp_V, X, tmp_ispin1) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                if (CheckBit_Ajt(tmp_ispin1, list_1[j], &tmp_off) == TRUE) {
                    dmv = tmp_v1[j] * tmp_V;
                    dam_pr += conj(tmp_v1[j]) * dmv;
                }
            }
        }
    }
    return dam_pr;
#endif
}


double complex X_child_CisAjtCkuAlv_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 int org_isite2,
 int org_ispin2,
 int org_isite3,
 int org_ispin3,
 int org_isite4,
 int org_ispin4,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 ) {
#ifdef MPI
    double complex dam_pr = 0;
    unsigned long int i_max = X->Check.idim_max;
    unsigned long int idim_max_buf;
    int iCheck, ierr, Fsgn;
    unsigned long int isite1, isite2, isite3, isite4;
    unsigned long int tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4;
    unsigned long int j, Adiff, Bdiff;
    double complex dmv;
    unsigned long int origin, tmp_off, tmp_off2;
    unsigned long int org_rankbit, ioff;
    int iFlgHermite = FALSE;
    MPI_Status statusMPI;

    iCheck = CheckBit_InterAllPE(org_isite1, org_ispin1, org_isite2, org_ispin2,
                                 org_isite3, org_ispin3, org_isite4, org_ispin4,
                                 X, (long unsigned int) myrank, &origin);
    isite1 = X->Def.Tpow[2 * org_isite1 + org_ispin1];
    isite2 = X->Def.Tpow[2 * org_isite2 + org_ispin2];
    isite3 = X->Def.Tpow[2 * org_isite3 + org_ispin3];
    isite4 = X->Def.Tpow[2 * org_isite4 + org_ispin4];

    //printf("debug: myrank=%d, isite1=%d, ispin1=%d, isite2=%d, ispin2=%d, isite3=%d, ispin3=%d, isite4=%d, ispin4=%d\n", myrank, org_isite1, org_ispin1, org_isite2, org_ispin2, org_isite3, org_ispin3, org_isite4, org_ispin4);
    if (iCheck == TRUE) {
        tmp_isite1 = X->Def.OrgTpow[2 * org_isite1 + org_ispin1];
        tmp_isite2 = X->Def.OrgTpow[2 * org_isite2 + org_ispin2];
        tmp_isite3 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
        tmp_isite4 = X->Def.OrgTpow[2 * org_isite4 + org_ispin4];
    }
    else {
        iCheck = CheckBit_InterAllPE(org_isite4, org_ispin4, org_isite3, org_ispin3,
                                     org_isite2, org_ispin2, org_isite1, org_ispin1,
                                     X, (long unsigned int) myrank, &origin);
        if (iCheck == TRUE) {
            tmp_V = conj(tmp_V);
            tmp_isite4 = X->Def.OrgTpow[2 * org_isite1 + org_ispin1];
            tmp_isite3 = X->Def.OrgTpow[2 * org_isite2 + org_ispin2];
            tmp_isite2 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
            tmp_isite1 = X->Def.OrgTpow[2 * org_isite4 + org_ispin4];
            iFlgHermite = TRUE;
            if (X->Large.mode == M_CORR|| X->Large.mode == M_CALCSPEC) {
                tmp_V = 0;
            }
        }
        else {
            return 0.0;
        }
    }

    if (myrank == origin) {
        if (isite1 == isite4 && isite2 == isite3) { // CisAjvCjvAis =Cis(1-njv)Ais=nis-nisnjv
            //calc nis
            dam_pr = X_child_CisAis_Hubbard_MPI(org_isite1, org_ispin1, tmp_V, X, tmp_v0, tmp_v1);
            //calc -nisniv
            dam_pr -= X_child_CisAisCjtAjt_Hubbard_MPI(org_isite1, org_ispin1, org_isite3, org_ispin3, tmp_V, X, tmp_v0,
                                                       tmp_v1);
        }
        else if (isite2 == isite3) { // CisAjvCjvAku= Cis(1-njv)Aku=-CisAkunjv+CisAku: j is in PE
            if (isite4 > isite1) Adiff = isite4 - isite1 * 2;
            else Adiff = isite1 - isite4 * 2;

            //calc CisAku
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_off) firstprivate(i_max, tmp_V, X, isite1, isite4, Adiff) shared(tmp_v1, tmp_v0, list_1)
            for (j = 1; j <= i_max; j++) {
                dam_pr += CisAjt(j, tmp_v0, tmp_v1, X, isite1, isite4, (isite1 + isite4), Adiff, tmp_V);
            }
            //calc -CisAku njv
            dam_pr -= X_child_CisAjtCkuAku_Hubbard_MPI(org_isite1, org_ispin1, org_isite4, org_ispin4, org_isite2,
                                                       org_ispin2, tmp_V, X, tmp_v0, tmp_v1);

            if (X->Large.mode != M_CORR) {  //for hermite
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_off) firstprivate(i_max, tmp_V, X, isite1, isite4, Adiff) shared(tmp_v1, tmp_v0)
                for (j = 1; j <= i_max; j++) {
                    dam_pr += CisAjt(j, tmp_v0, tmp_v1, X, isite4, isite1, (isite1 + isite4), Adiff, tmp_V);
                }
                //calc -njvCkuAis
                dam_pr -= X_child_CisAisCjtAku_Hubbard_MPI(org_isite2, org_ispin2, org_isite4, org_ispin4, org_isite1,
                                                           org_ispin1, tmp_V, X, tmp_v0, tmp_v1);
            }
        }
        else {// CisAjtCkuAis = -CisAisCkuAjt: i is in PE
            dam_pr = -X_child_CisAisCjtAku_Hubbard_MPI(org_isite1, org_ispin1, org_isite3, org_ispin3, org_isite2,
                                                       org_ispin2, tmp_V, X, tmp_v0, tmp_v1);

            if (X->Large.mode != M_CORR) {  //for hermite: CisAkuCjtAis=-CisAisCjtAku
                dam_pr = -X_child_CisAisCjtAku_Hubbard_MPI(org_isite1, org_ispin1, org_isite2, org_ispin2, org_isite3,
                                                           org_ispin3, tmp_V, X, tmp_v0, tmp_v1);
            }
        }

        return dam_pr;
    }//myrank =origin
    else {
        //printf("debug: myrank=%d, origin=%d\n", myrank, origin);
        ierr = MPI_Sendrecv(&X->Check.idim_max, 1, MPI_UNSIGNED_LONG, origin, 0,
                            &idim_max_buf, 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);

        ierr = MPI_Sendrecv(list_1, X->Check.idim_max + 1, MPI_UNSIGNED_LONG, origin, 0,
                            list_1buf, idim_max_buf + 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);

        ierr = MPI_Sendrecv(tmp_v1, X->Check.idim_max + 1, MPI_DOUBLE_COMPLEX, origin, 0,
                            v1buf, idim_max_buf + 1, MPI_DOUBLE_COMPLEX, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);

        if (org_isite1 + 1 > X->Def.Nsite && org_isite2 + 1 > X->Def.Nsite
            && org_isite3 + 1 > X->Def.Nsite && org_isite4 + 1 > X->Def.Nsite) {

            if (isite2 > isite1) Adiff = isite2 - isite1 * 2;
            else Adiff = isite1 - isite2 * 2;
            if (isite4 > isite3) Bdiff = isite4 - isite3 * 2;
            else Bdiff = isite3 - isite4 * 2;

            if (iFlgHermite == FALSE) {
                Fsgn = X_GC_CisAjt((long unsigned int) myrank, X, isite2, isite1, (isite1 + isite2), Adiff, &tmp_off2);
                Fsgn *= X_GC_CisAjt(tmp_off2, X, isite4, isite3, (isite3 + isite4), Bdiff, &tmp_off);
                tmp_V *= Fsgn;
            }
            else {
                Fsgn = X_GC_CisAjt((long unsigned int) myrank, X, isite3, isite4, (isite3 + isite4), Bdiff, &tmp_off2);
                Fsgn *= X_GC_CisAjt(tmp_off2, X, isite1, isite2, (isite1 + isite2), Adiff, &tmp_off);
                tmp_V *= Fsgn;
            }
            dam_pr = 0;
            if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, ioff) firstprivate(idim_max_buf, tmp_V, X) shared(v1buf, tmp_v1, tmp_v0, list_2_1, list_2_2, list_1buf)
                for (j = 1; j <= idim_max_buf; j++) {
                    if (GetOffComp(list_2_1, list_2_2, list_1buf[j],
                                   X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff) == TRUE) {
                        dmv = tmp_V * v1buf[j];
                        tmp_v0[ioff] += dmv;
                        dam_pr += conj(tmp_v1[ioff]) * dmv;
                    }
                }
            }
            else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, ioff) firstprivate(idim_max_buf, tmp_V, X) shared(v1buf, tmp_v1, tmp_v0, list_2_1, list_2_2, list_1buf)
                for (j = 1; j <= idim_max_buf; j++) {
                    if (GetOffComp(list_2_1, list_2_2, list_1buf[j],
                                   X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff) == TRUE) {
                        dmv = tmp_V * v1buf[j];
                        dam_pr += conj(tmp_v1[ioff]) * dmv;
                    }
                }
            }
        }//org_isite1+1 > X->Def.Nsite && org_isite2+1 > X->Def.Nsite
            // && org_isite3+1 > X->Def.Nsite && org_isite4+1 > X->Def.Nsite
        else {

            org_rankbit = X->Def.OrgTpow[2 * X->Def.Nsite] * origin;
            dam_pr = 0;
            if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {

#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, tmp_off, Fsgn, ioff) firstprivate(idim_max_buf, tmp_V, X, tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4, org_rankbit, org_isite1, org_ispin1, org_isite2, org_ispin2, org_isite3, org_ispin3, org_isite4, org_ispin4) shared(v1buf, tmp_v1, tmp_v0, list_1buf, list_2_1, list_2_2)
                for (j = 1; j <= idim_max_buf; j++) {
                    if (GetSgnInterAll(tmp_isite3, tmp_isite4, tmp_isite1, tmp_isite2, &Fsgn, X,
                                       list_1buf[j] + org_rankbit,
                                       &tmp_off) == TRUE) {

                        if (GetOffComp(list_2_1, list_2_2, tmp_off,
                                       X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff) == TRUE) {
                            dmv = tmp_V * v1buf[j] * Fsgn;
                            tmp_v0[ioff] += dmv;
                            dam_pr += conj(tmp_v1[ioff]) * dmv;

                        }
                    }
                }
            }
            else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, tmp_off, Fsgn, ioff) firstprivate(idim_max_buf, tmp_V, X, tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4, org_rankbit, org_isite1, org_ispin1, org_isite2, org_ispin2, org_isite3, org_ispin3, org_isite4, org_ispin4) shared(v1buf, tmp_v1, tmp_v0, list_1buf, list_2_1, list_2_2)
                for (j = 1; j <= idim_max_buf; j++) {
                    if (GetSgnInterAll(tmp_isite3, tmp_isite4, tmp_isite1, tmp_isite2, &Fsgn, X,
                                       list_1buf[j] + org_rankbit, &tmp_off) == TRUE) {

                        if (GetOffComp(list_2_1, list_2_2, tmp_off,
                                       X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff) == TRUE) {
                            dmv = tmp_V * v1buf[j] * Fsgn;
                            dam_pr += conj(tmp_v1[ioff]) * dmv;

                        }
                    }
                }
            }
        }
    }

    return dam_pr;
#endif
}

double complex X_child_CisAjtCkuAku_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 int org_isite2,
 int org_ispin2,
 int org_isite3,
 int org_ispin3,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 ) {
#ifdef MPI
    double complex dam_pr = 0.0;
    unsigned long int i_max = X->Check.idim_max;
    unsigned long int idim_max_buf, ioff;
    int iCheck, ierr, Fsgn;
    unsigned long int isite1, isite2, isite3;
    unsigned long int tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4;
    unsigned long int j, Asum, Adiff;
    double complex dmv;
    unsigned long int origin, tmp_off;
    unsigned long int org_rankbit;
    MPI_Status statusMPI;

    iCheck = CheckBit_InterAllPE(org_isite1, org_ispin1, org_isite2, org_ispin2, org_isite3, org_ispin3, org_isite3,
                                 org_ispin3, X, (long unsigned int) myrank, &origin);
    isite1 = X->Def.Tpow[2 * org_isite1 + org_ispin1];
    isite2 = X->Def.Tpow[2 * org_isite2 + org_ispin2];
    isite3 = X->Def.Tpow[2 * org_isite3 + org_ispin3];

    if (iCheck == TRUE) {

        tmp_isite1 = X->Def.OrgTpow[2 * org_isite1 + org_ispin1];
        tmp_isite2 = X->Def.OrgTpow[2 * org_isite2 + org_ispin2];
        tmp_isite3 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
        tmp_isite4 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
        Asum = tmp_isite1 + tmp_isite2;
        if (tmp_isite2 > tmp_isite1) Adiff = tmp_isite2 - tmp_isite1 * 2;
        else Adiff = tmp_isite1 - tmp_isite2 * 2;
    }
    else {
        iCheck = CheckBit_InterAllPE(org_isite3, org_ispin3, org_isite3, org_ispin3, org_isite2, org_ispin2, org_isite1,
                                     org_ispin1, X, (long unsigned int) myrank, &origin);
        if (iCheck == TRUE) {
            tmp_V = conj(tmp_V);
            tmp_isite4 = X->Def.OrgTpow[2 * org_isite1 + org_ispin1];
            tmp_isite3 = X->Def.OrgTpow[2 * org_isite2 + org_ispin2];
            tmp_isite2 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
            tmp_isite1 = X->Def.OrgTpow[2 * org_isite3 + org_ispin3];
            Asum = tmp_isite3 + tmp_isite4;
            if (tmp_isite4 > tmp_isite3) Adiff = tmp_isite4 - tmp_isite3 * 2;
            else Adiff = tmp_isite3 - tmp_isite4 * 2;
            if (X->Large.mode == M_CORR|| X->Large.mode == M_CALCSPEC) {
                tmp_V = 0;
            }
        }
        else {
            return 0.0;
        }
    }

    if (myrank == origin) {// only k is in PE
        //for hermite
#pragma omp parallel for default(none) reduction(+:dam_pr) firstprivate(i_max, Asum, Adiff, isite1, isite2, tmp_V, X) private(j) shared(tmp_v0, tmp_v1)
        for (j = 1; j <= i_max; j++) {
            dam_pr += CisAjt(j, tmp_v0, tmp_v1, X, isite1, isite2, Asum, Adiff, tmp_V);
        }

        if (X->Large.mode != M_CORR) {
#pragma omp parallel for default(none) reduction(+:dam_pr) firstprivate(i_max,X,Asum,Adiff,isite1,isite2, tmp_V) private(j) shared(tmp_v0, tmp_v1)
            for (j = 1; j <= i_max; j++) {
                dam_pr += CisAjt(j, tmp_v0, tmp_v1, X, isite2, isite1, Asum, Adiff, tmp_V);
            }
        }
        return dam_pr;
    }//myrank =origin
    else {
        ierr = MPI_Sendrecv(&X->Check.idim_max, 1, MPI_UNSIGNED_LONG, origin, 0,
                            &idim_max_buf, 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);
        ierr = MPI_Sendrecv(list_1, X->Check.idim_max + 1, MPI_UNSIGNED_LONG, origin, 0,
                            list_1buf, idim_max_buf + 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);

        ierr = MPI_Sendrecv(tmp_v1, X->Check.idim_max + 1, MPI_DOUBLE_COMPLEX, origin, 0,
                            v1buf, idim_max_buf + 1, MPI_DOUBLE_COMPLEX, origin, 0, MPI_COMM_WORLD, &statusMPI);
        if (ierr != 0) exitMPI(-1);

        if (org_isite1 + 1 > X->Def.Nsite && org_isite2 + 1 > X->Def.Nsite) {
            if (isite2 > isite1) Adiff = isite2 - isite1 * 2;
            else Adiff = isite1 - isite2 * 2;
            SgnBit(((long unsigned int) myrank & Adiff), &Fsgn);
            tmp_V *= Fsgn;

            if (org_isite3 + 1 > X->Def.Nsite) {
                if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, ioff, dmv) firstprivate(idim_max_buf, tmp_V, X) shared(v1buf, tmp_v1, tmp_v0, list_1buf, list_2_1, list_2_2)
                    for (j = 1; j <= idim_max_buf; j++) {
                        dmv = tmp_V * v1buf[j];
                        GetOffComp(list_2_1, list_2_2, list_1buf[j],
                                   X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff);
                        tmp_v0[ioff] += dmv;
                        dam_pr += conj(tmp_v1[ioff]) * dmv;
                    }
                }
                else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, ioff, dmv) firstprivate(idim_max_buf, tmp_V, X) shared(v1buf, tmp_v1, tmp_v0, list_1buf, list_2_1, list_2_2)
                    for (j = 1; j <= idim_max_buf; j++) {
                        dmv = tmp_V * v1buf[j];
                        GetOffComp(list_2_1, list_2_2, list_1buf[j],
                                   X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff);
                        dam_pr += conj(tmp_v1[ioff]) * dmv;
                    }
                }
            }
            else { //org_isite3 <= X->Def.Nsite
                if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, ioff, dmv, tmp_off) firstprivate(idim_max_buf, tmp_V, X, isite3) shared(v1buf, tmp_v1, tmp_v0,list_1buf, list_2_1, list_2_2)
                    for (j = 1; j <= idim_max_buf; j++) {
                        if (CheckBit_Ajt(isite3, list_1buf[j], &tmp_off) == TRUE) {
                            dmv = tmp_V * v1buf[j];
                            GetOffComp(list_2_1, list_2_2, list_1buf[j],
                                       X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff);
                            tmp_v0[ioff] += dmv;
                            dam_pr += conj(tmp_v1[ioff]) * dmv;
                        }
                    }

                }
                else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, ioff, dmv, tmp_off) firstprivate(idim_max_buf, tmp_V, X, isite3) shared(v1buf, tmp_v1, tmp_v0,list_1buf, list_2_1, list_2_2)
                    for (j = 1; j <= idim_max_buf; j++) {
                        if (CheckBit_Ajt(isite3, list_1buf[j], &tmp_off) == TRUE) {
                            dmv = tmp_V * v1buf[j];
                            GetOffComp(list_2_1, list_2_2, list_1buf[j],
                                       X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff);
                            dam_pr += conj(tmp_v1[ioff]) * dmv;
                        }
                    }
                }
            }
        }
        else {
            org_rankbit = X->Def.OrgTpow[2 * X->Def.Nsite] * origin;
            if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, ioff, tmp_off, Fsgn) firstprivate(idim_max_buf, tmp_V, X, tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4, org_rankbit) shared(v1buf, tmp_v1, tmp_v0, list_1buf, list_2_1, list_2_2)
                for (j = 1; j <= idim_max_buf; j++) {
                    /*
                    if(GetSgnInterAll(tmp_isite3, tmp_isite4, tmp_isite1, tmp_isite2, &Fsgn, X, list_1buf[j]+org_rankbit, &tmp_off)==TRUE){
                    */
                    if (GetSgnInterAll(tmp_isite4, tmp_isite3, tmp_isite2, tmp_isite1, &Fsgn, X,
                                       list_1buf[j] + org_rankbit, &tmp_off) == TRUE) {
                        dmv = tmp_V * v1buf[j] * Fsgn;
                        GetOffComp(list_2_1, list_2_2, tmp_off,
                                   X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff);
                        tmp_v0[ioff] += dmv;
                        dam_pr += conj(tmp_v1[ioff]) * dmv;

                    }
                }
            }
            else {
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv, ioff, tmp_off, Fsgn) firstprivate(idim_max_buf, tmp_V, X, tmp_isite1, tmp_isite2, tmp_isite3, tmp_isite4, org_rankbit) shared(v1buf, tmp_v1, tmp_v0, list_1buf, list_2_1, list_2_2)
                for (j = 1; j <= idim_max_buf; j++) {
                    /*
                    if(GetSgnInterAll(tmp_isite3, tmp_isite4, tmp_isite1, tmp_isite2, &Fsgn, X, list_1buf[j]+org_rankbit, &tmp_off)==TRUE){
                    */
                    if (GetSgnInterAll(tmp_isite4, tmp_isite3, tmp_isite2, tmp_isite1, &Fsgn, X,
                                       list_1buf[j] + org_rankbit, &tmp_off) == TRUE) {
                        dmv = tmp_V * v1buf[j] * Fsgn;
                        GetOffComp(list_2_1, list_2_2, tmp_off,
                                   X->Large.irght, X->Large.ilft, X->Large.ihfbit, &ioff);
                        dam_pr += conj(tmp_v1[ioff]) * dmv;

                    }
                }
            }
        }
    }
    return dam_pr;
#endif
}
  

double complex X_child_CisAisCjtAku_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 int org_isite3,
 int org_ispin3,
 int org_isite4,
 int org_ispin4,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 ){
#ifdef MPI
  double complex dam_pr=0;
  
  dam_pr=X_child_CisAjtCkuAku_Hubbard_MPI
    (
     org_isite4, org_ispin4, org_isite3, org_ispin3,
     org_isite1, org_ispin1, conj(tmp_V), X, tmp_v0, tmp_v1
     );
  
  return conj(dam_pr);
  
#endif
}

double complex X_child_CisAis_Hubbard_MPI
(
 int org_isite1,
 int org_ispin1,
 double complex tmp_V,
 struct BindStruct *X,
 double complex *tmp_v0,
 double complex *tmp_v1
 ) {
#ifdef MPI
    double complex dam_pr = 0.0;
    unsigned long int i_max = X->Check.idim_max;
    unsigned long int j, isite1, tmp_off;
    double complex dmv;
//  MPI_Status statusMPI;

    isite1 = X->Def.Tpow[2 * org_isite1 + org_ispin1];
    if (org_isite1 + 1 > X->Def.Nsite) {
        if (CheckBit_Ajt(isite1, (unsigned long int) myrank, &tmp_off) == FALSE) {
            return 0.0;
        }
        if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) { // for multply
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                dmv = tmp_v1[j] * tmp_V;
                tmp_v0[j] += dmv;
                dam_pr += conj(tmp_v1[j]) * dmv;
            }
        }
        else {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1) \
  firstprivate(i_max, tmp_V, X) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                dmv = tmp_v1[j] * tmp_V;
                dam_pr += conj(tmp_v1[j]) * dmv;
            }
        }
    }
    else {
        if (X->Large.mode == M_MLTPLY|| X->Large.mode == M_CALCSPEC) { // for multply
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1, list_1) \
  firstprivate(i_max, tmp_V, X, isite1) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                if (X_CisAis(list_1[j], X, isite1) != 0) {
                    dmv = tmp_v1[j] * tmp_V;
                    tmp_v0[j] += dmv;
                    dam_pr += conj(tmp_v1[j]) * dmv;
                }
            }

        }
        else {
#pragma omp parallel for reduction(+:dam_pr) default(none) shared(tmp_v0, tmp_v1, list_1) \
  firstprivate(i_max, tmp_V, X, isite1) private(dmv, j, tmp_off)
            for (j = 1; j <= i_max; j++) {
                if (X_CisAis(list_1[j], X, isite1) != 0) {
                    dmv = tmp_v1[j] * tmp_V;
                    dam_pr += conj(tmp_v1[j]) * dmv;
                }
            }
        }
    }
    return dam_pr;
#endif
}




/**
  *
  * Single creation/annihilation operator
  * in the inter process region for HubbardGC.
  *
  * @author Mitsuaki Kawamura (The University of Tokyo)
  * @author Kazuyoshi Yoshimi (The University of Tokyo)
  * @author Youhei Yamaji (The University of Tokyo)
  */
double complex X_GC_Cis_MPI(
        int org_isite,
        int org_ispin,
        double complex tmp_trans,
        double complex *tmp_v0 /**< [out] Result v0 += H v1*/,
        double complex *tmp_v1 /**< [in] v0 += H v1*/,
        unsigned long int idim_max,
        double complex *tmp_v1buf,
        unsigned long int *Tpow
) {
#ifdef MPI
    int mask2, state2, ierr, origin, bit2diff, Fsgn;
    unsigned long int idim_max_buf, j;
    MPI_Status statusMPI;
    double complex trans, dmv, dam_pr;

    // org_isite >= Nsite
    mask2 = (int) Tpow[2 * org_isite + org_ispin];

    origin = myrank ^ mask2; // XOR
    state2 = origin & mask2;

    //if state2 = mask2, the state (org_isite, org_ispin) is not occupied in myrank
    //origin: if the state (org_isite, org_ispin) is occupied in myrank, the state is not occupied in origin.

    bit2diff = myrank - ((2*mask2-1) & myrank);

    //SgnBit((unsigned long int) (origin & bit2diff), &Fsgn); // Fermion sign
    SgnBit((unsigned long int) (bit2diff), &Fsgn); // Fermion sign

    ierr = MPI_Sendrecv(&idim_max, 1, MPI_UNSIGNED_LONG, origin, 0,
                        &idim_max_buf, 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    ierr = MPI_Sendrecv(tmp_v1, idim_max + 1, MPI_DOUBLE_COMPLEX, origin, 0,
                        tmp_v1buf, idim_max_buf + 1, MPI_DOUBLE_COMPLEX, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    if (state2 == mask2) {
        trans = 0;
    }
    else if (state2 == 0) {
        trans = (double) Fsgn * tmp_trans;
    }
    else return 0;

    dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv) \
  firstprivate(idim_max_buf, trans) shared(tmp_v1buf, tmp_v1, tmp_v0)
    for (j = 0; j < idim_max_buf; j++) {
        dmv = trans * tmp_v1buf[j + 1];
        tmp_v0[j + 1] += dmv;
        dam_pr += conj(tmp_v1[j + 1]) * dmv;
    }
    return (dam_pr);
#endif
}/*double complex X_GC_Cis_MPI*/


/**
  *
  * Single creation/annihilation operator
  * in the inter process region for HubbardGC.
  *
  * @author Mitsuaki Kawamura (The University of Tokyo)
  * @author Kazuyoshi Yoshimi (The University of Tokyo)
  * @author Youhei Yamaji (The University of Tokyo)
  */
double complex X_GC_Ajt_MPI(
        int org_isite,
        int org_ispin,
        double complex tmp_trans,
        double complex *tmp_v0 /**< [out] Result v0 += H v1*/,
        double complex *tmp_v1 /**< [in] v0 += H v1*/,
        unsigned long int idim_max,
        double complex *tmp_v1buf,
        unsigned long int *Tpow
) {
#ifdef MPI
    int mask2, state2, ierr, origin, bit2diff, Fsgn;
    unsigned long int idim_max_buf, j;
    MPI_Status statusMPI;
    double complex trans, dmv, dam_pr;

    // org_isite >= Nsite
    mask2 = (int) Tpow[2 * org_isite + org_ispin];

    origin = myrank ^ mask2; // XOR
    state2 = origin & mask2;

    //if state2 = mask2, the state (org_isite, org_ispin) is not occupied in myrank
    //origin: if the state (org_isite, org_ispin) is occupied in myrank, the state is not occupied in origin.

    bit2diff = myrank - ((2*mask2-1) & myrank);

    //SgnBit((unsigned long int) (origin & bit2diff), &Fsgn); // Fermion sign
    SgnBit((unsigned long int) (bit2diff), &Fsgn); // Fermion sign

    ierr = MPI_Sendrecv(&idim_max, 1, MPI_UNSIGNED_LONG, origin, 0,
                        &idim_max_buf, 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    ierr = MPI_Sendrecv(tmp_v1, idim_max + 1, MPI_DOUBLE_COMPLEX, origin, 0,
                        tmp_v1buf, idim_max_buf + 1, MPI_DOUBLE_COMPLEX, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    if (state2 == 0) {
        trans = 0;
    }
    else if (state2 == mask2) {
        trans = (double) Fsgn * tmp_trans;
    }
    else return 0;

    dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv) \
  firstprivate(idim_max_buf, trans) shared(tmp_v1buf, tmp_v1, tmp_v0)
    for (j = 0; j < idim_max_buf; j++) {
        dmv = trans * tmp_v1buf[j + 1];
        tmp_v0[j + 1] += dmv;
        dam_pr += conj(tmp_v1[j + 1]) * dmv;
    }
    return (dam_pr);
#endif
}/*double complex X_GC_Ajt_MPI*/



double complex X_Cis_MPI(
        int org_isite,
        unsigned int org_ispin,
        double complex tmp_trans,
        double complex *tmp_v0,
        double complex *tmp_v1,
        double complex *tmp_v1buf,
        unsigned long int idim_max,
        long unsigned int *Tpow,
        long unsigned int *list_1_org,
        long unsigned int *list_1buf_org,
        long unsigned int *list_2_1_target,
        long unsigned int *list_2_2_target,
        long unsigned int _irght,
        long unsigned int _ilft,
        long unsigned int _ihfbit
) {
#ifdef MPI
    int mask2, state2, ierr, origin, bit2diff, Fsgn;
    unsigned long int idim_max_buf, j, ioff;
    MPI_Status statusMPI;
    double complex trans, dmv, dam_pr;

    // org_isite >= Nsite
    mask2 = (int) Tpow[2 * org_isite + org_ispin];

    origin = myrank ^ mask2; // XOR
    state2 = origin & mask2;

    //if state2 = mask2, the state (org_isite, org_ispin) is not occupied in myrank
    //origin: if the state (org_isite, org_ispin) is occupied in myrank, the state is not occupied in origin.

    bit2diff = myrank - ((2*mask2-1) & myrank);

    SgnBit((unsigned long int) (bit2diff), &Fsgn); // Fermion sign

    ierr = MPI_Sendrecv(&idim_max, 1, MPI_UNSIGNED_LONG, origin, 0,
                        &idim_max_buf, 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    ierr = MPI_Sendrecv(list_1_org, idim_max + 1, MPI_UNSIGNED_LONG, origin, 0,
                        list_1buf_org, idim_max_buf + 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    ierr = MPI_Sendrecv(tmp_v1, idim_max + 1, MPI_DOUBLE_COMPLEX, origin, 0,
                        tmp_v1buf, idim_max_buf + 1, MPI_DOUBLE_COMPLEX, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    if (state2 == mask2) {
        trans = 0;
    }
    else if (state2 == 0) {
        trans = (double) Fsgn * tmp_trans;
    }
    else return 0;

    dam_pr = 0.0;
#pragma omp parallel for default(none) private(j, dmv) \
  firstprivate(idim_max_buf, trans, ioff, _irght, _ilft, _ihfbit, list_2_1_target, list_2_2_target) shared(tmp_v1buf, tmp_v1, tmp_v0, list_1buf_org)
    for (j = 1; j <= idim_max_buf; j++) {//idim_max_buf -> original
        GetOffComp(list_2_1_target, list_2_2_target, list_1buf_org[j],
                   _irght, _ilft, _ihfbit, &ioff);
        dmv = trans * tmp_v1buf[j];
        tmp_v0[ioff] += dmv;
    }
    return (dam_pr);
#endif
}/*double complex X_GC_Cis_MPI*/


double complex X_Ajt_MPI(
        int org_isite,
        unsigned int org_ispin,
        double complex tmp_trans,
        double complex *tmp_v0,
        double complex *tmp_v1,
        double complex *tmp_v1buf,
        unsigned long int idim_max,
        long unsigned int *Tpow,
        long unsigned int *list_1_org,
        long unsigned int *list_1buf_org,
        long unsigned int *list_2_1_target,
        long unsigned int *list_2_2_target,
        long unsigned int _irght,
        long unsigned int _ilft,
        long unsigned int _ihfbit
){
#ifdef MPI
    int mask2, state2, ierr, origin, bit2diff, Fsgn;
    unsigned long int idim_max_buf, j, ioff;
    MPI_Status statusMPI;
    double complex trans, dmv, dam_pr;

    // org_isite >= Nsite
    mask2 = (int) Tpow[2 * org_isite + org_ispin];

    origin = myrank ^ mask2; // XOR
    state2 = origin & mask2;

    //if state2 = mask2, the state (org_isite, org_ispin) is not occupied in myrank
    //origin: if the state (org_isite, org_ispin) is occupied in myrank, the state is not occupied in origin.

    bit2diff = myrank - ((2*mask2-1) & myrank);

    SgnBit((unsigned long int) (bit2diff), &Fsgn); // Fermion sign
    printf("Debug0: rank=%d test\n", myrank);

    ierr = MPI_Sendrecv(&idim_max, 1, MPI_UNSIGNED_LONG, origin, 0,
                        &idim_max_buf, 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    ierr = MPI_Sendrecv(list_1_org, idim_max + 1, MPI_UNSIGNED_LONG, origin, 0,
                        list_1buf_org, idim_max_buf + 1, MPI_UNSIGNED_LONG, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    ierr = MPI_Sendrecv(tmp_v1, idim_max + 1, MPI_DOUBLE_COMPLEX, origin, 0,
                        tmp_v1buf, idim_max_buf + 1, MPI_DOUBLE_COMPLEX, origin, 0, MPI_COMM_WORLD, &statusMPI);
    if (ierr != 0) exitMPI(-1);

    if (state2 == 0) {
        trans = 0;
    }
    else if (state2 == mask2) {
        trans = (double) Fsgn * tmp_trans;
    }
    else return 0;

    dam_pr = 0.0;
#pragma omp parallel for default(none) private(j, dmv) \
  firstprivate(idim_max_buf, trans, ioff, _irght, _ilft, _ihfbit, list_2_1_target, list_2_2_target) shared(tmp_v1buf, tmp_v1, tmp_v0, list_1buf_org)
    for (j = 1; j <= idim_max_buf; j++) {
        GetOffComp(list_2_1_target, list_2_2_target, list_1buf_org[j],
                   _irght, _ilft, _ihfbit, &ioff);
        dmv = trans * tmp_v1buf[j];
        tmp_v0[ioff] += dmv;
    }
    return (dam_pr);
#endif
}

