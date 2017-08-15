#ifndef UPDATE_H
#define UPDATE_H

#include <vector>
#include "mpi_interface.h"

template<template<class, class, size_t> class stream_type, class datatype, size_t init_queue_size = 10>
struct updateStream : stream_type<mpi_interface::mpi_main_connector<datatype>, datatype, init_queue_size> {

    using update_connector = mpi_interface::mpi_main_connector<datatype>;

    updateStream(mpi_driver::mpi_context&& ct) : stream_type<update_connector, datatype, init_queue_size>(std::move(ct)) {
        std::vector<int> counts{ 1,1 }; datatype dt;
        context.datatype = mpi_driver::createCustomDatatype(dt, counts.begin(), MPI_INT, MPI_CHAR);
    }

    ~updateStream() {
        MPI_Type_free(&(context.datatype));
    }
};

#endif