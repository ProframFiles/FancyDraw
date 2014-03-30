#pragma once
#include <sstream>
#include <akjStringRef.hpp>

namespace akj
{
	class cFormat
	{
	public:
		// adapted from http://www.stroustrup.com/C++11FAQ.html#variadic-templates
		template<typename T, typename... Args>
		static std::string Write(const char* format_string,
			T first_arg, Args... other_args)
		{
			std::stringstream buf;
			while (format_string && *format_string)
			{
				if (*format_string == '%' && *++format_string != '%')
				{
					Write(buf, format_string, first_arg);
					Write(buf, ++format_string, other_args...);
					return buf.str();
				}
				buf.put(*format_string++);
			}
			//too many arguments in the format string
			assert(false);
			return std::string();
		}
	private:
		template<typename T>
		static void Write(std::stringstream& buf,const char* format_string ,T arg){
			buf << arg;
		}

		static void Write(std::stringstream& buf, const char* format_string,const char* arg){
			buf << arg;
		}

		static void Write(std::stringstream& buf, const char* format_string ,cStringRef arg){
			buf.write(arg.data(), arg.size());
		}
	};
}
