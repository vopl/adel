#ifndef _SCOM_SERVICE_HPP_
#define _SCOM_SERVICE_HPP_

#include "utils/options.hpp"


namespace scom
{
	class Service
	{
	public:
		Service(utils::OptionsPtr optionsPtr);
		virtual ~Service();

		static utils::OptionsPtr prepareOptions(const char *prefix);

		void start();
		void stop();

	private:
	};
}
#endif
