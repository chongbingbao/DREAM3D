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

#include "AbaqusSurfaceMeshWriter.h"

#include <stdio.h>
#include "MXA/MXA.h"
#include "MXA/Utilities/MXADir.h"
#include "MXA/Utilities/MXAFileInfo.h"


#include "DREAM3DLib/Common/ScopedFileMonitor.hpp"

#define TRI_ELEMENT_TYPE "SFM3D3"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbaqusSurfaceMeshWriter::AbaqusSurfaceMeshWriter() :
  AbstractFilter(),
  m_SurfaceMeshFaceLabelsArrayName(DREAM3D::FaceData::SurfaceMeshFaceLabels),
  m_OutputFile(""),
  m_SurfaceMeshFaceLabels(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbaqusSurfaceMeshWriter::~AbaqusSurfaceMeshWriter()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbaqusSurfaceMeshWriter::setupFilterParameters()
{
  std::vector<FilterParameter::Pointer> parameters;
  {
    FilterParameter::Pointer option = FilterParameter::New();
    option->setHumanLabel("Output File");
    option->setPropertyName("OutputFile");
    option->setWidgetType(FilterParameter::OutputFileWidget);
    option->setValueType("string");
    option->setFileExtension("*.inp");
    parameters.push_back(option);
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbaqusSurfaceMeshWriter::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  /* Code to read the values goes between these statements */
  /* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE BEGIN*/
  setOutputFile(reader->readValue("OutputFile", getOutputFile()));
  /* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE END*/
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int AbaqusSurfaceMeshWriter::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  writer->writeValue("OutputFile", getOutputFile() );
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbaqusSurfaceMeshWriter::dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles)
{
  setErrorCondition(0);
  if (m_OutputFile.empty() == true)
  {
    setErrorCondition(-1003);
    addErrorMessage(getHumanLabel(), "Abaqus Output file is not set.", getErrorCondition());
  }

  SurfaceMeshDataContainer* sm = getSurfaceMeshDataContainer();
  if (NULL == sm)
  {
    setErrorCondition(-384);
    addErrorMessage(getHumanLabel(), "SurfaceMeshDataContainer is missing", getErrorCondition());
  }
  else
  {
    if (sm->getFaces().get() == NULL)
    {
      setErrorCondition(-385);
      addErrorMessage(getHumanLabel(), "SurfaceMesh DataContainer missing Triangles",getErrorCondition());
    }
    if (sm->getVertices().get() == NULL)
    {
      setErrorCondition(-386);
      addErrorMessage(getHumanLabel(), "SurfaceMesh DataContainer missing Nodes",getErrorCondition());
    }
    std::stringstream ss;
    GET_PREREQ_DATA(sm, DREAM3D, FaceData, SurfaceMeshFaceLabels, ss, -30, int32_t, Int32ArrayType, sm->getNumFaceTuples(), 2)
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbaqusSurfaceMeshWriter::preflight()
{
  /* Place code here that sanity checks input arrays and input values. Look at some
  * of the other DREAM3DLib/Filters/.cpp files for sample codes */
  dataCheck(true, 1, 1, 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AbaqusSurfaceMeshWriter::execute()
{
  int err = 0;
  std::stringstream ss;

  SurfaceMeshDataContainer* sm = getSurfaceMeshDataContainer();
  dataCheck(false, 1, 1, 1);
  if(getErrorCondition() < 0)
  {
    return;
  }

  // Make sure any directory path is also available as the user may have just typed
  // in a path without actually creating the full path
  std::string parentPath = MXAFileInfo::parentPath(getOutputFile());
  if(!MXADir::mkdir(parentPath, true))
  {
    std::stringstream ss;
    ss << "Error creating parent path '" << parentPath << "'";
    notifyErrorMessage(ss.str(), -1);
    setErrorCondition(-1);
    return;
  }

  DREAM3D::SurfaceMesh::VertListPointer_t nodesPtr = sm->getVertices();
  DREAM3D::SurfaceMesh::FaceListPointer_t trianglePtr = sm->getFaces();

  // Get the Labels(GrainIds or Region Ids) for the triangles
  Int32ArrayType::Pointer faceLabelsPtr = boost::dynamic_pointer_cast<Int32ArrayType>(sm->getFaceData(DREAM3D::FaceData::SurfaceMeshFaceLabels));
  int32_t* faceLabels = faceLabelsPtr->GetPointer(0);

  // Store all the unique Spins
  std::set<int> uniqueSpins;
  for (int i = 0; i < trianglePtr->GetNumberOfTuples(); i++)
  {
    uniqueSpins.insert(faceLabels[i*2]);
    uniqueSpins.insert(faceLabels[i*2+1]);
  }

  FILE* f = fopen(m_OutputFile.c_str(), "wb");
  ScopedFileMonitor fileMonitor(f);

  err = writeHeader(f, nodesPtr->GetNumberOfTuples(), trianglePtr->GetNumberOfTuples(), uniqueSpins.size()-1);
  err = writeNodes(f);
  err = writeTriangles(f);
  err = writeGrains(f);

  setErrorCondition(0);
  notifyStatusMessage("Complete");

  return;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int AbaqusSurfaceMeshWriter::writeHeader(FILE* f, int nodeCount, int triCount, int grainCount)
{
  if (NULL == f)
  {
    return -1;
  }
  fprintf(f, "*HEADING\n");
  fprintf(f, "** File Created with DREAM3D Version %s\n", DREAM3DLib::Version::Complete().c_str());
  fprintf(f, "** There are 3 Sections to this INP File: Nodes, Elements and Sets of Elements for each grain\n");
  fprintf(f, "** This file represents a trianguglar based mesh. The element type selected is %s for the triangles\n", TRI_ELEMENT_TYPE);
  fprintf(f, "** This file is an experimental output from DREAM3D. The user is responsible for verifying all elements in Abaqus\n");
  fprintf(f, "** We have selected to use a 'shell' element type currently. No boundary elements are written\n");
  fprintf(f, "**Number of Nodes: %d     Number of Triangles: %d   Number of Grains: %d\n", nodeCount, triCount, grainCount);
  fprintf(f, "*PREPRINT,ECHO=NO,HISTORY=NO,MODEL=NO\n");
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int AbaqusSurfaceMeshWriter::writeNodes(FILE* f)
{
  DREAM3D::SurfaceMesh::VertListPointer_t nodesPtr = getSurfaceMeshDataContainer()->getVertices();
  DREAM3D::SurfaceMesh::Vert_t* nodes = nodesPtr->GetPointer(0);
  size_t numNodes = nodesPtr->GetNumberOfTuples();
  int err = 0;
  fprintf(f, "*Node,NSET=NALL\n");
  //1, 72.520433763730, 70.306420652241, 100.000000000000


  for(size_t i = 1; i <= numNodes; ++i)
  {
    DREAM3D::SurfaceMesh::Vert_t& n = nodes[i-1];
    fprintf(f, "%lu, %0.6f, %0.6f, %0.6f\n", i, n.pos[0], n.pos[1], n.pos[2]);
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int AbaqusSurfaceMeshWriter::writeTriangles(FILE* f)
{
  int err = 0;
  DREAM3D::SurfaceMesh::FaceListPointer_t trianglePtr = getSurfaceMeshDataContainer()->getFaces();
  DREAM3D::SurfaceMesh::Face_t* triangles = trianglePtr->GetPointer(0);
  size_t numTri = trianglePtr->GetNumberOfTuples();

  fprintf(f, "*ELEMENT, TYPE=%s\n", TRI_ELEMENT_TYPE);
  for(size_t i = 1; i <= numTri; ++i)
  {

    // When we get the node index, add 1 to it because Abaqus number is 1 based.
    int nId0 = triangles[i-1].verts[0] + 1;
    int nId1 = triangles[i-1].verts[1] + 1;
    int nId2 = triangles[i-1].verts[2] + 1;
    fprintf(f, "%lu, %d, %d, %d\n", i, nId0, nId1, nId2);
  }
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int AbaqusSurfaceMeshWriter::writeGrains(FILE* f)
{

  //*Elset, elset=Grain1
  //1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16

  int err = 0;

  std::stringstream ss;
  SurfaceMeshDataContainer* sm = getSurfaceMeshDataContainer();
  DREAM3D::SurfaceMesh::VertListPointer_t nodesPtr = sm->getVertices();
  DREAM3D::SurfaceMesh::FaceListPointer_t trianglePtr = sm->getFaces();

  // Get the Labels(GrainIds or Region Ids) for the triangles
  Int32ArrayType::Pointer faceLabelsPtr = boost::dynamic_pointer_cast<Int32ArrayType>(sm->getFaceData(DREAM3D::FaceData::SurfaceMeshFaceLabels));
  int32_t* faceLabels = faceLabelsPtr->GetPointer(0);

  int nTriangles = trianglePtr->GetNumberOfTuples();

  // Store all the unique Spins
  std::set<int> uniqueSpins;
  for (int i = 0; i < nTriangles; i++)
  {
    uniqueSpins.insert(faceLabels[i*2]);
    uniqueSpins.insert(faceLabels[i*2+1]);
  }

  int spin = 0;

  //Loop over the unique Spins
  for (std::set<int>::iterator spinIter = uniqueSpins.begin(); spinIter != uniqueSpins.end(); ++spinIter )
  {
    spin = *spinIter;
    if(spin < 0) { continue; }

    fprintf(f, "*ELSET, ELSET=Grain%d\n", spin);

    ss.str("");
    ss << "Writing ELSET for Grain Id " << spin;
    notifyStatusMessage(ss.str());


    // Loop over all the triangles for this spin
    int lineCount = 0;
    for(int t = 0; t < nTriangles; ++t)
    {
      if (faceLabels[t*2] != spin && faceLabels[t*2+1] != spin)
      {
        continue; // We do not match either spin so move to the next triangle
      }

      // Only print 15 Triangles per line
      if (lineCount == 15)
      {
        fprintf(f, ", %d\n", t);
        lineCount = 0;
      }
      else if(lineCount == 0) // First value on the line
      {
        fprintf(f,"%d", t);
        lineCount++;
      }
      else
      {
        fprintf(f,", %d", t);
        lineCount++;
      }

    }
    // Make sure we have a new line at the end of the section
    if (lineCount != 0)
    {
      fprintf(f, "\n");
    }
  }
  return err;
}



