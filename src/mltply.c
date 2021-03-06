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
#include <bitcalc.h>
#include "mltply.h"
#include "mltplySpin.h"
#include "mltplyHubbard.h"
#include "wrapperMPI.h"
#include "CalcTime.h"
#include "mltplyCommon.h"

/**
 *
 *
 * @param X
 * @param tmp_v0
 * @param tmp_v1
 *
 * @return
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
int mltply(struct BindStruct *X, double complex *tmp_v0,double complex *tmp_v1) {
    long unsigned int j=0;
  long unsigned int irght=0;
  long unsigned int ilft=0;
  long unsigned int ihfbit=0;

  double complex dam_pr;

  long unsigned int i_max;

  StartTimer(1);
  i_max = X->Check.idim_max;
  X->Large.prdct = 0.0;
  dam_pr = 0.0;

  if(i_max!=0){
    if (X->Def.iFlgGeneralSpin == FALSE) {
      if (GetSplitBitByModel(X->Def.Nsite, X->Def.iCalcModel, &irght, &ilft, &ihfbit) != 0) {
        return -1;
      }
    }
    else{
      if(X->Def.iCalcModel==Spin){
        if (GetSplitBitForGeneralSpin(X->Def.Nsite, &ihfbit, X->Def.SiteToBit) != 0) {
          return -1;
        }
      }
    }
  }
  else{
    irght=0;
    ilft=0;
    ihfbit=0;
  }
  X->Large.i_max = i_max;
  X->Large.irght = irght;
  X->Large.ilft = ilft;
  X->Large.ihfbit = ihfbit;
  X->Large.mode = M_MLTPLY;

  StartTimer(100);
#pragma omp parallel for default(none) reduction(+:dam_pr) firstprivate(i_max) shared(tmp_v0, tmp_v1, list_Diagonal)
  for (j = 1; j <= i_max; j++) {
    tmp_v0[j] += (list_Diagonal[j]) * tmp_v1[j];
    dam_pr += (list_Diagonal[j]) * conj(tmp_v1[j]) * tmp_v1[j];
  }
  X->Large.prdct += dam_pr;
  StopTimer(100);
  
  switch (X->Def.iCalcModel) {
  case HubbardGC:
    mltplyHubbardGC(X, tmp_v0, tmp_v1);
    break;
      
  case KondoGC:
  case Hubbard:
  case Kondo:
    mltplyHubbard(X, tmp_v0, tmp_v1);
    break;
      
  case Spin:
    mltplySpin(X, tmp_v0, tmp_v1);
    break;
      
  case SpinGC:
    mltplySpinGC(X, tmp_v0, tmp_v1);
    break;
      
  default:
    return -1;
  }
  
  X->Large.prdct = SumMPI_dc(X->Large.prdct);  
  StopTimer(1);
  return 0;
}
