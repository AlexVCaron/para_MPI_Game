#ifndef INCOPIABLE_H
#define INCOPIABLE_H

class Incopiable {
public:
    Incopiable(const Incopiable&) = delete;
    Incopiable& operator=(const Incopiable&) = delete;
protected:
    constexpr Incopiable() = default;
    ~Incopiable() = default;
};

#endif