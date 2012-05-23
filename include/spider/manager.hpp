#ifndef _SPIDER_MANAGER_HPP_
#define _SPIDER_MANAGER_HPP_

#include "utils/options.hpp"
#include "async/service.hpp"
#include <boost/date_time/posix_time/posix_time_duration.hpp>

namespace spider
{
	class Manager
	{
	public:
		Manager(utils::OptionsPtr optionsPtr);
		virtual ~Manager();

		static utils::OptionsPtr prepareOptions(const char *prefix);

		async::Service asrv();
		void run();

	private:
		async::Service _asrv;

		//options
		size_t _numWorkers;
		size_t _fiber_stackSize;
		size_t _fiber_maxAmount;

		boost::posix_time::time_duration _fiber_gc_period;
		size_t _fiber_gc_spare_all;
		size_t _fiber_gc_spare_idle;

	private:
		void onSignal(const boost::system::error_code& error, int signal_number);
	};
}
#endif
