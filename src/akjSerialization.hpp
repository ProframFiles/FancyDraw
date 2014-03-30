#pragma once
#include "RawOstream.hpp"
#include "FatalError.hpp"
#include "Compression.hpp"
#include "MemoryBuffer.hpp"
#include "akjHashing.hpp"
#include "lzma/LzmaLib.h"

namespace akj
{

	

	struct tDeSerializerTrait{};
	struct tSerializerTrait{};
	class cSerialization
	{
		struct tFalseTag{};
		struct tTrueTag{};

		template <typename T, typename tInner=T, typename tAlloc=T >
		struct tTypeTraits
		{
			typedef tFalseTag IsPOD;
			typedef tFalseTag HasWrite;
		};

		template <> struct tTypeTraits<unsigned char>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};
		template <> struct tTypeTraits<uint16_t>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};
		template <> struct tTypeTraits<uint32_t>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};
		template <> struct tTypeTraits<uint64_t>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};

		template <> struct tTypeTraits<char>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};
		template <> struct tTypeTraits<int16_t>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};
		template <> struct tTypeTraits<int32_t>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};
		template <> struct tTypeTraits<int64_t>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};

		template <> struct tTypeTraits<float>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};
		template <> struct tTypeTraits<double>{
			typedef tTrueTag IsPOD;
			typedef tTrueTag HasWrite;
		};

		template <> struct tTypeTraits<cStringRef>{
			typedef tFalseTag IsPOD;
			typedef tTrueTag HasWrite;
		};

		template <> struct tTypeTraits<std::string>{
			typedef tFalseTag IsPOD;
			typedef tTrueTag HasWrite;
		};

		template <> struct tTypeTraits<AlignedBuffer<16>>{
			typedef tFalseTag IsPOD;
			typedef tTrueTag HasWrite;
		};

		template <typename tInner, typename tAlloc>
		struct tTypeTraits<std::vector<tInner, tAlloc>, tInner, tAlloc> {
			typedef tFalseTag IsPOD;
			typedef tTrueTag HasWrite;
		};

		template <class tWriter>
		class cCheckSum
		{
			friend tWriter;
		public:
			~cCheckSum()
			{
				Done();
			}
			uint32_t Done()
			{
				if(mParent == nullptr) return 0;
				uint32_t new_count = mParent->ByteCount();
				uint32_t back_step = new_count - mStart;
				uint32_t hash_size = back_step - sizeof(mStart)*2;
				cStringRef range = 
					mParent->ByteRange(mStart+sizeof(mStart)*2, hash_size);
				uint32_t hash = 0;
				if(!range.empty())
				{
					hash = Hash32(range);
				}
				mParent->WriteBehind(hash_size, back_step);
				mParent->WriteBehind(hash, back_step - sizeof(hash_size));
				mParent = nullptr;
				return hash;
			}

			cCheckSum(cCheckSum&& other)
				:mParent(other.mParent)
				,mStart(other.mStart)
			{
				other.mParent = nullptr;
			};

		private:
			cCheckSum(const cCheckSum& other){};
			const cCheckSum& operator=(const cCheckSum& other){return *this;};
			cCheckSum(uint32_t byte_count, tWriter& parent)
				:mStart(byte_count)
				,mParent(&parent)
			{
				// placeholders for the byte count and the checksum
				mParent->Write(static_cast<uint32_t>(0));
				mParent->Write(static_cast<uint32_t>(0));
			}
			
			uint32_t mStart;
			tWriter* mParent;
		};

		//////////////////////////////////////////////////////////////////////////
		// Just gets the size of the serialized thing
		//////////////////////////////////////////////////////////////////////////

		class cSerializedSizer
		{
		public:
			
			friend class cCheckSum<cSerializedSizer>;

			cCheckSum<cSerializedSizer> StartCheckSum()
			{
				return cCheckSum<cSerializedSizer>(0,*this);
			}
			template <typename T>
			void WriteBehind(const T& val, size_t rewind_bytes){}

			cStringRef ByteRange(uint32_t start, uint32_t end)
			{
				return cStringRef();
			}
			uint32_t ByteCount()
			{
				return static_cast<uint32_t>(mSize);
			}


			cSerializedSizer()
				: mSize(0)
			{}
			~cSerializedSizer(){}
			

			template <typename T>
			void Write(const T& val)
			{
				WriteImpl(val, tTypeTraits<T>::IsPOD());
			}

			void WriteSize(size_t val)
			{
				mSize += 8;
			}

			template <typename T>
			void Write(const std::vector<T>& vec)
			{
				mSize += sizeof(int64_t);
				mSize += sizeof(int64_t);
				WriteVectorImpl(vec, tTypeTraits<T>::HasWrite());
			}

			size_t mSize;
	
		private:
		
			template <typename T>
			void WriteVectorImpl(const std::vector<T>& vec, tTrueTag has_write )
			{
				for (size_t i = 0; i < vec.size(); ++i)
				{
					Write(vec[i]);
				}
			}

			template <typename T>
			void WriteVectorImpl(const std::vector<T>& vec, tFalseTag has_write)
			{
				for (size_t i = 0; i < vec.size(); ++i)
				{
					vec[i].Serialize(*this);
				}
			}

			template <typename T>
			void WriteImpl(const T& val, tTrueTag is_pod)
			{
				mSize += sizeof(T);
			}

			void WriteImpl(const std::string& val, tFalseTag is_pod)
			{
				WriteImpl(cStringRef(val), is_pod);
			}

			void WriteImpl(cStringRef ref, tFalseTag is_pod)
			{ 
				mSize += ref.size()+sizeof(uint64_t); 
			}

			void WriteImpl(const AlignedBuffer<16>& buf, tFalseTag is_pod)
			{
				mSize += buf.size() + sizeof(uint64_t);
			}

		};

		//////////////////////////////////////////////////////////////////////////
		// Writes to a pre-allocated buffer,
		// This is the implementation for a Little Endian system
		// That's our typical target so it's the default and doesn't need
		// To do anything special
		//////////////////////////////////////////////////////////////////////////

		class cByteWriterLE
		{
		public:
			cByteWriterLE(AlignedBuffer<16>& buf)
				:mBuffer(buf)
				,mDataPointer(buf.ptr<char>())
			{}
	
			friend class cCheckSum<cByteWriterLE>;

			cCheckSum<cByteWriterLE> StartCheckSum()
			{
				return std::move(cCheckSum<cByteWriterLE>(ByteCount(), *this));
			}

			~cByteWriterLE(){};

			template <typename T>
			void Write(const T& val)
			{
				WriteImpl(val, tTypeTraits<T>::IsPOD());
			}

			void WriteSize(size_t val)
			{
				Write(static_cast<uint64_t>(val));
			}

			void WriteImpl(const std::string& str,  tFalseTag is_pod)
			{
				WriteImpl(cStringRef(str), is_pod);
			}

			void WriteImpl(const AlignedBuffer<16>& buf,  tFalseTag is_pod)
			{
				WriteSize(buf.size());
				const char* data = buf.ptr<char>();
				memcpy(mDataPointer, data, buf.size());
				mDataPointer += buf.size();
			}

			void WriteImpl(const cStringRef& ref,  tFalseTag is_pod) 
			{
				WriteSize(ref.size());
				const char* data = ref.data();
				for (size_t i = 0; i < ref.size() ; ++i)
				{
					*mDataPointer++ = *data++;
				}
			}

			template <typename T>
			void WriteImpl(const std::vector<T>& vec, tFalseTag is_pod)
			{
				Write(static_cast<uint64_t>(vec.size()));
				Write(static_cast<uint64_t>(vec.capacity()));
				WriteVectorImpl(vec, tTypeTraits<T>::HasWrite());
			}


		private:
			template <typename T>
			void WriteVectorImpl(const std::vector<T>& vec, tTrueTag has_write)
			{
				for (size_t i = 0; i < vec.size(); ++i)
				{
					Write(vec[i]);
				}
			}

			template <typename T>
			void WriteVectorImpl(const std::vector<T>& vec, tFalseTag has_write)
			{
				for (size_t i = 0; i < vec.size(); ++i)
				{
					vec[i].Serialize(*this);
				}
			}

			template <typename tPOD>
			void WriteImpl(tPOD val, tTrueTag is_pod)
			{
				CheckEnd();
				*reinterpret_cast<tPOD*>(mDataPointer) = val;
				mDataPointer += sizeof(tPOD);
			}

			void FastForward(size_t bytes)
			{
				
				mDataPointer += bytes;
			};

			template <typename T>
			void WriteBehind(const T& val, size_t rewind_bytes)
			{
				Rewind(rewind_bytes);
				char* pointer_start = mDataPointer;
				Write(val);
				size_t written = mDataPointer - pointer_start;
				FastForward(rewind_bytes - written);
			}

			void Rewind(size_t bytes)
			{
				mDataPointer -= bytes;
			}
			cStringRef ByteRange(uint32_t start, uint32_t count)
			{
				return cStringRef(mBuffer.ptr<char>() + start, count);
			}
			uint32_t ByteCount()
			{
				return static_cast<uint32_t>(mDataPointer - mBuffer.data());
			}
			void CheckEnd()
			{
#ifdef _DEBUG
				intptr_t diff = static_cast<intptr_t>(mDataPointer 
																			- (mBuffer.data() + mBuffer.size()));
				AKJ_ASSERT_AND_THROW(diff < 0);
#endif
			}

			AlignedBuffer<16>& mBuffer;
			char* mDataPointer;
		};


		//////////////////////////////////////////////////////////////////////////
		// De-serializing class fro little endian systems. As with the writer,
		// Doesn't need to do anything special
		//////////////////////////////////////////////////////////////////////////
		class cByteReaderLE
		{
		public:
			typedef tDeSerializerTrait tFunction;
			cByteReaderLE(cStringRef buf)
				: mBuffer(buf)
				, mDataPointer(buf.data())
			{}

			template <typename T>
			void Read(T& val)
			{
				ReadImpl(tTypeTraits<T>::IsPOD(), tTypeTraits<T>::HasWrite(), val);
			}

			void Skip(size_t num_bytes)
			{
				mDataPointer += num_bytes;
			}

			size_t Tell() const
			{
				return mDataPointer - mBuffer.data();
			}

			void ReadSize(size_t& val)
			{
				uint64_t tmp;
				Read(tmp);
				val = static_cast<size_t>(tmp);
			}

			void ReadBytes(uint8_t* buf, uint32_t bytes)
			{
				if(mDataPointer + bytes <= mBuffer.end())
				{
					memcpy(buf, mDataPointer, bytes);
				}
				mDataPointer += bytes;
			}


			bool CheckSum()
			{
				uint32_t num_bytes;
				Read(num_bytes);
				const intptr_t space_remaining = SpaceRemaining();
				if( space_remaining < 
					static_cast<intptr_t>(num_bytes+sizeof(num_bytes)) )
				{
					AKJ_THROW("Insufficient bytes in buffer for checksum");
				}
				uint32_t expected_check;
				Read(expected_check);
				Hash32 hash(cStringRef(mDataPointer, num_bytes));

				if( hash != expected_check)
				{
					AKJ_THROW("Checksum Failed");
					return false;
				}
				return true;
			}

			void ReadImpl(tFalseTag is_pod, tTrueTag has_write, std::string& val)
			{
				val.clear();
				size_t elements;
				ReadSize(elements);
				val.reserve(elements);
				for(size_t i=0; i< elements; ++i)
				{
					val.push_back(*mDataPointer++);
				}
			}

			void Read(cStringRef& str, AlignedBuffer<16>& buf)
			{
				size_t elements;
				ReadSize(elements);
				buf.reset(elements+1);
				char* ptr = buf.ptr<char>();
				memcpy(ptr, mDataPointer,elements);
				ptr[elements] = 0;
				mDataPointer += elements;
				str = cStringRef(ptr, elements);
			}

			void Read(AlignedBuffer<16>& buf)
			{
				size_t elements;
				ReadSize(elements);
				buf.reset(elements+1);
				char* ptr = buf.ptr<char>();
				memcpy(ptr, mDataPointer, elements);
				ptr[elements] = 0;
				mDataPointer += elements;
			}

			template <typename T>
			void Read(std::vector<T>& val)
			{
				size_t elements;
				ReadSize(elements);
				size_t capacity;
				ReadSize(capacity);
				val.clear();
				val.reserve(capacity);
				ReadVectorImpl(val, elements, tTypeTraits<T>::HasWrite());
			}

		private:

			template <typename T>
			void ReadVectorImpl(T& val, size_t elements, tTrueTag has_write)
			{
				for(size_t i = 0; i< elements; ++i)
				{
					val.emplace_back();
					Read(val.back());
				}
			}

			template <typename T>
			void ReadVectorImpl(T& val, size_t elements, tFalseTag has_write)
			{
				for (size_t i = 0; i < elements; ++i)
				{
					val.emplace_back(*this, tFunction());
				}
			}

			template <typename T>
			void ReadImpl(tTrueTag is_pod, tTrueTag has_write, T& val)
			{
				val = *reinterpret_cast<const T*>(mDataPointer);
				mDataPointer += sizeof(T);
			}

			intptr_t SpaceRemaining()
			{
				return -(mDataPointer - (mBuffer.data() + mBuffer.size()));
			}

			void CheckEnd()
			{
#ifdef _DEBUG
				AKJ_ASSERT_AND_THROW(SpaceRemaining() > 0);
#endif
			}


			cStringRef mBuffer;
			const char* mDataPointer;

		};

		//////////////////////////////////////////////////////////////////////////
		// system dependent typedefs
		//////////////////////////////////////////////////////////////////////////
		typedef cByteWriterLE tByteWriter;
		typedef cByteReaderLE tByteReader;


	public:
		cSerialization();
		~cSerialization();
		template <typename T>
		static size_t SerializeLZ4(const T& val, AlignedBuffer<16>& buf)
		{
			cSerializedSizer sizer;
			val.Serialize(sizer);
			buf.reset(sizer.mSize);
			tByteWriter writer(buf);
			val.Serialize(writer);
			OwningPtr<MemoryBuffer> tmp;
			int ret = lz4::compress(cStringRef(buf.data(), buf.size()), tmp);
			if(ret < 0) AKJ_THROW("Error while compressing LZ4 data");
			// 11 bytes for the header
			buf.reset(tmp->getBufferSize() + 8 + 3);
			char* ptr = buf.ptr<char>();
			*ptr++ = 'L';
			*ptr++ = 'Z';
			*ptr++ = '4';
			*reinterpret_cast<uint64_t*>(ptr) = static_cast<uint64_t>(sizer.mSize);
			ptr += 8;
			memcpy(ptr, tmp->getBufferStart(), tmp->getBufferSize());
			return buf.size();
		}

		template <typename T>
		static size_t SerializeLZMA(const T& val, AlignedBuffer<16>& buf)
		{
			cSerializedSizer sizer;
			val.Serialize(sizer);
			buf.reset(sizer.mSize);
			tByteWriter writer(buf);
			val.Serialize(writer);
			const uint32_t header_size = 4+8+LZMA_PROPS_SIZE;

			// I don't know how much an LZMA archive can expand, we'll say it won't be larger than 
			// the original size + 64 bytes
			cAlignedBuffer temp(sizer.mSize+64);
			unsigned char out_props[LZMA_PROPS_SIZE] = {};
			size_t out_props_size = LZMA_PROPS_SIZE;

			size_t out_len = temp.size()-header_size;

			int ret = LzmaCompress(temp.data(), &out_len,
															buf.data(), buf.size(),
															out_props, &out_props_size,
															-1, 0, -1, -1, -1, -1, 1);
			if(ret != SZ_OK) AKJ_THROW("Error while compressing LZMA data");
			// 17 bytes for the header
			buf.reset(out_len+header_size);
			char* ptr = buf.ptr<char>();
			*ptr++ = 'L';
			*ptr++ = 'Z';
			*ptr++ = 'M';
			*ptr++ = 'A';
			*reinterpret_cast<uint64_t*>(ptr) = static_cast<uint64_t>(sizer.mSize);
			ptr += 8;
			memcpy(ptr, out_props, out_props_size);
			memcpy(ptr+out_props_size, temp.data(), out_len);
			return buf.size();
		}


		template <typename T>
		static size_t SerializedSize(const T& val)
		{
			cSerializedSizer sizer;
			val.Serialize(sizer);
			return sizer.mSize;
		}

		template <typename T>
		static size_t Serialize(const T& val, AlignedBuffer<16>& buf)
		{
			cSerializedSizer sizer;
			val.Serialize(sizer);
			buf.reset(sizer.mSize);
			tByteWriter writer(buf);
			val.Serialize(writer);
			return buf.size();
		}

		static tByteReader Reader(cStringRef data)
		{
			return tByteReader(data);
		}

		static tByteReader ReaderLZ4(cStringRef data, AlignedBuffer<16>& temp_data)
		{
			const char* ptr = data.data();
			AKJ_ASSERT(ptr[0]=='L' && ptr[1]=='Z' && ptr[2]=='4');
			size_t bytes = 
				static_cast<size_t>(*reinterpret_cast<const uint64_t*>(ptr+3));

			temp_data.reset(bytes);

			int ret = lz4::uncompress_into(data.drop_front(11), 
																temp_data.ptr<char>(), bytes);
			
			if(ret < 0){
				AKJ_THROW("LZ4 Deserialization failed");
			}
			
			return tByteReader(cStringRef(temp_data.ptr<char>(), bytes));
		}
		
		static tByteReader ReaderLZMA(cStringRef data, AlignedBuffer<16>& temp_data)
		{
			const char* ptr = data.data();
			const uint32_t header_size = 4+8+LZMA_PROPS_SIZE;
			AKJ_ASSERT_AND_THROW(ptr[0]=='L' && ptr[1]=='Z' && ptr[2]=='M'&& ptr[3]=='A');
			size_t bytes = static_cast<size_t>(*reinterpret_cast<const uint64_t*>(ptr+4));
			size_t out_bytes = bytes;
			size_t in_bytes = data.size() - header_size;
			temp_data.reset(bytes);
			const unsigned char* in_props = (const uchar*)data.data()+header_size-LZMA_PROPS_SIZE;
			size_t in_props_size = LZMA_PROPS_SIZE;

			int ret = LzmaUncompress(temp_data.data(), &out_bytes,
																(const uchar*)data.drop_front(header_size).data(), &in_bytes,
																in_props, in_props_size);
			
			if(ret != SZ_OK){
				AKJ_THROW("LZ4 Deserialization failed with error " + Twine(ret));
			}
			
			return tByteReader(cStringRef(temp_data.ptr<char>(), bytes));
		}

		static tByteReader ReaderCompressed(cStringRef data, AlignedBuffer<16>& temp_data)
		{
			AKJ_ASSERT_AND_THROW(data.size()>=3);
			if(data.substr(0, 3) == "LZ4"){
				return ReaderLZ4(data, temp_data);
			}
			else if(data.size() >=4 && data.substr(0, 4) == "LZMA") {
				return ReaderLZMA(data, temp_data);
			}
			else{
				AKJ_THROW("Unrecognized compressed data format");
			}
		}

	private:

	};

}

