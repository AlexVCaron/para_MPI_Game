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
    using ct_type = typename impl::context_type;

    std::deque<m_type> queue;

    template<class direction, class ... Args>
    void request(ct_type& ct, Args&& ... args) {
        request(direction(), ct, args ...);
    };

    template<class direction, class req, class ... Args>
    void request(req* rq, ct_type& ct, Args&& ... args) {
        request(direction(), rq, ct, args ...);
    };

    bool isEmpty() { return queue.empty(); }

    ~connecteur() { queue.clear(); }
private:

    template<class direction, class ... Args>
    void request(direction, ct_type& ct, Args&& ... args) {
        request(direction(), ct, args ...);
    }

    template<class direction, class req, class ... Args>
    void request(direction, req* rq, ct_type& ct, Args&& ... args) {
        request(direction(), rq, ct, args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive, ct_type& ct, Args&& ... args)
    {
        queue.push_back(impl().resolve(ct, args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send, ct_type& ct, Args&& ... args)
    {
        impl().resolve(ct, args ...);
    }

    template<class req, class ... Args>
    void request(canal_direction::_receive, req* request, ct_type& ct, Args&& ... args)
    {
        queue.push_back(impl().resolve(request, ct, args ...));
    }
    template<class req, class ... Args>
    void request(canal_direction::_send, req* request, ct_type& ct, Args&& ... args)
    {
        impl().resolve(request, ct, args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive_all, ct_type& ct, Args&& ... args)
    {
        queue.push_back(impl().resolveAll(ct, args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send_all, ct_type& ct, Args&& ... args)
    {
        impl().resolveAll(ct, args ...);
    }

    template<class ... Args>
    void request(canal_direction::_receive_all, req_type& request, ct_type& ct, Args&& ... args)
    {
        queue.push_back(impl().resolveAll(request, ct, args ...));
    }
    template<class ... Args>
    void request(canal_direction::_send_all, req_type& request, ct_type& ct, Args&& ... args)
    {
        impl().resolveAll(request, ct, args ...);
    }
};

#endif