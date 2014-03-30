#pragma once
#include "akjTypeTraits.hpp"

namespace akj
{
	//////////////////////////////////////////////////////////////////////////
	// A bunch of work to just get a type that seamlessly casts to 
	// unisigned char* and char* (while acting like a pointer otherwise)
	// not sure if I actually plugged all the holes W.R.T. accidental
	// conversion to bool or other arithmetic types, but I tried
	//////////////////////////////////////////////////////////////////////////
	struct cDataPtr
	{
		explicit cDataPtr(void* ptr):mPtr(reinterpret_cast<uint8_t*>(ptr)){}
		cDataPtr(unsigned char* ptr):mPtr(ptr){}
		uint8_t* mPtr;
		operator uint8_t*(){return mPtr;}
		operator char*(){return reinterpret_cast<char*>(mPtr);}
		operator const uint8_t*() const {return mPtr;}
		operator const char*() const {return reinterpret_cast<char*>(mPtr);}
		operator void*(){return mPtr;}
		operator const void*() const {mPtr;}

		uintptr_t AsUInt() const { return reinterpret_cast<uintptr_t>(mPtr); }

		template <typename tPointer>
		tPointer* As(){ return reinterpret_cast<tPointer*>(mPtr);}
		template <typename tPointer>
		const tPointer* As() const { return reinterpret_cast<tPointer*>(mPtr);}

		template <uint32_t tAlign>
		cDataPtr AlignedTo()
		{
			return cDataPtr(mPtr - Misalignment(tAlign, tNumberTraits::kLog2OfNum));
		}

		template <uint32_t tAlign>
		bool IsAligned()
		{
			return IsAlignedImpl(tAlign, tNumberTraits<tAlign>::tPowerOfTwo());
		}

		operator bool()
		{
			return mPtr != nullptr;
		}
		bool operator !(){return mPtr == nullptr;}

		template <typename tArithmetic>
		cDataPtr operator +(tArithmetic num){return cDataPtr(mPtr+num);}
		
		intptr_t operator -(cDataPtr num){
			return num.mPtr-num.mPtr;
		}

		bool operator <(cDataPtr num){ return num.mPtr < mPtr;}
		bool operator ==(cDataPtr num){ return num.mPtr == mPtr;}
		bool operator >(cDataPtr num){ return num.mPtr > mPtr;}
		bool operator <=(cDataPtr num){ return num.mPtr <= mPtr;}
		bool operator >=(cDataPtr num){ return num.mPtr >= mPtr;}
		bool operator !=(cDataPtr num){ return num.mPtr != mPtr;}
	private:
		inline bool IsAlignedImpl(uint32_t num, tTrue)
		{
			return ((reinterpret_cast<uintptr_t>(mPtr)&(num-1)) == 0);
		}
		uintptr_t Misalignment(uint32_t num, uint32_t log2ofnum)
		{
			return ((reinterpret_cast<uintptr_t>(mPtr))&(num-1));
		}
		// private to make sure we don't compare with any arithmetic types
		bool operator <(uint64_t num){ return false;}
		bool operator ==(uint64_t num){ return false;}
		bool operator >(uint64_t num){ return false;}
		bool operator <=(uint64_t num){ return false;}
		bool operator >=(uint64_t num){ return false;}
		bool operator !=(uint64_t num){ return false;}
	};

	// not sure if this is a valid pointer operation, but
	// addition is commutative, so it should be ok
	template <typename tArithmetic>
	cDataPtr operator +(tArithmetic num, cDataPtr ptr)
	{ 
		return cDataPtr(num + ptr.mPtr);
	}
	// can't do this generally
	template <typename tArithmetic>
	inline intptr_t operator -(tArithmetic num, cDataPtr ptr)
	{ 
		CompileErrorForType<tArithmetic>();
		return cDataPtr(nullptr);
	}
	template <>
	inline intptr_t operator - <const char*>(const char* ptr, cDataPtr data)
	{ 
		return ptr - data.operator const char*();
	}

	template <>
	inline intptr_t operator - <char*>(char* ptr, cDataPtr data)
	{ 
		return ptr - data.operator char*();
	}
}
