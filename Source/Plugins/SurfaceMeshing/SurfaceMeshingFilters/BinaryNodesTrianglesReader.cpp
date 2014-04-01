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
#include "BinaryNodesTrianglesReader.h"

#include <QtCore/QtDebug>
#include <QtCore/QString>
#include <sstream>


#include "DREAM3DLib/Common/ScopedFileMonitor.hpp"
#include "DREAM3DLib/DataContainers/MeshStructs.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"

#include "BinaryNodesTrianglesReader.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BinaryNodesTrianglesReader::BinaryNodesTrianglesReader() :
  SurfaceMeshFilter(),
  m_SurfaceDataContainerName(DREAM3D::Defaults::SurfaceDataContainerName),
  m_VertexAttributeMatrixName(DREAM3D::Defaults::VertexAttributeMatrixName),
  m_FaceAttributeMatrixName(DREAM3D::Defaults::FaceAttributeMatrixName),
  m_BinaryNodesFile(""),
  m_BinaryTrianglesFile("")
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BinaryNodesTrianglesReader::~BinaryNodesTrianglesReader()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BinaryNodesTrianglesReader::setupFilterParameters()
{
  FilterParameterVector parameters;
  {
    FilterParameter::Pointer parameter = FilterParameter::New();
    parameter->setHumanLabel("Binary Nodes Input File");
    parameter->setPropertyName("BinaryNodesFile");
    parameter->setWidgetType(FilterParameterWidgetType::InputFileWidget);
    parameter->setValueType("QString");
    parameters.push_back(parameter);
  }
  {
    FilterParameter::Pointer parameter = FilterParameter::New();
    parameter->setHumanLabel("Binary Triangles Input File");
    parameter->setPropertyName("BinaryTrianglesFile");
    parameter->setWidgetType(FilterParameterWidgetType::InputFileWidget);
    parameter->setValueType("QString");
    parameters.push_back(parameter);
  }


  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BinaryNodesTrianglesReader::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);

  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int BinaryNodesTrianglesReader::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  /* Place code that will write the inputs values into a file. reference the
   AbstractFilterParametersWriter class for the proper API to use. */
  writer->writeValue("BinaryNodesFile", getBinaryNodesFile() );
  writer->writeValue("BinaryTrianglesFile", getBinaryTrianglesFile() );
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BinaryNodesTrianglesReader::dataCheck()
{
  SurfaceDataContainer* sm = getDataContainerArray()->createNonPrereqDataContainer<SurfaceDataContainer, AbstractFilter>(this, getSurfaceDataContainerName());
  if(getErrorCondition() < 0) { return; }

  if (getBinaryNodesFile().isEmpty() == true)
  {
    QString ss = QObject::tr("%1 needs the Binary Nodes File path set and it was not.").arg(ClassName());
    setErrorCondition(-387);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  if (getBinaryNodesFile().isEmpty() == true)
  {
    QString ss = QObject::tr("%1 needs the Binary Nodes File path set and it was not.").arg(ClassName());
    setErrorCondition(-387);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BinaryNodesTrianglesReader::preflight()
{
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BinaryNodesTrianglesReader::execute()
{
  int err = 0;
  QString ss;
  setErrorCondition(err);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  /* Place all your code to execute your filter here. */
  err = read();
  setErrorCondition(err);

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int BinaryNodesTrianglesReader::read()
{
  int err = 0;
  setErrorCondition(err);

  SurfaceDataContainer* sm = getDataContainerArray()->getDataContainerAs<SurfaceDataContainer>(getSurfaceDataContainerName());

  // Open the Nodes file for reading
  FILE* nodesFile = fopen(m_BinaryNodesFile.toLatin1().data(), "rb+");
  if(nodesFile == NULL)
  {
    QString ss = QObject::tr("Error opening nodes file '%1'").arg(m_BinaryNodesFile);
    setErrorCondition(786);
    return getErrorCondition();
  }
  ScopedFileMonitor nodesMonitor(nodesFile);

  // Calculate how many nodes are in the file based on the file size
  fseek(nodesFile, 0, SEEK_END);
  size_t fLength = ftell(nodesFile);
  size_t nNodes = fLength / SurfaceMesh::NodesFile::ByteCount;
  fseek(nodesFile, 0, SEEK_SET);
  fLength = ftell(nodesFile);
  if(0 != fLength)
  {
    QString ss = QObject::tr("%1: Error Could not rewind to beginning of file after nodes count.'%2'").arg(getNameOfClass()).arg(m_BinaryNodesFile);
    setErrorCondition(787);
    return getErrorCondition();
  }
  {
    QString ss = QObject::tr("Calc Node Count from Nodes.bin File: ").arg(nNodes);
    notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
  }
  // Open the triangles file for reading
  FILE* triFile = fopen(m_BinaryTrianglesFile.toLatin1().data(), "rb+");
  if(triFile == NULL)
  {
    QString ss = QObject::tr("%1: Error opening Triangles file '%2'").arg(getNameOfClass()).arg(m_BinaryTrianglesFile);
    setErrorCondition(788);
    return getErrorCondition();
  }

  ScopedFileMonitor trianglesMonitor(triFile);
  // Calculate how many Triangles are in the file based in the file size
  fseek(triFile, 0, SEEK_END);
  fLength = ftell(triFile);
  size_t nTriangles = fLength / SurfaceMesh::TrianglesFile::ByteCount;
  fseek(triFile, 0, SEEK_SET);
  fLength = ftell(triFile);
  if(0 != fLength)
  {

    QString ss = QObject::tr("%1: Error Could not rewind to beginning of file after triangles count.'%2'").arg(getNameOfClass()).arg(m_BinaryTrianglesFile);
    setErrorCondition(789);
    return getErrorCondition();
  }


  {
    QString ss = QObject::tr("Calc Triangle Count from Triangles.bin File: %1").arg(nTriangles);
    notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
  }

  // Allocate all the nodes
  VertexArray::Pointer m_NodeListPtr = VertexArray::CreateArray(nNodes, DREAM3D::VertexData::SurfaceMeshNodes);
  VertexArray::Vert_t* m_NodeList = m_NodeListPtr->getPointer(0);

  QVector<size_t> dims(1, 1);
  Int8ArrayType::Pointer nodeTypePtr = Int8ArrayType::CreateArray(nNodes, dims, DREAM3D::VertexData::SurfaceMeshNodeType);
  nodeTypePtr->initializeWithZeros();
  int8_t* nodeType = nodeTypePtr->getPointer(0);

  {
    QString ss  = QObject::tr("Reading Nodes file into Memory");
    notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
  }
  size_t nread = 0;
  SurfaceMesh::NodesFile::NodesFileRecord_t nRecord;

  for (size_t i = 0; i < nNodes; i++)
  {
    nread = fread(&nRecord, SurfaceMesh::NodesFile::ByteCount, 1, nodesFile); // Read one set of positions from the nodes file
    if(nread != 1)
    {
      break;
    }
    VertexArray::Vert_t& node = m_NodeList[nRecord.nodeId];
    node.pos[0] = nRecord.x;
    node.pos[1] = nRecord.y;
    node.pos[2] = nRecord.z;
    nodeType[nRecord.nodeId] = nRecord.nodeKind;
  }

  {
    QString ss = QObject::tr("Reading Triangles file into Memory");
    notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
  }

  // Allocate all the Triangle Objects
  FaceArray::Pointer m_TriangleListPtr = FaceArray::CreateArray(nTriangles, DREAM3D::FaceData::SurfaceMeshFaces, m_NodeListPtr.get());

  FaceArray::Face_t* m_TriangleList = m_TriangleListPtr->getPointer(0);
  ::memset(m_TriangleList, 0xAB, sizeof(FaceArray::Face_t) * nTriangles);

  QVector<size_t> dim(1, 2);
  DataArray<int32_t>::Pointer faceLabelPtr = DataArray<int32_t>::CreateArray(nTriangles, dim, DREAM3D::FaceData::SurfaceMeshFaceLabels);
  int32_t* faceLabels = faceLabelPtr->getPointer(0);
  faceLabelPtr->initializeWithZeros();


  SurfaceMesh::TrianglesFile::TrianglesFileRecord_t tRecord;
  for (size_t i = 0; i < nTriangles; i++)
  {
    // Read from the Input Triangles Temp File
    nread = fread(&tRecord, SurfaceMesh::TrianglesFile::ByteCount, 1, triFile);
    if(nread != 1)
    {
      break;
    }

    FaceArray::Face_t& triangle = m_TriangleList[tRecord.triId];

    triangle.verts[0] = tRecord.nodeId_0;
    triangle.verts[1] = tRecord.nodeId_1;
    triangle.verts[2] = tRecord.nodeId_2;
    faceLabels[tRecord.triId * 2] = tRecord.label_0;
    faceLabels[tRecord.triId * 2 + 1] = tRecord.label_1;
  }

  sm->setVertices(m_NodeListPtr);
  sm->setFaces(m_TriangleListPtr);
  sm->getAttributeMatrix(getFaceAttributeMatrixName())->addAttributeArray(faceLabelPtr->getName(), faceLabelPtr);
  sm->getAttributeMatrix(getVertexAttributeMatrixName())->addAttributeArray(nodeTypePtr->getName(), nodeTypePtr);

  // The ScopedFileMonitor classes will take care of closing the files

  return getErrorCondition();
}


