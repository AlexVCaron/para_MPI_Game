#ifndef ACTION_H
#define ACTION_H
#include <mpi.h>
#include <vector>
#include "canal_types.h"
#include "mpi_driver.h"
#include "Carte.h"

using action_connector = connecteur<canal_acteur<mpi_driver::broadcaster_mpi<std::pair<int, int>>>>;

template<class datatype>
struct request
{
    MPI_Request* rq;
    datatype* data;
};

template<class datatype, size_t init_queue_size = 10>
struct actionStream
{
private:
    action_connector connector;
    std::vector<request<datatype>> action_handles{ init_queue_size };
    typename std::vector<request<datatype>>::iterator fill;
    bool completed(typename std::vector<request<datatype>>::iterator req)
    {
        MPI_Status status; int completed;
        MPI_Test(req->rq, &completed, &status);
        return completed;
    }

public:
    mpi_driver::mpi_context context;

    explicit actionStream(mpi_driver::mpi_context&& ct) : fill{ action_handles.begin() }, context{ std::move(ct) } { };

    void scale(size_t size) { assert(size > action_handles.size()); action_handles.resize(size); }

    size_t size() { return action_handles.size(); }

    bool empty() { return fill == action_handles.begin(); }

    void clear()
    {
        for (auto it = action_handles.begin(); it != fill + 1; ++it)
        {
            MPI_Cancel(it->rq);
            MPI_Request_free(it->rq);
        }
        fill = action_handles.begin();
    }

    actionStream& operator>>(typename std::vector<request<datatype>>::iterator& req) {
        while (fill != action_handles.end() && completed(fill))
            connector.request<canal_direction::_receive, request_type::is_async>(fill++, context);
        req = action_handles.begin();
        return *this;
    }

    datatype unpack(typename std::vector<request<datatype>>::iterator& req)
    {
        MPI_Status message;
        if (!completed(req)) MPI_Request_get_status(*(req->rq), nullptr, &message);
        MPI_Request_free(req->rq);
        for (auto me = req, next = req + 1;me != fill; ++me, ++next) std::swap(*me, *next);
        --fill;
        return *((fill + 1)->data);
    }

    explicit operator bool() const
    {
        return true;
    }

    ~actionStream()
    {
        clear();
    }
};

#endif