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

#include "AlignSectionsMutualInformation.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Common/DREAM3DMath.h"
#include "DREAM3DLib/Common/OrientationMath.h"
#include "DREAM3DLib/Common/DREAM3DRandom.h"
#include "DREAM3DLib/Common/DataArray.hpp"

#include "DREAM3DLib/OrientationOps/CubicOps.h"
#include "DREAM3DLib/OrientationOps/HexagonalOps.h"
#include "DREAM3DLib/OrientationOps/OrthoRhombicOps.h"

#include "DREAM3DLib/GenericFilters/FindCellQuats.h"

#define ERROR_TXT_OUT 1
#define ERROR_TXT_OUT1 1

using namespace std;

const static float m_pi = M_PI;

#define NEW_SHARED_ARRAY(var, type, size)\
  boost::shared_array<type> var##Array(new type[size]);\
  type* var = var##Array.get();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AlignSectionsMutualInformation::AlignSectionsMutualInformation() :
AlignSections(),
m_GoodVoxelsArrayName(DREAM3D::CellData::GoodVoxels),
m_PhasesArrayName(DREAM3D::CellData::Phases),
m_QuatsArrayName(DREAM3D::CellData::Quats),
m_GrainIdsArrayName(DREAM3D::CellData::GrainIds),
m_CrystalStructuresArrayName(DREAM3D::EnsembleData::CrystalStructures),
m_MisorientationTolerance(5.0f),
m_GrainIds(NULL),
m_Quats(NULL),
m_PhasesC(NULL),
m_GoodVoxels(NULL),
m_CrystalStructures(NULL)
{
  Seed = MXA::getMilliSeconds();

  m_HexOps = HexagonalOps::New();
  m_OrientationOps.push_back(m_HexOps.get());
  m_CubicOps = CubicOps::New();
  m_OrientationOps.push_back(m_CubicOps.get());
  m_OrthoOps = OrthoRhombicOps::New();
  m_OrientationOps.push_back(m_OrthoOps.get());

  graincounts = NULL;
  INIT_DataArray(m_GrainCounts, int);
  setupFilterOptions();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AlignSectionsMutualInformation::~AlignSectionsMutualInformation()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::setupFilterOptions()
{
  std::vector<FilterOption::Pointer> options;
  {
    FilterOption::Pointer option = FilterOption::New();
    option->setHumanLabel("Misorientation Tolerance");
    option->setPropertyName("MisorientationTolerance");
    option->setWidgetType(FilterOption::DoubleWidget);
    option->setValueType("float");
    option->setCastableValueType("double");
    options.push_back(option);
  }
  setFilterOptions(options);
}
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::writeFilterOptions(AbstractFilterOptionsWriter* writer)
{
  writer->writeValue("MisorientationTolerance", getMisorientationTolerance() );
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles)
{
  setErrorCondition(0);
  std::stringstream ss;
  DataContainer* m = getDataContainer();

  CREATE_NON_PREREQ_DATA   (m, DREAM3D,             CellData, GrainIds, ss, int32_t, Int32ArrayType, 0, voxels, 1);
//  CREATE_NON_PREREQ_DATA_TEST(m, m_GrainIdsArrayName, CellData, GrainIds, ss, int32_t, Int32ArrayType, 0, voxels, 1);

  GET_PREREQ_DATA(m, DREAM3D, CellData, Quats, ss, -301, float, FloatArrayType, voxels, 5);
  if(getErrorCondition() == -301)
  {
    setErrorCondition(0);
    FindCellQuats::Pointer find_cellquats = FindCellQuats::New();
    find_cellquats->setObservers(this->getObservers());
    find_cellquats->setDataContainer(getDataContainer());
    if(preflight == true) find_cellquats->preflight();
    if(preflight == false) find_cellquats->execute();
    GET_PREREQ_DATA(m, DREAM3D, CellData, Quats, ss, -301, float, FloatArrayType, voxels, 5);
  }
  GET_PREREQ_DATA_SUFFIX(m, DREAM3D, CellData, Phases, C, ss, -302,  int32_t, Int32ArrayType, voxels, 1);
  GET_PREREQ_DATA(m, DREAM3D, CellData, GoodVoxels, ss, -303, bool, BoolArrayType, voxels, 1);

  typedef DataArray<unsigned int> XTalStructArrayType;
  GET_PREREQ_DATA(m, DREAM3D, EnsembleData, CrystalStructures, ss, -304, unsigned int, XTalStructArrayType, ensembles, 1);

  setErrorMessage(ss.str());
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::preflight()
{
  dataCheck(true, 1, 1, 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::execute()
{
  setErrorCondition(0);
  DataContainer* m = getDataContainer();
  if (NULL == m)
  {
    setErrorCondition(-1);
    std::stringstream ss;
    ss << getNameOfClass() << " DataContainer was NULL";
    setErrorMessage(ss.str());
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
  notify("Aligning Sections Complete", 0, Observable::UpdateProgressMessage);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::find_shifts(std::vector<int> &xshifts, std::vector<int> &yshifts)
{
  DataContainer* m = getDataContainer();
  //int64_t totalPoints = m->totalPoints();

  ofstream outFile;
  string filename = "aligntest.txt";
  outFile.open(filename.c_str());

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

  float disorientation = 0;
  float mindisorientation = 100000000;
  float **mutualinfo12 = NULL;
  float *mutualinfo1 = NULL;
  float *mutualinfo2 = NULL;
  int graincount1, graincount2;
  int newxshift = 0;
  int newyshift = 0;
  int oldxshift = 0;
  int oldyshift = 0;
  float count = 0;
  int slice = 0;
  int xspot, yspot;
  float w;
  float n1, n2, n3;
  float q1[5];
  float q2[5];
  int refgnum, curgnum;
  int refposition = 0;
  int curposition = 0;
  DimType newPosition;
  DimType currentPosition;
  unsigned int phase1, phase2;

  form_grains_sections();

  std::vector<std::vector<float> >  misorients;
  misorients.resize(dims[0]);
  for (DimType a = 0; a < dims[0]; a++)
  {
	  misorients[a].resize(dims[1], 0.0);
  }
  for (DimType iter = 1; iter < dims[2]; iter++)
  {
    std::stringstream ss;
    ss << "Aligning Sections - Determining Shifts - " << ((float)iter/dims[2])*100 << " Percent Complete";
  //  notify(ss.str(), 0, Observable::UpdateProgressMessage);
    mindisorientation = 100000000;
    slice = (dims[2] - 1) - iter;
    graincount1 = graincounts[slice];
    graincount2 = graincounts[slice + 1];
    mutualinfo12 = new float *[graincount1];
    mutualinfo1 = new float[graincount1];
    mutualinfo2 = new float[graincount2];

    for (int a = 0; a < graincount1; a++)
    {
        mutualinfo1[a] = 0.0f;
        mutualinfo12[a] = new float[graincount2];
        for (int b = 0; b < graincount2; b++)
        {
          mutualinfo12[a][b] = 0.0f;
          mutualinfo2[b] = 0.0f;
        }
    }
    oldxshift = -1;
    oldyshift = -1;
    newxshift = 0;
    newyshift = 0;
    for (DimType a = 0; a < dims[0]; a++)
    {
      for (DimType b = 0; b < dims[1]; b++)
      {
        misorients[a][b] = 0;
      }
    }
    while (newxshift != oldxshift || newyshift != oldyshift)
    {
      oldxshift = newxshift;
      oldyshift = newyshift;
      for (int j = -3; j < 4; j++)
      {
        for (int k = -3; k < 4; k++)
        {
          disorientation = 0;
          count = 0;
          if(misorients[k + oldxshift + size_t(dims[0] / 2)][j + oldyshift + (size_t)(dims[1] / 2)] == 0 && abs(k + oldxshift) < (dims[0] / 2)
              && (j + oldyshift) < (dims[1] / 2))
          {
            for (DimType l = 0; l < dims[1]; l = l + 4)
            {
              for (DimType n = 0; n < dims[0]; n = n + 4)
              {
                if((l + j + oldyshift) >= 0 && (l + j + oldyshift) < dims[1] && (n + k + oldxshift) >= 0 && (n + k + oldxshift) < dims[0])
                {
                  refposition = ((slice + 1) * dims[0] * dims[1]) + (l * dims[0]) + n;
                  curposition = (slice * dims[0] * dims[1]) + ((l + j + oldyshift) * dims[0]) + (n + k + oldxshift);
                  refgnum = m_GrainIds[refposition];
                  curgnum = m_GrainIds[curposition];
                  if(curgnum >= 0 && refgnum >= 0)
                  {
                      mutualinfo12[curgnum][refgnum]++;
                      mutualinfo1[curgnum]++;
                      mutualinfo2[refgnum]++;
	                  count++;
                  }
                }
                else
                {
                  mutualinfo12[0][0]++;
                  mutualinfo1[0]++;
                  mutualinfo2[0]++;
                }
              }
            }
            float ha = 0;
            float hb = 0;
            float hab = 0;
            for (int b = 0; b < graincount1; b++)
            {
                mutualinfo1[b] = mutualinfo1[b] / float(count);
                if(mutualinfo1[b] != 0) ha = ha + mutualinfo1[b] * log(mutualinfo1[b]);
            }
            for (int c = 0; c < graincount2; c++)
            {
                mutualinfo2[c] = mutualinfo2[c] / float(count);
                if(mutualinfo2[c] != 0) hb = hb + mutualinfo2[c] * log(mutualinfo2[c]);
            }
            for (int b = 0; b < graincount1; b++)
            {
                for (int c = 0; c < graincount2; c++)
                {
                  mutualinfo12[b][c] = mutualinfo12[b][c] / float(count);
                  if(mutualinfo12[b][c] != 0) hab = hab + mutualinfo12[b][c] * log(mutualinfo12[b][c]);
                  float value = 0;
                  if(mutualinfo1[b] > 0 && mutualinfo2[c] > 0) value = (mutualinfo12[b][c] / (mutualinfo1[b] * mutualinfo2[c]));
                  if(value != 0) disorientation = disorientation + (mutualinfo12[b][c] * log(value));
                }
            }
            for (int b = 0; b < graincount1; b++)
            {
                for (int c = 0; c < graincount2; c++)
                {
                  mutualinfo12[b][c] = 0;
                  mutualinfo1[b] = 0;
                  mutualinfo2[c] = 0;
                }
            }
            disorientation = 1.0 / disorientation;
            misorients[k + oldxshift + int(dims[0] / 2)][j + oldyshift + int(dims[1] / 2)] = disorientation;
            if(disorientation < mindisorientation)
            {
              newxshift = k + oldxshift;
              newyshift = j + oldyshift;
              mindisorientation = disorientation;
            }
          }
        }
      }
    }
    xshifts[iter] = xshifts[iter-1] + newxshift;
    yshifts[iter] = yshifts[iter-1] + newyshift;
	outFile << slice << "	" << slice+1 << "	" << newxshift << "	" << newyshift << "	" << xshifts[iter] << "	" << yshifts[iter] << endl;
    delete [] mutualinfo1;
    delete [] mutualinfo2;
	for(int i = 0; i < graincount1; i++)
	{
		delete mutualinfo12[i];
	}
	delete [] mutualinfo12;
    mutualinfo1 = NULL;
    mutualinfo2 = NULL;
    mutualinfo12 = NULL;
  }

  m->removeCellData(DREAM3D::CellData::GrainIds);

  outFile.close();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::form_grains_sections()
{
  DREAM3D_RANDOMNG_NEW()
  DataContainer* m = getDataContainer();

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

 // int64_t totalPoints = m->totalPoints();


  int point = 0;
  int seed = 0;
  int noseeds = 0;
  int graincount = 1;
  int neighbor;
  // int currentpoint;
  float q1[5];
  float q2[5];
//  float qs[5];
  float w;
  float n1;
  float n2;
  float n3;
  DimType randx = 0;
  DimType randy = 0;
  DimType good = 0;
  DimType x, y, z;
  DimType col, row;
  size_t size = 0;
  size_t initialVoxelsListSize = 1000;

  graincounts = m_GrainCounts->WritePointer(0, dims[2]);

  std::vector<int> voxelslist(initialVoxelsListSize, -1);
  DimType neighpoints[4];
  neighpoints[0] = -dims[0];
  neighpoints[1] = -1;
  neighpoints[2] = 1;
  neighpoints[3] = dims[0];

  unsigned int phase1, phase2;
  for (DimType slice = 0; slice < dims[2]; slice++)
  {
    std::stringstream ss;
    ss << "Aligning Sections - Identifying Grains on Sections - " << ((float)slice/dims[2])*100 << " Percent Complete";
 //   notify(ss.str(), 0, Observable::UpdateProgressMessage);
    graincount = 1;
    noseeds = 0;
    while (noseeds == 0)
    {
      seed = -1;
      randx = int(float(rg.genrand_res53()) * float(dims[0]));
      randy = int(float(rg.genrand_res53()) * float(dims[1]));
      for (DimType j = 0; j < dims[1]; ++j)
      {
        for (DimType i = 0; i < dims[0]; ++i)
        {
          x = randx + i;
          y = randy + j;
          z = slice;
          if(x > dims[0] - 1) x = x - dims[0];
          if(y > dims[1] - 1) y = y - dims[1];
          point = (z * dims[0] * dims[1]) + (y * dims[0]) + x;
          if(m_GoodVoxels[point] == true && m_GrainIds[point] == 0 && m_PhasesC[point] > 0)
          {
            seed = point;
          }
          if(seed > -1) break;
        }
        if(seed > -1) break;
      }
      if(seed == -1) noseeds = 1;
      if(seed >= 0)
      {
        size = 0;
        m_GrainIds[seed] = graincount;
        voxelslist[size] = seed;
        size++;
        for (size_t j = 0; j < size; ++j)
        {
          int currentpoint = voxelslist[j];
          col = currentpoint % dims[0];
          row = (currentpoint / dims[0]) % dims[1];
          q1[0] = 1;
          q1[1] = m_Quats[currentpoint * 5 + 1];
          q1[2] = m_Quats[currentpoint * 5 + 2];
          q1[3] = m_Quats[currentpoint * 5 + 3];
          q1[4] = m_Quats[currentpoint * 5 + 4];
          phase1 = m_CrystalStructures[m_PhasesC[currentpoint]];
          for (int i = 0; i < 4; i++)
          {
            good = 1;
            neighbor = currentpoint + neighpoints[i];
            if((i == 0) && row == 0) good = 0;
            if((i == 3) && row == (dims[1] - 1)) good = 0;
            if((i == 1) && col == 0) good = 0;
            if((i == 2) && col == (dims[0] - 1)) good = 0;
            if(good == 1 && m_GrainIds[neighbor] <= 0 && m_PhasesC[neighbor] > 0)
            {
              w = 10000.0;
              q2[0] = 1;
              q2[1] = m_Quats[neighbor * 5 + 1];
              q2[2] = m_Quats[neighbor * 5 + 2];
              q2[3] = m_Quats[neighbor * 5 + 3];
              q2[4] = m_Quats[neighbor * 5 + 4];
              phase2 = m_CrystalStructures[m_PhasesC[neighbor]];
              if(phase1 == phase2) w = m_OrientationOps[phase1]->getMisoQuat(q1, q2, n1, n2, n3);
              if(w < m_MisorientationTolerance)
              {
                m_GrainIds[neighbor] = graincount;
                voxelslist[size] = neighbor;
                size++;
                if(size >= voxelslist.size()) voxelslist.resize(size + initialVoxelsListSize, -1);
              }
            }
          }
        }
        voxelslist.erase(std::remove(voxelslist.begin(), voxelslist.end(), -1), voxelslist.end());
        graincount++;
        voxelslist.clear();
        voxelslist.resize(initialVoxelsListSize, -1);
      }
    }
    graincounts[slice] = graincount;
  }
}
