#ifndef BASE_STREAM_H
#define BASE_STREAM_H

#include "mpi.h"
#include "mpi_driver.h"
#include <cassert>

template<class datatype>
struct request
{
    MPI_Request rq;
    datatype data;
};

template<class connector_t, class datatype, size_t init_queue_size = 10>
struct mpi_stream_frame {
protected:
    using handles_it = typename std::vector<request<datatype>>::iterator;
    connector_t connector;
    std::vector<request<datatype>> action_handles{ init_queue_size + 1 };
    std::vector<unsigned int> action_handles_busy;
    size_t fill;
    bool completed(typename std::vector<request<datatype>>::iterator req)
    {
        MPI_Status status; int completed = false;
        try {
            MPI_Test(&(req->rq), &completed, &status);
        } catch( std::exception e) {}
        return completed;
    }

public:
    mpi_driver::mpi_context context;
    mpi_stream_frame() = delete;
    explicit mpi_stream_frame(mpi_driver::mpi_context ct) : context{ std::move(ct) } { action_handles_busy.resize(init_queue_size + 1, false); fill = 0; };
    mpi_stream_frame(mpi_stream_frame&& oth) noexcept : connector{ oth.connector }, action_handles{ oth.action_handles }, action_handles_busy{ oth.action_handles_busy }, fill { oth.fill } {

    }
    mpi_stream_frame(mpi_stream_frame& oth)  : connector{ oth.connector }, action_handles{ oth.action_handles }, action_handles_busy{ oth.action_handles_busy }, fill{ oth.fill } {

    }

    typename std::vector<request<datatype>>::iterator begin() { return action_handles.begin(); }

    void scale(size_t size) { assert(size > action_handles.size()); action_handles.resize(size); action_handles_busy.resize(size, false); }

    size_t size() { return action_handles.size(); }

    bool empty() { return fill == 0; }

    void clear()
    {
        for (auto it = action_handles.begin() + fill; it != action_handles.begin(); --it)
        {
            std::cout << "destroying requests !!!!!!!" << std::endl;
            MPI_Cancel(&(it->rq));
            MPI_Request_free(&(it->rq));
        }
        fill = 0;
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
    explicit in_stream(mpi_driver::mpi_context ct) : mpi_stream_frame{ std::move(ct) } {};
    in_stream& operator>>(request_it& req) {
        while (fill != action_handles.size() - 1 && completed(action_handles.begin() + fill)) {
            ++fill;
            connector.request<canal_direction::_receive>(&(action_handles[fill].rq), context);
        }
        req = action_handles.begin();
        return *this;
    }

    datatype unpack(typename std::vector<request<datatype>>::iterator& req, int& caller)
    {
        MPI_Status status;
        if (!completed(req)) MPI_Request_get_status(req->rq, nullptr, &status);
        req->data = connector.queue.at(req - action_handles.begin());
        connector.queue.erase(connector.queue.begin() + (req - action_handles.begin()));
        action_handles_busy[req - action_handles.begin()] = false;
        --fill;
        caller = fill + 2;
        return action_handles[fill + 1].data;
    }
};

template<class connector_t, class datatype, size_t init_queue_size = 10>
struct a_in_stream : in_stream<connector_t, datatype, init_queue_size> {
    explicit a_in_stream(mpi_driver::mpi_context ct) : in_stream{ std::move(ct) } {};
    a_in_stream& operator>>(request_it& req) {
        req = action_handles.begin();
        if (fill == action_handles.size()) fill = 0;
        while (fill != action_handles.size() - 1) {
            std::cout << "AC_IN | processing " << fill << std::endl;
            if (!(action_handles_busy[fill])) {
                action_handles_busy[fill] = true;
                //context.target = fill + 1;
                connector.request<canal_direction::_receive>(&(action_handles[fill].rq), context);
                if (completed(action_handles.begin() + fill)) req = action_handles.begin() + fill;
            }
            if (completed(action_handles.begin() + fill)) req = action_handles.begin() + fill;
            ++fill;
        }
        return *this;
    }
};

template<class connector_t, class datatype, size_t init_queue_size = 10>
struct out_stream : mpi_stream_frame<connector_t, datatype, init_queue_size> {
    explicit out_stream(mpi_driver::mpi_context ct) : mpi_stream_frame{ std::move(ct) } {};
    out_stream& operator<<(datatype mess) {
        MPI_Request req; MPI_Status status;
        connector.request<canal_direction::_send>(&req, mess, context);
        MPI_Wait(&req, &status);
        if (req != MPI_REQUEST_NULL) MPI_Request_free(&req);
        return *this;
    }
};

#endif
