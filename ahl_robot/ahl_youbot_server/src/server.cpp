#include "ahl_youbot_server/server.hpp"
#include "ahl_youbot_server/state/alarm.hpp"
#include "ahl_youbot_server/state/disabled.hpp"
#include "ahl_youbot_server/state/float.hpp"
#include "ahl_youbot_server/state/lock.hpp"
#include "ahl_youbot_server/state/move.hpp"
#include "ahl_youbot_server/state/ready.hpp"

using namespace ahl_youbot;

Server::Server()
{
  state_[State::ALARM]    = StatePtr(new Alarm());
  state_[State::DISABLED] = StatePtr(new Disabled());
  state_[State::FLOAT]    = StatePtr(new Float());
  state_[State::LOCK]     = StatePtr(new Lock());
  state_[State::MOVE]     = StatePtr(new Move());
  state_[State::READY]    = StatePtr(new Ready());

  state_type_ = State::DISABLED;

  ros::NodeHandle local_nh("ahl_youbot_server");

  server_float_ = local_nh.advertiseService(
    "command/float", &Server::floatCB, this);
  server_set_joint_ = local_nh.advertiseService(
    "command/set_joint", &Server::setJointCB, this);
  server_joint_space_control_ = local_nh.advertiseService(
    "command/joint_space_control", &Server::jointSpaceControlCB, this);
  server_task_space_control_ = local_nh.advertiseService(
    "command/task_space_control", &Server::taskSpaceControlCB, this);
  server_task_space_hybrid_control_ = local_nh.advertiseService(
    "command/task_space_hybrid_control", &Server::taskSpaceHybridControlCB, this);

  action_server_ = ActionServerPtr(new ActionServer());
}

bool Server::floatCB(
  ahl_robot_srvs::Float::Request& req,
  ahl_robot_srvs::Float::Response& res)
{
  return state_[state_type_]->callFloat(req, res);
}

bool Server::setJointCB(
  ahl_robot_srvs::SetJoint::Request& req,
  ahl_robot_srvs::SetJoint::Response& res)
{
  return state_[state_type_]->callSetJoint(req, res);
}

bool Server::jointSpaceControlCB(
  ahl_robot_srvs::JointSpaceControl::Request& req,
  ahl_robot_srvs::JointSpaceControl::Response& res)
{
  return state_[state_type_]->callJointSpaceControl(req, res);
}

bool Server::taskSpaceControlCB(
  ahl_robot_srvs::TaskSpaceControl::Request& req,
  ahl_robot_srvs::TaskSpaceControl::Response& res)
{
  return state_[state_type_]->callTaskSpaceControl(req, res);
}

bool Server::taskSpaceHybridControlCB(
  ahl_robot_srvs::TaskSpaceHybridControl::Request& req,
  ahl_robot_srvs::TaskSpaceHybridControl::Response& res)
{
  return state_[state_type_]->callTaskSpaceHybridControl(req, res);
}

