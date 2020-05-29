#pragma once
#include <boost/exception/all.hpp>

class Exception : public virtual boost::exception
                    , public virtual std::exception
{
};
