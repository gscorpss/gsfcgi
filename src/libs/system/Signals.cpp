#include "Signals.h"
#include <signal.h>
#include "Exceptions.h"

namespace gssystem
{

Signals::Signals (const std::vector< int >& signals)
{
    sigemptyset (&signal_set);
    for (int signal : signals)
        sigaddset (&signal_set, signal);
    int status = sigprocmask (SIG_BLOCK, &signal_set, NULL);
    if (status != 0)
        throw gssystem::ErrnoException ("Set signal mask");
}

int Signals::wait()
{
    int signal = 0;
    sigwait (&signal_set, &signal);
    return signal;
}

} // namespace gssystem
