#ifndef __AHL_YOUBOT_SERVER_STATE_HPP
#define __AHL_YOUBOT_SERVER_STATE_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <std_srvs/Empty.h>
#include "ahl_youbot_server/ahl_robot_srvs.hpp"
#include "ahl_youbot_server/ahl_robot_actions.hpp"
#include "ahl_youbot_server/action/action_server.hpp"

namespace ahl_youbot
{

  class State
  {
  public:
    enum Type
    {
      DISABLED,
      READY,
      FLOAT,
      MOVE,
      LOCK,
      ALARM,
      STATE_NUM,
    };

    State(const ActionServerPtr& action_server);
    virtual ~State() {}

    virtual std::string getState()
    {
      return std::string("N/A");
    }

    virtual bool callCancel(
      std_srvs::Empty::Request& req,
      std_srvs::Empty::Response& res);
    virtual bool callFloat(
      ahl_robot_srvs::Float::Request& req,
      ahl_robot_srvs::Float::Response& res);
    virtual bool callSetJoint(
      ahl_robot_srvs::SetJoint::Request& req,
      ahl_robot_srvs::SetJoint::Response& res);
    virtual bool callJointSpaceControl(
      ahl_robot_srvs::JointSpaceControl::Request& req,
      ahl_robot_srvs::JointSpaceControl::Response& res);
    virtual bool callTaskSpaceControl(
      ahl_robot_srvs::TaskSpaceControl::Request& req,
      ahl_robot_srvs::TaskSpaceControl::Response& res);
    virtual bool callTaskSpaceHybridControl(
      ahl_robot_srvs::TaskSpaceHybridControl::Request& req,
      ahl_robot_srvs::TaskSpaceHybridControl::Response& res);

    virtual const ActionServerPtr& getActionServer() const
    {
      return action_server_;
    };

  protected:
    virtual void convertServiceToAction(
      ahl_robot_srvs::Float::Request& req,
      ahl_robot_actions::FloatGoal& goal);
    virtual void convertServiceToAction(
      ahl_robot_srvs::SetJoint::Request& req,
      ahl_robot_actions::SetJointGoal& goal);
    virtual void convertServiceToAction(
      ahl_robot_srvs::JointSpaceControl::Request& req,
      ahl_robot_actions::JointSpaceControlGoal& goal);
    virtual void convertServiceToAction(
      ahl_robot_srvs::TaskSpaceControl::Request& req,
      ahl_robot_actions::TaskSpaceControlGoal& goal);
    virtual void convertServiceToAction(
      ahl_robot_srvs::TaskSpaceHybridControl::Request& req,
      ahl_robot_actions::TaskSpaceHybridControlGoal& goal);

  private:
    ActionServerPtr action_server_;
  };

  typedef boost::shared_ptr<State> StatePtr;
}

#endif /* __AHL_YOUBOT_SERVER_STATE_HPP */