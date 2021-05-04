//
// Created by MassiveAtoms on 5/3/21.
//
#include <array>
#include <functional>
#ifndef PO2_BIGTYPE_H
#define PO2_BIGTYPE_H

class Big{
  public:
    Big (): internal{0},quick{0}{};
    Big(int x):internal{x},quick{x}{};
  private:
    std::array<int, 50> internal;
    int quick;
    friend bool operator==(const Big& left, const Big& right);
    friend class std::hash<Big>;
};
inline bool operator==(const Big& left, const Big& right){
    return left.internal == right.internal;
}

namespace std
{
    template<> struct hash<Big>
    {
        std::size_t operator()(Big const& b) const noexcept
        {
            return std::hash<int>{}(b.internal[0]) * std::hash<int>{}(b.quick);
        }
    };
}



#endif  // PO2_BIGTYPE_H
