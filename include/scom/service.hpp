#ifndef _SCOM_SERVICE_HPP_
#define _SCOM_SERVICE_HPP_

#include "utils/options.hpp"
#include <boost/date_time/posix_time/ptime.hpp>

namespace scom
{

	///////////////////////////////////////////////////////////////////////////////
	struct Auth
	{
		boost::int64_t	_id;
		std::string		_secret;
	};

	///////////////////////////////////////////////////////////////////////////////
	struct PageRule
	{
		enum EAccess
		{
			ea_ignore	=0,
			ea_allow	=1,
			ea_deny		=2,
		};

		enum EKind
		{
			ek_null			=0,
			ek_domain		=1,
			ek_path			=2,
			ek_reference	=4,
		};

		std::string		_baseUri;

		int				_kindAndAccess;
		int				_kindAndAccessMin;
		int				_kindAndAccessMax;

		int				_maxAmount;
	};

	///////////////////////////////////////////////////////////////////////////////
	enum EError
	{
		ee_ok,
		ee_internalError,
		ee_badId,
		ee_badStage,

	};

	///////////////////////////////////////////////////////////////////////////////
	struct Status
	{
		boost::posix_time::ptime _destroyTime;

		enum EStage
		{
			es_init		=0,
			es_load		=10,
			es_merge	=20,
			es_report	=30,
			es_fail		=40,
		} _stage;

		bool _isStarted;

		int _workVolume;
		int _workProcessed;
	};

	/////////////////////////////////////////////////////////////////////////////////
	namespace impl
	{
		class Service;
		typedef boost::shared_ptr<Service> ServicePtr;
	}

	/////////////////////////////////////////////////////////////////////////////////
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

		EError create(
			Auth &auth,
			std::string password);

		EError ping(
			Status &status,
			const Auth &auth);

		EError setup(
			const Auth &auth,
			const std::vector<PageRule> &srcRules,
			const std::vector<PageRule> &dstRules);

		EError start(
			const Auth &auth);

		EError stop(
			const Auth &auth);

		/*??? как получать результат
		EError getPages(
			std::vector<std::string> &srcUris,
			std::vector<std::string> &dstUris,
			const Auth &auth);
		*/

		/*???
		EError getResult(
			const Auth &auth,
			const std::vector<std::string> &srcUris,
			const std::vector<std::string> &dstUris);
		*/
		EError destroy(
			const Auth &auth);

	private:
	};
}
#endif
