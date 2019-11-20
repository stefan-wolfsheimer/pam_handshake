#pragma once
#include "ipam_client.h"
#include <utility>
#include <condition_variable>
#include <thread>

namespace PamHandshake
{
  class Server;

  class Session : public IPamClient
  {
  public:
    enum class State
      {
        Running,
        Ready,
        Waiting,
        Answer,
        Next,
        Authenticated,
        NotAuthenticated
      };
    Session(Server * _server);
    virtual ~Session();
    virtual void promptEchoOn(const char * msg, pam_response_t * resp) override;
    virtual void promptEchoOff(const char * msg, pam_response_t * resp) override;
    virtual void errorMsg(const char * msg) override;
    virtual void textInfo(const char * msg) override;

    static const char *  StateToString(const State & s);
    std::pair<State, std::string> pull(const char * answer,
                                       std::size_t len);

  private:
    mutable std::mutex mutex;
    Server * server;
    bool verbose;
    std::pair<State, std::string> nextMessage;
    std::string nextAnswer;
    std::condition_variable cv;
    std::thread t;
    void worker();
    inline void transition(State s);
  }; 

}
