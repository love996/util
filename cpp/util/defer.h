#pragma once
#include <functional>

class Defer {
public:
    template <typename Func, typename ...Args>
    Defer(Func &&f, Args &&...args)
        :_f(std::bind(std::forward<Func>(f), std::forward<Args>(args)...))
    {   
    }   

    ~Defer()
    {   
        _f();
    }   
private:
    std::function<void()> _f; 
};
