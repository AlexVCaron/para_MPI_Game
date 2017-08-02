#ifndef BROADCASTER_TRAITS
#define BROADCASTER_TRAITS

template<typename broadcaster_type>
struct broadcaster_traits
{
    static broadcaster_type make_broadcaster() { return broadcaster_type::make_broadcaster(); }
};

#endif