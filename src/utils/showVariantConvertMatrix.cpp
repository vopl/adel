#include "utils/variant.hpp"

using namespace utils;
using namespace std;
using namespace boost;

//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	Variant v;

	struct SP
	{
		const char *_name;
		Variant::EType _et;
	};

	SP _sps[] ={
		{"Void", Variant::etVoid}
#define ENUM_VARIANT_TYPE(T) ,{#T, Variant::et##T}
	ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
	};

	size_t nTypes = sizeof(_sps)/sizeof(SP);

	for(size_t i(0); i<nTypes; i++)
	{
		cout<<std::setw(25)<<" "<<' ';
		for(size_t j(0); j<i; j++)
		{
			cout<<"| ";
		}
		cout<<_sps[i]._name<<"\n";
	}

	cout<<std::setw(25)<<" "<<' ';
	for(size_t j(0); j<nTypes; j++)
	{
		cout<<"| ";
	}
	cout<<endl;

	cout<<std::setw(25)<<" ";
	size_t count=0;
	for(size_t i(0); i<nTypes+1; i++)
	{
		cout<<"--";
	}
	cout<<endl;
	for(size_t i(0); i<nTypes; i++)
	{
		cout<<std::setw(25)<<_sps[i]._name<<'|';
		v.setType(_sps[i]._et);
		v.setNull(false);
// 		if(_sps[i]._et == Variant::etVariantPtr)
// 		{
// 			v.as<VariantPtr>().reset(new Variant("tratata"));
// 		}
		for(size_t i(0); i<nTypes; i++)
		{
			if(v.canConvert(_sps[i]._et))
			{
				count++;
				cout<<"<>";
			}
			else
			{
				cout<<"  ";
			}
		}
		cout<<'|';
		cout<<endl;
	}

	cout<<std::setw(25)<<" ";
	for(size_t i(0); i<nTypes+1; i++)
	{
		cout<<"--";
	}
	cout<<endl;

	cout<<count<<"/"<<nTypes*nTypes<<endl;

	return EXIT_SUCCESS;
}

