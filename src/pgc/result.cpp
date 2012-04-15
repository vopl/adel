#include "pch.hpp"
#include "pgc/result.hpp"
#include "impl/result.hpp"

namespace pgc
{
// 	//////////////////////////////////////////////////////////////////////////
// 	Result::Result()
// 	{
// 	}


	//////////////////////////////////////////////////////////////////////////
	EResultStatus Result::status()
	{
		return _impl->status();
	}

	//////////////////////////////////////////////////////////////////////////
	const char *Result::errorMsg()
	{
		return _impl->errorMsg();
	}

	//////////////////////////////////////////////////////////////////////////
	const char *Result::errorCode()
	{
		return _impl->errorCode();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t Result::cmdRows()
	{
		return _impl->cmdRows();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t Result::rows()
	{
		return _impl->rows();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t Result::cols()
	{
		return _impl->cols();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t Result::colIdx(const char *colName)
	{
		return _impl->colIdx(colName);
	}

	//////////////////////////////////////////////////////////////////////////
	const char *Result::colName(size_t columnIdx)
	{
		return _impl->colName(columnIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	utils::Variant::EType Result::colType(size_t columnIdx)
	{
		return _impl->colType(columnIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::isNull(size_t columnIdx, size_t rowIdx)
	{
		return _impl->isNull(columnIdx, rowIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetch(utils::Variant &v, size_t columnIdx, size_t rowIdx)
	{
		return _impl->fetch(v, columnIdx, rowIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetchRowList(utils::Variant &v, size_t rowIdx)
	{
		return _impl->fetchRowList(v, rowIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetchRowMap(utils::Variant &v, size_t rowIdx)
	{
		return _impl->fetchRowMap(v, rowIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetchRowsList(utils::Variant &v, size_t rowBeginIdx, size_t rowEndIdx)
	{
		return _impl->fetchRowsList(v, rowBeginIdx, rowEndIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetchRowsMap(utils::Variant &v, size_t rowBeginIdx, size_t rowEndIdx)
	{
		return _impl->fetchRowsMap(v, rowBeginIdx, rowEndIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetchRowList(utils::Variant &v, const std::deque<size_t> &colIndices, size_t rowIdx)
	{
		return _impl->fetchRowList(v, colIndices, rowIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetchRowMap(utils::Variant &v, const std::deque<size_t> &colIndices, size_t rowIdx)
	{
		return _impl->fetchRowMap(v, colIndices, rowIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetchRowsList(utils::Variant &v, const std::deque<size_t> &colIndices, size_t rowBeginIdx, size_t rowEndIdx)
	{
		return _impl->fetchRowsList(v, colIndices, rowBeginIdx, rowEndIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetchRowsMap(utils::Variant &v, const std::deque<size_t> &colIndices, size_t rowBeginIdx, size_t rowEndIdx)
	{
		return _impl->fetchRowsMap(v, colIndices, rowBeginIdx, rowEndIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	bool Result::fetchColumn(utils::Variant &v, size_t columnIdx, size_t rowBeginIdx, size_t rowEndIdx)
	{
		return _impl->fetchColumn(v, columnIdx, rowBeginIdx, rowEndIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::int32_t Result::fetchInt32(size_t columnIdx, size_t rowIdx)
	{
		return _impl->fetchInt32(columnIdx, rowIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::uint32_t Result::fetchUInt32(size_t columnIdx, size_t rowIdx)
	{
		return _impl->fetchUInt32(columnIdx, rowIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	std::string Result::fetchString(size_t columnIdx, size_t rowIdx)
	{
		return _impl->fetchString(columnIdx, rowIdx);
	}
}
