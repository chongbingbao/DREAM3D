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

#include "BetaOps.h"

#include "DREAM3DLib/Math/DREAM3DMath.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BetaOps::~BetaOps()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BetaOps::BetaOps()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int BetaOps::calculateParameters(std::vector<float> &data, FloatArrayType::Pointer outputs)
{
  int err = 0;
  float avg = 0;
  float stddev = 0;
  float alpha = 0;
  float beta = 0;
  if(data.size() > 1)
  {
	  for(size_t i = 0; i < data.size(); i++)
	  {
		avg = avg + data[i];
	  }
	  avg = avg/float(data.size());
	  for(size_t i = 0; i < data.size(); i++)
	  {
		stddev = stddev + ((avg - data[i])*(avg - data[i]));
	  }
	  stddev = stddev/float(data.size());
	  alpha = avg * (((avg * (1 - avg)) / stddev) - 1);
	  beta = (1 - avg) * (((avg * (1 - avg)) / stddev) - 1);
  }
  else
  {
	  alpha = 0;
	  beta = 0;
  }
  outputs->SetValue(0, alpha);
  outputs->SetValue(1, beta);
  return err;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int BetaOps::calculateCorrelatedParameters(std::vector<std::vector<float> > &data, VectorOfFloatArray outputs)
{
  int err = 0;
  float avg = 0;
  float stddev = 0;
  float alpha = 0;
  float beta = 0;
  for(size_t i = 0; i < data.size(); i++)
  {
	  avg = 0;
	  stddev = 0;
	  if(data[i].size() > 1)
	  {
		  for(size_t j = 0; j < data[i].size(); j++)
		  {
			avg = avg + data[i][j];
		  }
		  avg = avg/float(data[i].size());
		  for(size_t j = 0; j < data[i].size(); j++)
		  {
			stddev = stddev + ((avg - data[i][j])*(avg - data[i][j]));
		  }
		  stddev = stddev/float(data[i].size());
		  alpha = avg * (((avg * (1 - avg)) / stddev) - 1);
		  beta = (1 - avg) * (((avg * (1 - avg)) / stddev) - 1);
	  }
	  else
	  {
		  alpha = 0;
		  beta = 0;
	  }
	  outputs[0]->SetValue(i, alpha);
	  outputs[1]->SetValue(i, beta);
  }
  return err;
}

