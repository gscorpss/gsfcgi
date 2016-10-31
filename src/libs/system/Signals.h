#ifndef GS_SYSTEM_SIGNALS_H
#define GS_SYSTEM_SIGNALS_H
#include <vector>
#include <signal.h>

namespace gssystem
{

class Signals
{
public:
    Signals(const std::vector<int>& signals);

    int wait();

private:
    sigset_t signal_set;
};

} // namespace gssystem

#endif //GS_SYSTEM_SIGNALS_H
