#ifndef MPI_INTERFACE_H
#define MPI_INTERFACE_H

#include <mpi.h>
#include "mpi_driver.h"
#include "canal.h"
#include "connecteur.h"
#include <vector>
#include <iostream>

namespace mpi_interface
{
    template<class message_type>
    using mpi_connector_juge = connecteur<canal_juge<mpi_driver::broadcaster_type<message_type>>>;
    template<class message_type>
    using mpi_connector_carte = connecteur<canal_carte<mpi_driver::master_broadcaster_mpi<message_type>>>;
    template<class message_type>
    using mpi_connector_acteur = connecteur<canal_acteur<mpi_driver::master_broadcaster_mpi<message_type>>>;

    struct canal_juge_tag {};
    struct canal_carte_tag {};

    constexpr int root_rank = 0;

    struct MPI_Scope
    {
    private:
        int nb_procs;
        int rang_;
        void initMPI(int &argc, char **argv)
        {
            MPI_Init(&argc, &argv);
        }
    public:
        MPI_Scope(const MPI_Scope&) = delete;
        MPI_Scope& operator=(const MPI_Scope&) = delete;
        MPI_Scope(int argc, char *argv[]) {
            initMPI(argc, argv);
            MPI_Comm_size(MPI_COMM_WORLD, &nb_procs);
            MPI_Comm_rank(MPI_COMM_WORLD, &rang_); 
        }
        int nb_processus() const noexcept {
            return nb_procs;
        }
        int rang() const noexcept {
            return rang_;
        }
        ~MPI_Scope() {
            MPI_Finalize();
        }
    };

    struct init_payload
    {
        int canal_juge_tag = 0;
        int canal_carte_tag = 1;
        int actor_rank = -1;
    };
    struct mpi_variables
    {
        int count;
        std::vector<int> blocks;
        std::vector<MPI_Datatype> types;
        std::vector<MPI_Aint> displacements;
    };

    inline void realizeInitHandshake (const int rang)
    {
        init_payload i_pl;
        mpi_variables mpi_vars;
        mpi_vars.count = 3;
        mpi_vars.blocks.push_back(1);mpi_vars.blocks.push_back(1);mpi_vars.blocks.push_back(1);
        mpi_vars.types.push_back(MPI_INT);mpi_vars.types.push_back(MPI_INT);mpi_vars.types.push_back(MPI_INT);

        MPI_Aint intex, intlb;
        MPI_Type_get_extent(MPI_INT, &intlb, &intex);

        mpi_vars.displacements.push_back(static_cast<MPI_Aint>(0));mpi_vars.displacements.push_back(intex);mpi_vars.displacements.push_back(intex + intex);

        MPI_Datatype obj_type;
        MPI_Type_create_struct(mpi_vars.count, &(*mpi_vars.blocks.begin()), &(*mpi_vars.displacements.begin()), &(*mpi_vars.types.begin()), &obj_type);
        MPI_Type_commit(const_cast<MPI_Datatype*>(&obj_type));

        mpi_connector_carte<init_payload> connector_carte{};
        auto init_context(mpi_driver::make_mpi_context(
            0, 0, MPI_COMM_WORLD, obj_type
            ));
        init_context.count = 1;

        connector_carte.request<canal_direction::_send_all>(i_pl, init_context);

        if (rang != root_rank) {
            i_pl = connector_carte.queue.front();
            std::cout << "I am an actor process #" << rang << " | Hello World !!! | c : " << i_pl.canal_carte_tag << " j : " << i_pl.canal_juge_tag << " q_s : " << connector_carte.queue.size() << " |" << std::endl;
        }
        else
        {
            std::cout << "I am the lead process | c : " << i_pl.canal_carte_tag << " j : " << i_pl.canal_juge_tag << " q_s : " << connector_carte.queue.size() << " |" << std::endl;
        }
    }
}

#endif