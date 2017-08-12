#ifndef MPI_INTERFACE_H
#define MPI_INTERFACE_H

#include <mpi.h>
#include "mpi_driver.h"
#include "canal.h"
#include "connecteur.h"
#include <vector>
#include <iostream>
#include <string>
#include <forward_list>
#include "Incopiable.h"

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

    inline void realizeInitHandshake(const int rang)
    {
        std::vector<int> blocks{ 1,1,1 }; auto it = blocks.begin();
        MPI_Datatype datatype = mpi_driver::createCustomDatatype(it, MPI_INT, MPI_INT, MPI_INT);

        init_payload i_pl;

        mpi_connector_carte<init_payload> connector_carte{};
        auto init_context(mpi_driver::make_mpi_context(
            0, 0, MPI_COMM_WORLD, datatype
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

        MPI_Type_free(&datatype);
    }

    struct signal_handle : Incopiable
    {
        MPI_Win* signal_window;

        explicit signal_handle(MPI_Win& window) : signal_window{ &window } {}

        template<class T, MPI_Datatype datatype>
        void put(T message, int target)
        {
            MPI_Put(&message, 1, datatype, target, 0, 1, datatype, *signal_window);
        }
    };

    struct read_only_signal
    {
        MPI_Win* signal_window;

        explicit read_only_signal(MPI_Win& window) : signal_window{ &window } {}

        template<class T, MPI_Datatype datatype>
        T get(int target)
        {
            T message;
            MPI_Get(&message, 1, datatype, target, 0, 1, datatype, *signal_window);
            return std::move(message);
        }

    };
}

#endif