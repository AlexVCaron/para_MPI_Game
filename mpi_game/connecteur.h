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

    template<class direction, class sync = request_type::is_sync, class ... Args>
    void request(Args&& ... args) {
        request(direction(), sync(), args ...);
    };

    bool isEmpty() { return queue.empty(); }

    ~connecteur() { queue.clear(); }
private:

    template<class direction, class sync, class ... Args>
    void request(direction, request_type::is_sync, Args&& ... args) {
        request(direction(), args ...);
    }

    template<class direction, class ... Args>
    void request(direction, request_type::is_async, req_type& req, Args&& ... args) {
        request(direction(), req, args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive, request_type::is_sync, Args&& ... args)
    {
        queue.push_back(impl().resolve(args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send, request_type::is_sync, Args&& ... args)
    {
        impl().resolve(args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive, request_type::is_async, req_type& request, Args&& ... args)
    {
        queue.push_back(impl().resolveAll(request, args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send, request_type::is_async, req_type& request, Args&& ... args)
    {
        impl().resolveAll(request, args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive_all, request_type::is_sync, Args&& ... args)
    {
        queue.push_back(impl().resolveAll(args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send_all, request_type::is_sync, Args&& ... args)
    {
        impl().resolveAll(args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive_all, request_type::is_async, req_type& request, Args&& ... args)
    {
        queue.push_back(impl().resolveAll(request, args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send_all, request_type::is_async, req_type& request, Args&& ... args)
    {
        impl().resolveAll(request, args ...);
    }
};

#endif