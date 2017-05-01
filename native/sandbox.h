#pragma once

#include <string>
#include <vector>
#include <unistd.h>
#include <boost/filesystem.hpp>

enum RunStatus {
    EXITED = 0, // App exited normally.
    SIGNALED = 01, // App is kill by some signal.
};

struct ExecutionResult
{
    int Status;
    // If exited, this is the exit code; if signaled, this is the signal number.
    int Code;
};

struct SandboxParameter
{
    // Time limit is done by querying cpuacct cgroup every 100ms. This is done in the js code.
    // int timeLimit;

    // Memory limit in bytes.
    // -1 for no limit.
    int64_t memoryLimit;
    // The maximum child process count created by the executable. Typically less than 10. -1 for no limit.
    int processLimit;
    // Redirect stdin / stdout before chrooting.
    // Useful when debugging; 
    // You can use `socat -d -d pty,raw,echo=0 -` to create a device in /dev/pts and redirect stdio to that pts.
    // And then execute a shell (/bin/sh) in the sandbox to debug problems.
    bool redirectBeforeChroot;
    // Mount `/proc`?
    bool mountProc;
    // This directory will be chrooted into before running our binary.
    // Make sure this is not writable by `nobody` user!
    // Make sure there are empty directory `/sandbox/binary` and `/sandbox/working` in the chroot directory.
    boost::filesystem::path chrootDirectory;
    // This directory will be mounted to /sandbox/binary.
    // Also, make sure it is not writable.
    // Currently no special measure is taken to ensure readonly to this directory,
    // but this may be forced readonly in the future.
    boost::filesystem::path binaryDirectory;
    // This directory will be mounted to /sandbox/working,
    // and be chdired into before running our executable.
    // Please make sure this directory is writable if you want your executable to give out any output.
    //
    // You can copy your input data into this directory.
    // Please make your input data not writable by `nobody` user, or it may be overwritten.
    boost::filesystem::path workingDirectory;
    // This executable is the file that will be run.
    // You may specify your native binary (or file with #! interpreter)
    // located in your binaryDirectory, such as `/sandbox/binary/a.out`,
    // or it may be an interpreter such as `/usr/bin/python` (this must be in your chroot filesystem)
    //
    // Tip: if you want to run a series of command,
    // You can create a .sh script and execute it.
    std::string executablePath;
    // These are the parameters passed to the guest executable.
    std::vector<std::string> executableParameters;
    std::vector<std::string> environmentVariables;
    // This is the input file that will be redirected to the executable as Standard Input.
    // Note that if you specify a relative path, it will be relative to the `/sandbox/working` directory.
    // Or you may specify an absoulete path (though this is usually not not the case).
    // If left empty, no stdin will be redirected.
    std::string stdinRedirection;
    // Stdout redirection, same as above.
    std::string stdoutRedirection;
    // Same as above.
    std::string stderrRedirection;

    // The user the guest executable will be run as. 
    // Please don't share users among sandboxes, 
    // i.e. use a DISTINCTIVE user for EACH sandbox. The process limit is on each user, not each session.
    std::string userName;

    // The cgroup name of the sandbox. Must be unique.
    std::string cgroupName;
};

pid_t StartSandbox(const SandboxParameter &/*, std::function<void(int)>*/);
 
ExecutionResult SBWaitForProcess(pid_t pid);