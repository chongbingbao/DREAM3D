/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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

#include "AlignSectionsFeatureCentroid.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Math/DREAM3DMath.h"
#include "DREAM3DLib/OrientationOps/OrientationOps.h"
#include "DREAM3DLib/Utilities/DREAM3DRandom.h"
#include "DREAM3DLib/DataArrays/DataArray.hpp"

#include "DREAM3DLib/OrientationOps/CubicOps.h"
#include "DREAM3DLib/OrientationOps/HexagonalOps.h"
#include "DREAM3DLib/OrientationOps/OrthoRhombicOps.h"

#include "DREAM3DLib/GenericFilters/FindCellQuats.h"

#define ERROR_TXT_OUT 1
#define ERROR_TXT_OUT1 1



const static float m_pi = static_cast<float>(M_PI);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AlignSectionsFeatureCentroid::AlignSectionsFeatureCentroid() :
  AlignSections(),
  m_GoodVoxelsArrayName(DREAM3D::CellData::GoodVoxels),
  m_ReferenceSlice(0),
  m_UseReferenceSlice(false),
  m_GoodVoxels(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AlignSectionsFeatureCentroid::~AlignSectionsFeatureCentroid()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeatureCentroid::setupFilterParameters()
{
  // Run the superclass first.
  //AlignSections::setupFilterParameters();
  std::vector<FilterParameter::Pointer> parameters = getFilterParameters();
  {
    FilterParameter::Pointer option = FilterParameter::New();
    option->setHumanLabel("Use Reference Slice");
    option->setPropertyName("UseReferenceSlice");
    option->setWidgetType(FilterParameter::BooleanWidget);
    option->setValueType("bool");
    parameters.push_back(option);
  }
  {
    FilterParameter::Pointer option = FilterParameter::New();
    option->setHumanLabel("Reference Slice");
    option->setPropertyName("ReferenceSlice");
    option->setWidgetType(FilterParameter::IntWidget);
    option->setValueType("int");
    option->setUnits("");
    parameters.push_back(option);
  }
  {
    FilterParameter::Pointer option = FilterParameter::New();
    option->setHumanLabel("Write Alignment Shift File");
    option->setPropertyName("WriteAlignmentShifts");
    option->setWidgetType(FilterParameter::BooleanWidget);
    option->setValueType("bool");
    parameters.push_back(option);
  }
  {
    FilterParameter::Pointer option = FilterParameter::New();
    option->setHumanLabel("Alignment File");
    option->setPropertyName("AlignmentShiftFileName");
    option->setWidgetType(FilterParameter::OutputFileWidget);
    option->setValueType("string");
    parameters.push_back(option);
  }
  setFilterParameters(parameters);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeatureCentroid::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  /* Code to read the values goes between these statements */
/* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE BEGIN*/
/* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE END*/
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int AlignSectionsFeatureCentroid::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  writer->writeValue("UseReferenceSlice", getUseReferenceSlice() );
  writer->writeValue("ReferenceSlice", getReferenceSlice() );
  writer->writeValue("WriteAlignmentShifts", getWriteAlignmentShifts());
  writer->writeValue("AlignmentShiftFileName", getAlignmentShiftFileName());
    writer->closeFilterGroup();
    return ++index; // we want to return the next index that was just written to
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeatureCentroid::dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles)
{
  setErrorCondition(0);
  std::stringstream ss;
  VoxelDataContainer* m = getVoxelDataContainer();

  if(true == getWriteAlignmentShifts() && getAlignmentShiftFileName().empty() == true)
  {
    ss << "The Alignment Shift file name must be set before executing this filter.";
    setErrorCondition(-1);
    addErrorMessage(getHumanLabel(), ss.str(), getErrorCondition());
  }

  GET_PREREQ_DATA(m, DREAM3D, CellData, GoodVoxels, ss, -303, bool, BoolArrayType, voxels, 1)
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeatureCentroid::preflight()
{
  dataCheck(true, 1, 1, 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeatureCentroid::execute()
{
  setErrorCondition(0);
  VoxelDataContainer* m = getVoxelDataContainer();
  if(NULL == m)
  {
    setErrorCondition(-999);
    notifyErrorMessage("The DataContainer Object was NULL", -999);
    return;
  }

  int64_t totalPoints = m->getTotalPoints();
  size_t numgrains = m->getNumFieldTuples();
  size_t numensembles = m->getNumEnsembleTuples();
  dataCheck(false, totalPoints, numgrains, numensembles);
  if (getErrorCondition() < 0)
  {
    return;
  }

  AlignSections::execute();

  // If there is an error set this to something negative and also set a message
  notifyStatusMessage("Aligning Sections Complete");
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeatureCentroid::find_shifts(std::vector<int> &xshifts, std::vector<int> &yshifts)
{
  VoxelDataContainer* m = getVoxelDataContainer();
  //int64_t totalPoints = m->totalPoints();

  std::ofstream outFile;
  if (getWriteAlignmentShifts() == true) {
    outFile.open(getAlignmentShiftFileName().c_str());
  }
  size_t udims[3] = {0,0,0};
  m->getDimensions(udims);
#if (CMP_SIZEOF_SIZE_T == 4)
  typedef int32_t DimType;
#else
  typedef int64_t DimType;
#endif
  DimType dims[3] = {
    static_cast<DimType>(udims[0]),
    static_cast<DimType>(udims[1]),
    static_cast<DimType>(udims[2]),
  };

  int newxshift = 0;
  int newyshift = 0;
  int count = 0;
  int slice = 0;
  int64_t point;
  //  int xspot, yspot;
  float xRes = m->getXRes();
  float yRes = m->getYRes();
  std::vector<float> xCentroid(dims[2],0.0);
  std::vector<float> yCentroid(dims[2],0.0);

  for (DimType iter = 0; iter < dims[2]; iter++)
  {
    count = 0;
    xCentroid[iter] = 0;
    yCentroid[iter] = 0;
    std::stringstream ss;
    ss << "Aligning Sections - Determining Shifts - " << ((float)iter/dims[2])*100 << " Percent Complete";
    //  notifyStatusMessage(ss.str());
    slice = static_cast<int>( (dims[2] - 1) - iter );
    for (DimType l = 0; l < dims[1]; l++)
    {
      for (DimType n = 0; n < dims[0]; n++)
      {
        point = static_cast<int>( ((slice) * dims[0] * dims[1]) + (l * dims[0]) + n );
        if(m_GoodVoxels[point] == true)
        {
          xCentroid[iter] = xCentroid[iter] + (float(n)*xRes);
          yCentroid[iter] = yCentroid[iter] + (float(l)*yRes);
          count++;
        }
      }
    }
    xCentroid[iter] = xCentroid[iter]/float(count);
    yCentroid[iter] = yCentroid[iter]/float(count);
  }
  //int refSlice;
  for (DimType iter = 1; iter < dims[2]; iter++)
  {
    slice = static_cast<int>( (dims[2] - 1) - iter );
    if(m_UseReferenceSlice == true)
    {
      xshifts[iter] = int((xCentroid[iter]-xCentroid[m_ReferenceSlice])/xRes);
      yshifts[iter] = int((yCentroid[iter]-yCentroid[m_ReferenceSlice])/yRes);
    }
    else
    {
      xshifts[iter] = xshifts[iter-1] + int((xCentroid[iter]-xCentroid[iter-1])/xRes);
      yshifts[iter] = yshifts[iter-1] + int((yCentroid[iter]-yCentroid[iter-1])/yRes);
    }
    if (getWriteAlignmentShifts() == true) {
      outFile << slice << "	" << slice+1 << "	" << newxshift << "	" << newyshift << "	" << xshifts[iter] << "	" << yshifts[iter] << " " << xCentroid[iter] << " " << yCentroid[iter] << std::endl;
    }
  }
  if (getWriteAlignmentShifts() == true) {
    outFile.close();
  }
}
