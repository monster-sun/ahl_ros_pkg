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

#ifndef __AHL_ROBOT_CONTROLLER_PARAM_HPP
#define __AHL_ROBOT_CONTROLLER_PARAM_HPP

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <Eigen/Dense>
#include <dynamic_reconfigure/server.h>
//#include "ahl_robot_controller/param_base.hpp"
#include "ahl_robot_controller/ParamConfig.h"

namespace ahl_ctrl
{

  class Param// : public ParamBase
  {
  public:
    Param(unsigned int dof);

    const Eigen::MatrixXd& getKpJoint()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return Kp_joint_;
    }

    const Eigen::MatrixXd& getKvJoint()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return Kv_joint_;
    }

    const Eigen::MatrixXd& getKpTask()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return Kp_task_;
    }

    const Eigen::MatrixXd& getKvTask()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return Kv_task_;
    }

    const Eigen::MatrixXd& getKvDamp()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return Kv_damp_;
    }

    const Eigen::MatrixXd& getKpLimit()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return Kp_limit_;
    }

    const Eigen::MatrixXd& getKvLimit()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return Kv_limit_;
    }

    double getPosErrorMax()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return pos_error_max_;
    }

    double getOriErrorMax()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return ori_error_max_;
    }

    const Eigen::Vector3d& getG()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return g_;
    }

    double getKpWheel()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return kp_wheel_;
    }

    double getKvWheel()
    {
      boost::mutex::scoped_lock lock(mutex_);
      return kv_wheel_;
    }

  private:
    void update(ahl_robot_controller::ParamConfig& config, uint32_t level);
    boost::mutex mutex_;

    dynamic_reconfigure::Server<ahl_robot_controller::ParamConfig> server_;
    dynamic_reconfigure::Server<ahl_robot_controller::ParamConfig>::CallbackType f_;

    unsigned int dof_;

    double kp_;
    double kv_;
    double kv_damp_;
    double kp_limit_;
    double kv_limit_;

    Eigen::MatrixXd Kp_joint_;
    Eigen::MatrixXd Kv_joint_;
    Eigen::MatrixXd Kp_task_;
    Eigen::MatrixXd Kv_task_;
    Eigen::MatrixXd Kv_damp_;
    Eigen::MatrixXd Kp_limit_;
    Eigen::MatrixXd Kv_limit_;
    double pos_error_max_;
    double ori_error_max_;
    double kp_wheel_;
    double kv_wheel_;
    Eigen::Vector3d g_;
  };

  typedef boost::shared_ptr<Param> ParamPtr;
}

#endif /* __AHL_ROBOT_CONTROLLER_PARAM_HPP */
