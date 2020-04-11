#pragma once
#include <functional>

class Defer {
public:
    Defer(std::function<void()> f)
        :_f(f)
    {   
    }   
    ~Defer()
    {   
        _f();
    }   
private:
    std::function<void()> _f; 
};
