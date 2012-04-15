#ifndef _PGC_PCH_HPP_
#define _PGC_PCH_HPP_


#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/cstdint.hpp>

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include <set>
#include <deque>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

#include <libpq-fe.h>

#include "utils/variant.hpp"
#include "utils/fixEndian.hpp"
#include "utils/ntoa.hpp"
#include "utils/julian.hpp"
#include "utils/implAccess.hpp"
#include "utils/aton.hpp"

#include "async/service.hpp"
#include "async/future.hpp"
#include "async/mutex.hpp"
#include "async/futureWaiter.hpp"


#endif
