#ifndef _PGC_BINDDATA_HPP_
#define _PGC_BINDDATA_HPP_

#include "utils/variant.hpp"
#include "connectionHolder.hpp"

namespace pgc { namespace impl
{
	using namespace utils;

	class BindData
	{
		ConnectionHolderPtr	_con;

		std::vector<Oid>	_typ;
		std::vector<char *>	_val;
		std::vector<int>	_len;
		std::vector<int>	_fmt;
		std::vector<char>	_own;

		void resizeArrays(size_t s);

	private:
		template <class Sequence>
		void bindSeq(const Sequence &v);

		template <class Associative>
		void bindAssoc(const Associative &v);

		void bindScalar(const Variant &v);

	private:
		bool bindFiller(const Variant &v, size_t idx);

	public:
		BindData(const Variant &data, ConnectionHolderPtr con);
		~BindData();

		const std::vector<Oid> &	typ;
		const std::vector<char *> &	val;
		const std::vector<int> &	len;
		const std::vector<int> &	fmt;


	};
	typedef boost::shared_ptr<BindData> BindDataPtr;
}}

#endif
