#include "pch.hpp"
#include "utils/htmlEntities.hpp"

#include <utf8proc/utf8proc.h>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/spirit/include/qi_uint.hpp>

namespace utils
{
	namespace
	{
		namespace qi = boost::spirit::qi;
		using namespace qi;
		namespace px = boost::phoenix;

		symbols<char, boost::uint32_t> entityInit()
		{
			symbols<char, boost::uint32_t> res;
			res.add("quot", 	34);
			res.add("amp", 	38);
			res.add("apos", 	39);
			res.add("lt", 	60);
			res.add("gt", 	62);
			res.add("nbsp", 	160);
			res.add("iexcl", 	161);
			res.add("cent", 	162);
			res.add("pound", 	163);
			res.add("curren", 	164);
			res.add("yen", 	165);
			res.add("brvbar", 	166);
			res.add("sect", 	167);
			res.add("uml", 	168);
			res.add("copy", 	169);
			res.add("ordf", 	170);
			res.add("laquo", 	171);
			res.add("not", 	172);
			res.add("shy", 	173);
			res.add("reg", 	174);
			res.add("macr", 	175);
			res.add("deg", 	176);
			res.add("plusmn", 	177);
			res.add("sup2", 	178);
			res.add("sup3", 	179);
			res.add("acute", 	180);
			res.add("micro", 	181);
			res.add("para", 	182);
			res.add("middot", 	183);
			res.add("cedil", 	184);
			res.add("sup1", 	185);
			res.add("ordm", 	186);
			res.add("raquo", 	187);
			res.add("frac14", 	188);
			res.add("frac12", 	189);
			res.add("frac34", 	190);
			res.add("iquest", 	191);
			res.add("Agrave", 	192);
			res.add("Aacute", 	193);
			res.add("Acirc", 	194);
			res.add("Atilde", 	195);
			res.add("Auml", 	196);
			res.add("Aring", 	197);
			res.add("AElig", 	198);
			res.add("Ccedil", 	199);
			res.add("Egrave", 	200);
			res.add("Eacute", 	201);
			res.add("Ecirc", 	202);
			res.add("Euml", 	203);
			res.add("Igrave", 	204);
			res.add("Iacute", 	205);
			res.add("Icirc", 	206);
			res.add("Iuml", 	207);
			res.add("ETH", 	208);
			res.add("Ntilde", 	209);
			res.add("Ograve", 	210);
			res.add("Oacute", 	211);
			res.add("Ocirc", 	212);
			res.add("Otilde", 	213);
			res.add("Ouml", 	214);
			res.add("times", 	215);
			res.add("Oslash", 	216);
			res.add("Ugrave", 	217);
			res.add("Uacute", 	218);
			res.add("Ucirc", 	219);
			res.add("Uuml", 	220);
			res.add("Yacute", 	221);
			res.add("THORN", 	222);
			res.add("szlig", 	223);
			res.add("agrave", 	224);
			res.add("aacute", 	225);
			res.add("acirc", 	226);
			res.add("atilde", 	227);
			res.add("auml", 	228);
			res.add("aring", 	229);
			res.add("aelig", 	230);
			res.add("ccedil", 	231);
			res.add("egrave", 	232);
			res.add("eacute", 	233);
			res.add("ecirc", 	234);
			res.add("euml", 	235);
			res.add("igrave", 	236);
			res.add("iacute", 	237);
			res.add("icirc", 	238);
			res.add("iuml", 	239);
			res.add("eth", 	240);
			res.add("ntilde", 	241);
			res.add("ograve", 	242);
			res.add("oacute", 	243);
			res.add("ocirc", 	244);
			res.add("otilde", 	245);
			res.add("ouml", 	246);
			res.add("divide", 	247);
			res.add("oslash", 	248);
			res.add("ugrave", 	249);
			res.add("uacute", 	250);
			res.add("ucirc", 	251);
			res.add("uuml", 	252);
			res.add("yacute", 	253);
			res.add("thorn", 	254);
			res.add("yuml", 	255);
			res.add("OElig", 	338);
			res.add("oelig", 	339);
			res.add("Scaron", 	352);
			res.add("scaron", 	353);
			res.add("Yuml", 	376);
			res.add("fnof", 	402);
			res.add("circ", 	710);
			res.add("tilde", 	732);
			res.add("Alpha", 	913);
			res.add("Beta", 	914);
			res.add("Gamma", 	915);
			res.add("Delta", 	916);
			res.add("Epsilon", 	917);
			res.add("Zeta", 	918);
			res.add("Eta", 	919);
			res.add("Theta", 	920);
			res.add("Iota", 	921);
			res.add("Kappa", 	922);
			res.add("Lambda", 	923);
			res.add("Mu", 	924);
			res.add("Nu", 	925);
			res.add("Xi", 	926);
			res.add("Omicron", 	927);
			res.add("Pi", 	928);
			res.add("Rho", 	929);
			res.add("Sigma", 	931);
			res.add("Tau", 	932);
			res.add("Upsilon", 	933);
			res.add("Phi", 	934);
			res.add("Chi", 	935);
			res.add("Psi", 	936);
			res.add("Omega", 	937);
			res.add("alpha", 	945);
			res.add("beta", 	946);
			res.add("gamma", 	947);
			res.add("delta", 	948);
			res.add("epsilon", 	949);
			res.add("zeta", 	950);
			res.add("eta", 	951);
			res.add("theta", 	952);
			res.add("iota", 	953);
			res.add("kappa", 	954);
			res.add("lambda", 	955);
			res.add("mu", 	956);
			res.add("nu", 	957);
			res.add("xi", 	958);
			res.add("omicron", 	959);
			res.add("pi", 	960);
			res.add("rho", 	961);
			res.add("sigmaf", 	962);
			res.add("sigma", 	963);
			res.add("tau", 	964);
			res.add("upsilon", 	965);
			res.add("phi", 	966);
			res.add("chi", 	967);
			res.add("psi", 	968);
			res.add("omega", 	969);
			res.add("thetasym", 	977);
			res.add("upsih", 	978);
			res.add("piv", 	982);
			res.add("ensp", 	8194);
			res.add("emsp", 	8195);
			res.add("thinsp", 	8201);
			res.add("zwnj", 	8204);
			res.add("zwj", 	8205);
			res.add("lrm", 	8206);
			res.add("rlm", 	8207);
			res.add("ndash", 	8211);
			res.add("mdash", 	8212);
			res.add("lsquo", 	8216);
			res.add("rsquo", 	8217);
			res.add("sbquo", 	8218);
			res.add("ldquo", 	8220);
			res.add("rdquo", 	8221);
			res.add("bdquo", 	8222);
			res.add("dagger", 	8224);
			res.add("Dagger", 	8225);
			res.add("bull", 	8226);
			res.add("hellip", 	8230);
			res.add("permil", 	8240);
			res.add("prime", 	8242);
			res.add("Prime", 	8243);
			res.add("lsaquo", 	8249);
			res.add("rsaquo", 	8250);
			res.add("oline", 	8254);
			res.add("frasl", 	8260);
			res.add("euro", 	8364);
			res.add("image", 	8465);
			res.add("weierp", 	8472);
			res.add("real", 	8476);
			res.add("trade", 	8482);
			res.add("alefsym", 	8501);
			res.add("larr", 	8592);
			res.add("uarr", 	8593);
			res.add("rarr", 	8594);
			res.add("darr", 	8595);
			res.add("harr", 	8596);
			res.add("crarr", 	8629);
			res.add("lArr", 	8656);
			res.add("uArr", 	8657);
			res.add("rArr", 	8658);
			res.add("dArr", 	8659);
			res.add("hArr", 	8660);
			res.add("forall", 	8704);
			res.add("part", 	8706);
			res.add("exist", 	8707);
			res.add("empty", 	8709);
			res.add("nabla", 	8711);
			res.add("isin", 	8712);
			res.add("notin", 	8713);
			res.add("ni", 	8715);
			res.add("prod", 	8719);
			res.add("sum", 	8721);
			res.add("minus", 	8722);
			res.add("lowast", 	8727);
			res.add("radic", 	8730);
			res.add("prop", 	8733);
			res.add("infin", 	8734);
			res.add("ang", 	8736);
			res.add("and", 	8743);
			res.add("or", 	8744);
			res.add("cap", 	8745);
			res.add("cup", 	8746);
			res.add("int", 	8747);
			res.add("there4", 	8756);
			res.add("sim", 	8764);
			res.add("cong", 	8773);
			res.add("asymp", 	8776);
			res.add("ne", 	8800);
			res.add("equiv", 	8801);
			res.add("le", 	8804);
			res.add("ge", 	8805);
			res.add("sub", 	8834);
			res.add("sup", 	8835);
			res.add("nsub", 	8836);
			res.add("sube", 	8838);
			res.add("supe", 	8839);
			res.add("oplus", 	8853);
			res.add("otimes", 	8855);
			res.add("perp", 	8869);
			res.add("sdot", 	8901);
			res.add("lceil", 	8968);
			res.add("rceil", 	8969);
			res.add("lfloor", 	8970);
			res.add("rfloor", 	8971);
			res.add("lang", 	9001);
			res.add("rang", 	9002);
			res.add("loz", 	9674);
			res.add("spades", 	9824);
			res.add("clubs", 	9827);
			res.add("hearts", 	9829);
			res.add("diams", 	9830);
			return res;
		}
		static const symbols<char, boost::uint32_t> entity = entityInit();


		class Accumuler
		{
			mutable std::string &text;
		public:
			Accumuler(std::string &res)
				: text(res)
			{

			}

			template <typename Context>
			void operator()(boost::uint32_t code, Context&, bool&) const
			{
				char buf[8];
				ssize_t size = utf8proc_encode_char(code, (::uint8_t *)buf);
				buf[size] = 0;
				text += buf;
			}

			template <typename Context>
			void operator()(char c, Context&, bool&) const
			{
				text += c;
			}
		};

	}
	//////////////////////////////////////////////////////////////////////////
	std::string htmlEntitiesDecode(const std::string &text)
	{
		std::string res;
		res.reserve(text.size());
		Accumuler accumuler(res);
		qi::parse(text.begin(), text.end(), 
			*(
			(lit('&') >> ((lit('#') >> uint_) | boost::spirit::ascii::no_case[entity]) >> lit(';'))[accumuler] |
			char_[accumuler]
		)
			);

		return res;
	}
}