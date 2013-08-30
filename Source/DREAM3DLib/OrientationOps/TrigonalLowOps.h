/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
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

#ifndef TrigonalLowOPS_H_
#define TrigonalLowOPS_H_

#include "MXA/Common/MXASetGetMacros.h"

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/Common/DataArray.hpp"
#include "DREAM3DLib/OrientationOps/OrientationOps.h"
#include "DREAM3DLib/Math/QuaternionMath.hpp"





/**
 * @class TrigonalLowOps TrigonalLowOps.h DREAM3DLib/Common/OrientationOps/TrigonalLowOps.h
 * @brief
 * @author Michael A. Jackson for BlueQuartz Software
 * @author Michael A. Groeber for USAF Research Laboratory, Materials Lab
 * @date May 5, 2011
 * @version 1.0
 */
class DREAM3DLib_EXPORT TrigonalLowOps : public OrientationOps
{
  public:
    MXA_SHARED_POINTERS(TrigonalLowOps)
    MXA_TYPE_MACRO_SUPER(TrigonalLowOps, OrientationOps)
    MXA_STATIC_NEW_MACRO(TrigonalLowOps)

    TrigonalLowOps();
    virtual ~TrigonalLowOps();

    static const int k_OdfSize = 124416;
    static const int k_MdfSize = 124416;
    static const int k_NumSymQuats = 3;

    virtual bool getHasInversion() { return true; }
    virtual int getODFSize() { return k_OdfSize; }
    virtual int getMDFSize() { return k_MdfSize; }
    virtual int getNumSymOps() { return k_NumSymQuats; }

    virtual float getMisoQuat(QuatF &q1, QuatF &q2, float &n1, float &n2, float &n3);
    virtual void getQuatSymOp(int i, QuatF &q);
    virtual void getRodSymOp(int i, float *r);
    virtual void getMatSymOp(int i, float g[3][3]);
    virtual void getODFFZRod(float &r1, float &r2, float &r3);
    virtual void getMDFFZRod(float &r1, float &r2, float &r3);
    virtual void getNearestQuat(QuatF &q1, QuatF &q2);
    virtual int getMisoBin(float r1, float r2, float r3);
    virtual bool inUnitTriangle(float eta, float chi);
    virtual void determineEulerAngles(int choose, float &synea1, float &synea2, float &synea3);
    virtual void determineRodriguesVector(int choose, float &r1, float &r2, float &r3);
    virtual int getOdfBin(float r1, float r2, float r3);
    virtual void getSchmidFactorAndSS(float loadx, float loady, float loadz, float &schmidfactor, int &slipsys);
    virtual void getmPrime(QuatF &q1, QuatF &q2, float LD[3], float &mPrime);
    virtual void getF1(QuatF &q1, QuatF &q2, float LD[3], bool maxSF, float &F1);
    virtual void getF1spt(QuatF &q1, QuatF &q2, float LD[3], bool maxSF, float &F1spt);
    virtual void getF7(QuatF &q1, QuatF &q2, float LD[3], bool maxSF, float &F7);

    virtual void generateSphereCoordsFromEulers(FloatArrayType *eulers, FloatArrayType *c1, FloatArrayType *c2, FloatArrayType *c3);


    /**
     * @brief generateIPFColor Generates an RGB Color from a Euler Angle and Reference Direction
     * @param eulers Pointer to the 3 component Euler Angle
     * @param refDir Pointer to the 3 Component Reference Direction
     * @param rgb [output] The pointer to store the RGB value
     * @param convertDegrees Are the input angles in Degrees
     */

    virtual void generateIPFColor(double* eulers, double* refDir, uint8_t* rgb, bool convertDegrees);
    /**
     * @brief generateIPFColor Generates an RGB Color from a Euler Angle and Reference Direction
     * @param e0 First component of the Euler Angle
     * @param e1 Second component of the Euler Angle
     * @param e2 Third component of the Euler Angle
     * @param dir0 First component of the Reference Direction
     * @param dir1 Second component of the Reference Direction
     * @param dir2 Third component of the Reference Direction
     * @param rgb [output] The pointer to store the RGB value
     * @param convertDegrees Are the input angles in Degrees
     */
    virtual void generateIPFColor(double e0, double e1, double phi2, double dir0, double dir1, double dir2, uint8_t* rgb, bool convertDegrees);

    /**
     * @brief generateRodriguesColor Generates an RGB Color from a Rodrigues Vector
     * @param r1 First component of the Rodrigues Vector
     * @param r2 Second component of the Rodrigues Vector
     * @param r3 Third component of the Rodrigues Vector
     * @param rgb [output] The pointer to store the RGB value
     */
    virtual void generateRodriguesColor(float r1, float r2, float r3, unsigned char* rgb);

    /**
     * @brief generateMisorientationColor Generates a color based on the method developed by C. Schuh and S. Patala.
     * @param q Quaternion
     * @param refDir The sample reference direction
     * @return A DREAM3D::Rgb value
     */
    virtual DREAM3D::Rgb generateMisorientationColor(const QuatF &q, double* refDir);

    /**
     * @brief generatePoleFigure This method will generate a number of pole figures for this crystal symmetry and the Euler
     * angles that are passed in.
     * @param eulers The Euler Angles to generate the pole figure from.
     * @param imageSize The size in Pixels of the final RGB Image.
     * @param numColors The number of colors to use in the RGB Image. Less colors can give the effect of contouring.
     * @return A std::vector of UInt8ArrayType pointers where each one represents a 2D RGB array that can be used to initialize
     * an image object from other libraries and written out to disk.
     */
    virtual std::vector<UInt8ArrayType::Pointer> generatePoleFigure(PoleFigureConfiguration_t &config);

  protected:
    float _calcMisoQuat(const QuatF quatsym[6], int numsym,
                  QuatF &q1, QuatF &q2,
                  float &n1, float &n2, float &n3);

  private:
    TrigonalLowOps(const TrigonalLowOps&); // Copy Constructor Not Implemented
    void operator=(const TrigonalLowOps&); // Operator '=' Not Implemented
};

#endif /* TrigonalLowOPS_H_ */
