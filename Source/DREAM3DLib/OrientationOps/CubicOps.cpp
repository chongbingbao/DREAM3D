/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "CubicOps.h"
// Include this FIRST because there is a needed define for some compiles
// to expose some of the constants needed below
#include <limits>

#include "DREAM3DLib/Math/DREAM3DMath.h"
#include "DREAM3DLib/Math/OrientationMath.h"
#include "DREAM3DLib/Common/ModifiedLambertProjection.h"
#include "DREAM3DLib/IOFilters/VtkRectilinearGridWriter.h"
#include "DREAM3DLib/Utilities/ImageUtilities.h"
#include "DREAM3DLib/Utilities/ColorTable.h"
#include "DREAM3DLib/Utilities/ColorUtilities.h"

#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/partitioner.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/task_group.h>
#include <tbb/task.h>
#endif


namespace Detail
{

  static const float CubicDim1InitValue = powf((0.75f * ((DREAM3D::Constants::k_Pi / 4.0f) - sinf((DREAM3D::Constants::k_Pi / 4.0f)))), (1.0f / 3.0f));
  static const float CubicDim2InitValue = powf((0.75f * ((DREAM3D::Constants::k_Pi / 4.0f) - sinf((DREAM3D::Constants::k_Pi / 4.0f)))), (1.0f / 3.0f));
  static const float CubicDim3InitValue = powf((0.75f * ((DREAM3D::Constants::k_Pi / 4.0f) - sinf((DREAM3D::Constants::k_Pi / 4.0f)))), (1.0f / 3.0f));
  static const float CubicDim1StepValue = CubicDim1InitValue / 9.0f;
  static const float CubicDim2StepValue = CubicDim2InitValue / 9.0f;
  static const float CubicDim3StepValue = CubicDim3InitValue / 9.0f;
  namespace CubicHigh
  {
    static const int symSize0 = 6;
    static const int symSize1 = 12;
    static const int symSize2 = 8;
  }
}


static const QuatF CubicQuatSym[24] =
{
  QuaternionMathF::NewXYZW(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f),
  QuaternionMathF::NewXYZW(1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f),
  QuaternionMathF::NewXYZW(0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f),
  QuaternionMathF::NewXYZW(0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f),
  QuaternionMathF::NewXYZW(DREAM3D::Constants::k_1OverRoot2, 0.000000000f, 0.000000000f, DREAM3D::Constants::k_1OverRoot2),
  QuaternionMathF::NewXYZW(0.000000000f, DREAM3D::Constants::k_1OverRoot2, 0.000000000f, DREAM3D::Constants::k_1OverRoot2),
  QuaternionMathF::NewXYZW(0.000000000f, 0.000000000f, DREAM3D::Constants::k_1OverRoot2, DREAM3D::Constants::k_1OverRoot2),
  QuaternionMathF::NewXYZW(-DREAM3D::Constants::k_1OverRoot2, 0.000000000f, 0.000000000f, DREAM3D::Constants::k_1OverRoot2),
  QuaternionMathF::NewXYZW(0.000000000f, -DREAM3D::Constants::k_1OverRoot2, 0.000000000f, DREAM3D::Constants::k_1OverRoot2),
  QuaternionMathF::NewXYZW(0.000000000f, 0.000000000f, -DREAM3D::Constants::k_1OverRoot2, DREAM3D::Constants::k_1OverRoot2),
  QuaternionMathF::NewXYZW(DREAM3D::Constants::k_1OverRoot2, DREAM3D::Constants::k_1OverRoot2, 0.000000000f, 0.000000000f),
  QuaternionMathF::NewXYZW(-DREAM3D::Constants::k_1OverRoot2, DREAM3D::Constants::k_1OverRoot2, 0.000000000f, 0.000000000f),
  QuaternionMathF::NewXYZW(0.000000000f, DREAM3D::Constants::k_1OverRoot2, DREAM3D::Constants::k_1OverRoot2, 0.000000000f),
  QuaternionMathF::NewXYZW(0.000000000f, -DREAM3D::Constants::k_1OverRoot2, DREAM3D::Constants::k_1OverRoot2, 0.000000000f),
  QuaternionMathF::NewXYZW(DREAM3D::Constants::k_1OverRoot2, 0.000000000f, DREAM3D::Constants::k_1OverRoot2, 0.000000000f),
  QuaternionMathF::NewXYZW(-DREAM3D::Constants::k_1OverRoot2, 0.000000000f, DREAM3D::Constants::k_1OverRoot2, 0.000000000f),
  QuaternionMathF::NewXYZW(0.500000000f, 0.500000000f, 0.500000000f, 0.500000000f),
  QuaternionMathF::NewXYZW(-0.500000000f, -0.500000000f, -0.500000000f, 0.500000000f),
  QuaternionMathF::NewXYZW(0.500000000f, -0.500000000f, 0.500000000f, 0.500000000f),
  QuaternionMathF::NewXYZW(-0.500000000f, 0.500000000f, -0.500000000f, 0.500000000f),
  QuaternionMathF::NewXYZW(-0.500000000f, 0.500000000f, 0.500000000f, 0.500000000f),
  QuaternionMathF::NewXYZW(0.500000000f, -0.500000000f, -0.500000000f, 0.500000000f),
  QuaternionMathF::NewXYZW(-0.500000000f, -0.500000000f, 0.500000000f, 0.500000000f),
  QuaternionMathF::NewXYZW(0.500000000f, 0.500000000f, -0.500000000f, 0.500000000f)
};

static const float CubicRodSym[24][3] = {{0.0f, 0.0f, 0.0f},
  {10000000000.0f, 0.0f, 0.0f},
  {0.0f, 10000000000.0f, 0.0f},
  {0.0f, 0.0f, 10000000000.0f},
  {1.0f, 0.0f, 0.0f},
  {0.0f, 1.0f, 0.0f},
  {0.0f, 0.0f, 1.0f},
  { -1.0f, 0.0f, 0.0f},
  {0.0f, -1.0f, 0.0f},
  {0.0f, 0.0f, -1.0f},
  {10000000000.0f, 10000000000.0f, 0.0f},
  { -10000000000.0f, 10000000000.0f, 0.0f},
  {0.0f, 10000000000.0f, 10000000000.0f},
  {0.0f, -10000000000.0f, 10000000000.0f},
  {10000000000.0f, 0.0f, 10000000000.0f},
  { -10000000000.0f, 0.0f, 10000000000.0f},
  {1.0f, 1.0f, 1.0f},
  { -1.0f, -1.0f, -1.0f},
  {1.0f, -1.0f, 1.0f},
  { -1.0f, 1.0f, -1.0f},
  { -1.0f, 1.0f, 1.0f},
  {1.0f, -1.0f, -1.0f},
  { -1.0f, -1.0f, 1.0f},
  {1.0f, 1.0f, -1.0}
};

static const float CubicSlipDirections[12][3] = {{0.0f, 1.0f, -1.0f},
  {1.0f, 0.0f, -1.0f},
  {1.0f, -1.0f, 0.0f},
  {1.0f, -1.0f, 0.0f},
  {1.0f, 0.0f, 1.0f},
  {0.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 0.0f},
  {0.0f, 1.0f, 1.0f},
  {1.0f, 0.0f, -1.0f},
  {1.0f, 1.0f, 0.0f},
  {1.0f, 0.0f, 1.0f},
  {0.0f, 1.0f, -1.0f}
};

static const float CubicSlipPlanes[12][3] = {{1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, -1.0f},
  {1.0f, 1.0f, -1.0f},
  {1.0f, 1.0f, -1.0f},
  {1.0f, -1.0f, 1.0f},
  {1.0f, -1.0f, 1.0f},
  {1.0f, -1.0f, 1.0f},
  { -1.0f, 1.0f, 1.0f},
  { -1.0f, 1.0f, 1.0f},
  { -1.0f, 1.0f, 1.0f}
};

static const float CubicMatSym[24][3][3] =
{
  { {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
  },

  { {1.0, 0.0,  0.0},
    {0.0, 0.0, -1.0},
    {0.0, 1.0,  0.0}
  },

  { {1.0,  0.0,  0.0},
    {0.0, -1.0,  0.0},
    {0.0,  0.0, -1.0}
  },

  { {1.0,  0.0, 0.0},
    {0.0,  0.0, 1.0},
    {0.0, -1.0, 0.0}
  },

  { {0.0, 0.0, -1.0},
    {0.0, 1.0,  0.0},
    {1.0, 0.0,  0.0}
  },

  { {0.0, 0.0, 1.0},
    {0.0, 1.0, 0.0},
    { -1.0, 0.0, 0.0}
  },

  { { -1.0, 0.0,  0.0},
    {0.0, 1.0,  0.0},
    {0.0, 0.0, -1.0}
  },

  { { -1.0,  0.0, 0.0},
    {0.0, -1.0, 0.0},
    {0.0,  0.0, 1.0}
  },

  { {0.0, 1.0, 0.0},
    { -1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0}
  },

  { {0.0, -1.0, 0.0},
    {1.0,  0.0, 0.0},
    {0.0,  0.0, 1.0}
  },

  { {0.0, -1.0, 0.0},
    {0.0,  0.0, 1.0},
    { -1.0,  0.0, 0.0}
  },

  { {0.0,  0.0, 1.0},
    { -1.0,  0.0, 0.0},
    {0.0, -1.0, 0.0}
  },

  { {0.0, -1.0,  0.0},
    {0.0,  0.0, -1.0},
    {1.0,  0.0,  0.0}
  },

  { {0.0,  0.0, -1.0},
    {1.0,  0.0,  0.0},
    {0.0, -1.0,  0.0}
  },

  { {0.0, 1.0,  0.0},
    {0.0, 0.0, -1.0},
    { -1.0, 0.0,  0.0}
  },

  { {0.0, 0.0, -1.0},
    { -1.0, 0.0,  0.0},
    {0.0, 1.0,  0.0}
  },

  { {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0},
    {1.0, 0.0, 0.0}
  },

  { {0.0, 0.0, 1.0},
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0}
  },

  { {0.0, 1.0,  0.0},
    {1.0, 0.0,  0.0},
    {0.0, 0.0, -1.0}
  },

  { { -1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0},
    {0.0, 1.0, 0.0}
  },

  { {0.0,  0.0, 1.0},
    {0.0, -1.0, 0.0},
    {1.0,  0.0, 0.0}
  },

  { { -1.0,  0.0,  0.0},
    {0.0,  0.0, -1.0},
    {0.0, -1.0,  0.0}
  },

  { {0.0,  0.0, -1.0},
    {0.0, -1.0,  0.0},
    { -1.0,  0.0,  0.0}
  },

  { {0.0, -1.0,  0.0},
    { -1.0,  0.0,  0.0},
    {0.0,  0.0, -1.0}
  }
};

using namespace Detail;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CubicOps::CubicOps()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CubicOps::~CubicOps()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
float CubicOps::getMisoQuat(QuatF& q1, QuatF& q2, float& n1, float& n2, float& n3)
{

  int numsym = 24;

  return _calcMisoQuat(CubicQuatSym, numsym, q1, q2, n1, n2, n3);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
float CubicOps::_calcMisoQuat(const QuatF quatsym[24], int numsym,
                              QuatF& q1, QuatF& q2,
                              float& n1, float& n2, float& n3)
{
  float wmin = 9999999.0f; //,na,nb,nc;
  QuatF qco;
  QuatF qc;
  QuatF q2inv;
  int type = 1;
  float sin_wmin_over_2 = 0.0;
  //  float _1, _2,  _6;

  QuaternionMathF::Copy(q2, q2inv);
  QuaternionMathF::Conjugate(q2inv);

  QuaternionMathF::Multiply(q2inv, q1, qc);
  QuaternionMathF::ElementWiseAbs(qc);

  //if qc.x is smallest
  if ( qc.x <= qc.y && qc.x <= qc.z && qc.x <= qc.w)
  {
    qco.x = qc.x;
    //if qc.y is next smallest
    if (qc.y <= qc.z && qc.y <= qc.w)
    {
      qco.y = qc.y;
      if(qc.z <= qc.w) { qco.z = qc.z, qco.w = qc.w; }
      else { qco.z = qc.w, qco.w = qc.z; }
    }
    //if qc.z is next smallest
    else if (qc.z <= qc.y && qc.z <= qc.w)
    {
      qco.y = qc.z;
      if(qc.y <= qc.w) { qco.z = qc.y, qco.w = qc.w; }
      else { qco.z = qc.w, qco.w = qc.y; }
    }
    //if qc.w is next smallest
    else
    {
      qco.y = qc.w;
      if(qc.y <= qc.z) { qco.z = qc.y, qco.w = qc.z; }
      else { qco.z = qc.z, qco.w = qc.y; }
    }
  }
  //if qc.y is smallest
  else if ( qc.y <= qc.x && qc.y <= qc.z && qc.y <= qc.w)
  {
    qco.x = qc.y;
    //if qc.x is next smallest
    if (qc.x <= qc.z && qc.x <= qc.w)
    {
      qco.y = qc.x;
      if(qc.z <= qc.w) { qco.z = qc.z, qco.w = qc.w; }
      else { qco.z = qc.w, qco.w = qc.z; }
    }
    //if qc.z is next smallest
    else if (qc.z <= qc.x && qc.z <= qc.w)
    {
      qco.y = qc.z;
      if(qc.x <= qc.w) { qco.z = qc.x, qco.w = qc.w; }
      else { qco.z = qc.w, qco.w = qc.x; }
    }
    //if qc.w is next smallest
    else
    {
      qco.y = qc.w;
      if(qc.x <= qc.z) { qco.z = qc.x, qco.w = qc.z; }
      else { qco.z = qc.z, qco.w = qc.x; }
    }
  }
  //if qc.z is smallest
  else if ( qc.z <= qc.x && qc.z <= qc.y && qc.z <= qc.w)
  {
    qco.x = qc.z;
    //if qc.x is next smallest
    if (qc.x <= qc.y && qc.x <= qc.w)
    {
      qco.y = qc.x;
      if(qc.y <= qc.w) { qco.z = qc.y, qco.w = qc.w; }
      else { qco.z = qc.w, qco.w = qc.y; }
    }
    //if qc.y is next smallest
    else if (qc.y <= qc.x && qc.y <= qc.w)
    {
      qco.y = qc.y;
      if(qc.x <= qc.w) { qco.z = qc.x, qco.w = qc.w; }
      else { qco.z = qc.w, qco.w = qc.x; }
    }
    //if qc.w is next smallest
    else
    {
      qco.y = qc.w;
      if(qc.x <= qc.y) { qco.z = qc.x, qco.w = qc.y; }
      else { qco.z = qc.y, qco.w = qc.x; }
    }
  }
  //if qc.w is smallest
  else
  {
    qco.x = qc.w;
    //if qc.x is next smallest
    if (qc.x <= qc.y && qc.x <= qc.z)
    {
      qco.y = qc.x;
      if(qc.y <= qc.z) { qco.z = qc.y, qco.w = qc.z; }
      else { qco.z = qc.z, qco.w = qc.y; }
    }
    //if qc.y is next smallest
    else if (qc.y <= qc.x && qc.y <= qc.z)
    {
      qco.y = qc.y;
      if(qc.x <= qc.z) { qco.z = qc.x, qco.w = qc.z; }
      else { qco.z = qc.z, qco.w = qc.x; }
    }
    //if qc.z is next smallest
    else
    {
      qco.y = qc.z;
      if(qc.x <= qc.y) { qco.z = qc.x, qco.w = qc.y; }
      else { qco.z = qc.y, qco.w = qc.x; }
    }
  }
  wmin = qco.w;
  if (((qco.z + qco.w) / (DREAM3D::Constants::k_Sqrt2)) > wmin)
  {
    wmin = ((qco.z + qco.w) / (DREAM3D::Constants::k_Sqrt2));
    type = 2;
  }
  if (((qco.x + qco.y + qco.z + qco.w) / 2) > wmin)
  {
    wmin = ((qco.x + qco.y + qco.z + qco.w) / 2);
    type = 3;
  }
  if (wmin < -1.0)
  {
    //  wmin = -1.0;
    wmin = DREAM3D::Constants::k_ACosNeg1;
    sin_wmin_over_2 = sinf(wmin);
  }
  else if (wmin > 1.0)
  {
    //   wmin = 1.0;
    wmin = DREAM3D::Constants::k_ACos1;
    sin_wmin_over_2 = sinf(wmin);
  }
  else
  {
    wmin = acos(wmin);
    sin_wmin_over_2 = sinf(wmin);
  }

  if(type == 1)
  {
    n1 = qco.x / sin_wmin_over_2;
    n2 = qco.y / sin_wmin_over_2;
    n3 = qco.z / sin_wmin_over_2;
  }
  if(type == 2)
  {
    n1 = ((qco.x - qco.y) / (DREAM3D::Constants::k_Sqrt2)) / sin_wmin_over_2;
    n2 = ((qco.x + qco.y) / (DREAM3D::Constants::k_Sqrt2)) / sin_wmin_over_2;
    n3 = ((qco.z - qco.w) / (DREAM3D::Constants::k_Sqrt2)) / sin_wmin_over_2;
  }
  if(type == 3)
  {
    n1 = ((qco.x - qco.y + qco.z - qco.w) / (2.0f)) / sin_wmin_over_2;
    n2 = ((qco.x + qco.y - qco.z - qco.w) / (2.0f)) / sin_wmin_over_2;
    n3 = ((-qco.x + qco.y + qco.z - qco.w) / (2.0f)) / sin_wmin_over_2;
  }
  float denom = sqrt((n1 * n1 + n2 * n2 + n3 * n3));
  n1 = n1 / denom;
  n2 = n2 / denom;
  n3 = n3 / denom;
  if(denom == 0) { n1 = 0.0, n2 = 0.0, n3 = 1.0; }
  if(wmin == 0) { n1 = 0.0, n2 = 0.0, n3 = 1.0; }
  wmin = 2.0f * wmin;
  return wmin;

}

void CubicOps::getODFFZRod(float& r1, float& r2, float& r3)
{
  int numsym = 24;

  _calcRodNearestOrigin(CubicRodSym, numsym, r1, r2, r3);
}

void CubicOps::getQuatSymOp(int i, QuatF& q)
{
  QuaternionMathF::Copy(CubicQuatSym[i], q);
}

void CubicOps::getRodSymOp(int i, float* r)
{
  r[0] = CubicRodSym[i][0];
  r[1] = CubicRodSym[i][1];
  r[2] = CubicRodSym[i][2];
}

void CubicOps::getMatSymOp(int i, float g[3][3])
{
  g[0][0] = CubicMatSym[i][0][0];
  g[0][1] = CubicMatSym[i][0][1];
  g[0][2] = CubicMatSym[i][0][2];
  g[1][0] = CubicMatSym[i][1][0];
  g[1][1] = CubicMatSym[i][1][1];
  g[1][2] = CubicMatSym[i][1][2];
  g[2][0] = CubicMatSym[i][2][0];
  g[2][1] = CubicMatSym[i][2][1];
  g[2][2] = CubicMatSym[i][2][2];
}

void CubicOps::getMDFFZRod(float& r1, float& r2, float& r3)
{
  float w, n1, n2, n3;
  float FZw, FZn1, FZn2, FZn3;

  OrientationOps::_calcRodNearestOrigin(CubicRodSym, 24, r1, r2, r3);
  OrientationMath::RodtoAxisAngle(r1, r2, r3, w, n1, n2, n3);

  FZw = w;
  n1 = fabs(n1);
  n2 = fabs(n2);
  n3 = fabs(n3);
  if(n1 > n2)
  {
    if(n1 > n3)
    {
      FZn1 = n1;
      if (n2 > n3) { FZn2 = n2, FZn3 = n3; }
      else { FZn2 = n3, FZn3 = n2; }
    }
    else { FZn1 = n3, FZn2 = n1, FZn3 = n2; }
  }
  else
  {
    if(n2 > n3)
    {
      FZn1 = n2;
      if (n1 > n3) { FZn2 = n1, FZn3 = n3; }
      else { FZn2 = n3, FZn3 = n1; }
    }
    else { FZn1 = n3, FZn2 = n2, FZn3 = n1; }
  }

  OrientationMath::AxisAngletoRod(FZw, FZn1, FZn2, FZn3, r1, r2, r3);
}

void CubicOps::getNearestQuat(QuatF& q1, QuatF& q2)
{
  int numsym = 24;

  _calcNearestQuat(CubicQuatSym, numsym, q1, q2);
}

void CubicOps::getFZQuat(QuatF& qr)
{
  int numsym = 24;

  _calcQuatNearestOrigin(CubicQuatSym, numsym, qr);
}

int CubicOps::getMisoBin(float r1, float r2, float r3)
{
  float dim[3];
  float bins[3];
  float step[3];

  OrientationMath::RodtoHomochoric(r1, r2, r3);

  dim[0] = Detail::CubicDim1InitValue;
  dim[1] = Detail::CubicDim2InitValue;
  dim[2] = Detail::CubicDim3InitValue;
  step[0] = Detail::CubicDim1StepValue;
  step[1] = Detail::CubicDim2StepValue;
  step[2] = Detail::CubicDim3StepValue;
  bins[0] = 18.0f;
  bins[1] = 18.0f;
  bins[2] = 18.0f;

  return _calcMisoBin(dim, bins, step, r1, r2, r3);
}

void CubicOps::determineEulerAngles(int choose, float& synea1, float& synea2, float& synea3)
{
  float init[3];
  float step[3];
  float phi[3];
  float r1, r2, r3;

  init[0] = Detail::CubicDim1InitValue;
  init[1] = Detail::CubicDim2InitValue;
  init[2] = Detail::CubicDim3InitValue;
  step[0] = Detail::CubicDim1StepValue;
  step[1] = Detail::CubicDim2StepValue;
  step[2] = Detail::CubicDim3StepValue;
  phi[0] = static_cast<float>(choose % 18);
  phi[1] = static_cast<float>((choose / 18) % 18);
  phi[2] = static_cast<float>(choose / (18 * 18));

  _calcDetermineHomochoricValues(init, step, phi, choose, r1, r2, r3);
  OrientationMath::HomochorictoRod(r1, r2, r3);
  getODFFZRod(r1, r2, r3);
  OrientationMath::RodtoEuler(r1, r2, r3, synea1, synea2, synea3);
}

void CubicOps::determineRodriguesVector(int choose, float& r1, float& r2, float& r3)
{
  float init[3];
  float step[3];
  float phi[3];

  init[0] = Detail::CubicDim1InitValue;
  init[1] = Detail::CubicDim2InitValue;
  init[2] = Detail::CubicDim3InitValue;
  step[0] = Detail::CubicDim1StepValue;
  step[1] = Detail::CubicDim2StepValue;
  step[2] = Detail::CubicDim3StepValue;
  phi[0] = static_cast<float>(choose % 18);
  phi[1] = static_cast<float>((choose / 18) % 18);
  phi[2] = static_cast<float>(choose / (18 * 18));

  _calcDetermineHomochoricValues(init, step, phi, choose, r1, r2, r3);
  OrientationMath::HomochorictoRod(r1, r2, r3);
  getMDFFZRod(r1, r2, r3);
}
int CubicOps::getOdfBin(float r1, float r2, float r3)
{
  float dim[3];
  float bins[3];
  float step[3];

  OrientationMath::RodtoHomochoric(r1, r2, r3);

  dim[0] = Detail::CubicDim1InitValue;
  dim[1] = Detail::CubicDim2InitValue;
  dim[2] = Detail::CubicDim3InitValue;
  step[0] = Detail::CubicDim1StepValue;
  step[1] = Detail::CubicDim2StepValue;
  step[2] = Detail::CubicDim3StepValue;
  bins[0] = 18.0f;
  bins[1] = 18.0f;
  bins[2] = 18.0f;

  return _calcODFBin(dim, bins, step, r1, r2, r3);
}

void CubicOps::getSchmidFactorAndSS(float loadx, float loady, float loadz, float& schmidfactor, int& slipsys)
{
  schmidfactor = 0.0;
  float theta1, theta2, theta3, theta4;
  float lambda1, lambda2, lambda3, lambda4, lambda5, lambda6;
  float schmid1, schmid2, schmid3, schmid4, schmid5, schmid6, schmid7, schmid8, schmid9, schmid10, schmid11, schmid12;

  float mag = loadx * loadx + loady * loady + loadz * loadz;
  mag = sqrt(mag);
  theta1 = (loadx + loady + loadz) / (mag * 1.732f);
  theta1 = fabs(theta1);
  theta2 = (loadx + loady - loadz) / (mag * 1.732f);
  theta2 = fabs(theta2);
  theta3 = (loadx - loady + loadz) / (mag * 1.732f);
  theta3 = fabs(theta3);
  theta4 = (-loadx + loady + loadz) / (mag * 1.732f);
  theta4 = fabs(theta4);
  lambda1 = (loadx + loady) / (mag * 1.414f);
  lambda1 = fabs(lambda1);
  lambda2 = (loadx + loadz) / (mag * 1.414f);
  lambda2 = fabs(lambda2);
  lambda3 = (loadx - loady) / (mag * 1.414f);
  lambda3 = fabs(lambda3);
  lambda4 = (loadx - loadz) / (mag * 1.414f);
  lambda4 = fabs(lambda4);
  lambda5 = (loady + loadz) / (mag * 1.414f);
  lambda5 = fabs(lambda5);
  lambda6 = (loady - loadz) / (mag * 1.414f);
  lambda6 = fabs(lambda6);
  schmid1 = theta1 * lambda6;
  schmid2 = theta1 * lambda4;
  schmid3 = theta1 * lambda3;
  schmid4 = theta2 * lambda3;
  schmid5 = theta2 * lambda2;
  schmid6 = theta2 * lambda5;
  schmid7 = theta3 * lambda1;
  schmid8 = theta3 * lambda5;
  schmid9 = theta3 * lambda4;
  schmid10 = theta4 * lambda1;
  schmid11 = theta4 * lambda2;
  schmid12 = theta4 * lambda6;
  schmidfactor = schmid1, slipsys = 0;

  if (schmid2 > schmidfactor) { schmidfactor = schmid2, slipsys = 1; }
  if (schmid3 > schmidfactor) { schmidfactor = schmid3, slipsys = 2; }
  if (schmid4 > schmidfactor) { schmidfactor = schmid4, slipsys = 3; }
  if (schmid5 > schmidfactor) { schmidfactor = schmid5, slipsys = 4; }
  if (schmid6 > schmidfactor) { schmidfactor = schmid6, slipsys = 5; }
  if (schmid7 > schmidfactor) { schmidfactor = schmid7, slipsys = 6; }
  if (schmid8 > schmidfactor) { schmidfactor = schmid8, slipsys = 7; }
  if (schmid9 > schmidfactor) { schmidfactor = schmid9, slipsys = 8; }
  if (schmid10 > schmidfactor) { schmidfactor = schmid10, slipsys = 9; }
  if (schmid11 > schmidfactor) { schmidfactor = schmid11, slipsys = 10; }
  if (schmid12 > schmidfactor) { schmidfactor = schmid12, slipsys = 11; }
}

void CubicOps::getmPrime(QuatF& q1, QuatF& q2, float LD[3], float& mPrime)
{
  float g1[3][3];
  float g2[3][3];
  float hkl1[3], uvw1[3];
  float hkl2[3], uvw2[3];
  float slipDirection[3], slipPlane[3];
  float schmidFactor1 = 0, schmidFactor2 = 0, maxSchmidFactor = 0;
  float directionComponent1 = 0, planeComponent1 = 0;
  float directionComponent2 = 0, planeComponent2 = 0;
  float planeMisalignment = 0, directionMisalignment = 0;
  int ss1 = 0, ss2 = 0;

  OrientationMath::QuattoMat(q1, g1);
  OrientationMath::QuattoMat(q2, g2);
  MatrixMath::Transpose3x3(g1, g1);
  MatrixMath::Transpose3x3(g2, g2);
  for(int i = 0; i < 12; i++)
  {
    slipDirection[0] = CubicSlipDirections[i][0];
    slipDirection[1] = CubicSlipDirections[i][1];
    slipDirection[2] = CubicSlipDirections[i][2];
    slipPlane[0] = CubicSlipPlanes[i][0];
    slipPlane[1] = CubicSlipPlanes[i][1];
    slipPlane[2] = CubicSlipPlanes[i][2];
    MatrixMath::Multiply3x3with3x1(g1, slipDirection, hkl1);
    MatrixMath::Multiply3x3with3x1(g1, slipPlane, uvw1);
    MatrixMath::Normalize3x1(hkl1);
    MatrixMath::Normalize3x1(uvw1);
    directionComponent1 = fabs(MatrixMath::CosThetaBetweenVectors(LD, uvw1));
    planeComponent1 = fabs(MatrixMath::CosThetaBetweenVectors(LD, hkl1));
    schmidFactor1 = directionComponent1 * planeComponent1;
    if(schmidFactor1 > maxSchmidFactor)
    {
      maxSchmidFactor = schmidFactor1;
      ss1 = i;
    }
  }
  slipDirection[0] = CubicSlipDirections[ss1][0];
  slipDirection[1] = CubicSlipDirections[ss1][1];
  slipDirection[2] = CubicSlipDirections[ss1][2];
  slipPlane[0] = CubicSlipPlanes[ss1][0];
  slipPlane[1] = CubicSlipPlanes[ss1][1];
  slipPlane[2] = CubicSlipPlanes[ss1][2];
  MatrixMath::Multiply3x3with3x1(g1, slipDirection, hkl1);
  MatrixMath::Multiply3x3with3x1(g1, slipPlane, uvw1);
  MatrixMath::Normalize3x1(hkl1);
  MatrixMath::Normalize3x1(uvw1);

  maxSchmidFactor = 0;
  for(int j = 0; j < 12; j++)
  {
    slipDirection[0] = CubicSlipDirections[j][0];
    slipDirection[1] = CubicSlipDirections[j][1];
    slipDirection[2] = CubicSlipDirections[j][2];
    slipPlane[0] = CubicSlipPlanes[j][0];
    slipPlane[1] = CubicSlipPlanes[j][1];
    slipPlane[2] = CubicSlipPlanes[j][2];
    MatrixMath::Multiply3x3with3x1(g2, slipDirection, hkl2);
    MatrixMath::Multiply3x3with3x1(g2, slipPlane, uvw2);
    MatrixMath::Normalize3x1(hkl2);
    MatrixMath::Normalize3x1(uvw2);
    directionComponent2 = fabs(MatrixMath::CosThetaBetweenVectors(LD, uvw2));
    planeComponent2 = fabs(MatrixMath::CosThetaBetweenVectors(LD, hkl2));
    schmidFactor2 = directionComponent2 * planeComponent2;
    if(schmidFactor2 > maxSchmidFactor)
    {
      maxSchmidFactor = schmidFactor2;
      ss2 = j;
    }
  }
  slipDirection[0] = CubicSlipDirections[ss2][0];
  slipDirection[1] = CubicSlipDirections[ss2][1];
  slipDirection[2] = CubicSlipDirections[ss2][2];
  slipPlane[0] = CubicSlipPlanes[ss2][0];
  slipPlane[1] = CubicSlipPlanes[ss2][1];
  slipPlane[2] = CubicSlipPlanes[ss2][2];
  MatrixMath::Multiply3x3with3x1(g2, slipDirection, hkl2);
  MatrixMath::Multiply3x3with3x1(g2, slipPlane, uvw2);
  MatrixMath::Normalize3x1(hkl2);
  MatrixMath::Normalize3x1(uvw2);
  planeMisalignment = fabs(MatrixMath::CosThetaBetweenVectors(hkl1, hkl2));
  directionMisalignment = fabs(MatrixMath::CosThetaBetweenVectors(uvw1, uvw2));
  mPrime = planeMisalignment * directionMisalignment;
}

void CubicOps::getF1(QuatF& q1, QuatF& q2, float LD[3], bool maxSF, float& F1)
{
  float g1[3][3];
  float g2[3][3];
  float hkl1[3], uvw1[3];
  float hkl2[3], uvw2[3];
  float slipDirection[3], slipPlane[3];
  float directionMisalignment = 0, totalDirectionMisalignment = 0;
  float schmidFactor1 = 0, schmidFactor2 = 0, maxSchmidFactor = 0;
  float directionComponent1 = 0, planeComponent1 = 0;
  float directionComponent2 = 0, planeComponent2 = 0;
  float maxF1 = 0;

  OrientationMath::QuattoMat(q1, g1);
  OrientationMath::QuattoMat(q2, g2);
  MatrixMath::Transpose3x3(g1, g1);
  MatrixMath::Transpose3x3(g2, g2);

  MatrixMath::Normalize3x1(LD);

  if(maxSF == true) { maxSchmidFactor = 0; }
  for(int i = 0; i < 12; i++)
  {
    slipDirection[0] = CubicSlipDirections[i][0];
    slipDirection[1] = CubicSlipDirections[i][1];
    slipDirection[2] = CubicSlipDirections[i][2];
    slipPlane[0] = CubicSlipPlanes[i][0];
    slipPlane[1] = CubicSlipPlanes[i][1];
    slipPlane[2] = CubicSlipPlanes[i][2];
    MatrixMath::Multiply3x3with3x1(g1, slipDirection, hkl1);
    MatrixMath::Multiply3x3with3x1(g1, slipPlane, uvw1);
    MatrixMath::Normalize3x1(hkl1);
    MatrixMath::Normalize3x1(uvw1);
    directionComponent1 = fabs(MatrixMath::CosThetaBetweenVectors(LD, uvw1));
    planeComponent1 = fabs(MatrixMath::CosThetaBetweenVectors(LD, hkl1));
    schmidFactor1 = directionComponent1 * planeComponent1;
    if(schmidFactor1 > maxSchmidFactor || maxSF == false)
    {
      totalDirectionMisalignment = 0;
      if(maxSF == true) { maxSchmidFactor = schmidFactor1; }
      for(int j = 0; j < 12; j++)
      {
        slipDirection[0] = CubicSlipDirections[j][0];
        slipDirection[1] = CubicSlipDirections[j][1];
        slipDirection[2] = CubicSlipDirections[j][2];
        slipPlane[0] = CubicSlipPlanes[j][0];
        slipPlane[1] = CubicSlipPlanes[j][1];
        slipPlane[2] = CubicSlipPlanes[j][2];
        MatrixMath::Multiply3x3with3x1(g2, slipDirection, hkl2);
        MatrixMath::Multiply3x3with3x1(g2, slipPlane, uvw2);
        MatrixMath::Normalize3x1(hkl2);
        MatrixMath::Normalize3x1(uvw2);
        directionComponent2 = fabs(MatrixMath::CosThetaBetweenVectors(LD, uvw2));
        planeComponent2 = fabs(MatrixMath::CosThetaBetweenVectors(LD, hkl2));
        schmidFactor2 = directionComponent2 * planeComponent2;
        directionMisalignment = fabs(MatrixMath::CosThetaBetweenVectors(uvw1, uvw2));
        totalDirectionMisalignment = totalDirectionMisalignment + directionMisalignment;
      }
      F1 = schmidFactor1 * directionComponent1 * totalDirectionMisalignment;
      if(maxSF == false)
      {
        if(F1 < maxF1) { F1 = maxF1; }
        else { maxF1 = F1; }
      }
    }
  }
}

void CubicOps::getF1spt(QuatF& q1, QuatF& q2, float LD[3], bool maxSF, float& F1spt)
{
  float g1[3][3];
  float g2[3][3];
  float hkl1[3], uvw1[3];
  float hkl2[3], uvw2[3];
  float slipDirection[3], slipPlane[3];
  float directionMisalignment = 0, totalDirectionMisalignment = 0;
  float planeMisalignment = 0, totalPlaneMisalignment = 0;
  float schmidFactor1 = 0, schmidFactor2 = 0, maxSchmidFactor = 0;
  float directionComponent1 = 0, planeComponent1 = 0;
  float directionComponent2 = 0, planeComponent2 = 0;
  float maxF1spt = 0;

  OrientationMath::QuattoMat(q1, g1);
  OrientationMath::QuattoMat(q2, g2);
  MatrixMath::Transpose3x3(g1, g1);
  MatrixMath::Transpose3x3(g2, g2);

  MatrixMath::Normalize3x1(LD);

  if(maxSF == true) { maxSchmidFactor = 0; }
  for(int i = 0; i < 12; i++)
  {
    slipDirection[0] = CubicSlipDirections[i][0];
    slipDirection[1] = CubicSlipDirections[i][1];
    slipDirection[2] = CubicSlipDirections[i][2];
    slipPlane[0] = CubicSlipPlanes[i][0];
    slipPlane[1] = CubicSlipPlanes[i][1];
    slipPlane[2] = CubicSlipPlanes[i][2];
    MatrixMath::Multiply3x3with3x1(g1, slipDirection, hkl1);
    MatrixMath::Multiply3x3with3x1(g1, slipPlane, uvw1);
    MatrixMath::Normalize3x1(hkl1);
    MatrixMath::Normalize3x1(uvw1);
    directionComponent1 = fabs(MatrixMath::CosThetaBetweenVectors(LD, uvw1));
    planeComponent1 = fabs(MatrixMath::CosThetaBetweenVectors(LD, hkl1));
    schmidFactor1 = directionComponent1 * planeComponent1;
    if(schmidFactor1 > maxSchmidFactor || maxSF == false)
    {
      totalDirectionMisalignment = 0;
      totalPlaneMisalignment = 0;
      if(maxSF == true) { maxSchmidFactor = schmidFactor1; }
      for(int j = 0; j < 12; j++)
      {
        slipDirection[0] = CubicSlipDirections[j][0];
        slipDirection[1] = CubicSlipDirections[j][1];
        slipDirection[2] = CubicSlipDirections[j][2];
        slipPlane[0] = CubicSlipPlanes[j][0];
        slipPlane[1] = CubicSlipPlanes[j][1];
        slipPlane[2] = CubicSlipPlanes[j][2];
        MatrixMath::Multiply3x3with3x1(g2, slipDirection, hkl2);
        MatrixMath::Multiply3x3with3x1(g2, slipPlane, uvw2);
        MatrixMath::Normalize3x1(hkl2);
        MatrixMath::Normalize3x1(uvw2);
        directionComponent2 = fabs(MatrixMath::CosThetaBetweenVectors(LD, uvw2));
        planeComponent2 = fabs(MatrixMath::CosThetaBetweenVectors(LD, hkl2));
        schmidFactor2 = directionComponent2 * planeComponent2;
        directionMisalignment = fabs(MatrixMath::CosThetaBetweenVectors(uvw1, uvw2));
        planeMisalignment = fabs(MatrixMath::CosThetaBetweenVectors(hkl1, hkl2));
        totalDirectionMisalignment = totalDirectionMisalignment + directionMisalignment;
        totalPlaneMisalignment = totalPlaneMisalignment + planeMisalignment;
      }
      F1spt = schmidFactor1 * directionComponent1 * totalDirectionMisalignment * totalPlaneMisalignment;
      if(maxSF == false)
      {
        if(F1spt < maxF1spt) { F1spt = maxF1spt; }
        else { maxF1spt = F1spt; }
      }
    }
  }
}

void CubicOps::getF7(QuatF& q1, QuatF& q2, float LD[3], bool maxSF, float& F7)
{
  float g1[3][3];
  float g2[3][3];
  float hkl1[3], uvw1[3];
  float hkl2[3], uvw2[3];
  float slipDirection[3], slipPlane[3];
  float directionMisalignment = 0, totalDirectionMisalignment = 0;
  float schmidFactor1 = 0, schmidFactor2 = 0, maxSchmidFactor = 0;
  float directionComponent1 = 0, planeComponent1 = 0;
  float directionComponent2 = 0, planeComponent2 = 0;
  float maxF7 = 0;

  OrientationMath::QuattoMat(q1, g1);
  OrientationMath::QuattoMat(q2, g2);
  MatrixMath::Transpose3x3(g1, g1);
  MatrixMath::Transpose3x3(g2, g2);

  MatrixMath::Normalize3x1(LD);

  for(int i = 0; i < 12; i++)
  {
    slipDirection[0] = CubicSlipDirections[i][0];
    slipDirection[1] = CubicSlipDirections[i][1];
    slipDirection[2] = CubicSlipDirections[i][2];
    slipPlane[0] = CubicSlipPlanes[i][0];
    slipPlane[1] = CubicSlipPlanes[i][1];
    slipPlane[2] = CubicSlipPlanes[i][2];
    MatrixMath::Multiply3x3with3x1(g1, slipDirection, hkl1);
    MatrixMath::Multiply3x3with3x1(g1, slipPlane, uvw1);
    MatrixMath::Normalize3x1(hkl1);
    MatrixMath::Normalize3x1(uvw1);
    directionComponent1 = fabs(MatrixMath::CosThetaBetweenVectors(LD, uvw1));
    planeComponent1 = fabs(MatrixMath::CosThetaBetweenVectors(LD, hkl1));
    schmidFactor1 = directionComponent1 * planeComponent1;
    if(schmidFactor1 > maxSchmidFactor || maxSF == false)
    {
      totalDirectionMisalignment = 0;
      if(maxSF == true) { maxSchmidFactor = schmidFactor1; }
      for(int j = 0; j < 12; j++)
      {
        slipDirection[0] = CubicSlipDirections[j][0];
        slipDirection[1] = CubicSlipDirections[j][1];
        slipDirection[2] = CubicSlipDirections[j][2];
        slipPlane[0] = CubicSlipPlanes[j][0];
        slipPlane[1] = CubicSlipPlanes[j][1];
        slipPlane[2] = CubicSlipPlanes[j][2];
        MatrixMath::Multiply3x3with3x1(g2, slipDirection, hkl2);
        MatrixMath::Multiply3x3with3x1(g2, slipPlane, uvw2);
        MatrixMath::Normalize3x1(hkl2);
        MatrixMath::Normalize3x1(uvw2);
        directionComponent2 = fabs(MatrixMath::CosThetaBetweenVectors(LD, uvw2));
        planeComponent2 = fabs(MatrixMath::CosThetaBetweenVectors(LD, hkl2));
        schmidFactor2 = directionComponent2 * planeComponent2;
        directionMisalignment = fabs(MatrixMath::CosThetaBetweenVectors(uvw1, uvw2));
        totalDirectionMisalignment = totalDirectionMisalignment + directionMisalignment;
      }
      F7 = directionComponent1 * directionComponent1 * totalDirectionMisalignment;
      if(maxSF == false)
      {
        if(F7 < maxF7) { F7 = maxF7; }
        else { maxF7 = F7; }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
namespace Detail
{
  namespace CubicHigh
  {
    class GenerateSphereCoordsImpl
    {
        FloatArrayType* m_Eulers;
        FloatArrayType* m_xyz001;
        FloatArrayType* m_xyz011;
        FloatArrayType* m_xyz111;

      public:
        GenerateSphereCoordsImpl(FloatArrayType* eulers, FloatArrayType* xyz001, FloatArrayType* xyz011, FloatArrayType* xyz111) :
          m_Eulers(eulers),
          m_xyz001(xyz001),
          m_xyz011(xyz011),
          m_xyz111(xyz111)
        {}
        virtual ~GenerateSphereCoordsImpl() {}

        void generate(size_t start, size_t end) const
        {
          float g[3][3];
          float gTranpose[3][3];
          float* currentEuler = NULL;
          float direction[3] = {0.0, 0.0, 0.0};

          for(size_t i = start; i < end; ++i)
          {
            currentEuler = m_Eulers->GetPointer(i * 3);
            if (i == 1154430)
            {
              std::cout << "GenerateSphereCoordsImpl:" << __LINE__ << std::endl;
            }
            OrientationMath::EulerToMat(currentEuler[0], currentEuler[1], currentEuler[2], g);
            MatrixMath::Transpose3x3(g, gTranpose);

            // -----------------------------------------------------------------------------
            // 001 Family
            direction[0] = 1.0;
            direction[1] = 0.0;
            direction[2] = 0.0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz001->GetPointer(i * 18));
            MatrixMath::Copy3x1(m_xyz001->GetPointer(i * 18), m_xyz001->GetPointer(i * 18 + 3));
            MatrixMath::Multiply3x1withConstant(m_xyz001->GetPointer(i * 18 + 3), -1);
            direction[0] = 0.0;
            direction[1] = 1.0;
            direction[2] = 0.0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz001->GetPointer(i * 18 + 6));
            MatrixMath::Copy3x1(m_xyz001->GetPointer(i * 18 + 6), m_xyz001->GetPointer(i * 18 + 9));
            MatrixMath::Multiply3x1withConstant(m_xyz001->GetPointer(i * 18 + 9), -1);
            direction[0] = 0.0;
            direction[1] = 0.0;
            direction[2] = 1.0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz001->GetPointer(i * 18 + 12));
            MatrixMath::Copy3x1(m_xyz001->GetPointer(i * 18 + 12), m_xyz001->GetPointer(i * 18 + 15));
            MatrixMath::Multiply3x1withConstant(m_xyz001->GetPointer(i * 18 + 15), -1);

            // -----------------------------------------------------------------------------
            // 011 Family
            direction[0] = DREAM3D::Constants::k_1OverRoot2;
            direction[1] = DREAM3D::Constants::k_1OverRoot2;
            direction[2] = 0.0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz011->GetPointer(i * 36));
            MatrixMath::Copy3x1(m_xyz011->GetPointer(i * 36), m_xyz011->GetPointer(i * 36 + 3));
            MatrixMath::Multiply3x1withConstant(m_xyz011->GetPointer(i * 36 + 3), -1);
            direction[0] = DREAM3D::Constants::k_1OverRoot2;
            direction[1] = 0.0;
            direction[2] = DREAM3D::Constants::k_1OverRoot2;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz011->GetPointer(i * 36 + 6));
            MatrixMath::Copy3x1(m_xyz011->GetPointer(i * 36 + 6), m_xyz011->GetPointer(i * 36 + 9));
            MatrixMath::Multiply3x1withConstant(m_xyz011->GetPointer(i * 36 + 9), -1);
            direction[0] = 0.0;
            direction[1] = DREAM3D::Constants::k_1OverRoot2;
            direction[2] = DREAM3D::Constants::k_1OverRoot2;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz011->GetPointer(i * 36 + 12));
            MatrixMath::Copy3x1(m_xyz011->GetPointer(i * 36 + 12), m_xyz011->GetPointer(i * 36 + 15));
            MatrixMath::Multiply3x1withConstant(m_xyz011->GetPointer(i * 36 + 15), -1);
            direction[0] = -DREAM3D::Constants::k_1OverRoot2;
            direction[1] = -DREAM3D::Constants::k_1OverRoot2;
            direction[2] = 0.0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz011->GetPointer(i * 36 + 18));
            MatrixMath::Copy3x1(m_xyz011->GetPointer(i * 36 + 18), m_xyz011->GetPointer(i * 36 + 21));
            MatrixMath::Multiply3x1withConstant(m_xyz011->GetPointer(i * 36 + 21), -1);
            direction[0] = -DREAM3D::Constants::k_1OverRoot2;
            direction[1] = 0.0;
            direction[2] = DREAM3D::Constants::k_1OverRoot2;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz011->GetPointer(i * 36 + 24));
            MatrixMath::Copy3x1(m_xyz011->GetPointer(i * 36 + 24), m_xyz011->GetPointer(i * 36 + 27));
            MatrixMath::Multiply3x1withConstant(m_xyz011->GetPointer(i * 36 + 27), -1);
            direction[0] = 0.0;
            direction[1] = -DREAM3D::Constants::k_1OverRoot2;
            direction[2] = DREAM3D::Constants::k_1OverRoot2;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz011->GetPointer(i * 36 + 30));
            MatrixMath::Copy3x1(m_xyz011->GetPointer(i * 36 + 30), m_xyz011->GetPointer(i * 36 + 33));
            MatrixMath::Multiply3x1withConstant(m_xyz011->GetPointer(i * 36 + 33), -1);

            // -----------------------------------------------------------------------------
            // 111 Family
            direction[0] = DREAM3D::Constants::k_1OverRoot3;
            direction[1] = DREAM3D::Constants::k_1OverRoot3;
            direction[2] = DREAM3D::Constants::k_1OverRoot3;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz111->GetPointer(i * 24));
            MatrixMath::Copy3x1(m_xyz111->GetPointer(i * 24), m_xyz111->GetPointer(i * 24 + 3));
            MatrixMath::Multiply3x1withConstant(m_xyz111->GetPointer(i * 24 + 3), -1);
            direction[0] = -DREAM3D::Constants::k_1OverRoot3;
            direction[1] = DREAM3D::Constants::k_1OverRoot3;
            direction[2] = DREAM3D::Constants::k_1OverRoot3;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz111->GetPointer(i * 24 + 6));
            MatrixMath::Copy3x1(m_xyz111->GetPointer(i * 24 + 6), m_xyz111->GetPointer(i * 24 + 9));
            MatrixMath::Multiply3x1withConstant(m_xyz111->GetPointer(i * 24 + 9), -1);
            direction[0] = DREAM3D::Constants::k_1OverRoot3;
            direction[1] = -DREAM3D::Constants::k_1OverRoot3;
            direction[2] = DREAM3D::Constants::k_1OverRoot3;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz111->GetPointer(i * 24 + 12));
            MatrixMath::Copy3x1(m_xyz111->GetPointer(i * 24 + 12), m_xyz111->GetPointer(i * 24 + 15));
            MatrixMath::Multiply3x1withConstant(m_xyz111->GetPointer(i * 24 + 15), -1);
            direction[0] = DREAM3D::Constants::k_1OverRoot3;
            direction[1] = DREAM3D::Constants::k_1OverRoot3;
            direction[2] = -DREAM3D::Constants::k_1OverRoot3;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz111->GetPointer(i * 24 + 18));
            MatrixMath::Copy3x1(m_xyz111->GetPointer(i * 24 + 18), m_xyz111->GetPointer(i * 24 + 21));
            MatrixMath::Multiply3x1withConstant(m_xyz111->GetPointer(i * 24 + 21), -1);
          }

        }

#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
        void operator()(const tbb::blocked_range<size_t>& r) const
        {
          generate(r.begin(), r.end());
        }
#endif
    };
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CubicOps::generateSphereCoordsFromEulers(FloatArrayType* eulers, FloatArrayType* xyz001, FloatArrayType* xyz011, FloatArrayType* xyz111)
{
  size_t nOrientations = eulers->GetNumberOfTuples();

  // Sanity Check the size of the arrays
  if (xyz001->GetNumberOfTuples() < nOrientations * Detail::CubicHigh::symSize0)
  {
    xyz001->Resize(nOrientations * Detail::CubicHigh::symSize0 * 3);
  }
  if (xyz011->GetNumberOfTuples() < nOrientations * Detail::CubicHigh::symSize1)
  {
    xyz011->Resize(nOrientations * Detail::CubicHigh::symSize1 * 3);
  }
  if (xyz111->GetNumberOfTuples() < nOrientations * Detail::CubicHigh::symSize2)
  {
    xyz111->Resize(nOrientations * Detail::CubicHigh::symSize2 * 3);
  }


#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
  tbb::task_scheduler_init init;
  bool doParallel = true;
#endif

#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
  if (doParallel == true)
  {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, nOrientations),
                      Detail::CubicHigh::GenerateSphereCoordsImpl(eulers, xyz001, xyz011, xyz111), tbb::auto_partitioner());
  }
  else
#endif
  {
    Detail::CubicHigh::GenerateSphereCoordsImpl serial(eulers, xyz001, xyz011, xyz111);
    serial.generate(0, nOrientations);
  }

#if 0
  float* a = xyz001->GetPointer(0);
  for(size_t i = 0; i < nOrientations * Detail::CubicHigh::symSize0 * 3; i++)
  {
    if(isnan(a[i]))
    {
      std::cout << "NAN (A)" << std::endl;
    }
  }

  float* b = xyz011->GetPointer(0);
  for(size_t i = 0; i < nOrientations * Detail::CubicHigh::symSize1 * 3; i++)
  {
    if(isnan(b[i]))
    {
      std::cout << "NAN (B)" << std::endl;
    }
  }



  float* c = xyz111->GetPointer(0);
    for(size_t i = 0; i < nOrientations * Detail::CubicHigh::symSize2 * 3; i++)
  {
    if(isnan(c[i]))
    {
      std::cout << "NAN (C)" << std::endl;
    }
  }
  #endif

}

/**
 * @brief Sorts the 3 values from low to high
 * @param a
 * @param b
 * @param c
 * @param sorted The array to store the sorted values.
 */
template<typename T>
void _TripletSort(T a, T b, T c, T* sorted)
{
  if ( a > b && a > c)
  {
    sorted[2] = a;
    if (b > c) { sorted[1] = b; sorted[0] = c; }
    else { sorted[1] = c; sorted[0] = b; }
  }
  else if ( b > a && b > c)
  {
    sorted[2] = b;
    if (a > c) { sorted[1] = a; sorted[0] = c; }
    else { sorted[1] = c; sorted[0] = a; }
  }
  else if ( a > b )
  {
    sorted[1] = a;
    sorted[0] = b;
    sorted[2] = c;
  }
  else if (a >= c && b >= c)
  {
    sorted[0] = c;
    sorted[1] = a;
    sorted[2] = b;
  }
  else
  { sorted[0] = a; sorted[1] = b; sorted[2] = c;}
}


/**
 * @brief Sorts the 3 values from low to high
 * @param a Input
 * @param b Input
 * @param c Input
 * @param x Output
 * @param y Output
 * @param z Output
 */
template<typename T>
void _TripletSort(T a, T b, T c, T& x, T& y, T& z)
{
  if ( a > b && a > c)
  {
    z = a;
    if (b > c) { y = b; x = c; }
    else { y = c; x = b; }
  }
  else if ( b > a && b > c)
  {
    z = b;
    if (a > c) { y = a; x = c; }
    else { y = c; x = a; }
  }
  else if ( a > b )
  {
    y = a;
    x = b;
    z = c;
  }
  else if (a >= c && b >= c)
  {
    x = c;
    y = a;
    z = b;
  }
  else
  { x = a; y = b; z = c;}
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool CubicOps::inUnitTriangle(float eta, float chi)
{
  float etaDeg = eta * DREAM3D::Constants::k_180OverPi;
  float chiMax;
  if(etaDeg > 45.0) { chiMax = sqrt(1.0 / (2.0 + tanf(0.5 * DREAM3D::Constants::k_Pi - eta) * tanf(0.5 * DREAM3D::Constants::k_Pi - eta))); }
  else { chiMax = sqrt(1.0 / (2.0 + tanf(eta) * tanf(eta))); }
  DREAM3DMath::boundF(chiMax, -1.0f, 1.0f);
  chiMax = acos(chiMax);
  if( eta < 0.0 || eta > (45.0 * DREAM3D::Constants::k_PiOver180) || chi < 0.0 || chi > chiMax ) { return false; }
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D::Rgb CubicOps::generateIPFColor(double* eulers, double* refDir, bool convertDegrees)
{
  return generateIPFColor(eulers[0], eulers[1], eulers[2], refDir[0], refDir[1], refDir[2], convertDegrees);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D::Rgb CubicOps::generateIPFColor(double phi1, double phi, double phi2, double refDir0, double refDir1, double refDir2, bool degToRad)
{
  if (degToRad == true)
  {
    phi1 = phi1 * DREAM3D::Constants::k_DegToRad;
    phi = phi * DREAM3D::Constants::k_DegToRad;
    phi2 = phi2 * DREAM3D::Constants::k_DegToRad;
  }

  QuatF qc;
  QuatF q1;
  float g[3][3];
  float p[3];
  float refDirection[3];
  float chi, eta;
  float _rgb[3] = { 0.0, 0.0, 0.0 };

  OrientationMath::EulertoQuat(q1, phi1, phi, phi2);

  for (int j = 0; j < 24; j++)
  {
    QuaternionMathF::Multiply(q1, CubicQuatSym[j], qc);

    OrientationMath::QuattoMat(qc, g);

    refDirection[0] = refDir0;
    refDirection[1] = refDir1;
    refDirection[2] = refDir2;
    MatrixMath::Multiply3x3with3x1(g, refDirection, p);
    MatrixMath::Normalize3x1(p);

    if(getHasInversion() == false && p[2] < 0) { continue; }
    else if(getHasInversion() == true && p[2] < 0) { p[0] = -p[0], p[1] = -p[1], p[2] = -p[2]; }
    chi = acos(p[2]);
    eta = atan2(p[1], p[0]);
    if(inUnitTriangle(eta, chi) == false) { continue; }
    else {break;}
  }

  float etaMin = 0.0;
  float etaMax = 45.0;
  float etaDeg = eta * DREAM3D::Constants::k_180OverPi;
  float chiMax;
  if(etaDeg > 45.0) { chiMax = sqrt(1.0 / (2.0 + tanf(0.5 * DREAM3D::Constants::k_Pi - eta) * tanf(0.5 * DREAM3D::Constants::k_Pi - eta))); }
  else { chiMax = sqrt(1.0 / (2.0 + tanf(eta) * tanf(eta))); }
  DREAM3DMath::boundF(chiMax, -1.0f, 1.0f);
  chiMax = acos(chiMax);

  _rgb[0] = 1.0 - chi / chiMax;
  _rgb[2] = fabs(etaDeg - etaMin) / (etaMax - etaMin);
  _rgb[1] = 1 - _rgb[2];
  _rgb[1] *= chi / chiMax;
  _rgb[2] *= chi / chiMax;
  _rgb[0] = sqrt(_rgb[0]);
  _rgb[1] = sqrt(_rgb[1]);
  _rgb[2] = sqrt(_rgb[2]);

  float max = _rgb[0];
  if (_rgb[1] > max) { max = _rgb[1]; }
  if (_rgb[2] > max) { max = _rgb[2]; }

  _rgb[0] = _rgb[0] / max;
  _rgb[1] = _rgb[1] / max;
  _rgb[2] = _rgb[2] / max;


  return RgbColor::dRgb(_rgb[0] * 255, _rgb[1] * 255, _rgb[2] * 255, 255);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D::Rgb CubicOps::generateRodriguesColor(float r1, float r2, float r3)
{
  float range1 = 2.0f * CubicDim1InitValue;
  float range2 = 2.0f * CubicDim2InitValue;
  float range3 = 2.0f * CubicDim3InitValue;
  float max1 = range1 / 2.0f;
  float max2 = range2 / 2.0f;
  float max3 = range3 / 2.0f;
  float red = (r1 + max1) / range1;
  float green = (r2 + max2) / range2;
  float blue = (r3 + max3) / range3;

  return RgbColor::dRgb(red * 255, green * 255, blue * 255, 255);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<UInt8ArrayType::Pointer> CubicOps::generatePoleFigure(PoleFigureConfiguration_t& config)
{
  std::vector<UInt8ArrayType::Pointer> poleFigures;
  std::string label0("<001>");
  std::string label1("<011>");
  std::string label2("<111>");


  int numOrientations = config.eulers->GetNumberOfTuples();

  // Create an Array to hold the XYZ Coordinates which are the coords on the sphere.
  // this is size for CUBIC ONLY, <001> Family
  FloatArrayType::Pointer xyz001 = FloatArrayType::CreateArray(numOrientations * Detail::CubicHigh::symSize0, 3, label0 + std::string("xyzCoords"));
  // this is size for CUBIC ONLY, <011> Family
  FloatArrayType::Pointer xyz011 = FloatArrayType::CreateArray(numOrientations * Detail::CubicHigh::symSize1, 3, label1 + std::string("xyzCoords"));
  // this is size for CUBIC ONLY, <111> Family
  FloatArrayType::Pointer xyz111 = FloatArrayType::CreateArray(numOrientations * Detail::CubicHigh::symSize2, 3, label2 + std::string("xyzCoords"));

  config.sphereRadius = 1.0f;

  // Generate the coords on the sphere **** Parallelized
  generateSphereCoordsFromEulers(config.eulers, xyz001.get(), xyz011.get(), xyz111.get());


  // These arrays hold the "intensity" images which eventually get converted to an actual Color RGB image
  // Generate the modified Lambert projection images (Squares, 2 of them, 1 for northern hemisphere, 1 for southern hemisphere
  DoubleArrayType::Pointer intensity001 = DoubleArrayType::CreateArray(config.imageDim * config.imageDim, 1, label0 + "_Intensity_Image");
  DoubleArrayType::Pointer intensity011 = DoubleArrayType::CreateArray(config.imageDim * config.imageDim, 1, label1 + "_Intensity_Image");
  DoubleArrayType::Pointer intensity111 = DoubleArrayType::CreateArray(config.imageDim * config.imageDim, 1, label2 + "_Intensity_Image");
#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
  tbb::task_scheduler_init init;
  bool doParallel = true;
  tbb::task_group* g = new tbb::task_group;

  if(doParallel == true)
  {
    g->run(GenerateIntensityMapImpl(xyz001.get(), &config, intensity001.get()));
    g->run(GenerateIntensityMapImpl(xyz011.get(), &config, intensity011.get()));
    g->run(GenerateIntensityMapImpl(xyz111.get(), &config, intensity111.get()));
    g->wait(); // Wait for all the threads to complete before moving on.
    delete g;
    g = NULL;
  }
  else
#endif
  {
    GenerateIntensityMapImpl m001(xyz001.get(), &config, intensity001.get());
    m001();
    GenerateIntensityMapImpl m011(xyz011.get(), &config, intensity011.get());
    m011();
    GenerateIntensityMapImpl m111(xyz111.get(), &config, intensity111.get());
    m111();
  }

  // Find the Max and Min values based on ALL 3 arrays so we can color scale them all the same
  double max = std::numeric_limits<double>::min();
  double min = std::numeric_limits<double>::max();

  double* dPtr = intensity001->GetPointer(0);
  size_t count = intensity001->GetNumberOfTuples();
  for(size_t i = 0; i < count; ++i)
  {
    if (dPtr[i] > max) { max = dPtr[i]; }
    if (dPtr[i] < min) { min = dPtr[i]; }
  }


  dPtr = intensity011->GetPointer(0);
  count = intensity011->GetNumberOfTuples();
  for(size_t i = 0; i < count; ++i)
  {
    if (dPtr[i] > max) { max = dPtr[i]; }
    if (dPtr[i] < min) { min = dPtr[i]; }
  }

  dPtr = intensity111->GetPointer(0);
  count = intensity111->GetNumberOfTuples();
  for(size_t i = 0; i < count; ++i)
  {
    if (dPtr[i] > max) { max = dPtr[i]; }
    if (dPtr[i] < min) { min = dPtr[i]; }
  }

  config.minScale = min;
  config.maxScale = max;

  UInt8ArrayType::Pointer image001 = UInt8ArrayType::CreateArray(config.imageDim * config.imageDim, 4, label0);
  UInt8ArrayType::Pointer image011 = UInt8ArrayType::CreateArray(config.imageDim * config.imageDim, 4, label1);
  UInt8ArrayType::Pointer image111 = UInt8ArrayType::CreateArray(config.imageDim * config.imageDim, 4, label2);


  poleFigures.push_back(image001);
  poleFigures.push_back(image011);
  poleFigures.push_back(image111);
#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
  g = new tbb::task_group;

  if(doParallel == true)
  {
    g->run(GenerateRgbaImageImpl(intensity001.get(), &config, image001.get()));
    g->run(GenerateRgbaImageImpl(intensity011.get(), &config, image011.get()));
    g->run(GenerateRgbaImageImpl(intensity111.get(), &config, image111.get()));
    g->wait(); // Wait for all the threads to complete before moving on.
    delete g;
    g = NULL;
  }
  else
#endif
  {
    GenerateRgbaImageImpl m001(intensity001.get(), &config, image001.get());
    m001();
    GenerateRgbaImageImpl m011(intensity011.get(), &config, image011.get());
    m011();
    GenerateRgbaImageImpl m111(intensity111.get(), &config, image111.get());
    m111();
  }


#if 0
  size_t dims[3] = {config.imageDim, config.imageDim, 1};
  float res[3] = {1.0, 1.0, 1.0};
  VtkRectilinearGridWriter::WriteDataArrayToFile("/tmp/" + intensity001->GetName() + ".vtk",
                                                 intensity001.get(), dims, res, "double", true );
  VtkRectilinearGridWriter::WriteDataArrayToFile("/tmp/" + intensity011->GetName() + ".vtk",
                                                 intensity011.get(), dims, res, "double", true );
  VtkRectilinearGridWriter::WriteDataArrayToFile("/tmp/" + intensity111->GetName() + ".vtk",
                                                 intensity111.get(), dims, res, "double", true );
#endif
  return poleFigures;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UInt8ArrayType::Pointer CubicOps::generateIPFTriangleLegend(int imageDim)
{

  UInt8ArrayType::Pointer image = UInt8ArrayType::CreateArray(imageDim * imageDim, 4, "Cubic High IPF Triangle Legend");
  uint32_t* pixelPtr = reinterpret_cast<uint32_t*>(image->GetPointer(0));

  float indexConst1 = 0.414 / imageDim;
  float indexConst2 = 0.207 / imageDim;
  float temp = 0.0f;
  float red1 = 0.0f;
  float green1 = 0.0f;
  float blue1 = 0.0f;
  float red2 = 0.0f;
  float green2 = 0.0f;
  float blue2 = 0.0f;
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float a = 0.0f;
  float b = 0.0f;
  float c = 0.0f;
  float check1 = 0.0f;
  float check2 = 0.0f;
  float val = 0.0f;
  float x1 = 0.0f;
  float y1 = 0.0f;
  float z1 = 0.0f;
  float denom = 0.0f;
  float phi = 0.0f;
  float x1alt = 0.0f;
  float theta = 0.0f;
  float k_RootOfHalf = sqrt(0.5);
  float cd[3];


  DREAM3D::Rgb color;
  size_t idx = 0;
  size_t yScanLineIndex = imageDim; // We use this to control where teh data is drawn. Otherwise the image will come out flipped vertically
  // Loop over every pixel in the image and project up to the sphere to get the angle and then figure out the RGB from
  // there.
  for (size_t yIndex = 0; yIndex < imageDim; ++yIndex)
  {
    yScanLineIndex--;
    for (size_t xIndex = 0; xIndex < imageDim; ++xIndex)
    {
      idx = (imageDim * yScanLineIndex) + xIndex;
      temp = 0;
      red1 = 0;
      green1 = 0;
      blue1 = 0;
      red2 = 0;
      green2 = 0;
      blue2 = 0;
      x = xIndex * indexConst1 + indexConst2;
      y = yIndex * indexConst1 + indexConst2;
      z = -1.0;
      a = (x * x + y * y + 1);
      b = (2 * x * x + 2 * y * y);
      c = (x * x + y * y - 1);
      check1 = b * b;
      check2 = 4 * a * c;
      val = (-b + sqrtf(b * b - 4.0 * a * c)) / (2.0 * a);
      x1 = (1 + val) * x;
      y1 = (1 + val) * y;
      z1 = val;
      denom = (x1 * x1) + (y1 * y1) + (z1 * z1);
      denom = sqrtf(denom);
      x1 = x1 / denom;
      y1 = y1 / denom;
      z1 = z1 / denom;

      red1 = x1 * (-k_RootOfHalf) + z1 * k_RootOfHalf;
      phi = acos(red1);
      x1alt = x1 / k_RootOfHalf;
      x1alt = x1alt / sqrt((x1alt * x1alt) + (y1 * y1));
      theta = acos(x1alt);

      if (phi < (45 * DREAM3D::Constants::k_PiOver180) ||
          phi > (90 * DREAM3D::Constants::k_PiOver180) ||
          theta > (35.26 * DREAM3D::Constants::k_PiOver180))
      {
        color = 0xFFFFFFFF;
      }
      else
      {
        //3) move that direction to a single standard triangle - using the 001-011-111 triangle)
        cd[0] = fabs(x1);
        cd[1] = fabs(y1);
        cd[2] = fabs(z1);

        // Sort the cd array from smallest to largest
        _TripletSort(cd[0], cd[1], cd[2], cd);

        color = generateIPFColor(0.0, 0.0, 0.0, cd[0], cd[1], cd[2], false);
      }
      pixelPtr[idx] = color;
    }
  }
  return image;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D::Rgb CubicOps::generateMisorientationColor(const QuatF& q, const QuatF& refFrame)
{
  float n1, n2, n3, w;
  float x, x1, x2, x3, x4, x5, x6, x7;
  float y, y1, y2, y3, y4, y5, y6, y7;
  float z, z1, z2, z3, z4, z5, z6, z7;
  float k, h, s, v;

  QuatF q1, q2;
  QuaternionMathF::Copy(q, q1);
  QuaternionMathF::Copy(refFrame, q2);

  //get disorientation
  w = getMisoQuat(q1, q2, n1, n2, n3);
  n1 = fabs(n1);
  n2 = fabs(n2);
  n3 = fabs(n3);

  _TripletSort(n1, n2, n3, z, y, x);

  //eq c9.1
  k = tan(w / 2.0f);
  x = x * k;
  y = y * k;
  z = z * k;

  //eq c9.2
  x1 = x;
  y1 = y;
  z1 = z;
  if(x >= DREAM3D::Constants::k_1Over3 && atan2(z, y) >= ((1.0f - 2.0f * x) / x))
  {
    y1 = (x * (y + z)) / (1.0f - x);
    z1 = (x * z * (y + z)) / (y * (1.0f - x));
  }

  //eq c9.3
  x2 = x1 - DREAM3D::Constants::k_Tan_OneEigthPi;
  y2 = y1 * DREAM3D::Constants::k_Cos_ThreeEightPi - z1 * DREAM3D::Constants::k_Sin_ThreeEightPi;
  z2 = y1 * DREAM3D::Constants::k_Sin_ThreeEightPi + z1 * DREAM3D::Constants::k_Cos_ThreeEightPi;

  //eq c9.4
  x3 = x2;
  y3 = y2 * (1 + (y2 / z2) * DREAM3D::Constants::k_Tan_OneEigthPi);
  z3 = z2 + y2 * DREAM3D::Constants::k_Tan_OneEigthPi;

  //eq c9.5
  x4 = x3;
  y4 = (y3 * DREAM3D::Constants::k_Cos_OneEigthPi) / DREAM3D::Constants::k_Tan_OneEigthPi;
  z4 = z3 - x3 / DREAM3D::Constants::k_Cos_OneEigthPi;

  //eq c9.6
  k = atan2(-x4, y4);
  x5 = x4 * (sin(k) + fabs(cos(k)));
  y5 = y4 * (sin(k) + fabs(cos(k)));
  z5 = z4;

  //eq c9.7
  k = atan2(-x5, y5);
  x6 = -sqrt(x5 * x5 + y5 * y5) * sin(2.0f * k);
  y6 = sqrt(x5 * x5 + y5 * y5) * cos(2.0f * k);
  z6 = z5;

  //eq c9.8 these hsv are from 0 to 1 in cartesian coordinates
  x7 = (x6 * DREAM3D::Constants::k_Sqrt3 - y6) / (2.0f * DREAM3D::Constants::k_Tan_OneEigthPi);
  y7 = (x6 + y6 * DREAM3D::Constants::k_Sqrt3) / (2.0f * DREAM3D::Constants::k_Tan_OneEigthPi);
  z7 = z6 * (DREAM3D::Constants::k_Cos_OneEigthPi / DREAM3D::Constants::k_Tan_OneEigthPi);

  //convert to traditional hsv (0-1)
  h = fmod(atan2f(y7, x7) + M_2PI, M_2PI) / M_2PI;
  s = sqrt(x7 * x7 + y7 * y7);
  v = z7;
  if(v > 0)
  {
    s = s / v;
  }

  DREAM3D::Rgb rgb = ColorUtilities::convertHSVtoRgb(h, s, v);

  //now standard 0-255 rgb, needs rotation
  return RgbColor::dRgb(255 - RgbColor::dGreen(rgb), RgbColor::dBlue(rgb), RgbColor::dRed(rgb), 0);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UInt8ArrayType::Pointer CubicOps::generateMisorientationTriangleLegend(float angle, int n1, int n2, int imageDim)
{

  BOOST_ASSERT(false);

  UInt8ArrayType::Pointer image = UInt8ArrayType::CreateArray(imageDim * imageDim, 4, "Cubic High Misorientation Triangle Legend");
  uint32_t* pixelPtr = reinterpret_cast<uint32_t*>(image->GetPointer(0));

  double maxk = sqrt(2.0) - 1;
  double maxdeg = 2 * atan(sqrt(6.0 * maxk * maxk - 4.0 * maxk + 1));
  double deg1 = 2 * atan(sqrt(2.0 * maxk * maxk));

  double A = angle * M_PI / 360;
  std::vector<double> B;
  std::vector< std::vector<double> > C;

  ///Generate regularly spaced array of points in misorientation space
  if(A <= M_PI / 8)
  {
    double theta1 = atan(1 / sin(M_PI_4));
    double theta2 = M_PI_2;
    B = DREAM3DMath::linspace(theta1, theta2, n2);
    for(int i = 0; i < n2; i++)
    {
      C.push_back(DREAM3DMath::linspace(asin(1 / tan(B[i])), M_PI_4, n1));
    }
  }
  else if(A > M_PI / 8 && A <= M_PI / 6)
  {
    double theta1 = atan(1 / sin(M_PI_4));
    double theta2 = acos(-(maxk * (1.0 / tan(A))) * (maxk * (1.0 / tan(A)))) / 2;
    double theta3 = M_PI_2;
    int frac1 = floor((n2 - 3) * (theta2 - theta1) / (theta3 - theta1));
    int frac2 = (n2 - 3) - frac1;
    std::vector<double> temptheta1 = DREAM3DMath::linspace(theta1, theta2, frac1 + 2);
    std::vector<double> temptheta2 = DREAM3DMath::linspace(theta2, theta3, frac2 + 2);
    temptheta2.erase(temptheta2.begin());
    B.insert(B.end(), temptheta1.begin(), temptheta1.end());
    B.insert(B.end(), temptheta2.begin(), temptheta2.end());
    for(int i = 0; i < n2; i++)
    {
      double theta = B[i];
      if(theta >= theta1 && theta <= theta2)
      {
        C.push_back(DREAM3DMath::linspace(asin(1 / tan(theta)), M_PI_4, n1));
      }
      else
      {
        C.push_back(DREAM3DMath::linspace(acos(maxk / (tan(A)*sin(theta))), M_PI_4, n1));
      }
    }
  }
  else if(A > M_PI / 6 && A <= deg1 / 2)
  {
    std::vector<double> thetaSort;
    double thetaa = acos((1 - sqrt(6 * tan(A) * tan(A) - 2)) / (3 * tan(A)));
    thetaSort.push_back(thetaa);
    thetaSort.push_back(M_PI_2);
    thetaSort.push_back(acos((2 - sqrt(6 * tan(A)*tan(A) - 2)) / (6 * tan(A))));
    double thetad = (acos(-(maxk / tan(A)) * (maxk / tan(A)))) / 2;
    thetaSort.push_back(thetad);
    std::sort(thetaSort.begin(), thetaSort.end());
    double theta1 = thetaSort[0];
    double theta2 = thetaSort[1];
    double theta3 = thetaSort[2];
    double theta4 = thetaSort[3];
    int frac1 = (floor((n2 - 4) * (theta2 - theta1) / (theta4 - theta1)));
    int frac2 = (floor((n2 - 4) * (theta3 - theta2) / (theta4 - theta1)));
    int frac3 = n2 - 4 - frac1 - frac2;
    std::vector<double> temptheta3 = DREAM3DMath::linspace(theta1, theta2, (frac1 + 2));
    std::vector<double> temptheta4 = DREAM3DMath::linspace(theta2, theta3, (frac2 + 2));
    temptheta4.erase(temptheta4.begin());
    std::vector<double>temptheta5 = DREAM3DMath::linspace(theta3, theta4, (frac3 + 2));
    temptheta5.erase(temptheta5.begin());
    B.insert(B.end(), temptheta3.begin(), temptheta3.end());
    B.insert(B.end(), temptheta4.begin(), temptheta4.end());
    B.insert(B.end(), temptheta5.begin(), temptheta5.end());

    for(int i = 0; i < n2; i++)
    {
      double theta = B[i];
      double phi1, phi2;
      if(thetaa <= thetad)
      {
        if(theta <= theta2)
        {
          phi1 = asin(1 / tan(theta));
          double k = (1.0 - tan(A) * cos(theta)) / (sqrt(2.0) * (tan(A) * sin(theta)));
          if(k > 1) { k = 1; }
          if(k < -1) { k = -1; }
          phi2 = asin(k) - M_PI_4;
        }
        else if(theta > theta2 && theta < theta3)
        {
          phi1 = acos((sqrt(2.0) - 1.0) / (tan(A) * sin(theta)));
          phi2 = M_PI_4;
        }
      }
      else
      {
        if(theta <= theta2)
        {
          phi1 = asin(1 / tan(theta));
        }
        else if(theta > theta2 && theta <= theta3)
        {
          phi1 = acos(maxk / (tan(A) * sin(theta)));
        }

        double d3 = tan(A) * cos(theta);
        double k = ((1 - d3) + sqrt(2 * (tan(A) * tan(A) - d3 * d3) - (1 - d3) * (1 - d3))) / (2 * tan(A) * sin(theta));
        if(k > 1) { k = 1; }
        if(k < -1) { k = -1; }
        phi2 = acos(k);
      }
      if(theta > theta3)
      {
        phi1 = acos(maxk / (tan(A) * sin(theta)));
        phi2 = M_PI_4;
      }
      C.push_back(DREAM3DMath::linspace(phi1, phi2, n1));
    }
  }
  else if(A >= deg1 / 2 && A <= maxdeg / 2)
  {
    double theta1 = acos(((1 - maxk) - sqrt(2 * (tan(A) * tan(A) - maxk * maxk) - (1 - maxk) * (1 - maxk))) / (2 * tan(A)));
    double theta2 = acos((1 - sqrt(6 * (tan(A) * tan(A)) - 2)) / (3 * tan(A)));
    double theta3 = acos((sqrt(tan(A) * tan(A) - 2 * (maxk * maxk))) / (tan(A)));
    int frac1 = (floor((n2 - 3) * (theta2 - theta1) / (theta3 - theta1)));
    int frac2 = (n2 - 3) - frac1;
    std::vector<double> temptheta1 = DREAM3DMath::linspace(theta1, theta2, (frac1 + 2));
    std::vector<double> temptheta2 = DREAM3DMath::linspace(theta2, theta3, (frac2 + 2));
    temptheta2.erase(temptheta2.begin());
    B.insert(B.end(), temptheta1.begin(), temptheta1.end());
    B.insert(B.end(), temptheta2.begin(), temptheta2.end());
    for(int i = 0; i < n2; i++)
    {
      double theta = B[i];
      double phi1, phi2;
      phi1 = acos(maxk / (tan(A) * sin(theta)));
      if(theta >= theta1 && theta < theta2)
      {
        double d3 = tan(A) * cos(theta);
        double k = ((1 - d3) + sqrt(2 * (tan(A) * tan(A) - d3 * d3) - (1 - d3) * (1 - d3))) / (2 * tan(A) * sin(theta));
        phi2 = acos(k);
      }
      else
      {
        phi2 = M_PI_4;
      }
      C.push_back(DREAM3DMath::linspace(phi1, phi2, n1));
    }
  }

  ///create image, fill with empty pixels, setup painter
  int width = 1000;
  double scale = width / tan(M_PI / 8);
  int height = ceil(0.349159 * scale);
  /*
  QPainter painter;
  image = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
  image.fill(0x00000000);
  painter.begin(&image);
  painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
  */

  ///draw standard sterographic triangle border (dashed)
  /*
  QPen pen;
  pen.setColor(Qt::black);
  pen.setWidth(2);
  pen.setStyle(Qt::DotLine);
  painter.setPen(pen);
  */
  double r = tan(A);
  std::vector<double> x, y, z;
  y = DREAM3DMath::linspace(0, r / sqrt(2.0), 100);
  for(int i = 0; i < y.size(); i++)
  {
    double k = r * r - y[i] * y[i];
    if(k < 0) { k = 0; }
    x.push_back(sqrt(k));
    z.push_back(0);
  }
  std::vector< std::pair<double, double> > ptsa = rodri2pair(x, y, z);
  //std::vector< std::pair<int, int> > pointlist=scalePoints(ptsa, scale);
  //std::vector<QPoint> qpointlist = pairToQPoint(pointlist);
  //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

  x.clear();
  y.clear();
  z.clear();
  x = DREAM3DMath::linspace(r / sqrt(3.0), r, 100);
  for(int i = 0; i < x.size(); i++)
  {
    double k = r * r - x[i] * x[i];
    if(k < 0) { k = 0; }
    y.push_back(sqrt((k) / 2));
    z.push_back(y[i]);
  }
  ptsa = rodri2pair(x, y, z);
  //pointlist=scalePoints(ptsa, scale);
  //qpointlist = pairToQPoint(pointlist);
  //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

  x.clear();
  y.clear();
  z.clear();
  x = DREAM3DMath::linspace(r / sqrt(3.0), r / sqrt(2.0), 100);
  for(int i = 0; i < x.size(); i++)
  {
    y.push_back(x[i]);
    double k = r * r - 2 * x[i] * x[i];
    if(k < 0) { k = 0; }
    z.push_back(sqrt(k));
  }
  ptsa = rodri2pair(x, y, z);
  //pointlist=scalePoints(ptsa, scale);
  //qpointlist = pairToQPoint(pointlist);
  //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

  ///find triangle vertices by converting misorientation space grid to pixels
  std::vector< std::pair<double, double> > ba;
  std::vector<double> d0, d1, d2, d3;
  int k = 0;
  for(int i = 0; i < n1; i++)
  {
    for(int j = 0; j < n2; j++)
    {
      d3.push_back(tan(A)*cos(B[j]));
      d2.push_back(tan(A)*sin(B[j])*sin(C[j][i]));
      d1.push_back(tan(A)*sin(B[j])*cos(C[j][i]));
      //double d = 1 - d1[k] * d1[k] - d2[k] * d2[k] - d3[k] * d3[k];
      k++;
    }
  }

  ///add corner vertices if the sst isn't filled
  if(A > M_PI / 8)
  {
    d1.push_back(0);
    d2.push_back(0);
    d3.push_back(0);

    if(A > M_PI / 8 && A <= M_PI / 6)
    {
    }
    else
    {
      d1.push_back(r / sqrt(3.0));
      d2.push_back(r / sqrt(3.0));
      d3.push_back(r / sqrt(3.0));
    }
  }
  ba = rodri2pair(d1, d2, d3);

  ///find triangles
  std::vector< std::pair<int, int> > scaledba;//=scalePoints(ba, scale);
  //std::vector<QPoint> qpointba = pairToQPoint(scaledba);


  std::vector< std::vector<int> > triList;
  /*
  tpp::Delaunay::Point tempP;
  vector<tpp::Delaunay::Point> v;
  for(int i=0; i<ba.size(); i++)
  {
      tempP[0]=scaledba[i].first;
      tempP[1]=scaledba[i].second;
      v.push_back(tempP);
  }
  tpp::Delaunay delobject(v);
  delobject.Triangulate();

  for(tpp::Delaunay::fIterator fit = delobject.fbegin(); fit != delobject.fend(); ++fit)
  {
      std::vector<int> triangle;
      triangle.push_back(delobject.Org(fit));
      triangle.push_back(delobject.Dest(fit));
      triangle.push_back(delobject.Apex(fit));
      triList.push_back(triangle);
  }
  */

  ///fill triangles
  for(int i = 0; i < ba.size(); i++)
  {
    QuatF quat, refQuat;
    refQuat.x = 0;
    refQuat.y = 0;
    refQuat.z = 0;
    refQuat.w = 1;
    //have rodrigues vector, need quat
    float tanAng = sqrt(d1[i] * d1[i] + d2[i] * d2[i] + d3[i] * d3[i]);
    float cosAng = cosf(atanf(tanAng));
    quat.x = d1[i] * cosAng * tanAng;
    quat.y = d2[i] * cosAng * tanAng;
    quat.z = d3[i] * cosAng * tanAng;
    quat.w = cosAng;
    DREAM3D::Rgb pix = generateMisorientationColor(quat, refQuat);
    //image.setPixel(qpointba[i].x(), qpointba[i].y(), pix);
  }

  std::pair<int, int> vert1, vert2, vert3;
  std::vector<int> triangle;

  for(int k = 0; k < triList.size(); k++)
  {
    triangle = triList[k];
    vert1 = scaledba[triangle[0]];
    vert2 = scaledba[triangle[1]];
    vert3 = scaledba[triangle[2]];

    //check that none of verticies are special spots
    bool color = true;
    if(A > M_PI / 8)
    {
      if(A > M_PI / 8 && A <= M_PI / 6)
      {
        //1 extra point at end
        if(triangle[0] == ba.size() - 1) { color = false; }
        if(triangle[1] == ba.size() - 1) { color = false; }
        if(triangle[2] == ba.size() - 1) { color = false; }
      }
      else
      {
        //2 extra points at end
        if(triangle[0] == ba.size() - 1) { color = false; }
        if(triangle[1] == ba.size() - 1) { color = false; }
        if(triangle[2] == ba.size() - 1) { color = false; }
        if(triangle[0] == ba.size() - 2) { color = false; }
        if(triangle[1] == ba.size() - 2) { color = false; }
        if(triangle[2] == ba.size() - 2) { color = false; }
      }
    }

    if(color)
    {
      double x1, x2, x3, y1, y2, y3;
      int r1, r2, r3, g1, g2, g3, b1, b2, b3;
      x1 = vert1.first;
      x2 = vert2.first;
      x3 = vert3.first;
      y1 = vert1.second;
      y2 = vert2.second;
      y3 = vert3.second;

      //find rectangle bounding triangle
      int xMin, xMax, yMin, yMax;
      xMin = std::min(std::min(x1, x2), x3);
      xMax = std::max(std::max(x1, x2), x3);
      yMin = std::min(std::min(y1, y2), y3);
      yMax = std::max(std::max(y1, y2), y3);

      //get colors of vertices
      /*
      QRgb pixval1=image.pixel(x1, y1);
      QRgb pixval2=image.pixel(x2, y2);
      QRgb pixval3=image.pixel(x3, y3);
      r1 = qRed(pixval1);
      r2 = qRed(pixval2);
      r3 = qRed(pixval3);
      g1 = qGreen(pixval1);
      g2 = qGreen(pixval2);
      g3 = qGreen(pixval3);
      b1 = qBlue(pixval1);
      b2 = qBlue(pixval2);
      b3 = qBlue(pixval3);
      */

      //loop over pixels in rectangle
      for(int i = xMin; i <= xMax; i++)
      {
        for(int j = yMin; j <= yMax; j++)
        {
          //determine barycentric coordinates
          double gamma1, gamma2, gamma3;
          double det = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
          gamma1 = ((y2 - y3) * ((double)i - x3) + (x3 - x2) * ((double)j - y3)) / det;
          gamma2 = ((y3 - y1) * ((double)i - x3) + (x1 - x3) * ((double)j - y3)) / det;
          gamma3 = 1.0 - gamma1 - gamma2;

          //check if pixel is inside triangle
          double minval = -0.0000000000000000001;//misses some boundary points for >=0
          if(gamma1 >= minval && gamma2 >= minval && gamma3 >= minval && gamma1 <= 1.0 && gamma2 <= 1.0 && gamma3 <= 1.0 )
          {
            if(gamma1 == 1.0 || gamma2 == 1.0 || gamma3 == 1.0)
            {
              //vertex
            }
            else
            {
              //edge or insdie
              uint8_t red = r1 * gamma1 + r2 * gamma2 + r3 * gamma3;
              uint8_t green = g1 * gamma1 + g2 * gamma2 + g3 * gamma3;
              uint8_t blue = b1 * gamma1 + b2 * gamma2 + b3 * gamma3;
              uint8_t alpha = 255;
              unsigned int pix = (alpha << 24) | (red << 16) | (green << 8) | blue;
              //image.setPixel(i, j, pix);
            }
          }
          else
          {
            //outside triangle
          }
        }
      }
    }
  }

  ///Draw Solid Border
  //pen.setStyle(Qt::SolidLine);
  //painter.setPen(pen);

  if(A <= M_PI / 8)
  {
    std::vector<double> x, y, z;
    y = DREAM3DMath::linspace(0.0f, r / sqrt(2.0f), 100);
    for(int i = 0; i < y.size(); i++)
    {
      z.push_back(0);
      double k = r * r - y[i] * y[i];
      if(k < 0) { k = 0; }
      x.push_back(sqrt(k));
    }
    ptsa = rodri2pair(x, y, z);
    //std::vector< std::pair<int, int> >pointlist=scalePoints(ptsa, scale);
    //std::vector<QPoint> qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    x = DREAM3DMath::linspace(r / sqrt(3.0), r, 100);
    for(int i = 0; i < x.size(); i++)
    {
      double k = (r * r - x[i] * x[i]) / 2;
      if(k < 0) { k = 0; }
      y.push_back(sqrt(k));
      z.push_back(y[i]);
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    x = DREAM3DMath::linspace(r / sqrt(3.0), r / sqrt(2.0), 100);
    for(int i = 0; i < x.size(); i++)
    {
      y.push_back(x[i]);
      double k = r * r - 2 * x[i] * x[i];
      if(k < 0) { k = 0; }
      z.push_back(sqrt(k));
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));
  }

  else if(A > M_PI / 8 && A <= M_PI / 6)
  {
    std::vector<double> x, y, z;
    double theta1 = atan(1 / sin(M_PI_4));
    double theta2 = acos(-(maxk * maxk) / (tan(A) * tan(A))) / 2;
    double theta3 = M_PI_2;
    double phi3 = acos(maxk / (tan(A) * sin(theta3)));

    y = DREAM3DMath::linspace(r * sin(phi3), r / (sqrt(2.0)), 100);
    for(int i = 0; i < y.size(); i++)
    {
      x.push_back(sqrt(r * r - y[i]*y[i]));
      z.push_back(0);
    }
    ptsa = rodri2pair(x, y, z);
    //std::vector< std::pair<int, int> >pointlist=scalePoints(ptsa, scale);
    //std::vector<QPoint> qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    z = DREAM3DMath::linspace(r * cos(theta1), r * cos(theta2), 100);
    for(int i = 0; i < z.size(); i++)
    {
      y.push_back(z[i]);
      x.push_back(sqrt(r * r - 2 * z[i]*z[i]));
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    z = DREAM3DMath::linspace(r * cos(theta1), r * cos(theta3), 100);
    for(int i = 0; i < z.size(); i++)
    {
      x.push_back(sqrt((r * r - z[i]*z[i]) / 2));
      y.push_back(x[i]);
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    z = DREAM3DMath::linspace(r * cos(theta2), r * cos(theta3), 100);
    for(int i = 0; i < z.size(); i++)
    {
      x.push_back(maxk);
      y.push_back(sqrt(r * r - maxk * maxk - z[i]*z[i]));
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

  }
  else if(A > M_PI / 6.0 && A <= deg1 / 2.0)
  {
    std::vector<double> x, y, z;
    double thetac = acos((2.0 - sqrt(6.0 * (tan(A) * tan(A)) - 2.0)) / (6.0 * tan(A)));
    double thetaa = acos((1.0 - sqrt(6.0 * (tan(A) * tan(A)) - 2.0)) / (3.0 * tan(A)));
    double thetad = acos(-(maxk * maxk) / (tan(A) * tan(A))) / 2.0;
    double thetab = M_PI_2;
    double phi3 = acos(maxk / (tan(A) * sin(thetab)));

    y = DREAM3DMath::linspace(r * sin(phi3), r / (sqrt(2.0)), 100.0);
    for(int i = 0; i < y.size(); i++)
    {
      z.push_back(0.0);
      x.push_back(sqrt(r * r - y[i]*y[i]));
    }
    ptsa = rodri2pair(x, y, z);
    //std::vector< std::pair<int, int> >pointlist=scalePoints(ptsa, scale);
    //std::vector<QPoint> qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    z = DREAM3DMath::linspace(r * cos(thetac), r * cos(thetad), 100);
    for(int i = 0; i < z.size(); i++)
    {
      y.push_back(z[i]);
      double k = r * r - 2 * (z[i] * z[i]);
      if(k < 0) { k = 0; }
      x.push_back(sqrt(k));
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    z = DREAM3DMath::linspace(r * cos(thetaa), r * cos(thetab), 100);
    for(int i = 0; i < z.size(); i++)
    {
      x.push_back(sqrt((r * r - (z[i]*z[i])) / 2));
      y.push_back(x[i]);
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    z = DREAM3DMath::linspace(r * cos(thetad), r * cos(thetab), 100);
    for(int i = 0; i < z.size(); i++)
    {
      x.push_back(maxk);
      y.push_back(sqrt(r * r - maxk * maxk - z[i]*z[i]));
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    std::vector<double> theta, tempd3, phi;
    theta = DREAM3DMath::linspace(thetac, thetaa, 100);
    for(int i = 0; i < theta.size(); i++)
    {
      tempd3.push_back(tan(A)*cos(theta[i]));
      double k = 2 * ((tan(A) * tan(A)) - tempd3[i] * tempd3[i]) - (1 - tempd3[i]) * (1 - tempd3[i]);
      if(k < 0) { k = 0; }
      phi.push_back(acos((((1 - tempd3[i]) + (sqrt(k))) / 2) / (tan(A)*sin(theta[i]))));
      z.push_back(r * cos(theta[i]));
      x.push_back(r * sin(theta[i])*cos(phi[i]));
      y.push_back(sqrt(r * r - x[i]*x[i] - z[i]*z[i]));
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));
  }
  else if(A >= deg1 / 2 && A <= maxdeg / 2)
  {
    std::vector<double> x, y, z;
    double theta1 = acos(((1 - maxk) - sqrt(2 * (tan(A) * tan(A) - maxk * maxk) - (1 - maxk) * (1 - maxk))) / (2 * tan(A)));
    double theta2 = acos((1 - sqrt(6 * tan(A) * tan(A) - 2)) / (3 * tan(A)));
    double theta3 = acos((sqrt(tan(A) * tan(A) - 2 * maxk * maxk)) / (tan(A)));

    z = DREAM3DMath::linspace(r * cos(theta2), r * cos(theta3), 100);
    for(int i = 0; i < z.size(); i++)
    {
      x.push_back(sqrt((r * r - z[i]*z[i]) / 2));
      y.push_back(x[i]);
    }
    ptsa = rodri2pair(x, y, z);
    //std::vector< std::pair<int, int> >pointlist=scalePoints(ptsa, scale);
    //std::vector<QPoint> qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    std::vector<double> theta, tempd3, phi;
    theta = DREAM3DMath::linspace(theta1, theta2, 100);
    for(int i = 0; i < theta.size(); i++)
    {
      tempd3.push_back(tan(A)*cos(theta[i]));
      double k = 2 * (tan(A) * tan(A) - tempd3[i] * tempd3[i]) - (1 - tempd3[i]) * (1 - tempd3[i]);
      if(k < 0) { k = 0; }
      phi.push_back(acos((((1 - tempd3[i]) + (sqrt(k))) / 2) / (tan(A)*sin(theta[i]))));
      x.push_back(r * sin(theta[i])*cos(phi[i]));
      z.push_back(r * cos(theta[i]));
      y.push_back(sqrt(r * r - x[i]*x[i] - z[i]*z[i]));
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));

    x.clear();
    y.clear();
    z.clear();
    z = DREAM3DMath::linspace(r * cos(theta1), r * cos(theta3), 100);
    for(int i = 0; i < z.size(); i++)
    {
      x.push_back(maxk);
      y.push_back(sqrt(r * r - maxk * maxk - z[i]*z[i]));
    }
    ptsa = rodri2pair(x, y, z);
    //pointlist=scalePoints(ptsa, scale);
    //qpointlist = pairToQPoint(pointlist);
    //painter.drawPolyline(qpointlist.data(), static_cast<int>(qpointlist.size()));
  }

  return image;
}

std::vector< std::pair<double, double> > CubicOps::rodri2pair(std::vector<double> x, std::vector<double> y, std::vector<double> z)
{
  std::vector< std::pair<double, double> > result;
  double q0, q1, q2, q3, ang, r, x1, y1, z1, rad, xPair, yPair, k;

  for(int i = 0; i < x.size(); i++)
  {
    //rodri2volpreserv
    q0 = sqrt(1 / (1 + x[i] * x[i] + y[i] * y[i] + z[i] * z[i]));
    q1 = x[i] * q0;
    q2 = y[i] * q0;
    q3 = z[i] * q0;
    ang = acos(q0);
    r = pow(1.5 * (ang - sin(ang) * cos(ang)), (1.0 / 3.0));
    x1 = q1 * r;
    y1 = q2 * r;
    z1 = q3 * r;
    if(sin(ang) != 0)
    {
      x1 = x1 / sin(ang);
      y1 = y1 / sin(ang);
      z1 = z1 / sin(ang);
    }

    //areapreservingx
    rad = sqrt(x1 * x1 + y1 * y1 + z1 * z1);
    if(rad == 0) { rad++; }
    k = 2 * (1 - fabs(x1 / rad));
    if(k < 0) { k = 0; }
    k = rad * sqrt(k);
    xPair = y1 * k;
    yPair = z1 * k;
    k = rad * rad - x1 * x1;
    if(k > 0)
    {
      xPair = xPair / sqrt(k);
      yPair = yPair / sqrt(k);
    }
    result.push_back(std::make_pair(xPair, yPair));
  }
  return result;
}

