#ifndef _UTILS_VARIANT_HPP_
#define _UTILS_VARIANT_HPP_

/*!	\defgroup variant Контейнер разнотипных данных (variant)
	\ingroup utils
*/

/*!	\ingroup variant
	\def UTILS_VARIANT_DBGDATA
	\brief Просмотр значений варианта при отладке в окне watch при использовании vs

	При использовании microsoft visual studio можно просматривать типизированные значения,
	содержащиеся в варианте в окне watch при отладке. Для этого необходимо добавить в 
	файл autoexp.dat текст из utils_Variant.txt и собрать проект без определенного макроса
	NDEBUG
*/
#if !defined NDEBUG && defined _MSC_VER || defined DOXYGEN
#	define UTILS_VARIANT_DBGDATA
#endif


#include "utils/dateTimeDuration.hpp"
#include <string>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/preprocessor.hpp>

#include <vector>
#include <map>
#include <set>
#include <deque>
#include <list>
#include <bitset>

namespace utils
{






	//////////////////////////////////////////////////////////////////////////
	/*!	\ingroup variant
		\def ENUM_VARIANT_TYPES
		\brief Макрос для перечисления внутренних типов варианта.

		Перечисляет имена всех внутренних типов варианта кроме Void. Используется совместно с
		макросом ENUM_VARIANT_TYPE, в котором должно содержатся использование одного имени 
		при перечислении.

		Например, так можно вывести на консоль все типы:
		\code
			#define ENUM_VARIANT_TYPE std::cout<<"type: "<<#n<<std::endl;
			ENUM_VARIANT_TYPES
			#undef ENUM_VARIANT_TYPE
		\endcode
	*/
#define ENUM_VARIANT_TYPES\
	ENUM_VARIANT_TYPE(String)\
	ENUM_VARIANT_TYPE(Float)\
	ENUM_VARIANT_TYPE(Double)\
	ENUM_VARIANT_TYPE(Int8)\
	ENUM_VARIANT_TYPE(Int16)\
	ENUM_VARIANT_TYPE(Int32)\
	ENUM_VARIANT_TYPE(Int64)\
	ENUM_VARIANT_TYPE(UInt8)\
	ENUM_VARIANT_TYPE(UInt16)\
	ENUM_VARIANT_TYPE(UInt32)\
	ENUM_VARIANT_TYPE(UInt64)\
	ENUM_VARIANT_TYPE(VectorChar)\
	ENUM_VARIANT_TYPE(Date)\
	ENUM_VARIANT_TYPE(Datetime)\
	ENUM_VARIANT_TYPE(VectorVariant)\
	ENUM_VARIANT_TYPE(MapStringVariant)\
	ENUM_VARIANT_TYPE(Bool)\
	ENUM_VARIANT_TYPE(Tm)\
	ENUM_VARIANT_TYPE(Bitset8)\
	ENUM_VARIANT_TYPE(Bitset16)\
	ENUM_VARIANT_TYPE(Bitset32)\
	ENUM_VARIANT_TYPE(Bitset64)\
	ENUM_VARIANT_TYPE(Bitset128)\
	ENUM_VARIANT_TYPE(Bitset256)\
	ENUM_VARIANT_TYPE(Bitset512)\
	ENUM_VARIANT_TYPE(DateDuration)\
	ENUM_VARIANT_TYPE(TimeDuration)\
	ENUM_VARIANT_TYPE(DateTimeDuration)\
	ENUM_VARIANT_TYPE(MapVariantVariant)\
	ENUM_VARIANT_TYPE(MultimapStringVariant)\
	ENUM_VARIANT_TYPE(MultimapVariantVariant)\
	ENUM_VARIANT_TYPE(SetVariant)\
	ENUM_VARIANT_TYPE(MultisetVariant)\
	ENUM_VARIANT_TYPE(DequeVariant)\
	ENUM_VARIANT_TYPE(ListVariant)\
	ENUM_VARIANT_TYPE(Char)\
	ENUM_VARIANT_TYPE(Uuid)\
	ENUM_VARIANT_TYPE(VariantPtr)\





	//////////////////////////////////////////////////////////////////////////
	class Variant;
	typedef boost::shared_ptr<Variant> VariantPtr;

	//////////////////////////////////////////////////////////////////////////
	/*!	\ingroup variant
		\brief Контейнер разнотипных данных

		Объекты данного класса способны хранить данные разных типов. Перечень этих типов
		жестко задан и не может быть расширен пользователем.

		Возможности:
			- типизированные NULL
			- конвертация некоторых типов друг в друга
			- бинарная сериализация
			- вывод значения в std::ostream в текстовом виде, для отладочных целей
			- загрузка значений из текстовых файлов (JSON подобная)
			- легкий доступ (только на чтение) к элементам массива, ассоциативного массива
	*/
	class Variant
	{
	public:
		/*!	\brief Числовые значения для типов
			
			Имена значений этого перечисления соответствуют именам типов с приставкой et.
		*/
		enum EType
		{
			/*!	\brief Неизвестно

				Это значение используется для выявления нештатного состояния 
				варианта, фактически означает что вариант испорчен
			*/
			etUnknown					=0,

			etVoid						=1,//!<		Пусто
			etString 					=2,//!<		Строка
			etFloat 					=3,//!<		Вещественное
			etDouble 					=4,//!<		Вещественное
			etInt8 						=5,//!<		Целое
			etInt16 					=6,//!<		Целое
			etInt32 					=7,//!<		Целое
			etInt64 					=8,//!<		Целое
			etUInt8 					=9,//!<		Целое
			etUInt16 					=10,//!<	Целое
			etUInt32 					=11,//!<	Целое
			etUInt64 					=12,//!<	Целое
			etVectorChar 				=13,//!<	Массив символов
			etDate 						=14,//!<	Дата
			etDatetime 					=15,//!<	Дата со временем
			etVectorVariant 			=16,//!<	Массив вариантов
			etMapStringVariant 			=17,//!<	Ассоциативны массив, строка - вариант
			etBool 						=18,//!<	Буль
			etTm 						=19,//!<	std::tm
			etBitset8 					=20,//!<	Набор бит
			etBitset16 					=21,//!<	Набор бит
			etBitset32 					=22,//!<	Набор бит
			etBitset64 					=23,//!<	Набор бит
			etBitset128 				=24,//!<	Набор бит
			etBitset256 				=25,//!<	Набор бит
			etBitset512 				=26,//!<	Набор бит
			etDateDuration 				=27,//!<	Количество дней
			etTimeDuration 				=28,//!<	Количество времени
			etDateTimeDuration 			=29,//!<	Количество дней и времени
			etMapVariantVariant 		=30,//!<	Ассоциативны массив, вариант - вариант
			etMultimapStringVariant 	=31,//!<	Ассоциативны массив, строка - вариант
			etMultimapVariantVariant 	=32,//!<	Ассоциативны массив, вариант - вариант
			etSetVariant 				=33,//!<	Множество вариантов
			etMultisetVariant 			=34,//!<	Множество вариантов
			etDequeVariant 				=35,//!<	Массив вариантов
			etListVariant 				=36,//!<	Список вариантов
			etChar 						=37,//!<	Символ
			etUuid 						=38,//!<	UUID
			etVariantPtr 				=39,//!<	Указатель на вариант
		};

	public:
		/*! @{
			\name Возможные типы хранимых значений, имена синхронно с EType
		*/
		typedef void								Void;
		typedef std::string 						String;
		typedef float 								Float;
		typedef double 								Double;
		typedef boost::int8_t 						Int8;
		typedef boost::int16_t 						Int16;
		typedef boost::int32_t 						Int32;
		typedef boost::int64_t 						Int64;
		typedef boost::uint8_t 						UInt8;
		typedef boost::uint16_t 					UInt16;
		typedef boost::uint32_t 					UInt32;
		typedef boost::uint64_t 					UInt64;
		typedef std::vector<char> 					VectorChar;
		typedef boost::gregorian::date 				Date;
		typedef boost::posix_time::ptime 			Datetime;
		typedef std::vector<Variant> 				VectorVariant;
		typedef std::map<std::string, Variant> 		MapStringVariant;
		typedef bool 								Bool;
		typedef std::tm 							Tm;
		typedef std::bitset<8> 						Bitset8;
		typedef std::bitset<16> 					Bitset16;
		typedef std::bitset<32> 					Bitset32;
		typedef std::bitset<64> 					Bitset64;
		typedef std::bitset<128> 					Bitset128;
		typedef std::bitset<256> 					Bitset256;
		typedef std::bitset<512> 					Bitset512;
		typedef boost::gregorian::date_duration 	DateDuration;
		typedef boost::posix_time::time_duration 	TimeDuration;
		typedef ::utils::DateTimeDuration 			DateTimeDuration;
		typedef std::map<Variant, Variant> 			MapVariantVariant;
		typedef std::multimap<std::string, Variant>	MultimapStringVariant;
		typedef std::multimap<Variant, Variant> 	MultimapVariantVariant;
		typedef std::set<Variant> 					SetVariant;
		typedef std::multiset<Variant> 				MultisetVariant;
		typedef std::deque<Variant> 				DequeVariant;
		typedef std::list<Variant> 					ListVariant;
		typedef char 								Char;
		typedef boost::uuids::uuid 					Uuid;
		typedef ::utils::VariantPtr 				VariantPtr;

		/*! @}
		*/

	public:
		/*!	\brief Проекция типа в числовое значение

			Сипользовать можно так
			\code
				utils::Variant::EType et;
				et = utils::Variant::Type2Enum<utils::Variant::String>::et;
				assert(utils::Variant::etString == et);
			\endcode
		*/
		template <class T> struct Type2Enum				{ static const EType et = etUnknown;	};


	public:
		/*!	\brief Деструктор
			
			Если есть значение - оно, так же, будет уничтожено
		*/
		~Variant();

		//////////////////////////////////////////////////////////////////////////
		/*!	\brief Конструктор постого варианта
			
			Созданный вариант будет иметь тип Void и будет установлен в NULL
		*/
		Variant();

		/*!	\brief Конструктор копирования
			
			Созданный вариант будет иметь такой же тип как и у оригинала.
			Если оригинал не NULL то значение, так же, будет скопировано
		*/
		Variant(const Variant &v);

		/*!	\brief Конструктор строкового значения
			
			Созданный вариант будет иметь тип String

			Если переданный указатель на строку NULL - то вариант будет NULL, 
			иначе вариант будет содержать нормальное значение.
		*/
		Variant(const char *v);//String

		/*! @{
			\name Типизированные конструкторы
		*/
#define ENUM_VARIANT_TYPE(T) Variant(const T &v);
		ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
		/*! @}
		*/




		//////////////////////////////////////////////////////////////////////////
		/*! @{
			\name Операторы присваивания
		*/
		Variant &operator=(const Variant &v);
		Variant &operator=(const char *v);//String
#define ENUM_VARIANT_TYPE(T) Variant &operator=(const T &v);
		ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
		/*! @}
		*/

		//////////////////////////////////////////////////////////////////////////
		/*!	\brief Обмен состояния двух вариантов

			\param [in,out] v второй вариант, с которым нужно обменять состояние

			Обмениванию подлежит все: и тип и признак NULL и само значение, если есть.
		*/
		void swap(Variant &v);

		//////////////////////////////////////////////////////////////////////////
		/*!	\brief Проверка на NULL

			\retval true вариант не содержит значения, любые вызовы, 
				которые работают со значением будут проваливаться (as, to, 
				convert, data)

			\retval false вариант содержит значение, можно его получить

			смотри так же setNull(bool n)
		*/
		bool isNull() const;

		/*!	\brief Проверка на тип значения
			
			\tparam T проверяемый тип, например utils::Variant::Float или 
				utils::Variant::ListVariant

			\retval true если тип значения идентичен T
			\retval false если тип значения не идентичен T

			Результат не зависит от признака NULL
			
			Пример
			\code
				Variant v("str");

				assert(v.is<Variant::String>());
				assert(!v.is<Variant::Int32>());

				v.setNull(true);

				assert(v.is<Variant::String>());
				assert(!v.is<Variant::Int32>());
			\endcode
		*/
		template<typename T> bool is() const;

		/*!	\brief Проверка на ассоциативный массив

			\retval true если тип значения один из
				- MapStringVariant
				- MapVariantVariant
				- MultimapStringVariant
				- MultimapVariantVariant

			Для таких типов можно использовать операторы разыменования (если не 
				NULL), очень удобно
				- operator[](const std::string &key);
				- operator[](const char *key);
				- operator[](const Variant &key);

			Например
			\code
				Variant v = MVM("one", 1, "two", 2);

				assert((int)v["two"] == 2);
			\endcode

			\retval false если тип значения другой
		
			В этом случае использование операторов[] будет приводить к неудаче

		*/
		bool isMap() const;

		/*!	\brief Проверка на массив

			\retval true если тип значения один из
				- VectorVariant:
				- DequeVariant:

			Для таких типов можно использовать операторы разыменования (если не 
				NULL), очень удобно
				- operator[](size_t idx);
				- operator[](int idx);

			Например
			\code
				Variant v = MVA("one", 1, "two", 2);

				assert((int)v[3] == 2);
			\endcode

			\retval false если тип значения другой
		
			В этом случае использование операторов[] будет приводить к неудаче
		*/
		bool isArray() const;

		/*!	\brief Проверка на скалярность значения

			\retval true если тип значения один из
				- String
				- Float
				- Double
				- Int8
				- Int16
				- Int32
				- Int64
				- UInt8
				- UInt16
				- UInt32
				- UInt64
				- VectorChar
				- Date
				- Datetime
				- Bool
				- Tm
				- Bitset8
				- Bitset16
				- Bitset32
				- Bitset64
				- Bitset128
				- Bitset256
				- Bitset512
				- DateDuration
				- TimeDuration
				- DateTimeDuration
				- Char
				- Uuid

			\retval false если тип значения другой
		*/
		bool isScalar() const;

		//////////////////////////////////////////////////////////////////////////
		/*!	\brief Управление признаком NULL

			Устанавливает новое состояние NULL.

			\param n новый признак NULL

			Детально, делает следующее:
				- если был NULL и n==true то ничего не происходит.
				- если был не NULL и n==false то ничего не происходит.
				- если был NULL и n==false то будет сконструировано новое значение, 
					конструктором по умолчанию от его типа.
				- если был не NULL и n==true то будет текущее значение будет разрушено.
		*/
		void setNull(bool n=true);

		//////////////////////////////////////////////////////////////////////////
		/*!	\brief Управление признаком NULL и типом

			Устанавливает новое состояние NULL и тип значения

			\tparam T новый тип значения
			\param n новый признак NULL

			Если тип T идентичен текущему типу значения - то поведение аналогично
			setNull(bool n).

			Если тип T и тип текущего значения разные - то
				- если был NULL и n==true то просто устанавливается новый тип.
				- если был не NULL и n==false то старое значение будет разрушено и
					будет сконструировано значение типа T.
				- если был NULL и n==false то будет сконструировано новое значение типа T, 
					конструктором по умолчанию.
				- если был не NULL и n==true то старое значение будет разрушено.
		*/
		template<typename T> void setNull(bool n=true);

		/*!	\brief Установка нового типа

			\tparam T новый тип значения

			Если текущее состояние NULL - то просто меняется признак типа.
	
			Если не NULL и тип текущего значениея идентичен T то ничего не происходит.
			
			Если не NULL и тип текущего значениея не идентичен T то текущее значение
			будет разрушено, будет сконструировано новое значение типа T, конструктором
			по умолчанию.
		*/
		template<typename T> void setType();

		/*!	\brief Установка нового типа
			
			Поведение аналогично template<typename T> void setType(), только тип значения 
			представлен числом.

			\tparam et новый тип значения

		*/
		void setType(EType et);

		//////////////////////////////////////////////////////////////////////////
		/*!	\brief Получение текущего типа
			
			\return текущий тип хранимого значения
		*/
		EType type() const;

		/*!	\brief Получение адреса текущего значения
			
			\pre состояние должно быть не NULL, иначе поведение не определено
			\pre тип значения должен быть не Void, иначе поведение не определено

			\return адрес объекта
		*/
		void *data();

		/*!	\brief Получение адреса текущего значения
			
			\pre состояние должно быть не NULL, иначе поведение не определено
			\pre тип значения должен быть не Void, иначе поведение не определено

			\return адрес объекта
		*/
		void const *data() const;

		/*!	\brief Доступ к хранимому значению

			\tparam T тип значения
			\param forceType приводить тип, если T другой. В случае приведения старое значение 
				будет разрушено и построено новое, с помощью конструктора по умолчанию от T

			Если тип хранимого значения и T не идентичны и forceType==false то 
				поведение не определено.

			\retval ссылка на хранимое значение
		*/
		template<typename T> T &as(bool forceType);

		/*!	\brief Доступ к хранимому значению по константе

			\tparam T тип значения

			Если тип хранимого значения и T не идентичны и forceType==false то 
				поведение не определено.

			\retval константная ссылка на хранимое значение
		*/
		template<typename T> T const &as() const;

		/*!	\brief Доступ к хранимому значению

			\tparam T тип значения

			Если тип хранимого значения и T не идентичны и forceType==false то 
				поведение не определено.

			\retval ссылка на хранимое значение
		*/
		template<typename T> T &as();

		//////////////////////////////////////////////////////////////////////////
		/*!	\brief Преобразование типа хранимого значения

			\tparam T тип сконвертированного значения

			\return значение типа T, полученное из хранимого значения

			В случае если преобразование не возможно - будет возвращено значение,
			построенное конструктором по умолчанию

			Не все типи конвертируемы между собой.
			Для наглядности вот такая матрица возможных преобразований, в ней
			в строках стоят типы источники, в столюцах - типы приемники,
			значком '<>' обозначена возможность преобразования.

			\verbatim
                          Void
                          | String
                          | | Float
                          | | | Double
                          | | | | Int8
                          | | | | | Int16
                          | | | | | | Int32
                          | | | | | | | Int64
                          | | | | | | | | UInt8
                          | | | | | | | | | UInt16
                          | | | | | | | | | | UInt32
                          | | | | | | | | | | | UInt64
                          | | | | | | | | | | | | VectorChar
                          | | | | | | | | | | | | | Date
                          | | | | | | | | | | | | | | Datetime
                          | | | | | | | | | | | | | | | VectorVariant
                          | | | | | | | | | | | | | | | | MapStringVariant
                          | | | | | | | | | | | | | | | | | Bool
                          | | | | | | | | | | | | | | | | | | Tm
                          | | | | | | | | | | | | | | | | | | | Bitset8
                          | | | | | | | | | | | | | | | | | | | | Bitset16
                          | | | | | | | | | | | | | | | | | | | | | Bitset32
                          | | | | | | | | | | | | | | | | | | | | | | Bitset64
                          | | | | | | | | | | | | | | | | | | | | | | | Bitset128
                          | | | | | | | | | | | | | | | | | | | | | | | | Bitset256
                          | | | | | | | | | | | | | | | | | | | | | | | | | Bitset512
                          | | | | | | | | | | | | | | | | | | | | | | | | | | DateDuration
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | TimeDuration
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | DateTimeDuration
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | MapVariantVariant
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | MultimapStringVariant
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | MultimapVariantVariant
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | SetVariant
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | MultisetVariant
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | DequeVariant
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | ListVariant
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | Char
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | Uuid
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | VariantPtr
                          | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
                         --------------------------------------------------------------------------------
                     Void|                                                                              |
                   String|  <><><><><><><><><><><><><><>    <><><><><><><><><><><><>              <><><>|
                    Float|  <><><><><><><><><><><>          <>                                    <>  <>|
                   Double|  <><><><><><><><><><><>          <>                                    <>  <>|
                     Int8|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                    Int16|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                    Int32|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                    Int64|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                    UInt8|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                   UInt16|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                   UInt32|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                   UInt64|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
               VectorChar|  <><><><><><><><><><><><>        <>                                    <>  <>|
                     Date|  <>                      <><>      <>                                      <>|
                 Datetime|  <>                      <><>      <>                                      <>|
            VectorVariant|    <><><><><><><><><><>      <><><>                      <><><><><><><><>  <>|
         MapStringVariant|    <><><><><><><><><><>      <><><>                      <><><><><><><><>  <>|
                     Bool|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                       Tm|  <>                      <><>      <>                                      <>|
                  Bitset8|  <>    <><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                 Bitset16|  <>    <><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                 Bitset32|  <>    <><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                 Bitset64|  <>    <><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                Bitset128|  <>    <><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                Bitset256|  <>    <><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                Bitset512|  <>    <><><><><><><><>          <>  <><><><><><><>                    <>  <>|
             DateDuration|  <>                                                <>  <>                  <>|
             TimeDuration|  <>                                                  <><>                  <>|
         DateTimeDuration|  <>                                                <><><>                  <>|
        MapVariantVariant|    <><><><><><><><><><>      <><><>                      <><><><><><><><>  <>|
    MultimapStringVariant|    <><><><><><><><><><>      <><><>                      <><><><><><><><>  <>|
   MultimapVariantVariant|    <><><><><><><><><><>      <><><>                      <><><><><><><><>  <>|
               SetVariant|    <><><><><><><><><><>      <><><>                      <><><><><><><><>  <>|
          MultisetVariant|    <><><><><><><><><><>      <><><>                      <><><><><><><><>  <>|
             DequeVariant|    <><><><><><><><><><>      <><><>                      <><><><><><><><>  <>|
              ListVariant|    <><><><><><><><><><>      <><><>                      <><><><><><><><>  <>|
                     Char|  <><><><><><><><><><><>          <>  <><><><><><><>                    <>  <>|
                     Uuid|  <>                                                                      <><>|
               VariantPtr|                                                                            <>|
                         --------------------------------------------------------------------------------
645/1521

			\endverbatim
		*/
		template<typename T> T to() const;

		/*!	\brief Преобразование типа хранимого значения

			\tparam T тип сконвертированного значения
			\param [out] dst ссылка на значение требуемого типа

			В случае если преобразование не возможно - dst не будет изменено

			Матрицу возможных преобразований смотри в utils::Variant::to() const
		*/
		template<typename T> void to(T &dst) const;

		/*!	\brief Приведение типа хранимого значения с конвертацией самого значения

			\tparam T новый тип хранимого значения

			Матрицу возможных преобразований смотри в utils::Variant::to() const
		*/
		template<typename T> void convert();

		/*!	\brief Приведение типа хранимого значения с конвертацией самого значения

			\param et новый тип хранимого значения

			Матрицу возможных преобразований смотри в utils::Variant::to() const
		*/
		void convert(EType et);

#ifdef _DEBUG
		template<typename T> bool canConvert() const;
		bool canConvert(EType et) const;
#endif

		//////////////////////////////////////////////////////////////////////////
		/*! @{
			\name Операторы приведения в типу, с конвертацией
			
			Поведение аналогично utils::Variant::to() const
		*/
#define ENUM_VARIANT_TYPE(T) operator T () const;
		ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE
		//////////////////////////////////////////////////////////////////////////
		/*! @}
		*/

	public:
		/*!	\brief Разыменование элемента ассоциативного массива

			\pre тип варианта должен быть один из:
				- MapStringVariant
				- MapVariantVariant
				- MultimapStringVariant
				- MultimapVariantVariant

			\param key искомый ключ

			\retval копия элемента, если ключ найден
			\retval NULL, если ключ не найден
		*/
		Variant operator[](const std::string &key);

		/*!	\brief Разыменование элемента ассоциативного массива

			\pre тип варианта должен быть один из:
				- MapStringVariant
				- MapVariantVariant
				- MultimapStringVariant
				- MultimapVariantVariant

			\param key искомый ключ

			\retval копия элемента, если ключ найден
			\retval NULL, если ключ не найден
		*/
		Variant operator[](const char *key);

		/*!	\brief Разыменование элемента ассоциативного массива или массива

			\pre тип варианта должен быть один из:
				- MapStringVariant
				- MapVariantVariant
				- MultimapStringVariant
				- MultimapVariantVariant
				- VectorVariant
				- DequeVariant

			\param key искомый ключ

			\retval копия элемента, если ключ найден
			\retval NULL, если ключ не найден
		*/
		Variant operator[](const Variant &key);

		/*!	\brief Разыменование элемента массива

			\pre тип варианта должен быть один из:
				- VectorVariant
				- DequeVariant

			\param idx искомый индекс

			\retval копия элемента, если индекс нормальный
			\retval NULL, если индекс вне допустимого диапазона
		*/
		Variant operator[](size_t idx);

		/*!	\brief Разыменование элемента массива

			\pre тип варианта должен быть один из:
				- VectorVariant
				- DequeVariant

			\param idx искомый индекс

			\retval копия элемента, если индекс нормальный
			\retval NULL, если индекс вне допустимого диапазона
		*/
		Variant operator[](int idx);

	public:
		/*!	\brief Отношение порядка

			Используется в ключах ассоциативных контейнеров. 

			Упорядочивает вариант вместе с хранимым значением. Для значений используется их
			operator<, если доступен. Если не доступен (std::tm например) - то на усмотрение 
			реализации. 
			
			\return буль признак
		*/
		bool operator <(const Variant &v) const;

		/*!	\brief Проверка на равенство
		*/
		bool operator ==(const Variant &v) const;

		/*!	\brief Проверка на неравенство
		*/
		bool operator !=(const Variant &v) const;

	public:
		/*!	\brief Cериализация значения в бинарный вид
			
			\param [out] size размер блока данных
			\return блок данных

			Сериализация портабельна между разрядностью операционной среды (32, 64), 
			между различными порядками байт (endians).
		*/
		boost::shared_array<char> serialize(boost::uint32_t &size) const;

		/*!	\brief Десериализация

			\param data блок данных
			\param size размер блока данных

			\retval true если вариант успешно восстановлен
			\retval false если вариант не восстановлен (данные испорчены)
		*/
		bool deserialize(boost::shared_array<char> data, boost::uint32_t size);

		/*!	\brief Загрузка значения из текстового файла
			
			Синтаксис отталкивается от JSON (JSON полностью покрыт).

			Пример:
			\verbatim
				//комментарий
				#комментарий
				/*
					комментарий
				*\/

				[//массив
					"строка", "стр\"ока", "стро'ка",
					'строка', 'стр\'ока', 'стро"ка',

					1.234f,//вещественное одинарной точности
					1.234,//вещественное двойной точности

					0x12fd,		//целое без знака 32 бита
					0x12fdu,	//целое без знака 32 бита
					0x12u8,		//целое без знака 8 бит
					0x12fdu16,	//целое без знака 16 бит
					0x12fdu32,	//целое без знака 32 бита
					0x12fdu64,	//целое без знака 64 бита

					4861,		//целое со знаком 32 бита
					-4861,		//целое со знаком 32 бита

					4861u,		//целое без знака 32 бита
					12u8,		//целое без знака 8 бит
					4861u16,	//целое без знака 16 бит
					4861u32,	//целое без знака 32 бита
					4861u64,	//целое без знака 64 бита

					4861i,		//целое со знаком 32 бита
					12i8,		//целое со знаком 8 бит
					4861i16,	//целое со знаком 16 бит
					4861i32,	//целое со знаком 32 бита
					4861i64,	//целое со знаком 64 бита

					-4861i,		//целое со знаком 32 бита
					-12i8,		//целое со знаком 8 бит
					-4861i16,	//целое со знаком 16 бит
					-4861i32,	//целое со знаком 32 бита
					-4861i64,	//целое со знаком 64 бита

					2012-02-15,	//дата

					16:55:14,			//время
					16:55:14.012345,	//время с микросекундами

					2012-02-15 16:55:14,		//дата время
					2012-02-15 16:55:14.012345,	//дата время

					true, t, on,		//буль
					false, f, off,		//буль

					194735BD-9EEC-455b-A58F-B115DE8E4995,	//uuid

					{//карта
						key1:'value1',
						key2='value2',
						'key3' : 'value1',
						"key4" = 'value2',
					},

					include "filename.js",	//подключение внешнего файла
				]
			\endverbatim
		*/
		bool load(const char *fileName, std::string *errors=NULL);

	protected:
		//буфер
		static const size_t _dataSize = sizeof(void *)<=4?12:24;
		char _data[_dataSize];

		//тип
		typedef boost::int16_t ETypeStorage;//негативный - null
		ETypeStorage _et;

		//для отладчика помогалка
#ifdef UTILS_VARIANT_DBGDATA
		void *_dbgData;
#endif
	};


	//////////////////////////////////////////////////////////////////////////
	//тип в число
	template <> struct Variant::Type2Enum<Variant::Void> { static const EType et = Variant::etVoid;	};
#define ENUM_VARIANT_TYPE(T) template <> struct Variant::Type2Enum<Variant::T> { static const EType et = Variant::et##T;	};
	ENUM_VARIANT_TYPES
#undef ENUM_VARIANT_TYPE


	//////////////////////////////////////////////////////////////////////////
	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::VectorVariant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::MapStringVariant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::MapVariantVariant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::MultimapStringVariant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::MultimapVariantVariant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::SetVariant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::MultisetVariant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::DequeVariant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::ListVariant &v);

	/*!	\ingroup variant
		\brief Отладочный вывод в поток
	*/
	std::ostream &operator <<(std::ostream &ostr, const Variant::VariantPtr &v);
}









//////////////////////////////////////////////////////////////////////////
//генераторы вектора и карты
/*
	вектор использовать так:
	MVA();//сформирует вариант типа etVectorVariant, не нул, пустой
	MVA(220);//с одним элементом типа int
	MVA(220, "elem2", MVA(2,3));//вектор из трех элементов, последний сам вектор из двух

	MVA может принять до MVX_MAXELEMS аргументов


	карту:
	MVM();//сформирует вариант типа etMapStringVariant, не нул, пустой
	MVM("220", 220);//с одной парой ключ "220", значение 220
	MVM("elem2", 220, "complex", MVA(2,3));//

	MVM может принять до MVX_MAXELEMS пар ключ-значение
*/
#ifndef MVX_MAXELEMS
#	define MVX_MAXELEMS 20
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
#define declareMVASet(z, n, _) res.push_back(BOOST_PP_CAT(t,n));

#define declareMVA(z, n, _) \
	template <BOOST_PP_ENUM_PARAMS(n, class T)> \
	inline Variant MVA(BOOST_PP_ENUM_BINARY_PARAMS(n, T, t)) \
	{ \
		Variant::VectorVariant res; \
		BOOST_PP_REPEAT_FROM_TO(0, n, declareMVASet, _) \
		return res; \
	} \

//////////////////////////////////////////////////////////////////////////////////////////////////////
#define declareMVMSet(z, n, _) res.insert(std::make_pair(BOOST_PP_CAT(t, BOOST_PP_MUL(n, 2)), BOOST_PP_CAT(t, BOOST_PP_INC(BOOST_PP_MUL(n, 2)))));

#define declareMVM(z, n, _) \
	template <BOOST_PP_ENUM_PARAMS(BOOST_PP_MUL(n, 2), class T)> \
	inline Variant MVM(BOOST_PP_ENUM_BINARY_PARAMS(BOOST_PP_MUL(n, 2), T, t)) \
	{ \
		Variant::MapStringVariant res; \
		BOOST_PP_REPEAT_FROM_TO(0, n, declareMVMSet, _) \
		return res; \
	} \

namespace utils
{
	/*!	\ingroup variant
		\brief Удобное формирование ассоциативного массива

		Принимает до 20 пар ключ-значние

		\return вариант ассоциативный массив, заполненный переданными парами

		Пример
		\code
			std::cout<<utils::MVM("one", 1, "two", 2);
			//выведет
			// MapStringVariant# {
			//         one => Int32# 1,
			//         two => Int32# 2,
			// }
		\endcode
	*/
	inline Variant MVM()
	{
		Variant::MapStringVariant res;
		return res;
	}

	/*!	\ingroup variant
		\brief Удобное формирование массива

		Принимает до 20 значний

		\return вариант массив, заполненный переданными значениями

		Пример
		\code
			std::cout<<utils::MVA("one", 1, "two", 2);
			//выведет
			// VectorVariant# [
			//         String# one,
			//         Int32# 1,
			//         String# two,
			//         Int32# 2,
			// ]
		\endcode
	*/
	inline Variant MVA()
	{
		Variant::VectorVariant res;
		return res;
	}


	BOOST_PP_REPEAT_FROM_TO(1, MVX_MAXELEMS, declareMVM, _)

	BOOST_PP_REPEAT_FROM_TO(1, MVX_MAXELEMS, declareMVA, _)

}

#undef declareMVASet
#undef declareMVA
#undef declareMVMSet
#undef declareMVM


#endif
