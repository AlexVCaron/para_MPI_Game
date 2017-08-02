#ifndef MPI_INTERFACE_H
#define MPI_INTERFACE_H

#include <mpi.h>
#include "mpi_driver.h"
#include "canal.h"
#include "connecteur.h"
#include <vector>

namespace mpi_interface
{
    template<MPI_Datatype mpi_datatype>
    using mpi_connector = connecteur<canal_juge<mpi_driver::broadcaster_type<mpi_datatype>>>;
    template<MPI_Datatype mpi_datatype>
    using mpi_connector_root = connecteur<canal_juge<mpi_driver::master_broadcaster_mpi<mpi_datatype>>>;

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

    struct init_payload {};
    struct mpi_variables
    {
        int count;
        std::vector<int> blocks;
        std::vector<MPI_Datatype> types;
        std::vector<MPI_Aint> displacements;
    };

    inline void realizeInitHandshake (const int rang, init_payload i_pl, mpi_variables mpi_vars)
    {
        const MPI_Datatype obj_type = MPI_CHAR;
        MPI_Type_create_struct(mpi_vars.count, &(*mpi_vars.blocks.begin()), &(*mpi_vars.displacements.begin()), &(*mpi_vars.types.begin()), const_cast<MPI_Datatype*>(&obj_type));
        MPI_Type_commit(const_cast<MPI_Datatype*>(&obj_type));

        if (rang == root_rank) {
            mpi_connector_root<obj_type> connector{};
            auto init_context(mpi_driver::make_mpi_context(
                0, 0, MPI_COMM_WORLD
                ));

            connector.request<canal_direction::_send_all>(i_pl, init_context);
        }
        else
        {
            mpi_connector<MPI_CHAR> connector{};
            auto init_context(mpi_driver::make_mpi_context(
                0, 0, MPI_COMM_WORLD
                ));
            connector.request<canal_direction::_receive>(i_pl, init_context);
        }
    }
}

#endif