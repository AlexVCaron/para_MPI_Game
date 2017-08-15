#ifndef BASE_STREAM_H
#define BASE_STREAM_H

#include "mpi.h"
#include "mpi_driver.h"
#include <cassert>

template<class datatype>
struct request_t
{
    MPI_Request rq;
    datatype data;
};

template<class connector_t, class datatype, size_t init_queue_size = 10>
struct mpi_stream_frame {
protected:
    using handles_it = typename std::vector<request_t<datatype>>::iterator;
    connector_t connector;
    std::vector<request_t<datatype>> action_handles{ init_queue_size + 1 };
    std::vector<unsigned int> action_handles_busy;
    size_t fill;
    bool completed(typename std::vector<request_t<datatype>>::iterator req)
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

    typename std::vector<request_t<datatype>>::iterator begin() { return action_handles.begin(); }

    void scale(size_t size) { assert(size > action_handles.size()); action_handles.resize(size); action_handles_busy.resize(size, false); }

    size_t size() { return action_handles.size(); }

    bool empty() { return connector.queue.empty(); }

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
    using request_it = typename std::vector<request_t<datatype*>>::iterator;
    explicit in_stream(mpi_driver::mpi_context ct) : mpi_stream_frame{ std::move(ct) } {};
    in_stream& operator>>(request_it& req) {
        connector.request<canal_direction::_receive>(context);
        return *this;
    }

    datatype* unpack(typename std::vector<request_t<datatype*>>::iterator& req, int& caller)
    {
        caller = context.sender;
        datatype* dt = connector.queue.front();
        connector.queue.pop_front();
        return dt;
    }
};

template<class connector_t, class datatype, size_t init_queue_size = 10>
struct a_in_stream : in_stream<connector_t, datatype, init_queue_size> {
    explicit a_in_stream(mpi_driver::mpi_context ct) : in_stream{ std::move(ct) } {};
    a_in_stream& operator>>(request_it& req) {
        connector.request<canal_direction::_receive>(context);
        return *this;
    }
};

template<class connector_t, class datatype, size_t init_queue_size = 10>
struct out_stream : mpi_stream_frame<connector_t, datatype, init_queue_size> {
    explicit out_stream(mpi_driver::mpi_context ct) : mpi_stream_frame{ std::move(ct) } {};
    out_stream& operator<<(datatype* mess) {
        connector.request<canal_direction::_send>(context, mess);
        return *this;
    }
};

#endif
