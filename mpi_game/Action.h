#ifndef ACTION_H
#define ACTION_H

#include "mpi_interface.h"

using action_connector = mpi_interface::mpi_main_connector<char>;

template<template<class, class, size_t> class stream_type, class datatype, size_t init_queue_size = 10>
struct actionStream : stream_type<action_connector, datatype, init_queue_size>
{
    actionStream() = delete;
    explicit actionStream(mpi_driver::mpi_context&& ct) : stream_type<action_connector, datatype, init_queue_size>(std::move(ct)) {}
};

#endif