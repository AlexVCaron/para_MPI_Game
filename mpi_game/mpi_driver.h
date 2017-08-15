#ifndef MPI_DRIVER
#define MPI_DRIVER

#include "mpi.h"
#include "canal_types.h"
#include <vector>

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
        int sender;
        mpi_context() {}
        mpi_context(mpi_context&& oth) noexcept : datatype{ oth.datatype }, count{ oth.count }, target{ oth.target }, tag{ oth.tag }, comm{ oth.comm }, status{ oth.status }, sender{ oth.sender } {}
        mpi_context(mpi_context& oth) : datatype{ oth.datatype }, count{ oth.count }, target{ oth.target }, tag{ oth.tag }, comm{ oth.comm }, status{ oth.status }, sender{ oth.sender } {}
        mpi_context& operator=(mpi_context oth) { 
            datatype = oth.datatype;
            count = oth.count;
            target = oth.target;
            tag = oth.tag;
            comm = oth.comm;
            status = oth.status;
            sender = oth.sender;
        }
    };

    template<class mess_type>
    struct broadcaster_mpi
    {
        using context_type = mpi_context;
        using direction = canal_direction::_bi;
        using message_type = mess_type*;
        using request_type = MPI_Request;

        template<class mpi_context_type>
        message_type resolve(mpi_context_type& context)
        {
            MPI_Status status; int number_amount;
            MPI_Probe(context.target, context.tag, context.comm, &status);
            MPI_Get_count(&status, context.datatype, &number_amount);
            std::cout << "preparing to receive " << number_amount << " from " << status.MPI_SOURCE << std::endl;
            message_type buff = static_cast<message_type>(malloc(sizeof(mess_type) * context.count));
            context.sender = status.MPI_SOURCE;
            MPI_Recv(buff, number_amount, context.datatype, status.MPI_SOURCE, context.tag, context.comm, &status);
            context.count = number_amount;
            return buff;
        }

        template<class mpi_context_type>
        void resolve(mpi_context_type& context, message_type message)
        {
            MPI_Send(message, context.count, context.datatype, context.target, context.tag, context.comm);
        }

        template<class mpi_context_type>
        message_type resolveAll(mpi_context_type& context)
        {
            message_type message = static_cast<message_type>(malloc(sizeof(mess_type) * context.count));
            MPI_Bcast(message, context.count, context.datatype, context.target, context.comm);
            return message;
        }
    };

    template<class message_type>
    struct master_broadcaster_mpi : broadcaster_mpi<message_type>
    {
        using direction = canal_direction::_bi_all;

        template<class mpi_context_type>
        message_type resolveAll(mpi_context_type& context, message_type message)
        {
            MPI_Bcast(message, context.count, context.datatype, context.target, context.comm);
            return message;
        }

        template<class mpi_context_type, class F>
        message_type resolveAll(mpi_context_type& context, message_type message, F buff_resolve)
        {
            message_type *buff = static_cast<message_type*>(malloc(sizeof(message_type) * context.count));
            MPI_Gather(message, 1, context.datatype, buff, 1, context.datatype, context.target, context.comm);
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

    inline mpi_context make_mpi_context(int target, int tag, MPI_Comm comm)
    {
        mpi_context ct;
        ct.target = target;
        ct.tag = tag;
        ct.comm = comm;
        return std::forward<mpi_context>(ct);
    }

    struct mpi_variables
    {
        int count = 0;
        std::vector<int> blocks;
        std::vector<MPI_Datatype> types;
        std::vector<MPI_Aint> displacements;
        mpi_variables() : blocks(size_t(0)), types(size_t(0)), displacements(size_t(0)) {}
        mpi_variables(mpi_variables& oth)
        {
            count = oth.count;
            blocks = oth.blocks;
            types = oth.types;
            displacements = oth.displacements;
        }
    };

    template<class ... Args>
    MPI_Datatype createCustomDatatype(std::vector<int>::iterator counts, Args&& ... t)
    {
        MPI_Datatype obj_type;
        mpi_variables mpi_vars;

        createCustomDatatype(mpi_vars, counts, t ...);

        MPI_Type_create_struct(mpi_vars.count, &(*mpi_vars.blocks.begin()), &(*mpi_vars.displacements.begin()), &(*mpi_vars.types.begin()), &obj_type);
        MPI_Type_commit(const_cast<MPI_Datatype*>(&obj_type));

        return obj_type;
    }

    template<class arg, class ... Args>
    void createCustomDatatype(mpi_variables& mpi_vars, std::vector<int>::iterator& counts, arg a, Args&& ... t)
    {
        createCustomDatatype(mpi_vars, counts, std::forward<arg>(a));
        ++counts;
        createCustomDatatype(mpi_vars, counts, t ...);
    }

    template<class arg>
    void createCustomDatatype(mpi_variables& mpi_vars, std::vector<int>::iterator& count, arg a)
    {
        mpi_vars.count += 1;
        int b = *count;
        mpi_vars.blocks.push_back(b);
        mpi_vars.types.push_back(a);

        MPI_Aint intex, intlb;
        MPI_Type_get_extent(a, &intlb, &intex);

        if (mpi_vars.count == 1)
        {
            mpi_vars.displacements.push_back(static_cast<MPI_Aint>(0));
        }
        else
        {
            mpi_vars.displacements.push_back(mpi_vars.displacements.back() + intex);
        }
    }

    template<class message_type>
    using broadcaster_type = broadcaster_mpi<message_type>;
}

#endif