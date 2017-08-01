#ifndef CANAL_TAGS_H
#define  CANAL_TAGS_H

template<class Broadcaster>
struct canal_tag
{
    typedef typename Broadcaster::direction direction;
    typedef Broadcaster canal_impl;
};

template<class Broadcaster>
struct canal_juge : canal_tag<Broadcaster> {};
template<class Broadcaster>
struct canal_carte : canal_tag<Broadcaster> {};
template<class Broadcaster>
struct canal_acteur : canal_tag<Broadcaster> {};

template<class c_tag>
struct canal_traits
{
    typedef c_tag type;
    
    typedef typename c_tag::direction direction;

    typedef typename c_tag::canal_impl impl_type;
    typedef typename impl_type::message_type message_type;
};

struct canal_direction
{
    typedef struct direction_in {} _send;
    typedef struct direction_out {} _receive;
    typedef struct direction_bi {} _bi;
};

#endif