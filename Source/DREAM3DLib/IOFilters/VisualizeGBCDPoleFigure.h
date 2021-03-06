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
#ifndef _VisualizeGBCDPoleFigure_H_
#define _VisualizeGBCDPoleFigure_H_

#include <string>

#include "MXA/Common/MXAEndian.h"

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/DataArrays/IDataArray.h"
#include "DREAM3DLib/OrientationOps/OrientationOps.h"
#include "DREAM3DLib/OrientationOps/CubicOps.h"
#include "DREAM3DLib/OrientationOps/HexagonalOps.h"
#include "DREAM3DLib/OrientationOps/OrthoRhombicOps.h"
#include "DREAM3DLib/SurfaceMeshingFilters/SurfaceMeshFilter.h"


typedef float real;

/**
 * @class VisualizeGBCDPoleFigure VisualizeGBCDPoleFigure.h DREAM3DLib/SurfaceMeshFilters/VisualizeGBCDPoleFigure.h
 * @brief This filter calculates the centroid of each triangle in the surface mesh.
 * @author Michael A. Jackson (BlueQuartz Software)
 * @date Dec 12, 2012
 * @version 1.0
 */
class DREAM3DLib_EXPORT VisualizeGBCDPoleFigure : public SurfaceMeshFilter
{
  public:
    DREAM3D_SHARED_POINTERS(VisualizeGBCDPoleFigure)
    DREAM3D_STATIC_NEW_MACRO(VisualizeGBCDPoleFigure)
    DREAM3D_TYPE_MACRO_SUPER(VisualizeGBCDPoleFigure, SurfaceMeshFilter)

    virtual ~VisualizeGBCDPoleFigure();

    DREAM3D_INSTANCE_STRING_PROPERTY(CrystalStructuresArrayName)
    DREAM3D_INSTANCE_STRING_PROPERTY(GBCDArrayName)
    DREAM3D_INSTANCE_PROPERTY(float, MisAngle)
    DREAM3D_INSTANCE_PROPERTY(FloatVec3Widget_t, MisAxis)
    DREAM3D_INSTANCE_STRING_PROPERTY(OutputFile)
    DREAM3D_INSTANCE_PROPERTY(unsigned int, CrystalStructure)


    /**
    * @brief This returns the group that the filter belonds to. You can select
    * a different group if you want. The string returned here will be displayed
    * in the GUI for the filter
    */
    virtual const std::string getGroupName() { return DREAM3D::FilterGroups::IOFilters; }
    virtual const std::string getSubGroupName() { return DREAM3D::FilterSubGroups::OutputFilters; }

    /**
    * @brief This returns a string that is displayed in the GUI. It should be readable
    * and understandable by humans.
    */
    virtual const std::string getHumanLabel() { return "Write VTK Pole Figure Visualization of GBCD"; }

    /**
    * @brief This method will instantiate all the end user settable options/parameters
    * for this filter
    */
    virtual void setupFilterParameters();

    /**
    * @brief This method will write the options to a file
    * @param writer The writer that is used to write the options to a file
    */
    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);

    /**
    * @brief This method will read the options from a file
    * @param reader The reader that is used to read the options from a file
    */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

    /**
    * @brief Reimplemented from @see AbstractFilter class
    */
    virtual void execute();

    /**
    * @brief This function runs some sanity checks on the DataContainer and inputs
    * in an attempt to ensure the filter can process the inputs.
    */
    virtual void preflight();

    bool verifyPathExists(const std::string &file);


  protected:
    VisualizeGBCDPoleFigure();

    /**
    * @brief Checks for the appropriate parameter values and availability of
    * arrays in the data container
    * @param preflight
    * @param voxels The number of voxels
    * @param fields The number of fields
    * @param ensembles The number of ensembles
    */
    void dataCheckSurfaceMesh(bool preflight, size_t voxels, size_t fields, size_t ensembles);

  private:
    std::vector<OrientationOps::Pointer> m_OrientationOps;

    unsigned int* m_CrystalStructures;
    double* m_GBCD;

    /**
     * @brief This function writes a set of Axis coordinates to that are needed
     * for a Rectilinear Grid based data set.
     * @param f The "C" FILE* pointer to the file being written to.
     * @param axis The name of the Axis that is being written
     * @param type The type of primitive being written (float, int, ...)
     * @param npoints The total number of points in the array
     * @param min The minimum value of the axis
     * @param max The maximum value of the axis
     * @param step The step value between each point on the axis.
     */

    int writeCoords(FILE* f, const char* axis, const char* type, int64_t npoints, float min, float step)
    {
      int err = 0;
      fprintf(f, "%s %lld %s\n", axis, npoints, type);
      float* data = new float[npoints];
      float d;
      for (int idx = 0; idx < npoints; ++idx)
      {
        d = idx * step + min;
        MXA::Endian::FromSystemToBig::convert<float>(d);
        data[idx] = d;
      }
      size_t totalWritten = fwrite(static_cast<void*>(data), sizeof(float), static_cast<size_t>(npoints), f);
      delete[] data;
      if (totalWritten != static_cast<size_t>(npoints) )
      {
        std::cout << "Error Writing Binary VTK Data into file " << std::endl;
        fclose(f);
        return -1;
      }
      return err;
    }


    VisualizeGBCDPoleFigure(const VisualizeGBCDPoleFigure&); // Copy Constructor Not Implemented
    void operator=(const VisualizeGBCDPoleFigure&); // Operator '=' Not Implemented
};

#endif /* _VisualizeGBCDPoleFigure_H_ */
