#ifndef ACTION_H
#define ACTION_H
#include <mpi.h>
#include <vector>
#include "canal_types.h"
#include "mpi_interface.h"


using action_connector = mpi_interface::mpi_connector_juge<char>;

template<template<class, class, size_t> class stream_type, class datatype, size_t init_queue_size = 10>
struct actionStream : stream_type<action_connector, datatype, init_queue_size> { };

#endif