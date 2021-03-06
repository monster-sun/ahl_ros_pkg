/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014, Daichi Yoshikawa
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Daichi Yoshikawa nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Daichi Yoshikawa
 *
 *********************************************************************/

#include "neural_network/core/sigmoid_table.hpp"
#include "neural_network/exceptions.hpp"

using namespace nn;

SigmoidTable::SigmoidTable(double gain, unsigned long resolution)
  : gain_(gain)
{
  if(gain_ < 0.0)
  {
    std::stringstream msg;
    msg << "Gain of sigmoid function (using table) should be bigger than zero." << std::endl
        << "        gain : " << gain_;
    nn::Exception("SigmoidTable::SigmoidTable", msg.str());
  }

  step_ = (INPUT_MAX - INPUT_MIN) / resolution;
  this->createTable(resolution);
}

void SigmoidTable::createTable(unsigned long resolution)
{
  double x = INPUT_MIN;
  for(unsigned long i = 0; i < resolution; ++i)
  {
    table_.push_back(sigmoid(x));
    x += step_;
  }
}

double SigmoidTable::sigmoid(double input)
{
  return 1.0 / (1.0 + std::exp(-gain_ * input));
}
