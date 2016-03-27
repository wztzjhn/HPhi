/*
HPhi  -  Quantum Lattice Model Simulator
Copyright (C) 2015 Takahiro Misawa, Kazuyoshi Yoshimi, Mitsuaki Kawamura, Youhei Yamaji, Synge Todo, Naoki Kawashima

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

struct StdIntList {
  /*
  Parameters for LATTICE
  */
  double a; /**< The lattice constant */
  double a0;
  double a1;
  int L;
  int W;
  double Lx;
  double Ly;
  double Wx;
  double Wy;
  int a0L;
  int a0W;
  int a1L;
  int a1W;
  double bW0;
  double bW1;
  double bL0;
  double bL1;
  int NCell;
  int **Cell;
  /*
  Parameters for MODEL
  */
  double mu;
  _Dcomplex t;
  _Dcomplex tp;
  _Dcomplex t0;
  _Dcomplex t1;
  _Dcomplex t1p;
  _Dcomplex t2;
  _Dcomplex t2p;
  double U;
  double V;
  double Vp;
  double V0;
  double V1;
  double V1p;
  double V2;
  double V2p;
  /**/
  double JAll;
  double JpAll;
  double J0All;
  double J1All;
  double J1pAll;
  double J2All;
  double J2pAll;
  double J[3][3];
  double Jp[3][3];
  double J0[3][3];
  double J1[3][3];
  double J1p[3][3];
  double J2[3][3];
  double J2p[3][3];
  double D[3][3];
  double h;
  double Gamma;
  double K;

  int nsite;
  int *locspinflag;
  int ntrans;
  int **transindx;
  _Dcomplex *trans;
  int nintr;
  int **intrindx;
  _Dcomplex *intr;

  double LargeValue;
  int S2;

  int ***list_6spin_pair;
  int **list_6spin_star;
  int num_pivot;
  int ishift_nspin;
};
