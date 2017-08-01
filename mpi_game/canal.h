#ifndef CANAL_H
#define CANAL_H

#include "canal_types.h"

template<class canal_type>
struct canal : canal_type
{
    using canal_dir = typename canal_traits<canal_type>::direction;
    using T = typename canal_traits<canal_type>::message_type;

    template<class ... Args>
    T resolve(Args&& ... args) const
    {
        using impl = typename canal_traits<canal_type>::impl_type;
        impl o_impl;
        return o_impl.resolve(args ...);
    }
};
#endif
