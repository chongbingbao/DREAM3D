/* ============================================================================
 * Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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

#include "H5MatrixStatsDataDelegate.h"

#include "H5Support/H5Lite.h"
#include "H5Support/H5Utilities.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5MatrixStatsDataDelegate::H5MatrixStatsDataDelegate()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5MatrixStatsDataDelegate::~H5MatrixStatsDataDelegate()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5MatrixStatsDataDelegate::readMatrixStatsData(MatrixStatsData* data, hid_t groupId)
{
  int err = 0;
  //Read the PhaseFraction
  err = readPhaseFraction(data, groupId);

  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5MatrixStatsDataDelegate::writeMatrixStatsData(MatrixStatsData* data, hid_t groupId)
{
  if (NULL == data)
  {
    return -1;
  }
  int err = 0;

  // Write the PhaseFraction
  err = writePhaseFraction(data, groupId);
  if (err < 0)
  {
    return err;
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5MatrixStatsDataDelegate::writePhaseFraction(MatrixStatsData* data, hid_t pid)
{
  float phaseFraction = data->getPhaseFraction();
  return H5Lite::writeScalarDataset(pid, DREAM3D::HDF5::PhaseFraction, phaseFraction);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5MatrixStatsDataDelegate::readPhaseFraction(MatrixStatsData* data, hid_t pid)
{
  float phaseFraction = 0.0f;
  int err = H5Lite::readScalarDataset(pid, DREAM3D::HDF5::PhaseFraction, phaseFraction);
  data->setPhaseFraction(phaseFraction);
  return err;
}
