#ifndef MATRIX_EXCEPT_H
#define MATRIX_EXCEPT_H

#include <sstream>
#include <exception>

namespace math
{
    class exception : public std::exception
    {
        std::string msg_;
    public:
        exception (const char* msg) throw() : msg_(msg) {;}
        exception (const std::ostringstream& msg) throw() : msg_(msg.str().c_str()) {;}
        exception () throw() : msg_() {;}
        exception (const exception& ex) throw() : msg_(ex.what()) {;}
        exception& operator= (const exception& ex) throw() {
            msg_ = ex.what();
            return *this;
        }
        virtual ~exception() throw() {;}
        virtual const char* what() const throw() { return msg_.c_str(); }
    };

    #define THROW(X)\
        math::exception ex(X);\
        throw(ex);

}

#endif
