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

#include <set>
#include <algorithm>
#include <yaml-cpp/yaml.h>
#include "ahl_robot/robot/parser.hpp"
#include "ahl_robot/utils/math.hpp"
#include "ahl_robot/definition.hpp"

using namespace ahl_robot;

void Parser::load(const std::string& path, const RobotPtr& robot)
{
  try
  {
    ifs_.open(path.c_str());
    if(ifs_.fail())
    {
      std::stringstream msg;
      msg << "Could not open " << path << ".";
      throw ahl_robot::Exception("ahl_robot::Parser::load", msg.str());
    }
    node_ = YAML::Load(ifs_);

    this->loadRobotInfo(robot);
    this->loadMobility(robot);
    this->loadManipulator(robot);

    this->computeTotalDOF(robot);
  }
  catch(YAML::Exception& e)
  {
    std::stringstream msg;
    msg << "Caught YAML::Exception." << std::endl << e.what();
    throw ahl_robot::Exception("ahl_robot::Parser::load", e.what());
  }
}

void Parser::loadRobotInfo(const RobotPtr& robot)
{
  std::string func = "ahl_robot::Parser::loadRobotInfo";
  Eigen::Vector3d rpy;

  this->checkTag(node_, yaml_tag::ROBOT_NAME, func);
  this->checkTag(node_, yaml_tag::ROBOT_XYZ, func);
  this->checkTag(node_, yaml_tag::ROBOT_RPY, func);
  this->checkTag(node_, yaml_tag::WORLD_FRAME, func);

  if(robot->getName() != node_[yaml_tag::ROBOT_NAME].as<std::string>())
  {
    std::stringstream msg;
    msg << "Robot name is not coincident." << std::endl
        << "  robot name : " << robot->getName() << std::endl
        << "  yaml robot name : " << node_[yaml_tag::ROBOT_NAME].as<std::string>();
    throw ahl_robot::Exception("ahl_robot::Parser::loadRobotInfo", msg.str());
  }

  Eigen::Vector3d p;
  this->loadVector3d(node_, yaml_tag::ROBOT_XYZ, p);
  robot->setPosition(p);

  Eigen::Quaternion<double> q;
  this->loadVector3d(node_, yaml_tag::ROBOT_RPY, rpy);
  math::rpyToQuaternion(rpy, q);
  robot->setOrientation(q);

  if(node_[yaml_tag::MACRO_MANIPULATOR_DOF])
  {
    unsigned int dof = node_[yaml_tag::MACRO_MANIPULATOR_DOF].as<double>();
    robot->setMacroManipulatorDOF(dof);
  }
}

void Parser::loadManipulator(const RobotPtr& robot)
{
  std::string func = "ahl_robot::Parser::loadManipulator";

  this->checkTag(node_, yaml_tag::DIFFERENTIATOR, func);
  YAML::Node node_dif = node_[yaml_tag::DIFFERENTIATOR];
  this->checkTag(node_dif, yaml_tag::DIFFERENTIATOR_UPDATE_RATE, func);
  this->checkTag(node_dif, yaml_tag::DIFFERENTIATOR_CUTOFF_FREQUENCY, func);

  double update_rate = node_dif[yaml_tag::DIFFERENTIATOR_UPDATE_RATE].as<double>();
  double cutoff_frequency = node_dif[yaml_tag::DIFFERENTIATOR_CUTOFF_FREQUENCY].as<double>();

  this->checkTag(node_, yaml_tag::MANIPULATORS, func);

  for(unsigned int i = 0; i < node_[yaml_tag::MANIPULATORS].size(); ++i)
  {
    ManipulatorPtr mnp = ManipulatorPtr(new Manipulator());

    mnp->setDifferentiatorUpdateRate(update_rate);
    mnp->setDifferentiatorCutoffFrequency(cutoff_frequency);

    this->checkTag(node_[yaml_tag::MANIPULATORS][i], yaml_tag::MNP_NAME, func);
    this->checkTag(node_[yaml_tag::MANIPULATORS][i], yaml_tag::LINKS, func);

    mnp->name = node_[yaml_tag::MANIPULATORS][i][yaml_tag::MNP_NAME].as<std::string>();

    YAML::Node node = node_[yaml_tag::MANIPULATORS][i][yaml_tag::LINKS];

    this->loadLinks(node, mnp);

    if(robot->getMobility())
    {
      MobilityPtr mobility = robot->getMobility();
      if(mobility->type == mobility::type::MECANUM_WHEEL)
      {
        mnp->setMobilityType(mobility::MOBILITY_2D);
      }
      else
      {
        mnp->setMobilityType(mobility::FIXED);
      }
    }

    mnp->macro_manipulator_dof = robot->getMacroManipulatorDOF();
    robot->add(mnp);
  }
}

void Parser::loadLinks(const YAML::Node& node, const ManipulatorPtr& mnp)
{
  std::string func = "ahl_robot::Parser::loadLinks";
  std::map<std::string, LinkPtr> link_map;
  std::map<std::string, double> init_q;

  for(unsigned int i = 0; i < node.size(); ++i)
  {
    LinkPtr link = LinkPtr(new Link());

    // name
    this->checkTag(node[i], yaml_tag::LINK_NAME, func);
    link->name = node[i][yaml_tag::LINK_NAME].as<std::string>();

    // joint_type
    this->checkTag(node[i], yaml_tag::JOINT_TYPE, func);
    link->joint_type = node[i][yaml_tag::JOINT_TYPE].as<std::string>();
    if(link->joint_type == joint::PRISMATIC_X ||
       link->joint_type == joint::PRISMATIC_Y ||
       link->joint_type == joint::PRISMATIC_Z)
    {
      link->ep = true;
    }

    // parent
    this->checkTag(node[i], yaml_tag::PARENT, func);
    link->parent = node[i][yaml_tag::PARENT].as<std::string>();

    // T
    Eigen::Vector3d xyz;
    Eigen::Vector3d rpy;
    this->checkTag(node[i], yaml_tag::LINK_XYZ, func);
    this->checkTag(node[i], yaml_tag::LINK_RPY, func);
    this->loadVector3d(node[i], yaml_tag::LINK_XYZ, xyz);
    this->loadVector3d(node[i], yaml_tag::LINK_RPY, rpy);
    math::xyzrpyToTransformationMatrix(xyz, rpy, link->T_org);

    // m, I
    this->checkTag(node[i], yaml_tag::MASS, func);
    this->checkTag(node[i], yaml_tag::INERTIA_MATRIX, func);
    link->m = node[i][yaml_tag::MASS].as<double>();
    this->loadMatrix3d(node[i], yaml_tag::INERTIA_MATRIX, link->I);

    // C
    this->checkTag(node[i], yaml_tag::CENTER_OF_MASS, func);
    this->loadVector3d(node[i], yaml_tag::CENTER_OF_MASS, link->C);

    // q_min, q_max, dq_max, tau_max
    if(link->joint_type == joint::REVOLUTE_X  ||
       link->joint_type == joint::REVOLUTE_Y  ||
       link->joint_type == joint::REVOLUTE_Z  ||
       link->joint_type == joint::PRISMATIC_X ||
       link->joint_type == joint::PRISMATIC_Y ||
       link->joint_type == joint::PRISMATIC_Z)
    {
      this->checkTag(node[i], yaml_tag::Q_MIN, func);
      this->checkTag(node[i], yaml_tag::Q_MAX, func);
      this->checkTag(node[i], yaml_tag::DQ_MAX, func);
      this->checkTag(node[i], yaml_tag::TAU_MAX, func);
      this->checkTag(node[i], yaml_tag::INIT_Q, func);

      link->q_min   = node[i][yaml_tag::Q_MIN].as<double>();
      link->q_max   = node[i][yaml_tag::Q_MAX].as<double>();
      link->dq_max  = node[i][yaml_tag::DQ_MAX].as<double>();
      link->tau_max = node[i][yaml_tag::TAU_MAX].as<double>();
      init_q[link->name] = node[i][yaml_tag::INIT_Q].as<double>();

      if(link->joint_type == joint::REVOLUTE_X)
        link->tf = TransformationPtr(new RevoluteX());
      else if(link->joint_type == joint::REVOLUTE_Y)
        link->tf = TransformationPtr(new RevoluteY());
      else if(link->joint_type == joint::REVOLUTE_Z)
        link->tf = TransformationPtr(new RevoluteZ());
      else if(link->joint_type == joint::PRISMATIC_X)
        link->tf = TransformationPtr(new PrismaticX());
      else if(link->joint_type == joint::PRISMATIC_Y)
        link->tf = TransformationPtr(new PrismaticY());
      else if(link->joint_type == joint::PRISMATIC_Z)
        link->tf = TransformationPtr(new PrismaticZ());
    }
    else if(link->joint_type == joint::FIXED)
    {
      link->tf = TransformationPtr(new Fixed());
    }
    else
    {
      std::stringstream msg;
      msg << "Joint type is invalid." << std::endl
          << "  link : " << link->name << std::endl
          << "  type : " << link->joint_type;
      throw ahl_robot::Exception(func, msg.str());
    }

    link_map[link->name] = link;
  }

  std::string link_name = link_map.begin()->first;
  std::map<std::string, LinkPtr>::iterator it;

  for(it = link_map.begin(); it != link_map.end(); ++it)
  {
    LinkPtr link = it->second;
    std::string parent = link->parent;

    if(parent == frame::WORLD)
      continue;

    if(link_map.find(parent) == link_map.end())
    {
      std::stringstream msg;
      msg << "Parent was not found." << std::endl
          << "  link   : " << link_name << std::endl
          << "  parent : " << parent;
      throw ahl_robot::Exception(func, msg.str());
    }

    link_map[parent]->child = link->name;
  }

  unsigned int op_num = 0;
  std::string end_effector_name = "";
  for(it = link_map.begin(); it != link_map.end(); ++it)
  {
    if(it->second->child == std::string(""))
    {
      ++op_num;
      end_effector_name = it->second->name;
    }
  }

  if(op_num == 0)
  {
    std::stringstream msg;
    msg << "End effector link was not found." << std::endl
        << "At least one joint type should be set to \"operational_point\".";
    throw ahl_robot::Exception(func, msg.str());
  }
  else if(op_num > 1)
  {
    std::stringstream msg;
    msg << "2 or more links's joint types are set to operational_point.";
    throw ahl_robot::Exception(func, msg.str());
  }

  link_name = end_effector_name;
  std::set<std::string> link_set;
  while(true)
  {
    mnp->link.push_back(link_map[link_name]);
    link_set.insert(link_name);
    std::string parent = link_map[link_name]->parent;

    if(parent == frame::WORLD || parent == std::string(""))
    {
      break;
    }
    else if(link_set.find(parent) != link_set.end())
    {
      std::stringstream msg;
      msg << "Link " << parent << " has been already set as parent link.";
      throw ahl_robot::Exception(func, msg.str());
    }

    link_name = parent;
  }

  std::reverse(mnp->link.begin(), mnp->link.end());
  this->setLinkToManipulator(init_q, mnp);
}

void Parser::loadMobility(const RobotPtr& robot)
{
  if(!node_[yaml_tag::MOBILITY])
  {
    std::cout << "Robot doesn't have mobility." << std::endl;
    return;
  }

  std::string func = "Parser::loadMobility";

  YAML::Node node_mob = node_[yaml_tag::MOBILITY];

  this->checkTag(node_mob, yaml_tag::MOBILITY_UPDATE_RATE, func);
  this->checkTag(node_mob, yaml_tag::MOBILITY_TYPE, func);
  this->checkTag(node_mob, yaml_tag::MOBILITY_CUTOFF_FREQUENCY_BASE, func);
  this->checkTag(node_mob, yaml_tag::MOBILITY_CUTOFF_FREQUENCY_WHEEL, func);
  this->checkTag(node_mob, yaml_tag::MOBILITY_COMMAND, func);
  this->checkTag(node_mob, yaml_tag::MOBILITY_JOINTS, func);
  this->checkTag(node_mob, yaml_tag::TREAD_WIDTH, func);
  this->checkTag(node_mob, yaml_tag::WHEEL_BASE, func);
  this->checkTag(node_mob, yaml_tag::WHEEL_RADIUS, func);

  MobilityPtr mobility = MobilityPtr(new Mobility());
  mobility->update_rate      = node_mob[yaml_tag::MOBILITY_UPDATE_RATE].as<double>();
  mobility->type             = node_mob[yaml_tag::MOBILITY_TYPE].as<std::string>();
  mobility->cutoff_frequency_base = node_mob[yaml_tag::MOBILITY_CUTOFF_FREQUENCY_BASE].as<double>();
  mobility->cutoff_frequency_wheel = node_mob[yaml_tag::MOBILITY_CUTOFF_FREQUENCY_WHEEL].as<double>();
  mobility->command          = node_mob[yaml_tag::MOBILITY_COMMAND].as<std::string>();
  mobility->tread_width      = node_mob[yaml_tag::TREAD_WIDTH].as<double>();
  mobility->wheel_base       = node_mob[yaml_tag::WHEEL_BASE].as<double>();
  mobility->wheel_radius     = node_mob[yaml_tag::WHEEL_RADIUS].as<double>();

  YAML::Node node_joints = node_mob[yaml_tag::MOBILITY_JOINTS];
  for(unsigned int i = 0; i < node_joints.size(); ++i)
  {
    this->checkTag(node_joints[i], yaml_tag::MOBILITY_JOINT_NAME, func);
    mobility->joint_name.push_back(node_joints[i][yaml_tag::MOBILITY_JOINT_NAME].as<std::string>());
  }

  mobility->init();
  robot->addMobility(mobility);
}

void Parser::loadVector3d(const YAML::Node& node, const std::string& tag, Eigen::Vector3d& v)
{
  if(node[tag].size() != 3)
  {
    std::stringstream msg;
    msg << tag << " has invalid size data." << std::endl
        << "  size : " << node[tag].size();
    throw ahl_robot::Exception("ahl_robot::Parser::loadVector3d", msg.str());
  }

  for(unsigned int i = 0; i < node[tag].size(); ++i)
  {
    v.coeffRef(i) = node[tag][i].as<double>();
  }
}

void Parser::loadMatrix3d(const YAML::Node& node, const std::string& tag, Eigen::Matrix3d& m)
{
  if(node[tag].size() != 9)
  {
    std::stringstream msg;
    msg << tag << " has invalid size data." << std::endl
        << "  size : " << node[tag].size();
    throw ahl_robot::Exception("ahl_robot::Parser::loadVector3d", msg.str());
  }

  for(unsigned int i = 0; i < 3; ++i)
  {
    m.coeffRef(i, 0) = node[tag][3 * i].as<double>();
    m.coeffRef(i, 1) = node[tag][3 * i + 1].as<double>();
    m.coeffRef(i, 2) = node[tag][3 * i + 2].as<double>();
  }
}

void Parser::setLinkToManipulator(const std::map<std::string, double>& init_q, const ManipulatorPtr& mnp)
{
  unsigned int dof = 0;

  // Push back link and count dof
  for(unsigned int i = 0; i < mnp->link.size(); ++i)
  {
    if(mnp->link[i]->joint_type == joint::REVOLUTE_X  ||
       mnp->link[i]->joint_type == joint::REVOLUTE_Y  ||
       mnp->link[i]->joint_type == joint::REVOLUTE_Z  ||
       mnp->link[i]->joint_type == joint::PRISMATIC_X ||
       mnp->link[i]->joint_type == joint::PRISMATIC_Y ||
       mnp->link[i]->joint_type == joint::PRISMATIC_Z)
    {
      ++dof;
    }
  }

  if(mnp->link.size() != dof + 1)
  {
    std::stringstream msg;
    msg << "mnp->link.size() != dof + 1" << std::endl
        << "The number of link should be DOF + 1,"
        << "including virtual link which is attached to operational point frame and following the last link." << std::endl
        << "  mnp : " << mnp->name << std::endl
        << "  mnp->link.size : " << mnp->link.size() << std::endl
        << "  dof : " << dof;
    throw ahl_robot::Exception("ahl_robot::Parser::setLinkToManipulator", msg.str());
  }

  // Initialize joint angles
  // Initialize q, dq, T, x, xr,
  Eigen::VectorXd q = Eigen::VectorXd::Zero(dof);

  int idx = 0;
  for(unsigned int i = 0; i < mnp->link.size(); ++i)
  {
    if(init_q.find(mnp->link[i]->name) == init_q.end())
      continue;

    q.coeffRef(idx) = const_cast< std::map<std::string, double>& >(init_q)[mnp->link[i]->name];
    ++idx;
  }

  mnp->T.resize(mnp->link.size());
  for(unsigned int i = 0; i < mnp->T.size(); ++i)
  {
    mnp->T[i] = mnp->link[i]->T_org;
  }
  mnp->init(dof, q);
  mnp->update(q);
}

void Parser::computeTotalDOF(const RobotPtr& robot)
{
  unsigned int macro_dof = robot->getMacroManipulatorDOF();
  unsigned int dof = macro_dof;

  MapManipulatorPtr::iterator it;
  MapManipulatorPtr mnp = robot->getManipulator();

  for(it = mnp.begin(); it != mnp.end(); ++it)
  {
    dof += it->second->dof - macro_dof;
  }

  robot->setDOF(dof);
}

void Parser::checkTag(const YAML::Node& node, const std::string& tag, const std::string& func)
{
  if(!node[tag])
  {
    std::stringstream msg;
    msg << "Could not find tag : " << tag;
    throw ahl_robot::Exception(func, msg.str());
  }
}

