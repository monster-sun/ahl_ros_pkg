#ifndef __AHL_YOUBOT_SERVER_DISABLED_HPP
#define __AHL_YOUBOT_SERVER_DISABLED_HPP

#include "ahl_youbot_server/state/state.hpp"

namespace ahl_youbot
{

  class Disabled : public State
  {
  public:
    Disabled(const ActionServerPtr& server, const ActionClientBasePtrMap& client)
      : State(server, client) {}

    virtual std::string getState()
    {
      return std::string("Disabled");
    }
  private:

  };

}

#endif /* __AHL_YOUBOT_SERVER_DISABLED_HPP */
