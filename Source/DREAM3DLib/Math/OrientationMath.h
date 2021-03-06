/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Michael A. Groeber (US Air Force Research Laboratory)
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
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef _OrientationMath_H_
#define _OrientationMath_H_

#include <vector>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/Math/DREAM3DMath.h"
#include "DREAM3DLib/Math/QuaternionMath.hpp"
#include "DREAM3DLib/Math/MatrixMath.h"
#include "DREAM3DLib/Utilities/DREAM3DRandom.h"

// Here the order of multiplication is q1*q2 not q2*q1
#define MULT_QUAT(q1, q2, out)\
  out[1] = q2[1] * q1[4] + q2[4] * q1[1] + q2[3] * q1[2] - q2[2] * q1[3];\
  out[2] = q2[2] * q1[4] + q2[4] * q1[2] + q2[1] * q1[3] - q2[3] * q1[1];\
  out[3] = q2[3] * q1[4] + q2[4] * q1[3] + q2[2] * q1[1] - q2[1] * q1[2];\
  out[4] = q2[4] * q1[4] - q2[1] * q1[1] - q2[2] * q1[2] - q2[3] * q1[3];\


/*
 * @class OrientationMath OrientationMath.h DREAM3DLib/Common/OrientationMath.h
 * @brief This class performs Crystallographic Misorientation Calculations using either the Bunge convention or the
 * Rodregues Convention.
 * @author Michael A. Jackson (BlueQuartz Software)
 * @author Michael A. Groeber (US Air Force Research Laboratory)
 * @date Feb 19, 2011
 * @version 1.0
 */
class DREAM3DLib_EXPORT OrientationMath
{
  public:
    DREAM3D_SHARED_POINTERS(OrientationMath)
    DREAM3D_TYPE_MACRO(OrientationMath)

    virtual ~OrientationMath();


    static void AxisAngletoHomochoric(float w, float n1, float n2, float n3, float &r1, float &r2, float &r3);
    static void AxisAngletoRod(float w, float n1, float n2, float n3, float &r1, float &r2, float &r3);
    static void AxisAngletoQuat(float w, float n1, float n2, float n3, QuatF &q);
    static void AxisAngletoMat(float w, float n1, float n2, float n3, float g[3][3]);

    static void ChangeAxisReferenceFrame(QuatF &q, float &n1, float &n2, float &n3);

    static void HomochorictoRod(float &r1, float &r2, float &r3);

    static void RodtoAxisAngle(float r1, float r2, float r3, float &w, float &n1, float &n2, float &n3);
    static void RodtoQuat(QuatF &q, float r1, float r2, float r3);
    static void RodtoHomochoric(float &r1, float &r2, float &r3);
    static void RodtoEuler(float r1, float r2, float r3, float &ea1, float &ea2, float &ea3);


    static void QuattoAxisAngle(QuatF &q, float &w, float &n1, float &n2, float &n3);
    static void QuattoMat(QuatF &q, float g[3][3]);
    static void QuattoRod(QuatF &q, float &r1, float &r2, float &r3);
    static void QuattoEuler(QuatF &q, float &ea1, float &ea2, float &ea3);


    static void EulertoQuat(QuatF &q, float ea1, float ea2, float ea3);

    /**
     * @brief EulertoMat This function creates a [3x3] Row major Rotation Matrix based on the
     * set of rotations described by Bunge, which is the following:
     * @li Rotation 1: rotate (counterclockwise) about the crystal z-axis through phi 1
     * @li Rotation 2: rotate (counterclockwise) about the rotated crystal x-axis through PHI
     * @li Rotation 3: rotate (counterclockwise) about the crystal z-axis through phi 2
     *
     * The rotations represent a <b>PASSIVE</b> rotation
     *
     * @param phi1 Radians: Valid range is 0 <= phi1 < 2Pi
     * @param phi Radians: Valid range is 0 <= PHI < Pi
     * @param phi2 Radians: Valid Range is 0 <= phi2 < 2Pi
     * @param g [OUTPUT] The [3x3] Rotation Matrix
     */
    static void EulerToMat(float phi1, float phi, float phi2, float g[3][3]);


    /**
     * @brief MatToEuler The function takes in a 3x3 Rotation Matrix generated with the @see EulerToMat function and
     * generates an equivelent Bunge convention Euler Angle. The programmer should note that the value of the Euler angle returned from this
     * method may NOT match the input Euler Angle used to generate the Rotation Matrix if the initial Euler Angle is outside
     * the normal range of Euler Angles
     * @param g [INPUT] Rotation Matrix
     * @param phi1 [OUTPUT] The Bunge phi1 angle
     * @param Phi [OUTPUT] The Bunge PHI angle
     * @param phi2 [OUTPUT] The Bunge phi2 angle
     */
    static void MatToEuler(float g[3][3], float &phi1, float &Phi, float &phi2);

    /**
     * @brief EulertoRod This function generates a Rodregues vector based on an input Bunge Euler angle.
     * @param r1 [OUTPUT]
     * @param r2 [OUTPUT]
     * @param r3 [OUTPUT]
     * @param phi1 [INPUT]
     * @param phi [INPUT]
     * @param phi2 [INPUT]
     */
    static void EulerToRod( const float phi1,  const float phi,  const float phi2, float &r1,  float &r2,  float &r3);

    static float MatrixMisorientation(float g1[3][3], float g2[3][3]);

    static void MultiplyQuaternionVector(QuatF &inQuat, float inVec[3], float outVec[3]);

    /**
    * @brief Does a passive Rotation of the coordinate system defined by  w,n1,n2,n3 of the point xyz using Quaternion
    * math to perform the rotation
    * @param angle
    * @param xAxis
    * @param yAxis
    * @param zAxis
    * @param x
    * @param y
    * @param z
    * @return The returned Quaternion is a "pure" Vector quaterion with a Zero value for the scalar (w) portion
    */
    static QuatF PassiveRotation(float angle, float xAxis, float yAxis, float zAxis, float x, float y, float z);

    /**
    * @brief Does an active Rotation of (xyz) in using the Axis-Angle w,n1,n2,n3 using Quaternion math to perform
    * the rotation
    * @param angle
    * @param xAxis
    * @param yAxis
    * @param zAxis
    * @param x
    * @param y
    * @param z
    * @return The returned Quaternion is a "pure" Vector quaterion with a Zero value for the scalar (w) portion
    */
    static QuatF ActiveRotation(float angle, float xAxis, float yAxis, float zAxis, float x, float y, float z);


  protected:
    OrientationMath();


  private:
    OrientationMath(const OrientationMath&); // Copy Constructor Not Implemented
    void operator=(const OrientationMath&); // Operator '=' Not Implemented
};

#endif /* _OrientationMath_H_ */
