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

#ifndef SegmentBetaGrains_H_
#define SegmentBetaGrains_H_

#include <vector>
#include <string>


#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/Common/IDataArray.h"

#include "DREAM3DLib/Common/AbstractFilter.h"
#include "DREAM3DLib/DataContainers/VoxelDataContainer.h"
#include "DREAM3DLib/Math/OrientationMath.h"

#include "DREAM3DLib/ReconstructionFilters/SegmentGrains.h"

/**
 * @class SegmentBetaGrains SegmentBetaGrains.h DREAM3DLib/ReconstructionFilters/SegmentBetaGrains.h
 * @brief
 * @author
 * @date Nov 19, 2011
 * @version 1.0
 */
class DREAM3DLib_EXPORT SegmentBetaGrains : public SegmentGrains
{
  public:
    DREAM3D_SHARED_POINTERS(SegmentBetaGrains)
    DREAM3D_STATIC_NEW_MACRO(SegmentBetaGrains)
    DREAM3D_TYPE_MACRO_SUPER(SegmentBetaGrains, AbstractFilter)

    virtual ~SegmentBetaGrains();

    //------ Required Cell Data
    DREAM3D_INSTANCE_STRING_PROPERTY(GoodVoxelsArrayName)
    DREAM3D_INSTANCE_STRING_PROPERTY(CellPhasesArrayName)
    DREAM3D_INSTANCE_STRING_PROPERTY(QuatsArrayName)
    //------ Created Cell Data
    DREAM3D_INSTANCE_STRING_PROPERTY(GrainIdsArrayName)
    //------ Created Field Data
    DREAM3D_INSTANCE_STRING_PROPERTY(ActiveArrayName)
    //------ Required Ensemble Data
    DREAM3D_INSTANCE_STRING_PROPERTY(CrystalStructuresArrayName)

    DREAM3D_INSTANCE_PROPERTY(float, MisorientationTolerance)
    DREAM3D_INSTANCE_PROPERTY(float, AxisTolerance)
    DREAM3D_INSTANCE_PROPERTY(float, AngleTolerance)
	DREAM3D_INSTANCE_PROPERTY(bool, RandomizeGrainIds)

    virtual const std::string getGroupName() { return DREAM3D::FilterGroups::ReconstructionFilters; }
	virtual const std::string getSubGroupName() {return DREAM3D::FilterSubGroups::SegmentationFilters;}
    virtual const std::string getHumanLabel() { return "Segment Fields (Prior Beta Grains)"; }

    virtual void setupFilterParameters();
    /**
    * @brief This method will write the options to a file
    * @param writer The writer that is used to write the options to a file
    */
    virtual void writeFilterParameters(AbstractFilterParametersWriter* writer);
    
    /**
    * @brief This method will read the options from a file
    * @param reader The reader that is used to read the options from a file
    */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader);

    /**
     * @brief Reimplemented from @see AbstractFilter class
     */
    virtual void execute();
    virtual void preflight();

    virtual int getSeed(size_t gnum);
    virtual bool determineGrouping(int referencepoint, int neighborpoint, size_t gnum);

  protected:
    SegmentBetaGrains();

  private:
    std::vector<OrientationMath*> m_OrientationOps;
    OrientationMath::Pointer m_CubicOps;
    OrientationMath::Pointer m_HexOps;
    OrientationMath::Pointer m_OrthoOps;

    int32_t* m_GrainIds;
    float* m_Quats;
    int32_t* m_CellPhases;
    bool* m_GoodVoxels;
    bool* m_Active;

    unsigned int* m_CrystalStructures;

	int check_for_burgers(float betaQuat[5], float alphaQuat[5]);
    void dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles);


    SegmentBetaGrains(const SegmentBetaGrains&); // Copy Constructor Not Implemented
    void operator=(const SegmentBetaGrains&); // Operator '=' Not Implemented
};

#endif /* SegmentBetaGrains_H_ */
