#ifndef _SCOM_LOG_HPP_
#define _SCOM_LOG_HPP_

#include "utils/options.hpp"
#include "utils/logInitializer.hpp"

#include <log4cplus/logger.h>
#include <boost/system/error_code.hpp>

namespace scom
{
	#include "utils/logTemplate.hpp"

	inline utils::OptionsPtr prepareOptionsLog()
	{
		utils::OptionsPtr options(new utils::Options("scom.log"));
		utils::LogInitializer::fillOptions(*options, utils::LogInitializer::_defaultConsole|utils::LogInitializer::_defaultFile);
		return options;
	}
	inline void initLog(utils::OptionsPtr options)
	{
		utils::LogInitializer::init(LOG_INSTANCE, *options);
	}

}

#endif
