#include <iostream>

#include "../teenyat.h"

#if defined (_WIN32) | defined (WIN32)
    #include "windows_debugger.hpp"
#else
  #include "linux_debugger.hpp"
#endif


int main(int argc, char* argv[]) {
    os::Process process {};
    auto result = 0;

    if (argc < 2) {
        std::cout << "Needs a child process\n";
        return -1;
    }

    os::SharedMemory memory = os::SharedMemory();
    result = memory.create();
    
    if (result < 0) {
        std::cout << "Error while creating shared memory " << result << "\n";
        return -1;
    }

    result = process.spawn(argv[1], argv + 1);

    if (result != 0) {
        std::cout << "Unable to spawn child process, error code: " << os::error_to_string(result) << "\n";
        return -1;
    }

    while (process.isRunning()) {
        memory.lockTeenyAT();

        {
            teenyat* t = memory.getTeenyAT();
            if (t != nullptr) {
                if (t->initialized) {
                    std::cout << "Yipe!\n";
                }
            }
        }

        memory.releaseTeenyAT();
    }

    if (process.exited()) {
        std::cout << "Debuggee process exited with status " << process.exitCode() << "\n";
    }

    result = memory.release();

    if (result != 0) {
        std::cout << "Unable to close shared memory: " << os::error_to_string(result) << "\n";
        return -1;
    }

    return 0;
}