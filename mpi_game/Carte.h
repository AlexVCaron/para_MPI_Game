#ifndef CARTE_H
#define CARTE_H
#include <vector>
#include "connecteur.h"
#include <mpi.h>
#include "mpi_driver.h"
#include "Action.h"
#include <iostream>
#include "Base.h"

namespace carte
{
    using datatype = char;

    const size_t MAX_QUEUE_SIZE = 10;

    class Scene
    {
        unsigned int nb_rat, nb_chasseurs, nb_fromages;
        std::vector<datatype> grille;

        void countGridElements()
        {
            for (size_t i = 0; i < grille.size(); ++i)
            {
                if (grille[i] == 'C') nb_chasseurs++;
                else if (grille[i] == 'R') nb_rat++;
                else if (grille[i] == 'F') nb_fromages++;
            }
        }

        struct update
        {
        private:
            using connector = connecteur<canal_acteur<mpi_driver::master_broadcaster_mpi<std::vector<char>>>>;
        public:
            std::vector<datatype> updates;
            connector update_connector;
        };

    public:
        Scene(std::vector<datatype>&& grille) : nb_rat{ 0 }, nb_chasseurs{ 0 }, nb_fromages{ 0 }, grille{ grille } { countGridElements(); }

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

        void propagateUpdate()
        {
            
        }

        void updateSelf(datatype data)
        {
            std::cout << data;
        }
    };

    class Juge
    {

        bool in_function = true;
        actionStream<in_stream, datatype, MAX_QUEUE_SIZE> action_stream;
        int nb_actors;
        Scene* scene_ptr;

    public:
        void listen(int source)
        {
            action_stream.context.target = source;
            std::vector<request<datatype>>::iterator actions;
            while (in_function && action_stream >> actions)
            {
                if (action_stream.empty()) {
                    processAction(action_stream.unpack(actions));
                }
                else
                {
                    scene_ptr->propagateUpdate();
                }
            }
            action_stream.clear();
        }

        void processAction(datatype action) const
        {
            scene_ptr->updateSelf(action);
        }

        void close() { in_function = false; }
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