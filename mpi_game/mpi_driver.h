#ifndef MPI_DRIVER
#define MPI_DRIVER

#include "mpi.h"
#include "function_traits.h"

namespace mpi_driver
{
    template<int B>
    struct root_rank { static const bool value = false; };

    template<>
    struct root_rank<1> { static const bool value = true; };

    struct mpi_context
    {
        MPI_Datatype datatype;
        int count;
        int target;
        int tag;
        MPI_Comm comm;
        MPI_Status* status;
    };

    template<class mess_type>
    struct broadcaster_mpi
    {
        using context_type = mpi_context;
        using direction = canal_direction::_bi;
        using message_type = mess_type;

        template<class mpi_context_type>
        message_type resolve(mpi_context_type& context)
        {
            message_type buff;
            MPI_Recv(&buff, context.count, context.datatype, context.target, context.tag, context.comm, &(*context.status));
            return buff;
        }

        template<class mpi_context_type>
        void resolve(message_type message, mpi_context_type& context)
        {
            MPI_Send(&message, context.count, context.datatype, context.target, context.tag, context.comm);
        }
    };

    template<class message_type>
    struct master_broadcaster_mpi : broadcaster_mpi<message_type>
    {
        using direction = canal_direction::_bi_all;

        template<class mpi_context_type>
        message_type resolveAll(message_type message, mpi_context_type& context)
        {
            message_type buff;
            MPI_Bcast(&buff, context.count, context.datatype, context.target, context.comm);
            return buff;
        }

        template<class mpi_context_type, class F>
        message_type resolveAll(message_type message, mpi_context_type& context, F buff_resolve)
        {
            message_type *buff = malloc(sizeof(message_type) * context.count);
            MPI_Gather(&message, 1, context.datatype, buff, 1, context.datatype, context.target, context.comm);
            return buff_resolve(buff, context.count);
        }
    };


    inline mpi_context make_mpi_context(int target, int tag, MPI_Comm comm, MPI_Datatype datatype)
    {
        mpi_context ct;
        ct.target = target;
        ct.tag = tag;
        ct.comm = comm;
        ct.datatype = datatype;
        return std::forward<mpi_context>(ct);
    }

    template<class message_type>
    using broadcaster_type = broadcaster_mpi<message_type>;
}

#endif