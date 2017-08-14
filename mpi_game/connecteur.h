#ifndef CONNECTEUR_H
#define CONNECTEUR_H
#include "canal.h"
#include <deque>

template<class canal_type>
struct connecteur
{
    using m_type = typename canal_traits<canal_type>::message_type;
    using impl = typename canal_traits<canal_type>::impl_type;
    using req_type = typename impl::request_type;

    std::deque<m_type> queue;

    template<class direction, class ... Args>
    void request(Args&& ... args) {
        request(direction(), args ...);
    };

    template<class direction, class req, class ... Args>
    void request(req* rq, Args&& ... args) {
        request(direction(), rq, args ...);
    };

    bool isEmpty() { return queue.empty(); }

    ~connecteur() { queue.clear(); }
private:

    template<class direction, class ... Args>
    void request(direction, Args&& ... args) {
        request(direction(), args ...);
    }

    template<class direction, class req, class ... Args>
    void request(direction, req* rq, Args&& ... args) {
        request(direction(), rq, args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive, Args&& ... args)
    {
        queue.push_back(impl().resolve(args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send, Args&& ... args)
    {
        impl().resolve(args ...);
    }

    template<class req, class ... Args>
    void request(canal_direction::_receive, req* request, Args&& ... args)
    {
        queue.push_back(impl().resolve(request, args ...));
    }
    template<class req, class ... Args>
    void request(canal_direction::_send, req* request, Args&& ... args)
    {
        impl().resolve(request, args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive_all, Args&& ... args)
    {
        queue.push_back(impl().resolveAll(args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send_all, Args&& ... args)
    {
        impl().resolveAll(args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive_all, req_type& request, Args&& ... args)
    {
        queue.push_back(impl().resolveAll(request, args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send_all, req_type& request, Args&& ... args)
    {
        impl().resolveAll(request, args ...);
    }
};

#endif