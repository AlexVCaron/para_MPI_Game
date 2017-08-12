#ifndef UPDATE_H
#define UPDATE_H

#include <vector>
#include "mpi_interface.h"

struct update_package {
    std::vector<std::pair<int, char>> updates;
};

using update_connector = mpi_interface::mpi_connector_carte<update_package>;

template<template<class, class, size_t> class stream_type, class datatype, size_t init_queue_size = 10>
struct update_stream : stream_type<action_connector, datatype, init_queue_size> { };

#endif