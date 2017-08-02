#ifndef CONNECTEUR_H
#define CONNECTEUR_H
#include "canal.h"
#include <deque>
#include "broadcaster_traits.h"

template<class canal_type>
struct connecteur
{
    using m_type = typename canal_traits<canal_type>::message_type;
    using impl = typename canal_traits<canal_type>::impl_type;

    std::deque<m_type> queue;

    template<class direction, class ... Args>
    void request(Args&& ... args) {
        request(direction(), args ...);
    };

    bool isEmpty() { return queue.empty(); }

    ~connecteur() { queue.clear(); }
private:
    template<class ... Args>
    void request(canal_direction::_receive, Args&& ... args)
    {
        queue.push_back(broadcaster_traits<impl>::make_broadcaster().resolve(args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send, Args&& ... args)
    {
        broadcaster_traits<impl>::make_broadcaster().resolve(args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive_all, Args&& ... args)
    {
        queue.push_back(broadcaster_traits<impl>::make_broadcaster().resolveAll(args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send_all, Args&& ... args)
    {
        broadcaster_traits<impl>::make_broadcaster().resolveAll(args ...);
    }
};

#endif