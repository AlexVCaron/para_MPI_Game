#ifndef MPI_BROADCASTER_DRIVER
#define MPI_BROADCASTER_DRIVER

#include "mpi.h"
#include <type_traits>

namespace mpi_broadcaster_driver
{

    template<MPI_Datatype mpi_datatype>
    struct mpi_types {};

    template<>
    struct mpi_types<MPI_CHAR>
    {
        static constexpr MPI_Datatype type() { return MPI_CHAR; }
        using serialization = char;
    };

    template<>
    struct mpi_types<MPI_INT>
    {
        static constexpr MPI_Datatype type() { return MPI_INT; }
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
    struct bc_mpi
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

        static bc_mpi<mpi_datatype> make_broadcaster()
        {
            return bc_mpi<mpi_datatype>{};
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
    using broadcaster_type = bc_mpi<mpi_types<mpi_datatype>::type()>;
}

#endif