#pragma once

namespace akj
{
	struct tTrue{};
	struct tFalse{};
	


	namespace private_space
	{
		////////////////////////////////////////////////////////////////////////
		// It's intended that this *never* be instantiated
		////////////////////////////////////////////////////////////////////////
		struct InvalidType{};
	}

	inline void HaltCompile(private_space::InvalidType){};
	template <typename T>
	struct HaltCompileClass
	{
		static void Halt(){ HaltCompile(T());};
	};

	//////////////////////////////////////////////////////////////////////////
	// Meant for selective template specialization: 
	// define a template, specialize the cases you want, and put this
	// in the general case to ensure that your templated thing doesn't
	// get instantiated for the types you didn't prepare for
	// an example of this is the CompilerErrorIfNotTrue function below
	//////////////////////////////////////////////////////////////////////////
	template <typename T>
	inline void CompileErrorForType()
	{
		HaltCompile(T());
	}
	
	template <typename T>
	inline void CompileErrorIfNotTrue()
	{
		HaltCompile(T());
	}
	template <>
	inline void CompileErrorIfNotTrue<tTrue>()
	{
		//ok, do nothing
	}

	template <typename A, typename B>
	struct tOr {  typedef HaltCompileClass<A> tResult; };
	template <> struct tOr<tTrue, tTrue> { typedef tTrue tResult; };
	template <> struct tOr<tTrue, tFalse> { typedef tTrue tResult; };
	template <> struct tOr<tFalse, tTrue> { typedef tTrue tResult; };
	template <> struct tOr<tFalse, tFalse> { typedef tFalse tResult; };

	template <typename A, typename B>
	struct tAnd {  typedef HaltCompileClass<A> tResult; };
	template <> struct tAnd<tTrue, tTrue> { typedef tTrue tResult; };
	template <> struct tAnd<tTrue, tFalse> { typedef tFalse tResult; };
	template <> struct tAnd<tFalse, tTrue> { typedef tFalse tResult; };
	template <> struct tAnd<tFalse, tFalse> { typedef tFalse tResult; };

		template <typename A>
	struct tNot {  typedef HaltCompileClass<A> tResult; };
	template <> struct tNot<tTrue> { typedef tFalse tResult; };
	template <> struct tNot<tFalse> { typedef tTrue tResult; };

	template <uint32_t num> struct tZeroTest{ typedef tFalse tResult; };
	template <> struct tZeroTest<0> { typedef tTrue tResult; };

	template <uint32_t N1, uint32_t N2>
	struct tEqualTest
	{
		typedef typename tZeroTest<N1 - N2>::tResult tResult;
	};

	template <uint32_t N1, uint32_t N2>
	struct tLesserOf
	{
		static const uint32_t result = N1 < N2? N1 : N2;
	};

	template <uint32_t N1, uint32_t N2>
	struct tLessTest
	{ typedef typename tAnd<
					typename tNot<tEqualTest<N1, N2>>,
					typename tEqualTest<tLesserOf<N1, N2>::result, N1>
					>::tResult tResult; 
	};


	// terminate the recursion when a left shift gets us to an odd number
	// that isn't one, or when we shift all the way to zero
	namespace private_space{
		template <uint32_t num, typename last_result>
		struct tPowerOfTwoTestImpl
		{
			typedef typename 
				tPowerOfTwoTestImpl<
					(num >> 1),
					typename tZeroTest<num&1>::tResult >::tResult 
				tResult;
		};
		
		// got an odd number that was one: this is a power of two
		template <>
		struct tPowerOfTwoTestImpl<0, tFalse>
		{
			typedef tTrue tResult;
		};

		// got an odd number that is not 1, not a power of two
		template <uint32_t num>
		struct tPowerOfTwoTestImpl<num, tFalse>
		{
			typedef tFalse tResult;
		};
	}

	template <uint32_t num>
	struct tPowerOfTwoTest
	{
		typedef typename 
			private_space::tPowerOfTwoTestImpl<(num >> 1),
			typename tZeroTest<num&1>::tResult >::tResult 
			tResult;
	};

	template <uint32_t num, uint32_t denom>
	struct IsDivisibleTest
	{
		typedef typename tZeroTest<num%denom>::tResult tResult;
	};


	// I'm pretty sure there are other powers of two above 256....
	// and I'm sure that finding them can be done in a generic way
	// at compile time using recursion or something (update: I did this
	// up above, seems to work well enough), but I'd rather keep 
	// things simple for both the compiler and myself. 
	// the only real use I've had for the tPowerOfTwo trait is for alignment
	// and 256 should be more than enough for that

	template <uint32_t number>
	struct tNumberTraits 
	{
		typedef tFalse tIsZero;
		typedef tFalse tIsOne;
		typedef tFalse tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<number, tDenom>::tResult;

	};

	template <> struct tNumberTraits<0>
	{
		typedef tTrue tIsZero;
		typedef tFalse tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<0, tDenom>::tResult;

		enum { kLog2OfNum = 1 };
	};
	template <> struct tNumberTraits<1>
	{
		typedef tFalse tIsZero;
		typedef tTrue tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<1, tDenom>::tResult;

		enum { kLog2OfNum = 0 };
	};
	template <> struct tNumberTraits<2>
	{
		typedef tFalse tIsZero;
		typedef tFalse tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<2, tDenom>::tResult;

		enum { kLog2OfNum = 1 };
	};
	template <> struct tNumberTraits<4>
	{
		typedef tFalse tIsZero;
		typedef tFalse tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<4, tDenom>::tResult;

		enum { kLog2OfNum = 2 };
	};
	template <> struct tNumberTraits<8>
	{
		typedef tFalse tIsZero;
		typedef tFalse tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<8, tDenom>::tResult;

		enum { kLog2OfNum = 3 };
	};
	template <> struct tNumberTraits<16>
	{
		typedef tFalse tIsZero;
		typedef tFalse tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<16, tDenom>::tResult;

		enum { kLog2OfNum = 4 };
	};
	template <> struct tNumberTraits<32>
	{
		typedef tFalse tIsZero;
		typedef tFalse tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<32, tDenom>::tResult;

		enum { kLog2OfNum = 5 };
	};
	template <> struct tNumberTraits<64>
	{
		typedef tFalse tIsZero;
		typedef tFalse tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<64, tDenom>::tResult;

		enum { kLog2OfNum = 6 };
	};
	template <> struct tNumberTraits<128>
	{
		typedef tFalse tIsZero;
		typedef tFalse tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<128, tDenom>::tResult;

		enum { kLog2OfNum = 7 };
	};
	template <> struct tNumberTraits<256>
	{
		typedef tFalse tIsZero;
		typedef tFalse tIsOne;
		typedef tTrue tPowerOfTwo;

		template <uint32_t tDenom>
		using tDivisibleBy = typename IsDivisibleTest<256, tDenom>::tResult;

		enum { kLog2OfNum = 8 };
	};




}
