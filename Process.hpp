/**
 * @file  Process.hpp
 * @brief Process
 *
 * Class definition for Process
 *
 * @author     Clay Freeman
 * @date       June 15, 2015
 */

#ifndef _PROCESS_HPP
#define _PROCESS_HPP

#include <string>
#include <sys/types.h>
#include <vector>

class Process {
  private:
    // Storage for file descriptors to std{err,in,out}
    int                      err = -1;
    int                      in  = -1;
    int                      out = -1;
    // Storage for the process ID
    pid_t                    pid = -1;
    // Storage for the path to the executable
    std::string              path{};
    // Storage for arguments and environment variables
    std::vector<std::string> argv{};
    std::vector<std::string> envp{};
  public:
    // Construct with path and optional argv and envs
    Process       (const std::string& path,
      const std::vector<std::string>& argv = {},
      const std::vector<std::string>& envs = {});

    // Argument and environment mutators
    void addArg   (const std::string& arg);
    void addArgs  (const std::vector<std::string>& argv);
    void addEnv   (const std::string& env);
    void addEnvs  (const std::vector<std::string>& envs);
    void clearArgs();
    void clearEnvs();

    void check();

    // Getters for pipes
    int   getErr() const { return this->err; }
    int   getIn()  const { return this->in;  }
    int   getOut() const { return this->out; }
    pid_t getPID() const { return this->pid; }

    // Process control mutators
    void stop     ();
    bool start    ();
};

#endif
