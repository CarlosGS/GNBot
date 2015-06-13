#ifndef X_STRING_H
#define X_STRING_H

#include <string>
#include <exception>

class XString : public std::exception
{
	protected:
		std::string text;
	public:
		virtual const char* what() const throw()
		{
			return text.c_str();
		}
		
		XString() {}
		XString(const std::string & s) : text(s) {}
		XString(const XString & s) : text(s.what()) {}
		XString(const char * s) : text(s) {}

		virtual ~XString() throw () {}

		XString operator+(const XString & s) const {
			return text + s.text;
		}
		XString operator+(const std::string & s) const {
			return text + s;
		}
		XString operator+(const char * s) const {
			return text + s;
		}

		 operator std::string() {
			 return text;
		 }

		 operator const std::string() {
			 return text;
		 }
};


#endif // X_STRING_H
