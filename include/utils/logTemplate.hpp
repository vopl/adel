//#include <log4cplus/logger.h>

template<bool Stub>
struct LoggerHolder
{
	static log4cplus::Logger _instance;
};
template <bool Stub> log4cplus::Logger LoggerHolder<Stub>::_instance;

#define LOG_INSTANCE (LoggerHolder<true>::_instance)

#define FLOG(msg)	LOG4CPLUS_FATAL(LOG_INSTANCE, msg)
#define ELOG(msg)	LOG4CPLUS_ERROR(LOG_INSTANCE, msg)
#define WLOG(msg)	LOG4CPLUS_WARN (LOG_INSTANCE, msg)
#define ILOG(msg)	LOG4CPLUS_INFO (LOG_INSTANCE, msg)
#define DLOG(msg)	LOG4CPLUS_DEBUG(LOG_INSTANCE, msg)
#define TLOG(msg)	LOG4CPLUS_TRACE(LOG_INSTANCE, msg)


