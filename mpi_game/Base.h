#ifndef BASE_STREAM_H
#define BASE_STREAM_H

#include "mpi.h"
#include "mpi_driver.h"
#include "connecteur.h"

template<class datatype>
struct request
{
    MPI_Request* rq;
    datatype* data;
};

template<class connector_t, class datatype, size_t init_queue_size = 10>
struct mpi_stream_frame {
private:
    connector_t connector;
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

    explicit mpi_stream_frame(mpi_driver::mpi_context&& ct) : fill{ action_handles.begin() }, context{ std::move(ct) } { };

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

    explicit operator bool() const
    {
        return true;
    }

    ~mpi_stream_frame()
    {
        clear();
    }
};

template<class connector_t, class datatype, size_t init_queue_size = 10>
struct in_stream : mpi_stream_frame<connector_t, datatype, init_queue_size> {
    using request_it = typename std::vector<request<datatype>>::iterator;
    explicit in_stream(mpi_driver::mpi_context&& ct) : mpi_stream_frame{ ct } {};
    in_stream& operator>>(request_it& req) {
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
        for (auto me = req, next = req + 1; me != fill; ++me, ++next) std::swap(*me, *next);
        --fill;
        return *((fill + 1)->data);
    }
};

template<class connector_t, class datatype, size_t init_queue_size = 10>
struct out_stream : mpi_stream_frame<connector_t, datatype, init_queue_size> {
    explicit out_stream(mpi_driver::mpi_context&& ct) : mpi_stream_frame{ ct } {};
    out_stream& operator<<(datatype mess) {
        MPI_Request req;
        connector.request<canal_direction::_send, request_type::is_async>(mess, req, context);
        MPI_Request_free(&req);
        return *this;
    }
};

#endif
