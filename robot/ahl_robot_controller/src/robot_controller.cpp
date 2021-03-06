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

#include <ahl_robot/definition.hpp>
#include "ahl_robot_controller/exception.hpp"
#include "ahl_robot_controller/robot_controller.hpp"
#include "ahl_robot_controller/mobility/mecanum_wheel.hpp"
#include "ahl_robot_controller/param.hpp"

using namespace ahl_ctrl;

RobotController::RobotController()
  : dof_(0)
{
}

void RobotController::init(const ahl_robot::RobotPtr& robot)
{
  robot_ = robot;
  param_ = ParamBasePtr(new Param(robot_));

  for(unsigned int i = 0; i < robot->getManipulatorName().size(); ++i)
  {
    mnp_.push_back(robot->getManipulator(robot->getManipulatorName()[i]));
  }
  mobility_ = robot->getMobility();

  if(mobility_)
  {
    if(mobility_->type == ahl_robot::mobility::type::MECANUM_WHEEL)
    {
      mobility_controller_ = MobilityControllerPtr(new MecanumWheel(mobility_, param_));
    }
    else
    {
      std::stringstream msg;
      msg << "Mobility type : " << mobility_->type << " is not supported.";
      throw ahl_ctrl::Exception("RobotController::init", msg.str());
    }
  }

  dof_ = robot_->getDOF();
  multi_task_ = MultiTaskPtr(new MultiTask(robot_));
}

void RobotController::init(const ahl_robot::RobotPtr& robot, const ParamBasePtr& param)
{
  robot_ = robot;
  param_ = param;

  for(unsigned int i = 0; i < robot->getManipulatorName().size(); ++i)
  {
    mnp_.push_back(robot->getManipulator(robot->getManipulatorName()[i]));
  }
  mobility_ = robot->getMobility();

  if(mobility_)
  {
    if(mobility_->type == ahl_robot::mobility::type::MECANUM_WHEEL)
    {
      mobility_controller_ = MobilityControllerPtr(new MecanumWheel(mobility_, param));
    }
    else
    {
      std::stringstream msg;
      msg << "Mobility type : " << mobility_->type << " is not supported.";
      throw ahl_ctrl::Exception("RobotController::init", msg.str());
    }
  }

  dof_ = robot_->getDOF();
  multi_task_ = MultiTaskPtr(new MultiTask(robot_));
}

void RobotController::addTask(const TaskPtr& task, int priority)
{
  task->setParam(param_);
  multi_task_->addTask(task, priority);
}

void RobotController::clearTask()
{
  multi_task_->clear();
}

void RobotController::updateModel()
{
  multi_task_->updateModel();
}

void RobotController::computeGeneralizedForce(Eigen::VectorXd& tau)
{
  multi_task_->computeGeneralizedForce(tau);

  unsigned int macro_dof = robot_->getMacroManipulatorDOF();

  for(unsigned int i = 0; i < macro_dof; ++i)
  {
    if(tau[i] > (*mnp_.begin())->link[i]->tau_max)
    {
      tau[i] = (*mnp_.begin())->link[i]->tau_max;
    }
    else if(tau[i] < -(*mnp_.begin())->link[i]->tau_max)
    {
      tau[i] = -(*mnp_.begin())->link[i]->tau_max;
    }
  }

  unsigned int idx_offset = macro_dof;

  for(unsigned int i = 0; i < mnp_.size(); ++i)
  {
    unsigned int mini_dof = mnp_[i]->dof - macro_dof;

    for(unsigned int j = 0; j < mini_dof; ++j)
    {
      if(tau[j + idx_offset] > mnp_[i]->link[j + macro_dof]->tau_max)
      {
        tau[j + idx_offset] = mnp_[i]->link[j + macro_dof]->tau_max;
      }
      else if(tau[j + idx_offset] < -mnp_[i]->link[j + macro_dof]->tau_max)
      {
        tau[j + idx_offset] = -mnp_[i]->link[j + macro_dof]->tau_max;
      }
    }

    idx_offset += mini_dof;
  }
}

void RobotController::computeBaseVelocityFromTorque(
  const Eigen::VectorXd& tau, Eigen::VectorXd& v_base, int mobility_dof)
{
  if(!mobility_controller_)
  {
    std::stringstream msg;
    msg << "Mobility controller is NULL." << std::endl
        << "Please check yaml file of robot including " << robot_->getName();
    throw ahl_ctrl::Exception("RobotController::computeBaseVelocityFromTorque", msg.str());
  }

  if((*mnp_.begin())->M.rows() >= mobility_dof && (*mnp_.begin())->M.cols() >= mobility_dof)
  {
    mobility_controller_->computeBaseVelocityFromTorque((*mnp_.begin())->M.block(0, 0, mobility_dof, mobility_dof), tau, v_base);
  }
  else
  {
    std::stringstream msg;
    msg << "Invalid combination of mobility dof and size of mass matrix." << std::endl
        << "  mobility_dof : " << mobility_dof << std::endl
        << "  M.rows : " << (*mnp_.begin())->M.rows() << std::endl
        << "  M.cols : " << (*mnp_.begin())->M.cols();
    throw ahl_ctrl::Exception("RobotController::computeBaseVelocityFromTorque", msg.str());
  }
}

void RobotController::computeWheelVelocityFromBaseVelocity(
  const Eigen::VectorXd& v_base, Eigen::VectorXd& v_wheel)
{
  if(!mobility_controller_)
  {
    std::stringstream msg;
    msg << "Mobility controller is NULL." << std::endl
        << "Please check yaml file of robot including " << robot_->getName();
    throw ahl_ctrl::Exception("RobotController::computeWheelVelocityFromBaseVelocity", msg.str());
  }

  mobility_controller_->computeWheelVelocityFromBaseVelocity(v_base, v_wheel);
}

void RobotController::computeWheelTorqueFromBaseVelocity(
  const Eigen::VectorXd& v_base, Eigen::VectorXd& tau_wheel)
{
  if(!mobility_controller_)
  {
    std::stringstream msg;
    msg << "Mobility controller is NULL." << std::endl
        << "Please check yaml file of robot including " << robot_->getName();
    throw ahl_ctrl::Exception("RobotController::computeWheelTorqueFromBaseVelocity", msg.str());
  }

  mobility_controller_->computeWheelTorqueFromBaseVelocity(v_base, tau_wheel);
}
