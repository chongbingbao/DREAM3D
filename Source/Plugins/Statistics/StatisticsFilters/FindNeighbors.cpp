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

#include "FindNeighbors.h"

#include <sstream>


#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/DataArrays/NeighborList.hpp"
#include "DREAM3DLib/DataArrays/IDataArray.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindNeighbors::FindNeighbors() :
  AbstractFilter(),
  m_CellFeatureAttributeMatrixPath(DREAM3D::Defaults::VolumeDataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, ""),
  m_SharedSurfaceAreaListArrayName(DREAM3D::FeatureData::SharedSurfaceAreaList),
  m_NeighborListArrayName(DREAM3D::FeatureData::NeighborList),
  m_FeatureIdsArrayPath(DREAM3D::Defaults::VolumeDataContainerName, DREAM3D::Defaults::CellAttributeMatrixName, DREAM3D::CellData::FeatureIds),
  m_FeatureIdsArrayName(DREAM3D::CellData::FeatureIds),
  m_FeatureIds(NULL),
  m_SurfaceVoxelsArrayName(DREAM3D::CellData::SurfaceVoxels),
  m_SurfaceVoxels(NULL),
  m_SurfaceFeaturesArrayName(DREAM3D::FeatureData::SurfaceFeatures),
  m_SurfaceFeatures(NULL),
  m_NumNeighborsArrayName(DREAM3D::FeatureData::NumNeighbors),
  m_NumNeighbors(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindNeighbors::~FindNeighbors()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindNeighbors::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(FilterParameter::New("Required Information", "", FilterParameterWidgetType::SeparatorWidget, "QString", true));
  parameters.push_back(FilterParameter::New("Feature Ids Array Path", "FeatureIdsArrayPath", FilterParameterWidgetType::DataArraySelectionWidget,"DataArrayPath", true));
  parameters.push_back(FilterParameter::New("Created Information", "", FilterParameterWidgetType::SeparatorWidget, "QString", true));
  parameters.push_back(FilterParameter::New("Feature Data  Attribute Matrix", "CellFeatureAttributeMatrixPath", FilterParameterWidgetType::AttributeMatrixSelectionWidget,"DataArrayPath", true));
  parameters.push_back(FilterParameter::New("Surface Voxels Array Name", "SurfaceVoxelsArrayName", FilterParameterWidgetType::StringWidget,"QString", true));
  parameters.push_back(FilterParameter::New("Surface Features Array Name", "SurfaceFeaturesArrayName", FilterParameterWidgetType::StringWidget,"QString", true));
  parameters.push_back(FilterParameter::New("Number Of Neighbors Array Name", "NumNeighborsArrayName", FilterParameterWidgetType::StringWidget,"QString", true));
  parameters.push_back(FilterParameter::New("Neighbor List Array Name", "NeighborListArrayName", FilterParameterWidgetType::StringWidget,"QString", true));
  parameters.push_back(FilterParameter::New("Neighbor Surface Area List Array Name", "SharedSurfaceAreaListArrayName", FilterParameterWidgetType::StringWidget,"QString", true));

  setFilterParameters(parameters);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindNeighbors::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setCellFeatureAttributeMatrixPath(reader->readDataArrayPath("CellFeatureAttributeMatrixPath", getCellFeatureAttributeMatrixPath() ) );
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath() ) );
  setSurfaceVoxelsArrayName(reader->readString("SurfaceVoxelsArrayName", getSurfaceVoxelsArrayName() ) );
  setSurfaceFeaturesArrayName(reader->readString("SurfaceFeaturesArrayName", getSurfaceFeaturesArrayName() ) );
  setNumNeighborsArrayName(reader->readString("NumNeighborsArrayName", getNumNeighborsArrayName() ) );
  setNeighborListArrayName(reader->readString("NeighborListArrayName", getNeighborListArrayName() ) );
  setSharedSurfaceAreaListArrayName(reader->readString("SharedSurfaceAreaListArrayName", getSharedSurfaceAreaListArrayName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int FindNeighbors::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  writer->writeValue("CellFeatureAttributeMatrixPath", getCellFeatureAttributeMatrixPath() );
  writer->writeValue("FeatureIdsArrayPath", getFeatureIdsArrayPath() );
  writer->writeValue("SurfaceVoxelsArrayName", getSurfaceVoxelsArrayName() );
  writer->writeValue("SurfaceFeaturesArrayName", getSurfaceFeaturesArrayName() );
  writer->writeValue("NumNeighborsArrayName", getNumNeighborsArrayName() );
  writer->writeValue("NeighborListArrayName", getNeighborListArrayName() );
  writer->writeValue("SharedSurfaceAreaListArrayName", getSharedSurfaceAreaListArrayName() );
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindNeighbors::dataCheck()
{
  setErrorCondition(0);

  // This is for convenience
  DataContainerArray::Pointer dca = getDataContainerArray();

  AttributeMatrix::Pointer cellAttrMat = getDataContainerArray()->getPrereqAttributeMatrixFromPath<VolumeDataContainer, AbstractFilter>(this, getFeatureIdsArrayPath(), -303);
  if(getErrorCondition() < 0 || NULL == cellAttrMat.get() ) { return; }

  QVector<size_t> dims(1, 1);
  // Cell Data
  m_FeatureIdsPtr = dca->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this, m_FeatureIdsArrayPath, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_FeatureIdsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  m_SurfaceVoxelsPtr = cellAttrMat->createNonPrereqArray<DataArray<int8_t>, AbstractFilter, int8_t>(this, m_SurfaceVoxelsArrayName, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_SurfaceVoxelsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_SurfaceVoxels = m_SurfaceVoxelsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  AttributeMatrix::Pointer cellFeatureAttrMat = getDataContainerArray()->getPrereqAttributeMatrixFromPath<VolumeDataContainer, AbstractFilter>(this, getCellFeatureAttributeMatrixPath(), -304);
  if(getErrorCondition() < 0) { return; }

  m_NumNeighborsPtr = cellFeatureAttrMat->createNonPrereqArray<DataArray<int32_t>, AbstractFilter, int32_t>(this, m_NumNeighborsArrayName, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_NumNeighborsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_NumNeighbors = m_NumNeighborsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  m_SurfaceFeaturesPtr = cellFeatureAttrMat->createNonPrereqArray<DataArray<bool>, AbstractFilter, bool>(this, m_SurfaceFeaturesArrayName, false, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_SurfaceFeaturesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_SurfaceFeatures = m_SurfaceFeaturesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  // Feature Data
  // Do this whole block FIRST otherwise the side effect is that a call to m->getNumCellFeatureTuples will = 0
  // because we are just creating an empty NeighborList object.
  // Now we are going to get a "Pointer" to the NeighborList object out of the DataContainer
  typedef NeighborList<int> IntNeighborList_t;
  typedef NeighborList<float> FloatNeighborList_t;

  //  m_NeighborList = NeighborList<int>::SafeObjectDownCast<IDataArray*, NeighborList<int>*>(cellFeatureAttrMat->getAttributeArray(m_NeighborListArrayName.getDataArrayName()).get());
  m_NeighborList = cellFeatureAttrMat->getArray<IntNeighborList_t>(m_NeighborListArrayName);
  if(NULL == m_NeighborList.get())
  {
    IntNeighborList_t::Pointer neighborlistPtr = IntNeighborList_t::New();
    neighborlistPtr->setName(m_NeighborListArrayName);
    neighborlistPtr->resize(cellFeatureAttrMat->getNumTuples());
    neighborlistPtr->setNumNeighborsArrayName(m_NumNeighborsArrayName);
    cellFeatureAttrMat->addAttributeArray(m_NeighborListArrayName, neighborlistPtr);
    if (neighborlistPtr.get() == NULL)
    {
      QString ss = QObject::tr("NeighborLists Array Not Initialized at Beginning of FindNeighbors Filter");
      setErrorCondition(-308);
      notifyErrorMessage(getHumanLabel(), ss, -308);
    }
    //   m_NeighborList = NeighborList<int>::SafeObjectDownCast<IDataArray*, NeighborList<int>* >(cellFeatureAttrMat->getAttributeArray(m_NeighborListArrayName.getDataArrayName()).get());
    m_NeighborList = cellFeatureAttrMat->getArray<IntNeighborList_t>(m_NeighborListArrayName);

  }

  // And we do the same for the SharedSurfaceArea list
  //m_SharedSurfaceAreaList = NeighborList<float>::SafeObjectDownCast<IDataArray*, NeighborList<float>*>(cellFeatureAttrMat->getAttributeArray(m_SharedSurfaceAreaListArrayName.getDataArrayName()).get());
  m_SharedSurfaceAreaList = cellFeatureAttrMat->getArray<FloatNeighborList_t>(m_SharedSurfaceAreaListArrayName);
  if(m_SharedSurfaceAreaList == NULL)
  {
    NeighborList<float>::Pointer sharedSurfaceAreaListPtr = NeighborList<float>::New();
    sharedSurfaceAreaListPtr->setName(m_SharedSurfaceAreaListArrayName);
    sharedSurfaceAreaListPtr->resize(cellFeatureAttrMat->getNumTuples());
    sharedSurfaceAreaListPtr->setNumNeighborsArrayName(m_NumNeighborsArrayName);
    cellFeatureAttrMat->addAttributeArray(m_SharedSurfaceAreaListArrayName, sharedSurfaceAreaListPtr);
    if (sharedSurfaceAreaListPtr.get() == NULL)
    {
      QString ss = QObject::tr("SurfaceAreaLists Array Not Initialized correctly at Beginning of FindNeighbors Filter");
      setErrorCondition(-308);
      notifyErrorMessage(getHumanLabel(), ss, -308);
    }
    //  m_SharedSurfaceAreaList = NeighborList<float>::SafeObjectDownCast<IDataArray*, NeighborList<float>*>(cellFeatureAttrMat->getAttributeArray(m_SharedSurfaceAreaListArrayName.getDataArrayName()).get());
    m_SharedSurfaceAreaList = cellFeatureAttrMat->getArray<FloatNeighborList_t>(m_SharedSurfaceAreaListArrayName);
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindNeighbors::preflight()
{
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindNeighbors::execute()
{
  setErrorCondition(0);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  VolumeDataContainer* m = getDataContainerArray()->getDataContainerAs<VolumeDataContainer>(m_FeatureIdsArrayPath.getDataContainerName());
  int64_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();
  size_t totalFeatures = m_NumNeighborsPtr.lock()->getNumberOfTuples();

  size_t udims[3] = {0, 0, 0};
  m->getDimensions(udims);
#if (CMP_SIZEOF_SIZE_T == 4)
  typedef int32_t DimType;
#else
  typedef int64_t DimType;
#endif
  DimType dims[3] =
  {
    static_cast<DimType>(udims[0]),
    static_cast<DimType>(udims[1]),
    static_cast<DimType>(udims[2]),
  };

  DimType neighpoints[6];
  neighpoints[0] = -dims[0] * dims[1];
  neighpoints[1] = -dims[0];
  neighpoints[2] = -1;
  neighpoints[3] = 1;
  neighpoints[4] = dims[0];
  neighpoints[5] = dims[0] * dims[1];

  float column, row, plane;
  int feature;
  size_t nnum;
  int onsurf = 0;
  int good = 0;
  int neighbor = 0;

  QVector<QVector<int> > neighborlist;
  QVector<QVector<float> > neighborsurfacearealist;

  int nListSize = 100;
  neighborlist.resize(totalFeatures);
  neighborsurfacearealist.resize(totalFeatures);
  for (int i = 1; i < totalFeatures; i++)
  {
    QString ss = QObject::tr("Finding Neighbors - Initializing Neighbor Lists - %1 Percent Complete").arg((static_cast<float>(i) / totalFeatures) * 100);
    //   notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
    m_NumNeighbors[i] = 0;
    neighborlist[i].resize(nListSize);
    neighborsurfacearealist[i].fill(-1.0, nListSize);
    m_SurfaceFeatures[i] = false;
  }

  for (int64_t j = 0; j < totalPoints; j++)
  {
    QString ss = QObject::tr("Finding Neighbors - Determining Neighbor Lists - %1 Percent Complete").arg((static_cast<float>(j) / totalPoints) * 100);
    //   notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
    onsurf = 0;
    feature = m_FeatureIds[j];
    if(feature > 0)
    {
      column = static_cast<float>( j % m->getXPoints() );
      row = static_cast<float>( (j / m->getXPoints()) % m->getYPoints() );
      plane = static_cast<float>( j / (m->getXPoints() * m->getYPoints()) );
      if((column == 0 || column == (m->getXPoints() - 1) || row == 0 || row == (m->getYPoints() - 1) || plane == 0 || plane == (m->getZPoints() - 1)) && m->getZPoints() != 1)
      {
        m_SurfaceFeatures[feature] = true;
      }
      if((column == 0 || column == (m->getXPoints() - 1) || row == 0 || row == (m->getYPoints() - 1)) && m->getZPoints() == 1)
      {
        m_SurfaceFeatures[feature] = true;
      }
      for (int k = 0; k < 6; k++)
      {
        good = 1;
        neighbor = static_cast<int>( j + neighpoints[k] );
        if(k == 0 && plane == 0) { good = 0; }
        if(k == 5 && plane == (m->getZPoints() - 1)) { good = 0; }
        if(k == 1 && row == 0) { good = 0; }
        if(k == 4 && row == (m->getYPoints() - 1)) { good = 0; }
        if(k == 2 && column == 0) { good = 0; }
        if(k == 3 && column == (m->getXPoints() - 1)) { good = 0; }
        if(good == 1 && m_FeatureIds[neighbor] != feature && m_FeatureIds[neighbor] > 0)
        {
          onsurf++;
          nnum = m_NumNeighbors[feature];
          neighborlist[feature].push_back(m_FeatureIds[neighbor]);
          nnum++;
          m_NumNeighbors[feature] = static_cast<int32_t>(nnum);
        }
      }
    }
    m_SurfaceVoxels[j] = onsurf;
  }

  // We do this to create new set of NeighborList objects
  for (size_t i = 1; i < totalFeatures; i++)
  {
    QString ss = QObject::tr("Finding Neighbors - Calculating Surface Areas - %1 Percent Complete").arg(((float)i / totalFeatures) * 100);
    //  notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);

    QMap<int, int> neighToCount;
    int numneighs = static_cast<int>( neighborlist[i].size() );

    // this increments the voxel counts for each feature
    for (int j = 0; j < numneighs; j++)
    {
      neighToCount[neighborlist[i][j]]++;
    }

    QMap<int, int>::Iterator neighiter = neighToCount.find(0);
    neighToCount.erase(neighiter);
    neighiter = neighToCount.find(-1);
    neighToCount.erase(neighiter);
    //Resize the features neighbor list to zero
    neighborlist[i].resize(0);
    neighborsurfacearealist[i].resize(0);

    for (QMap<int, int>::iterator iter = neighToCount.begin(); iter != neighToCount.end(); ++iter)
    {
      int neigh = iter.key(); // get the neighbor feature
      int number = iter.value(); // get the number of voxels
      float area = number * m->getXRes() * m->getYRes();

      // Push the neighbor feature id back onto the list so we stay synced up
      neighborlist[i].push_back(neigh);
      neighborsurfacearealist[i].push_back(area);
    }
    m_NumNeighbors[i] = int32_t( neighborlist[i].size() );

    // Set the vector for each list into the NeighborList Object
    NeighborList<int>::SharedVectorType sharedNeiLst(new std::vector<int>);
    sharedNeiLst->assign(neighborlist[i].begin(), neighborlist[i].end());
    m_NeighborList->setList(static_cast<int>(i), sharedNeiLst);

    NeighborList<float>::SharedVectorType sharedSAL(new std::vector<float>);
    sharedSAL->assign(neighborsurfacearealist[i].begin(), neighborsurfacearealist[i].end());
    m_SharedSurfaceAreaList->setList(static_cast<int>(i), sharedSAL);
  }

  notifyStatusMessage(getHumanLabel(), "Finding Neighbors Complete");
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindNeighbors::newFilterInstance(bool copyFilterParameters)
{
  FindNeighbors::Pointer filter = FindNeighbors::New();
  if(true == copyFilterParameters)
  {
    filter->setCellFeatureAttributeMatrixPath(getCellFeatureAttributeMatrixPath() );
    filter->setFeatureIdsArrayPath(getFeatureIdsArrayPath() );
    filter->setSurfaceVoxelsArrayName(getSurfaceVoxelsArrayName() );
    filter->setSurfaceFeaturesArrayName(getSurfaceFeaturesArrayName() );
    filter->setNumNeighborsArrayName(getNumNeighborsArrayName() );
    filter->setNeighborListArrayName(getNeighborListArrayName() );
    filter->setSharedSurfaceAreaListArrayName(getSharedSurfaceAreaListArrayName() );
  }
  return filter;
}