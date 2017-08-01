#ifndef CONNECTEUR_H
#define CONNECTEUR_H
#include "canal.h"
#include <deque>

template<class canal_type>
struct connecteur
{
    using m_type = typename canal_traits<canal_type>::message_type;
    using impl = typename canal_traits<canal_type>::impl_type;

    std::deque<m_type> queue;

    template<class direction, class ... Args>
    void request(Args&& ... args) {
        request(args ..., direction());
    };

    ~connecteur() { queue.clear(); }
private:
    void request(canal_direction::_receive)
    {
        impl i; queue.push_back(i.resolve());
    }
    void request(const m_type& t, canal_direction::_send)
    {
        impl i; i.resolve(t);
    }
};

#endif