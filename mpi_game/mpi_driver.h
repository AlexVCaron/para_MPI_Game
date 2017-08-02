#ifndef MPI_DRIVER
#define MPI_DRIVER

#include "mpi.h"

namespace mpi_driver
{
    template<int B>
    struct root_rank { static const bool value = false; };

    template<>
    struct root_rank<1> { static const bool value = true; };

    template<MPI_Datatype mpi_datatype>
    struct mpi_types
    {
        static constexpr MPI_Datatype type() { return mpi_datatype; }
    };

    template<>
    struct mpi_types<MPI_CHAR>
    {
        using serialization = char;
    };

    template<>
    struct mpi_types<MPI_INT>
    {
        using serialization = int;
    };

    struct mpi_context
    {
        int count;
        int target;
        int tag;
        MPI_Comm comm;
        MPI_Status* status;
    };

    template<MPI_Datatype mpi_datatype>
    struct broadcaster_mpi
    {
        using context_type = mpi_context;
        using direction = canal_direction::_bi;
        using message_type = typename mpi_types<mpi_datatype>::serialization;

        MPI_Datatype datatype = mpi_datatype;

        template<class mpi_context_type>
        message_type resolve(mpi_context_type& context)
        {
            message_type buff;
            MPI_Recv(&buff, context.count, datatype, context.target, context.tag, context.comm, &(*context.status));
            return buff;
        }

        template<class mpi_context_type>
        void resolve(message_type message, mpi_context_type& context)
        {
            MPI_Send(&message, context.count, datatype, context.target, context.tag, context.comm);
        }

        static broadcaster_mpi<mpi_datatype> make_broadcaster()
        {
            return broadcaster_mpi<mpi_datatype>{};
        }
    };

    template<MPI_Datatype mpi_datatype>
    struct master_broadcaster_mpi : broadcaster_mpi<mpi_datatype>
    {
        using direction = canal_direction::_bi_all;
        using message_type = typename mpi_types<mpi_datatype>::serialization;

        MPI_Datatype datatype = mpi_datatype;

        template<class mpi_context_type>
        void resolveAll(message_type message, mpi_context_type& context)
        {
            message_type buff;
            MPI_Bcast(&buff, context.count, datatype, context.target, context.comm);
            return buff;
        }
    };


    inline mpi_context make_mpi_context(int target, int tag, MPI_Comm comm)
    {
        mpi_context ct;
        ct.target = target;
        ct.tag = tag;
        ct.comm = comm;
        return std::forward<mpi_context>(ct);
    }

    template<MPI_Datatype mpi_datatype>
    using broadcaster_type = broadcaster_mpi<mpi_datatype>;
}

#endif