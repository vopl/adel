#ifndef _SCOM_SERVICE_HPP_
#define _SCOM_SERVICE_HPP_

#include "utils/options.hpp"


namespace scom
{
	namespace impl
	{
		class Service;
		typedef boost::shared_ptr<Service> ServicePtr;
	}

	class Service
	{
	protected:
		typedef impl::ServicePtr ImplPtr;
		ImplPtr _impl;

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
