/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2015, Daichi Yoshikawa
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

#ifndef __AHL_RRT_RANDOM_TREE_HPP
#define __AHL_RRT_RANDOM_TREE_HPP

#include <ros/ros.h>
#include "ahl_rrt/tree/random_tree_base.hpp"

#define ENABLE_VISUALIZATION

namespace ahl_rrt
{

  class RandomTree : public RandomTreeBase
  {
  public:
    virtual void init(const ParamPtr& param, const Eigen::VectorXd& init_x);
    virtual void build(const Eigen::VectorXd& dst_x);
  private:
    VertexPtr getNearestVertex(const Eigen::VectorXd& rand, const VertexPtr& root);
    VertexPtr getNewVertex(const Eigen::VectorXd& rand, const VertexPtr& nearest);
    bool reachedToGoal(const Eigen::VectorXd& new_x, const Eigen::VectorXd& dst_x);

    ParamPtr param_;
    VertexPtr root_;

#ifdef ENABLE_VISUALIZATION
  private:
    void publish(const VertexPtr& parent, const VertexPtr& child);
    ros::Publisher pub_;
#endif
  };

}

#endif /* __AHL_RRT_RANDOM_TREE_HPP */
