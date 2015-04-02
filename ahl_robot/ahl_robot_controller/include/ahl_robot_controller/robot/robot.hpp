#ifndef __AHL_ROBOT_CONTROLLER_ROBOT_HPP
#define __AHL_ROBOT_CONTROLLER_ROBOT_HPP

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <Eigen/Dense>
#include "ahl_robot_controller/robot/joint.hpp"
#include "ahl_robot_controller/robot/link.hpp"

namespace ahl_robot
{
  typedef std::map<std::string, JointPtr> Joints;
  typedef std::map<std::string, LinkPtr> Links;

  class Robot
  {
  public:
    Robot(const std::string& base_link_name);
    ~Robot() {}

    void fix();
    void unfix();
    void addJoint(const std::string& name, const JointPtr& joint);
    void addLink(const std::string& name, const LinkPtr& link);
    void specifyEndEffector(const std::string& name);

    const Joints& getJoints() const;
    const Links& getLinks() const;

    void print();
    void printNameList();

  private:
    bool is_static_;

    std::string base_link_name_;

    Joints joints_;
    Links links_;

    std::vector<std::string> ee_names_;

    Eigen::MatrixXd M_;
    Eigen::MatrixXd J_;
    Eigen::MatrixXd Jv_;
    Eigen::MatrixXd Jw_;
    Eigen::MatrixXd C_;
    Eigen::MatrixXd B_;
    Eigen::MatrixXd G_;
    Eigen::Vector3d g_;
  };

  typedef boost::shared_ptr<Robot> RobotPtr;
}

#endif /* __AHL_ROBOT_CONTROLLER_ROBOT_HPP */
