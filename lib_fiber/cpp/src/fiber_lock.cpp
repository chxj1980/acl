#include "stdafx.hpp"
#include "acl_cpp/stdlib/thread_mutex.hpp"
#include "acl_cpp/stdlib/mbox.hpp"
#include "acl_cpp/stdlib/trigger.hpp"
#include "fiber/fiber.hpp"
#include "fiber/fiber_lock.hpp"

namespace acl {

fiber_mutex::fiber_mutex(bool thread_safe /* = false */,
	unsigned int delay /* = 100 */, int max_retry /* = 0 */)
: delay_(delay)
, max_retry_(max_retry)
{
	if (thread_safe)
		thread_lock_ = new thread_mutex;
	else
		thread_lock_ = NULL;
	lock_ = acl_fiber_mutex_create();
}

fiber_mutex::~fiber_mutex(void)
{
	acl_fiber_mutex_free(lock_);
	delete thread_lock_;
}

bool fiber_mutex::lock(void)
{
	if (thread_lock_)
	{
		int i = 0;
		while (thread_lock_->try_lock() == false)
		{
			(void) fiber::delay(delay_);
			if (max_retry_ > 0 && i++ >= max_retry_)
				return false;
		}
	}

	if (fiber::scheduled())
		acl_fiber_mutex_lock(lock_);
	return true;
}

bool fiber_mutex::trylock(void)
{
	if (thread_lock_ && !thread_lock_->try_lock())
		return false;
	if (fiber::scheduled())
		return acl_fiber_mutex_trylock(lock_) == 0 ? false : true;
	else
		return true;
}

bool fiber_mutex::unlock(void)
{
	if (fiber::scheduled())
		acl_fiber_mutex_unlock(lock_);
	if (thread_lock_ && !thread_lock_->unlock())
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////////

fiber_rwlock::fiber_rwlock(void)
{
	rwlk_ = acl_fiber_rwlock_create();
}

fiber_rwlock::~fiber_rwlock(void)
{
	acl_fiber_rwlock_free(rwlk_);
}

void fiber_rwlock::rlock(void)
{
	acl_fiber_rwlock_rlock(rwlk_);
}

bool fiber_rwlock::tryrlock(void)
{
	return acl_fiber_rwlock_tryrlock(rwlk_) == 0 ? false : true;
}

void fiber_rwlock::runlock(void)
{
	acl_fiber_rwlock_runlock(rwlk_);
}

void fiber_rwlock::wlock(void)
{
	acl_fiber_rwlock_wlock(rwlk_);
}

bool fiber_rwlock::trywlock(void)
{
	return acl_fiber_rwlock_trywlock(rwlk_) == 0 ? false : true;
}

void fiber_rwlock::wunlock(void)
{
	acl_fiber_rwlock_wunlock(rwlk_);
}

} // namespace acl
