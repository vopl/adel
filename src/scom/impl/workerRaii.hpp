#ifndef _SCOM_IMPL_WORKERRAII_HPP_
#define _SCOM_IMPL_WORKERRAII_HPP_

namespace scom { namespace impl
{

	class WorkerRaii
	{
		async::Mutex	&_mtx;
		size_t			&_numWorkers;
		async::Event	&_evtDone;
	public:
		WorkerRaii(async::Mutex &mtx, size_t &numWorkers, async::Event &evtDone)
			: _mtx(mtx)
			, _numWorkers(numWorkers)
			, _evtDone(evtDone)
		{
		}

		~WorkerRaii()
		{
			{
				async::Mutex::ScopedLock sl(_mtx);
				_numWorkers--;
			}
			_evtDone.set(true);
		}
	};

}}
#endif
