#ifndef FINDFIELDNEIGHBORCAXISMISALIGNMENTS_H_
#define FINDFIELDNEIGHBORCAXISMISALIGNMENTS_H_

#include <vector>
#include <string>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/DataArrays/IDataArray.h"

#include "DREAM3DLib/Common/AbstractFilter.h"
#include "DREAM3DLib/OrientationOps/OrientationOps.h"
#include "DREAM3DLib/OrientationOps/CubicOps.h"
#include "DREAM3DLib/OrientationOps/HexagonalOps.h"
#include "DREAM3DLib/OrientationOps/OrthoRhombicOps.h"
#include "DREAM3DLib/DataContainers/VolumeDataContainer.h"
#include "DREAM3DLib/DataArrays/NeighborList.hpp"

/**
 * @class FindFieldNeighborCAxisMisalignments FindFieldNeighborCAxisMisalignments.h DREAM3DLib/GenericFilters/FindFieldNeighborCAxisMisalignments.h
 * @brief
 * @author Michael A Groeber (AFRL)
 * @date Nov 19, 2011
 * @version 1.0
 */
class DREAM3DLib_EXPORT FindFieldNeighborCAxisMisalignments : public AbstractFilter
{
  public:
    DREAM3D_SHARED_POINTERS(FindFieldNeighborCAxisMisalignments)
    DREAM3D_STATIC_NEW_MACRO(FindFieldNeighborCAxisMisalignments)
    DREAM3D_TYPE_MACRO_SUPER(FindFieldNeighborCAxisMisalignments, AbstractFilter)

    virtual ~FindFieldNeighborCAxisMisalignments();

    DREAM3D_INSTANCE_STRING_PROPERTY(AvgQuatsArrayName)
    DREAM3D_INSTANCE_STRING_PROPERTY(FieldPhasesArrayName)
    //------ Required Ensemble Data
    DREAM3D_INSTANCE_STRING_PROPERTY(CrystalStructuresArrayName)
    DREAM3D_INSTANCE_STRING_PROPERTY(NeighborListArrayName)
    //------ Created Field Data
    DREAM3D_INSTANCE_STRING_PROPERTY(CAxisMisalignmentListArrayName)
    DREAM3D_INSTANCE_STRING_PROPERTY(AvgCAxisMisalignmentsArrayName)

    virtual const std::string getGroupName() { return DREAM3D::FilterGroups::StatisticsFilters; }
    virtual const std::string getSubGroupName() { return DREAM3D::FilterSubGroups::CrystallographicFilters; }
    virtual const std::string getHumanLabel() { return "Find Field Neighbor C-Axis Misalignments"; }

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
    virtual void preflight();

  protected:
    FindFieldNeighborCAxisMisalignments();

  private:
    std::vector<OrientationOps::Pointer> m_OrientationOps;
    CubicOps::Pointer m_CubicOps;
    HexagonalOps::Pointer m_HexOps;
    OrthoRhombicOps::Pointer m_OrthoOps;

    float* m_AvgQuats;
    int32_t* m_FieldPhases;
    NeighborList<int>* m_NeighborList;
    NeighborList<float>* m_CAxisMisalignmentList;
    float* m_AvgCAxisMisalignments;

    unsigned int* m_CrystalStructures;

    void dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles);

    FindFieldNeighborCAxisMisalignments(const FindFieldNeighborCAxisMisalignments&); // Copy Constructor Not Implemented
    void operator=(const FindFieldNeighborCAxisMisalignments&); // Operator '=' Not Implemented
};

#endif /* FINDFIELDNEIGHBORCAXISMISALIGNMENTS_H_ */