#ifndef _PGC_RESULT_HPP_
#define _PGC_RESULT_HPP_

#include "utils/variant.hpp"

namespace pgc
{
	//////////////////////////////////////////////////////////////////////////
	enum EResultStatus
	{
		ersCommandOk,
		ersTuplesOk,
		ersError,
	};

	//////////////////////////////////////////////////////////////////////////
	namespace impl
	{
		class Result;
		typedef boost::shared_ptr<Result> ResultPtr;
	}

	//////////////////////////////////////////////////////////////////////////
	class Result
	{

	protected:
		typedef impl::ResultPtr ImplPtr;
		ImplPtr	_impl;

	public:
		//Result();

		EResultStatus status();
		const char *errorMsg();
		const char *errorCode();

		size_t cmdRows();
		size_t rows();
		size_t cols();

		size_t colIdx(const char *colName);
		const char *colName(size_t columnIdx);
		utils::Variant::EType colType(size_t columnIdx);
		bool isNull(size_t columnIdx, size_t rowIdx);

		bool fetch(utils::Variant &v, size_t columnIdx, size_t rowIdx);
		bool fetchRowList(utils::Variant &v, size_t rowIdx=0);
		bool fetchRowMap(utils::Variant &v, size_t rowIdx=0);
		bool fetchRowsList(utils::Variant &v, size_t rowBeginIdx=0, size_t rowEndIdx=(size_t)-1);
		bool fetchRowsMap(utils::Variant &v, size_t rowBeginIdx=0, size_t rowEndIdx=(size_t)-1);
		bool fetchRowList(utils::Variant &v, const std::deque<size_t> &colIndices, size_t rowIdx=0);
		bool fetchRowMap(utils::Variant &v, const std::deque<size_t> &colIndices, size_t rowIdx=0);
		bool fetchRowsList(utils::Variant &v, const std::deque<size_t> &colIndices, size_t rowBeginIdx=0, size_t rowEndIdx=(size_t)-1);
		bool fetchRowsMap(utils::Variant &v, const std::deque<size_t> &colIndices, size_t rowBeginIdx=0, size_t rowEndIdx=(size_t)-1);
		bool fetchColumn(utils::Variant &v, size_t columnIdx, size_t rowBeginIdx=0, size_t rowEndIdx=(size_t)-1);

		boost::int32_t fetchInt32(size_t columnIdx, size_t rowIdx);
		boost::uint32_t fetchUInt32(size_t columnIdx, size_t rowIdx);
		std::string fetchString(size_t columnIdx, size_t rowIdx);
	};
}
#endif
