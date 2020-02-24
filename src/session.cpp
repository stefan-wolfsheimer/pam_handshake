#include "session.h"
#include "server.h"
#include "pam_conversation.h"
#include <functional>
#include <iostream>
#include <security/pam_appl.h>
#include <string.h>
#include <stdlib.h>

using namespace PamHandshake;

Session::Session(Server * _server)
  : server(_server),
    verbose(_server->isVerbose()),
    nextMessage(std::make_pair(State::Running, "")),
    lastTime(std::time(nullptr)),
    t(std::bind(&Session::worker, this))
{
}

Session::~Session()
{
  t.join();
}

void Session::promptEchoOn(const char * msg, pam_response_t * resp)
{
  std::unique_lock<std::mutex> lk(mutex);
  if(verbose)
  {
    std::cout << "promptEchoOn \"" << msg << "\"" << std::endl;
    std::cout << "waiting for State::Ready" << std::endl;
  }
  cv.wait(lk, std::bind(&Session::statePredicate, this, State::Ready));
  if(nextMessage.first == State::Ready)
  {
    transition(State::Waiting);
    nextMessage.second = msg;

    lk.unlock();
    cv.notify_one();

    lk.lock();
    if(verbose)
    {
      std::cout << "waiting for State::Answer" << std::endl;
    }

    cv.wait(lk, std::bind(&Session::statePredicate, this, State::Answer));
    if(nextMessage.first == State::Answer)
    {
      transition(State::Next);
      resp->resp = (char*)malloc(nextAnswer.size() + 1);
      resp->resp = strdup(nextAnswer.c_str());
      if(verbose)
      {
        std::cout << "resp: " << resp->resp << std::endl;
      }
      lk.unlock();
      cv.notify_one();
    }
  }
}

void Session::promptEchoOff(const char * msg, pam_response_t * resp)
{
  std::unique_lock<std::mutex> lk(mutex);
  if(verbose)
  {
    std::cout << "promptEchoOff \"" << msg << "\"" << std::endl;
    std::cout << "waiting for State::Ready" << std::endl;
  }
  cv.wait(lk, std::bind(&Session::statePredicate, this, State::Ready));
  if(nextMessage.first == State::Ready)
  {
    transition(State::WaitingPw);
    nextMessage.second = msg;
    lk.unlock();
    cv.notify_one();

    lk.lock();
    if(verbose)
    {
      std::cout << "waiting for State::Answer" << std::endl;
    }
    cv.wait(lk, std::bind(&Session::statePredicate, this, State::Answer));
    if(nextMessage.first == State::Answer)
    {
      transition(State::Next);
      resp->resp = (char*)malloc(nextAnswer.size() + 1);
      resp->resp = strdup(nextAnswer.c_str());
      if(verbose)
      {
        std::cout << "resp: *****" << std::endl;
      }
      lk.unlock();
      cv.notify_one();
    }
  }
}

void Session::errorMsg(const char * msg)
{
  std::unique_lock<std::mutex> lk(mutex);
  if(verbose)
  {
    std::cout << "errorMsg \"" << msg << "\"" << std::endl;
    std::cout << "waiting for State::Ready" << std::endl;
  }
  cv.wait(lk, [this]{ return nextMessage.first == State::Ready;});
  if(nextMessage.first == State::Ready)
  {
    transition(State::Next);
    nextMessage.second = msg;
    lk.unlock();
    cv.notify_one();
  }
}

void Session::textInfo(const char * msg)
{
  std::unique_lock<std::mutex> lk(mutex);
  if(verbose)
  {
    std::cout << "textInfo " << msg << "\"" << std::endl;
    std::cout << "waiting for State::Ready" << std::endl;
  }
  cv.wait(lk, std::bind(&Session::statePredicate, this, State::Ready));
  if(nextMessage.first == State::Ready)
  {
    transition(State::Next);
    nextMessage.second = msg;
    lk.unlock();
    cv.notify_one();
  }
}

bool Session::canceled()
{
  std::lock_guard<std::mutex> lk(mutex);
  return (nextMessage.first == State::Timeout ||
          nextMessage.first == State::Error);
}

void Session::cancel()
{
  std::lock_guard<std::mutex> lk(mutex);
  lastTime = time(nullptr);
  nextMessage.first = State::Timeout;
}

void Session::refresh()
{
  std::lock_guard<std::mutex> lk(mutex);
  lastTime = time(nullptr);
}

std::time_t Session::getLastTime() const
{
  std::lock_guard<std::mutex> lk(mutex);
  return lastTime;
}

const char * Session::StateToString(const Session::State & s)
{
  switch(s)
  {
  case State::Running: return "RUNNING";
  case State::Ready: return "READY";
  case State::Waiting: return "WAITING";
  case State::WaitingPw: return "WAITING_PW";
  case State::Answer: return "ANSWER";
  case State::Next:  return "NEXT";
  case State::Error:  return "ERROR";
  case State::Timeout:  return "TIMEOUT";
  case State::Authenticated: return "STATE_AUTHENTICATED";
  case State::NotAuthenticated: return "NOT_AUTHENTICATED";
  }
}

Session::State Session::getState() const
{
  std::lock_guard<std::mutex> lk(mutex);
  auto s = nextMessage.first;
  return s;
}

std::pair<Session::State, std::string> Session::pull(const char * answer, std::size_t len)
{
  std::pair<State, std::string> ret;
  {
    std::lock_guard<std::mutex> lk(mutex);
    lastTime = time(nullptr);
    if(nextMessage.first > State::Next)
    {
      return nextMessage;
    }
    else if(nextMessage.first == State::Running ||
            nextMessage.first == State::Next)
    {
      transition(State::Ready);
    }
    else if(nextMessage.first == State::Waiting ||
            nextMessage.first == State::WaitingPw)
    {
      nextAnswer = std::string(answer, len);
      transition(State::Answer);
    }
    else if(nextMessage.first == State::Ready)
    {
      // should not end here
      transition(State::Error);
    }
  }
  cv.notify_one();
  {
    std::unique_lock<std::mutex> lk(mutex);
    if(verbose)
    {
      std::cout << "waiting for State::Waiting || State::WaitingPw || State::Next || >State::Next" << std::endl;
    }
    
    cv.wait(lk, [this]{ return nextMessage.first == State::Waiting ||
                               nextMessage.first == State::WaitingPw ||
                               nextMessage.first == State::Next ||
                               nextMessage.first >  State::Next; });
    ret = nextMessage;
  }
  return ret;
}

void Session::worker()
{
  bool result = false;
  bool err = false;
  try
  {
    if(server->hasConversationProgram())
    {
      result = pam_auth_check_wrapper(server->getConversationProgram(),
                                      server->getPamStackName(),
                                      *this,
                                      verbose);
    }
    else
    {
      result = pam_auth_check(server->getPamStackName(), *this, verbose);
    }
  }
  catch(const std::exception & ex) 
  {
    if(verbose)
    {
      std::cerr << ex.what() << std::endl;
    }
    err = true;
  }
  std::unique_lock<std::mutex> lk(mutex);
  cv.wait(lk, std::bind(&Session::statePredicate, this, State::Ready));
  if(nextMessage.first == State::Ready)
  {
    if(err)
    {
      transition(State::Error, false);
    }
    else if(result)
    {
      transition(State::Authenticated);
    }
    else
    {
      transition(State::NotAuthenticated);
    }
    lk.unlock();
    cv.notify_one();
  }
}

inline void Session::transition(State s, bool clean_str)
{
  if(verbose)
  {
    std::cout << "State " << Session::StateToString(nextMessage.first);
    nextMessage.first = s;
    std::cout << " --> " << Session::StateToString(nextMessage.first) << std::endl;
  }
  else
  {
    nextMessage.first = s;
  }
  if(clean_str)
  {
    nextMessage.second = "";
  }
}

inline bool Session::statePredicate(State s)
{
  return ( nextMessage.first == s ||
           nextMessage.first == State::Error ||
           nextMessage.first == State::Timeout );

}
