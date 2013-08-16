/* ============================================================================
 * Copyright (c) 2013 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2013 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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
 *                           FA8650-10-D-5210
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "ModifiedLambertProjection.h"

#include <set>


#include "DREAM3DLib/Common/DREAM3DMath.h"
#include "DREAM3DLib/Math/MatrixMath.h"

#define WRITE_LAMBERT_SQUARE_COORD_VTK 0

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ModifiedLambertProjection::ModifiedLambertProjection() :
  m_Dimension(0),
  m_StepSize(0.0f),
  m_SphereRadius(1.0f),
  m_MaxCoord(0.0),
  m_MinCoord(0.0)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ModifiedLambertProjection::~ModifiedLambertProjection()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ModifiedLambertProjection::Pointer ModifiedLambertProjection::CreateProjectionFromXYZCoords(FloatArrayType* coords, int dimension, float sphereRadius)
{

  size_t npoints = coords->GetNumberOfTuples();
  bool nhCheck = false;
  float sqCoord[2];
  int sqIndex = 0;
  ModifiedLambertProjection::Pointer squareProj = ModifiedLambertProjection::New();
  squareProj->initializeSquares(dimension, sphereRadius);


#if WRITE_LAMBERT_SQUARE_COORD_VTK
  std::stringstream ss;
  std::string filename("/tmp/");
  filename.append("ModifiedLambert_Square_Coords_").append(coords->GetName()).append(".vtk");
  FILE* f = NULL;
  f = fopen(filename.c_str(), "wb");
  if(NULL == f)
  {
    ss.str("");
    ss << "Could not open vtk viz file " << filename << " for writing. Please check access permissions and the path to the output location exists";
    return squareProj;
  }

  // Write the correct header
  fprintf(f, "# vtk DataFile Version 2.0\n");
  fprintf(f, "data set from DREAM3D\n");
  fprintf(f, "ASCII");
  fprintf(f, "\n");

  fprintf(f, "DATASET UNSTRUCTURED_GRID\nPOINTS %lu float\n", coords->GetNumberOfTuples() );
#endif

  for(int i = 0; i < npoints; ++i)
  {
    sqCoord[0] = 0.0; sqCoord[1] = 0.0;
    //get coordinates in square projection of crystal normal parallel to boundary normal
    nhCheck = squareProj->getSquareCoord(coords->GetPointer(i * 3), sqCoord);
#if WRITE_LAMBERT_SQUARE_COORD_VTK
    fprintf(f, "%f %f 0\n", sqCoord[0], sqCoord[1]);
#endif

    // Based on the XY coordinate, get the pointer index that the value corresponds to in the proper square
    sqIndex = squareProj->getSquareIndex(sqCoord);
    if (nhCheck == true)
    {
      //north increment by 1
      squareProj->addValue(ModifiedLambertProjection::NorthSquare, sqIndex, 1.0);
    }
    else
    {
      // south increment by 1
      squareProj->addValue(ModifiedLambertProjection::SouthSquare, sqIndex, 1.0);
    }
  }
#if WRITE_LAMBERT_SQUARE_COORD_VTK
  fclose(f);
#endif

  return squareProj;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::initializeSquares(int dims, float sphereRadius)
{
  m_Dimension = dims;
  m_SphereRadius = sphereRadius;
  // We want half the sphere area for each square because each square represents a hemisphere.
  float halfSphereArea = 4 * M_PI * sphereRadius * sphereRadius / 2.0;
  // The length of a side of the square is the square root of the area
  float squareEdge = sqrt(halfSphereArea);

  m_StepSize = squareEdge / static_cast<float>(m_Dimension);

  m_MaxCoord = squareEdge / 2.0;
  m_MinCoord = -squareEdge / 2.0;
  m_HalfDimension = static_cast<float>(m_Dimension) / 2.0;
  m_HalfDimensionTimesStepSize = m_HalfDimension * m_StepSize;

  m_NorthSquare = DoubleArrayType::CreateArray(m_Dimension * m_Dimension, 1, "NorthSquare");
  m_NorthSquare->initializeWithZeros();
  m_SouthSquare = DoubleArrayType::CreateArray(m_Dimension * m_Dimension, 1, "SouthSquare");
  m_SouthSquare->initializeWithZeros();


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ModifiedLambertProjection::writeHDF5Data(hid_t groupId)
{
  int err = 0;
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ModifiedLambertProjection::readHDF5Data(hid_t groupId)
{
  int err = 0;
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::addValue(Square square, int index, double value)
{
  if (square == NorthSquare)
  {
    double v = m_NorthSquare->GetValue(index) + value;
    m_NorthSquare->SetValue(index, v);
  }
  else
  {
    double v = m_SouthSquare->GetValue(index) + value;
    m_SouthSquare->SetValue(index, v);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double ModifiedLambertProjection::getValue(Square square, int index)
{
  if (square == NorthSquare)
  {
    return m_NorthSquare->GetValue(index);
  }
  else
  {
    return m_SouthSquare->GetValue(index);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double ModifiedLambertProjection::getInterpolatedValue(Square square, float* sqCoord)
{
  int abinMod, bbinMod;
  float modX = (sqCoord[0] + m_HalfDimensionTimesStepSize ) / m_StepSize;
  float modY = (sqCoord[1] + m_HalfDimensionTimesStepSize ) / m_StepSize;
  int abin = (int) modX;
  int bbin = (int) modY;
  modX -= abin;
  modY -= bbin;
  if(abin < m_Dimension-1) abinMod = abin+1;
  else abinMod = abin+1-m_Dimension;
  if(bbin < m_Dimension-1) bbinMod = bbin+1;
  else bbinMod = bbin+1-m_Dimension;
  if (square == NorthSquare)
  {
    float intensity1 = m_NorthSquare->GetValue((abin)+(bbin*m_Dimension));
    float intensity2 = m_NorthSquare->GetValue((abinMod)+(bbin*m_Dimension));
    float intensity3 = m_NorthSquare->GetValue((abin)+(bbinMod*m_Dimension));
    float intensity4 = m_NorthSquare->GetValue((abinMod)+(bbinMod*m_Dimension));
    float interpolatedIntensity = ((intensity1*(1-modX)*(1-modY))+(intensity2*(modX)*(1-modY))+(intensity3*(1-modX)*(modY))+(intensity4*(modX)*(modY)));
    return interpolatedIntensity;
  }
  else
  {
    float intensity1 = m_SouthSquare->GetValue((abin)+(bbin*m_Dimension));
    float intensity2 = m_SouthSquare->GetValue((abinMod)+(bbin*m_Dimension));
    float intensity3 = m_SouthSquare->GetValue((abin)+(bbinMod*m_Dimension));
    float intensity4 = m_SouthSquare->GetValue((abinMod)+(bbinMod*m_Dimension));
    float interpolatedIntensity = ((intensity1*(1-modX)*(1-modY))+(intensity2*(modX)*(1-modY))+(intensity3*(1-modX)*(modY))+(intensity4*(modX)*(modY)));
    return interpolatedIntensity;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ModifiedLambertProjection::getSquareCoord(float* xyz, float* sqCoord)
{
  bool nhCheck = false;
  float adjust = 1.0;
  if(xyz[2] >= 0.0)
  {
    adjust = -1.0;
    nhCheck = true;
  }
  if(fabs(xyz[0]) >= fabs(xyz[1]))
  {
    sqCoord[0] = (xyz[0]/fabs(xyz[0]) ) * sqrt(2.0*m_SphereRadius*(m_SphereRadius + (xyz[2]*adjust) ) ) * DREAM3D::Constants::k_HalfOfSqrtPi;
    sqCoord[1] = (xyz[0]/fabs(xyz[0]) ) * sqrt(2.0*m_SphereRadius*(m_SphereRadius + (xyz[2]*adjust) ) ) * ((DREAM3D::Constants::k_2OverSqrtPi)*atan(xyz[1]/xyz[0]));
  }
  else
  {
    sqCoord[0] = (xyz[1]/fabs(xyz[1]))*sqrt(2.0*m_SphereRadius*(m_SphereRadius+(xyz[2]*adjust)))*((DREAM3D::Constants::k_2OverSqrtPi)*atan(xyz[0]/xyz[1]));
    sqCoord[1] = (xyz[1]/fabs(xyz[1]))*sqrt(2.0*m_SphereRadius*(m_SphereRadius+(xyz[2]*adjust)))*(DREAM3D::Constants::k_HalfOfSqrtPi);
  }

  if (sqCoord[0] >= m_MaxCoord)
  {
    sqCoord[0] = (m_MaxCoord) - .0001;
  }
  if (sqCoord[1] >= m_MaxCoord)
  {
    sqCoord[1] = (m_MaxCoord) - .0001;
  }
  return nhCheck;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ModifiedLambertProjection::getSquareIndex(float* sqCoord)
{
  int x = (int)( (sqCoord[0] + m_MaxCoord) / m_StepSize);
  if (x >= m_Dimension)
  {
    x = m_Dimension - 1;
  }
  if (x < 0) { x = 0; }
  int y = (int)( (sqCoord[1] + m_MaxCoord) / m_StepSize);
  if (y >= m_Dimension)
  {
    y = m_Dimension - 1;
  }
  if (y < 0) { y = 0; }
  int index = y * m_Dimension + x;
  BOOST_ASSERT(index < m_Dimension *m_Dimension);
  return index;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::normalizeSquares()
{

  size_t npoints = m_NorthSquare->GetNumberOfTuples();
  double nTotal = 0;
  double sTotal = 0;

  double* north = m_NorthSquare->GetPointer(0);
  double* south = m_SouthSquare->GetPointer(0);

  // Get the Sum of all the bins
  for(size_t i = 0; i < npoints; ++i)
  {
    nTotal = nTotal + north[i];
    sTotal = sTotal + south[i];
  }
  double oneOverNTotal = 1.0/nTotal;
  double oneOverSTotal = 1.0/sTotal;

  // Divide each bin by the total of all the bins for that Hemisphere
  for(size_t i = 0; i < npoints; ++i)
  {
    north[i] = (north[i] * oneOverNTotal);
    south[i] = (south[i] * oneOverSTotal);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::normalizeSquaresToMRD()
{
  // First Normalize the squares
  normalizeSquares();
  size_t npoints = m_NorthSquare->GetNumberOfTuples();
  double* north = m_NorthSquare->GetPointer(0);
  double* south = m_SouthSquare->GetPointer(0);
  int dimSqrd = m_Dimension * m_Dimension;

  // Multiply Each Bin by the total number of bins
  for(size_t i = 0; i < npoints; ++i)
  {
    north[i] = north[i] * dimSqrd;
    south[i] = south[i] * dimSqrd;
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::createStereographicProjection(int dim, DoubleArrayType* stereoIntensity)
{
  int xpoints = dim;
  int ypoints = dim;

  int xpointshalf = xpoints / 2;
  int ypointshalf = ypoints / 2;

  float xres = 2.0 / (float)(xpoints);
  float yres = 2.0 / (float)(ypoints);
  float xtmp, ytmp;
  float sqCoord[2];
  float xyz[3];
  bool nhCheck = false;

  stereoIntensity->initializeWithZeros();
  double* intensity = stereoIntensity->GetPointer(0);


  for (int64_t y = 0; y < ypoints; y++)
  {
    for (int64_t x = 0; x < xpoints; x++)
    {
      //get (x,y) for stereographic projection pixel
      xtmp = float(x-xpointshalf)*xres+(xres * 0.5);
      ytmp = float(y-ypointshalf)*yres+(yres * 0.5);
      int index = y * xpoints + x;
      if((xtmp*xtmp+ytmp*ytmp) <= 1.0)
      {
        xyz[2] = -((xtmp*xtmp+ytmp*ytmp)-1)/((xtmp*xtmp+ytmp*ytmp)+1);
        xyz[0] = xtmp*(1+xyz[2]);
        xyz[1] = ytmp*(1+xyz[2]);


        for( int64_t m = 0; m < 2; m++)
        {
          if(m == 1) MatrixMath::Multiply3x1withConstant(xyz, -1.0);
          nhCheck = getSquareCoord(xyz, sqCoord);
          if (nhCheck == true)
          {
            //get Value from North square
            intensity[index] += getInterpolatedValue(ModifiedLambertProjection::NorthSquare, sqCoord);
          }
          else
          {
            //get Value from South square
            intensity[index] += getInterpolatedValue(ModifiedLambertProjection::SouthSquare, sqCoord);
          }
        }
        intensity[index]  = intensity[index] * 0.5;
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DoubleArrayType::Pointer ModifiedLambertProjection::createStereographicProjection(int dim)
{
  DoubleArrayType::Pointer stereoIntensity = DoubleArrayType::CreateArray(dim * dim, 1, "ModifiedLambertProjection_StereographicProjection");
  stereoIntensity->initializeWithZeros();
  return stereoIntensity;
}