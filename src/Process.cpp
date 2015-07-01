/**
 * @file  Process.cpp
 * @brief Process
 *
 * Class implementation for Process
 *
 * @author     Clay Freeman
 * @date       June 15, 2015
 */

#include <signal.h>
#include <string>
#include <sys/wait.h>
#include <vector>
#include <unistd.h>
#include "../include/Process.hpp"

Process::Process(const std::string& path, const std::vector<std::string>& args,
    const std::vector<std::string>& envs) {
  this->path = path;
  // Add the path to the argument list
  this->argv.push_back(path);
  if (args.empty() == false)
    this->addArgs(args);
  if (envs.empty() == false)
    this->addEnvs(envs);
}

void Process::addArg(const std::string& arg) {
  this->argv.push_back(arg);
}

void Process::addArgs(const std::vector<std::string>& args) {
  for (const std::string& arg : args)
    this->addArg(arg);
}


void Process::addEnv(const std::string& env) {
  this->envp.push_back(env);
}

void Process::addEnvs(const std::vector<std::string>& envs) {
  for (const std::string& env : envs)
    this->addEnv(env);
}

void Process::clearArgs() {
  this->argv.resize(0);
}

void Process::clearEnvs() {
  this->envp.resize(0);
}

void Process::check() {
  if (waitpid(this->pid, NULL, WNOHANG) > 0)
    this->stop();
}

void Process::stop() {
  // Close file descriptors to std{err,in,out}
  if (this->err != -1) {
    close(this->err);
    this->err = -1;
  }
  if (this->in != -1) {
    close(this->in);
    this->in = -1;
  }
  if (this->out != -1) {
    close(this->out);
    this->out = -1;
  }

  // Kill the process
  if (this->pid != -1) {
    kill(this->pid, SIGKILL);
    waitpid(this->pid, NULL, WNOHANG);
    this->pid = -1;
  }
}

/**
 * @brief Start
 *
 * Launches the Process object via fork/exec
 *
 * @return true upon success, false upon failure
 */
bool Process::start() {
  bool retVal = false;
  if (this->pid == -1) {
    // Prepare the pipes to std{err,in,out}
    int epipe[2], ipipe[2], opipe[2];
    pipe(epipe);
    pipe(ipipe);
    pipe(opipe);

    // Reduce complex objects to basic contents
    int i = 0;
    char** argv = new char*[this->argv.size() + 1];
    char** envp = new char*[this->envp.size() + 1];

    // Build an array of pointers to each item
    for (std::string& arg : this->argv)
      argv[i++] = &arg[0];
    // NULL the last pointer and reset i
    argv[i] = NULL;
    i = 0;

    // Build an array of pointers to each item
    for (std::string& env : this->envp)
      envp[i++] = &env[0];
    // NULL the last pointer and reset i
    envp[i] = NULL;

    // Fork the process and prepare for execution
    this->pid = fork();
    if (this->pid == 0) {
      // Prepare the pipes for usage
      close(epipe[0]);
      close(ipipe[1]);
      close(opipe[0]);
      dup2(epipe[1], STDERR_FILENO);
      dup2(ipipe[0], STDIN_FILENO);
      dup2(opipe[1], STDOUT_FILENO);
      close(epipe[1]);
      close(ipipe[0]);
      close(opipe[1]);

      // Create session and process group
      setsid();

      // Execute the command
      execve(this->path.c_str(), argv, envp);

      // Exit if unable to execve(...);
      _exit(1);
    }
    else {
      // Prepare the pipes for usage
      close(epipe[1]);
      close(ipipe[0]);
      close(opipe[1]);
      this->err = epipe[0];
      this->in  = ipipe[1];
      this->out = opipe[0];
    }

    delete[] argv;
    delete[] envp;

    // Update return value
    retVal = (this->pid != -1 ? 1 : 0);
  }
  return retVal;
}
