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
	/*
способ обработки
	не загружать
	загружать, брать ссылки
	загружать, брать слова

простые
	домен/уровень (от до)
	для заданного урла с путем, уровень пути (от, до)
	рег выражение на урл

цепочечные
	урл и ссылочность от него на столько то уровней (от до)

к каждому правилу - количество
	 */
	struct PageRule
	{
		enum EAccess
		{
			ea_null		=0x00,
			ea_ignore	=0x01,
			ea_useLinks	=0x02,
			ea_useWords	=0x04,
			ea_mask		=0xff,
		};

		enum EKind
		{
			ek_null			=0x0000,
			ek_domain		=0x0100,
			ek_regex		=0x0200,
			ek_path			=0x0300,
			ek_reference	=0x0400,
			ek_mask			=0xff00,
		};

		std::string		_value;

		int				_kindAndAccess;
		int				_kindAndAccessMin;
		int				_kindAndAccessMax;
		int				_amount;
	};

	///////////////////////////////////////////////////////////////////////////////
	enum EError
	{
		ee_ok,
		ee_internalError,
		ee_badId,
		ee_badStage,
		ee_badDomain,
		ee_badRegex,
		ee_badUri,
		ee_badRange,
		ee_badAmount,
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
			const std::vector<PageRule> &rules);

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
