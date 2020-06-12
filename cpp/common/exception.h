#pragma once
#include <boost/exception/all.hpp>

class Exception : public virtual boost::exception
                , public virtual std::exception
{
};

using ErrorMsg = boost::error_info<struct tag_err_msg, std::string>;
