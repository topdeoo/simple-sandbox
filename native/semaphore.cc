#include <time.h>
#include <system_error>

#include <errno.h>
#include <sys/mman.h>

#include "utils.h"
#include "semaphore.h"

using std::system_error;

PosixSemaphore::PosixSemaphore(bool interprocess, int initialValue)
{
    auto result = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    EnsureNot(result, MAP_FAILED, "Mapping memory for semaphore");

    m_semaphore = reinterpret_cast<sem_t *>(result);
    Ensure(sem_init(m_semaphore, interprocess ? 1 : 0, initialValue));
}

void PosixSemaphore::Post()
{
    Ensure(sem_post(m_semaphore));
}

void PosixSemaphore::Wait()
{
    Ensure(sem_wait(m_semaphore));
}

bool PosixSemaphore::TryWait()
{
    int result = sem_trywait(m_semaphore);
    if (result == 0)
    {
        return true;
    }
    else if (errno == EAGAIN)
    {
        return false;
    }
    else
    {
        throw system_error(errno, std::system_category(), "Failed to wait for semaphore");
    }
}

bool PosixSemaphore::TimedWait(time_t sec, long nsec)
{
    timespec tWait;
    tWait.tv_sec = sec;
    tWait.tv_nsec = nsec;
    int result = sem_timedwait(m_semaphore, &tWait);
    if (result == 0)
    {
        return true;
    }
    else if (errno == ETIMEDOUT)
    {
        return false;
    }
    else
    {
        throw system_error(errno, std::system_category(), "Failed to wait for semaphore");
    }
}

PosixSemaphore::~PosixSemaphore()
{
    // Shouldn't throw in a destructor.
    (void)munmap(m_semaphore, sizeof(sem_t));
    (void)sem_destroy(m_semaphore);
}