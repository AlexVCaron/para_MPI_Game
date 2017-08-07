#ifndef CARTE_H
#define CARTE_H
#include <vector>
#include "connecteur.h"
#include <mpi.h>
#include "mpi_driver.h"

namespace carte
{

    class Juge
    {
        using action_connector = connecteur<canal_acteur<mpi_driver::broadcaster_mpi<std::pair<int, int>>>>;
        bool in_function = true;
        std::vector<MPI_Request> action_handles;
        action_connector connector;
        int nb_actors;
        mpi_driver::mpi_context context;
    public:
        void listen(int source)
        {
            for (int i = 0; i < nb_actors; ++i)
            {
                action_handles.emplace_back(connector.request<canal_direction::_receive, request_type::is_async>(context));
            }

            while(in_function)
            {
                
            }
        }

        void close() { in_function = false; }
    };

    class Scene
    {
        unsigned int nb_rat, nb_chasseurs, nb_fromages;
        std::vector<char> grille;

        void countGridElements()
        {
            for (size_t i = 0; i < grille.size(); ++i)
            {
                if (grille[i] == 'C') nb_chasseurs++;
                else if (grille[i] == 'R') nb_rat++;
                else if (grille[i] == 'F') nb_fromages++;
            }
        }

    public:
        Scene(std::vector<char>&& grille) : nb_rat{ 0 }, nb_chasseurs{ 0 }, nb_fromages{ 0 }, grille{ grille } { countGridElements(); }

        template <class T>
        using actor_ct = connecteur<canal_acteur<mpi_driver::master_broadcaster_mpi<T>>>;

        void assignRoles(unsigned int nb_actor_procs)
        {
            actor_ct<char> connector;
            auto context = mpi_driver::make_mpi_context(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_CHAR);
            for (unsigned int i = 0; i < nb_actor_procs; ++i)
            {
                connector.request<canal_direction::_send>(i > nb_rat, context);
            }
        }
    };

    class Carte
    {

        Juge juge;
        Scene scene;

        void initializeActors(unsigned int nb_actor_procs)
        {
            scene.assignRoles(nb_actor_procs);
        }

        void endGame()
        {
            
        }

    public:
        Carte() = delete;
        Carte(unsigned int nb_actor_procs, std::vector<char>&& grille) : scene{ std::move(grille) }
        {
            
        }

        void startGame()
        {
            juge.listen(MPI_ANY_SOURCE);
        }

    };




}

#endif