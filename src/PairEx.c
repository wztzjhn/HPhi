#include "bitcalc.h"
#include "mltplyCommon.h"
#include "PairEx.h"
#include "PairExHubbard.h"
#include "PairExSpin.h"

///
/// Calculation of pair excited state
/// Target System: Hubbard, Kondo, Spin
/// \param X define list to get and put information of calculation
/// \param tmp_v0 [out] Result v0 = H v1
/// \param tmp_v1 [in] v0 = H v1
/// \returns TRUE: Normally finished
/// \returns FALSE: Unnormally finished
/// \authour Kazuyoshi Yoshimi
/// \version 1.2
int GetPairExcitedState
(
 struct BindStruct *X,
 double complex *tmp_v0, /**< [out] Result v0 = H v1*/
 double complex *tmp_v1 /**< [in] v0 = H v1*/
 )
{
    int iret;
    long unsigned int irght,ilft,ihfbit;

  //  i_max = X->Check.idim_max;
    if(X->Def.iFlgGeneralSpin == FALSE) {
        if (GetSplitBitByModel(X->Def.Nsite, X->Def.iCalcModel, &irght, &ilft, &ihfbit) != 0) {
            return -1;
        }
    }
    else {
        if (GetSplitBitForGeneralSpin(X->Def.Nsite, &ihfbit, X->Def.SiteToBit) != 0) {
            return -1;
        }
    }

  X->Large.i_max    =  X->Check.idim_maxOrg;
  X->Large.irght    = irght;
  X->Large.ilft     = ilft;
  X->Large.ihfbit   = ihfbit;
  X->Large.mode=M_CALCSPEC;

    switch(X->Def.iCalcModel){
  case HubbardGC:
      iret=GetPairExcitedStateHubbardGC(X, tmp_v0, tmp_v1);
    break;

  case KondoGC:
  case Hubbard:
  case Kondo:
      iret=GetPairExcitedStateHubbard(X, tmp_v0, tmp_v1);
    break;

    case Spin: // for the Sz-conserved spin system
      iret =GetPairExcitedStateSpin(X, tmp_v0, tmp_v1);
      break;

    case SpinGC:
      iret=GetPairExcitedStateSpinGC(X,tmp_v0, tmp_v1);
      break;

    default:
        iret =FALSE;
      break;
    }

    return iret;
}
