///////////////////////////////
// example client application
//////////////////////////////

#include <iostream>
#include <curl/curl.h>
#include <cstring>
#include <tuple>


/**
 * Parse an integer value from argv[i]
 *
 * \param argc total number of arguments
 * \param const char ** argv argument values
 * \param int & i current possition. (the value is incremented by 1
 * \param bool & argError set to true if an error occurred
 * \return the value parsed from argv[i]
 */
static int parseInt(int argc, const char ** argv, int & i, bool & argError)
{
  int ret = 0;
  ++i;
  if(i < argc)
  {
    try
    {
      ret = stoi(std::string(argv[i]));
    }
    catch(std::exception ex)
    {
      std::cerr << "invalid number " << argv[i] << std::endl;
      argError = true;
    }
  }
  else
  {
    std::cerr << "missing argument " << argv[i-1] << " N" << std::endl;
    argError = true;
  }
  return ret;
}

/**
 * Parse a string value from argv[i]
 *
 * \param argc total number of arguments
 * \param const char ** argv argument values
 * \param int & i current possition. (the value is incremented by 1
 * \param bool & argError set to true if an error occurred
 * \return the value parsed from argv[i]
 */
static std::string parseString(int argc, const char ** argv, int & i, bool & argError)
{
  int ret = 0;
  ++i;
  if(i < argc)
  {
    return std::string(argv[i]);
  }
  else
  {
    std::cerr << "missing argument " << argv[i-1] << " N" << std::endl;
    argError = true;
  }
  return std::string("");
}

class DataBuffer
{
public:
  DataBuffer(const std::string & _buffer,
             bool _verbose=false) : buffer(_buffer), uploaded(0), verbose(_verbose) {}

  CURLcode init(CURL *curl) const
  {
    CURLcode res;
    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    if(res != CURLE_OK)
    {
      return res;
    }
    res = curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    if(res != CURLE_OK)
    {
      return res;
    }

    res = curl_easy_setopt(curl, CURLOPT_INFILESIZE, buffer.size());
    if(res != CURLE_OK)
    {
      return res;
    }

    res = curl_easy_setopt(curl, CURLOPT_READFUNCTION, &DataBuffer::read);
    if(res != CURLE_OK)
    {
      return res;
    }
    res = curl_easy_setopt(curl, CURLOPT_READDATA, this);
    if(res != CURLE_OK)
    {
      return res;
    }

    {
      struct curl_slist *chunk = NULL;
      chunk = curl_slist_append(chunk, (std::string("Content-Length: ") + std::to_string(buffer.size())).c_str());
      chunk = curl_slist_append(chunk,"Expect:");
      res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
      if(res != CURLE_OK)
      {
        return res;
      }
    }
    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &DataBuffer::write);
    return res;
  }

  const std::string & getResult() const
  {
    return result;
  }
private:
  static std::size_t read(void *ptr, size_t size, size_t nmemb, void *data)
  {
    auto self = static_cast<DataBuffer*>(data);
    size_t left = self->buffer.size() - self->uploaded;
    size_t max_chunk = size * nmemb;
    size_t retcode = left < max_chunk ? left : max_chunk;
    std::memcpy(ptr, self->buffer.c_str() + self->uploaded, retcode);
    self->uploaded += retcode;
    return retcode;
  }

  static std::size_t write(void *contents, size_t size, size_t nmemb, void *data)
  {
    auto self = static_cast<DataBuffer*>(data);
    size_t realsize = size * nmemb;
    self->result.append((char*) contents, realsize);
    return realsize;
  }

  std::string buffer;
  std::string result;
  size_t uploaded;
  bool verbose;
};

std::tuple<int, std::string> curl_create_session(bool unixSocket,
                                                 const std::string & addr,
                                                 long port,
                                                 bool verbose)
{
  std::tuple<int, std::string, std::string> ret;
  long response_code;
  CURL *curl = curl_easy_init();
  CURLcode res;
  DataBuffer data("");
  std::string baseurl;
  int http_code = 500;
  std::string message;

  if(curl)
  {
    if(unixSocket)
    {
      curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, addr.c_str());
      curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/new");
    }
    else
    {
      curl_easy_setopt(curl, CURLOPT_PORT, port);
      curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
    }
    //curl_easy_setopt(curl, CURLOPT_PUT, 0L);
    //curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, nullptr);
 
    if(verbose)
    {
      std::cout << "curl POST " << addr << std::endl;
    }
    res = data.init(curl);
    if(res != CURLE_OK)
    {
      curl_easy_cleanup(curl);
      std::string msg = "curl_easy_perform() failed:";
      msg+= curl_easy_strerror(res);
      throw std::runtime_error(msg);
    }
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
      curl_easy_cleanup(curl);
      std::string msg = "curl_easy_perform() failed:";
      msg+= curl_easy_strerror(res);
      throw std::runtime_error(msg);
    }
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    http_code = response_code;
    message = data.getResult();
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  else
  {
    throw std::runtime_error("curl init failed");
  }
  return std::make_tuple(http_code, message);
}

std::tuple<int, std::string, std::string> exec_curl(bool unixSocket,
                                                    const std::string & addr,
                                                    long port,
                                                    const std::string & session,
                                                    const std::string & input,
                                                    bool verbose)
{
  std::tuple<int, std::string, std::string> ret;
  long response_code;
  CURL *curl = curl_easy_init();
  CURLcode res;
  DataBuffer data(input, verbose);
  std::string baseurl;
  int http_code = 500;
  std::string next_state;
  std::string message;

  if(curl)
  {
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);
    if(verbose)
    {
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }
    if(unixSocket)
    {
      curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, addr.c_str());
      baseurl = "http://localhost";
    }
    else
    {
      baseurl = addr;
      curl_easy_setopt(curl, CURLOPT_PORT, port);
    }
    std::string url = baseurl + "/" + session;
    if(verbose)
    {
      std::cout << "curl " << url << std::endl;
      std::cout << "data: '" << input << "'" << std::endl;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    res = data.init(curl);
    if(res != CURLE_OK)
    {
      curl_easy_cleanup(curl);
      std::string msg = "curl_easy_perform() failed:";
      msg+= curl_easy_strerror(res);
      throw std::runtime_error(msg);
    }
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
      curl_easy_cleanup(curl);
      std::string msg = "curl_easy_perform() failed:";
      msg+= curl_easy_strerror(res);
      throw std::runtime_error(msg);
    }
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    http_code = response_code;
    message = data.getResult();
    std::size_t pos = message.find('\r');
    if(pos == std::string::npos)
    {
      next_state = message;
      message = "";
    }
    else
    {
      next_state.append(message.begin(), message.begin() +  pos);
      pos++;
      if(pos < message.size() && message[pos] == '\n')
      {
        pos++;
      }
      message.erase(message.begin(),
                    message.begin() + pos);
    }
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  else
  {
    throw std::runtime_error("curl init failed");
  }
  return std::make_tuple(http_code, next_state, message);
}

static bool isError(const std::string & s)
{
  return s == "ERROR";
}

static bool isFinal(const std::string & s)
{
  return isError(s) ||
    (s == "NOT_AUTHENTICATED") ||
    (s == "STATE_AUTHENTICATED");
}


int main(int argc, const char ** argv)
{
  /* set default arguments */
  uint16_t port = 8080;
  bool printHelp = false;
  bool argError = false;
  bool verbose = false;
  bool unixSocket = false;
  bool addrGiven = false;
  std::string addr = "0.0.0.0";

  /* parse and validate arguments  */
  for(int i = 1; i < argc; ++i)
  {
    std::string arg(argv[i]);
    if(arg == "--port" || arg == "-p")
    {
      port = parseInt(argc, argv, i, argError);
    }
    else if(arg == "--socket" || arg == "-s")
    {
      unixSocket = true;
    }
    else if(arg == "--addr" || arg == "-a")
    {
      bool addrError = false;
      addr = parseString(argc, argv, i, addrError);
      if(!addrError)
      {
        addrGiven = true;
      }
      else
      {
        argError = true;
      }
    }
    else if(arg == "--help" || arg == "-h")
    {
      printHelp = true;
    }
    else if(arg == "--verbose" || arg == "-v")
    {
      verbose = true;
    }
    else
    {
      std::cerr << "invalid argument: " << arg << std::endl;
      argError = true;
    }
  }
  if(unixSocket && !addrGiven)
  {
    std::cerr << "option --socket requires --addr arguemnt" << std::endl;
    argError = true;
  }
  if(printHelp || argError)
  {
    std::cout << argv[0] << "[OPTIONS]" << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "--port|-p PORT" << std::endl;
    std::cout << "--addr|-a IPv4" << std::endl;
    std::cout << "--socket|-s" << std::endl;
    std::cout << "--verbose|-v" << std::endl;
    std::cout << "--help|-h" << std::endl;
    if(argError)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }

  try
  {
    curl_global_init(CURL_GLOBAL_ALL);
    int http_code;
    std::string next_state;
    std::string message;
    std::string session;
    std::string answer;
    std::tuple<int, std::string> res;
    std::tie(http_code, session) = curl_create_session(unixSocket,
                                                       addr,
                                                       port,
                                                       verbose);
    if(verbose)
    {
      std::cout << "http_code: " << http_code << " session: " << session << std::endl;
    }
    if(http_code != 200)
    {
      std::string msg("http error ");
      msg+= std::to_string(http_code);
      throw std::runtime_error(msg);
    }
    while(true)
    {
      std::tie(http_code,
               next_state,
               message) = exec_curl(unixSocket,
                                    addr,
                                    port,
                                    session,
                                    answer,
                                    verbose);
      if(verbose)
      {
        std::cout << "http_code: " << http_code << " next_state: '" << next_state << "' message: '" << message << "'" << std::endl;
      }
      answer = "";
      if(next_state == "WAITING")
      {
        std::cout << message << std::endl;
        std::getline(std::cin, answer);
        std::cout << "'" << answer << "'" << std::endl;
      }
      else if(isFinal(next_state))
      {
        break;
      }
    }
    if(isError(next_state))
    {
      throw std::runtime_error("PAM session failed");
    }

  }
  catch(const std::exception & ex)
  {
    std::cerr << "client failed:" << ex.what() << std::endl;
    curl_global_cleanup();
    return 8;
  }
  curl_global_cleanup();
  return 0;
}
