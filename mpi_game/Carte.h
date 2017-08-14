#ifndef CARTE_H
#define CARTE_H
#include <vector>
#include "connecteur.h"
#include <mpi.h>
#include "mpi_driver.h"
#include "Action.h"
#include "Update.h"
#include <iostream>
#include "Base.h"
#include <algorithm>
#include <thread>

using namespace std;

namespace carte
{
    using datatype = char;
    using action_datatype = int;

    const size_t MAX_QUEUE_SIZE = 10;
    

    class Scene
    {
        struct update_pack { int pos; char val; };
        using update_datatype = update_pack;
        using update_metatype = int*;
        using update_meta_stream_t = updateStream<out_stream, update_metatype, 1>;
        using update_data_stream_t = updateStream<out_stream, update_datatype, 1>;

        update_meta_stream_t update_m_stream;
        update_data_stream_t update_d_stream;

        unsigned int nb_rat, nb_chasseurs, nb_fromages;
        std::vector<datatype> grille; int width; int height;
        std::vector<unsigned int> actor_pos;

        mpi_interface::signal_handle end_o_game_signal;

        void countGridElements()
        {
            for (size_t i = 0; i < grille.size(); ++i)
            {
                if (grille[i] == 'C') nb_chasseurs++;
                else if (grille[i] == 'R') nb_rat++;
                else if (grille[i] == 'F') nb_fromages++;
            }
        }

        std::vector<update_pack> updates;

    public:
        Scene(std::vector<datatype>&& grille, MPI_Win* end_o_game_w) : end_o_game_signal{ end_o_game_w }, nb_rat { 0 }, nb_chasseurs{ 0 }, nb_fromages{ 0 }, grille{ grille },
                                                                       update_m_stream(mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD, MPI_INT)),
                                                                       update_d_stream(mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD))
        {
            update_m_stream.context.count = 2;
            countGridElements();
        }


        void endGame()
        {
            for (unsigned int i = 1; i < nb_chasseurs + nb_rat; ++i) end_o_game_signal.put<MPI_C_BOOL>(true, i);
        }

        void assignRoles(unsigned int nb_actor_procs)
        {
            mpi_interface::mpi_main_connector<char> connector;
            auto context = mpi_driver::make_mpi_context(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_CHAR);
            context.count = 1;
            for (unsigned int i = 1; i <= nb_actor_procs; ++i)
            {
                std::string me = (i > nb_rat) ? "cat" : "rat";
                std::cout << "map initializing actor nb " << i << " . It a " << me << "." << std::endl;
                context.target = i;
                if(i > nb_rat) connector.request<canal_direction::_send>('0', context);
                else connector.request<canal_direction::_send>('1', context);
            }
        }

        void triggerMeow(int actor) 
        {
            std::vector<unsigned int> pos;
            int i_a = actor_pos[actor] % width, j_a = actor_pos[actor] / width;
            for (int i = std::max(0, i_a - 4); i < std::min(width, i_a + 3); ++i) {
                for (int j = std::max(0, j_a - 4); j < std::min(height, j_a + 3); ++j) {
                    if (grille[i * width + j] == 'R')  pos.push_back(i * width + j);
                }
            }
            int meta[2] = { -1, 0 };
            std::for_each(pos.begin(), pos.end(), [&](unsigned int pos){
                update_m_stream.context.target = *(std::find(actor_pos.begin(), actor_pos.end(), pos));
                update_m_stream << meta;
            });
        }

        void propagateUpdate()
        {
            int meta[2] = { 1 , updates.size() };
            update_d_stream.context.count = updates.size();
            for (int i = 1; i <= nb_chasseurs + nb_rat; ++i) {
                update_m_stream.context.target = i;
                update_d_stream.context.target = i;
                update_m_stream << meta;
                update_d_stream << updates.front();
            }
            
        }

        void updateSelf(std::pair<int,action_datatype> data)
        {
            std::swap(grille[actor_pos[data.first]], grille[data.second]);
            update_pack pack_1, pack_2; 
            pack_1.pos = actor_pos[data.first]; pack_1.val = grille[actor_pos[data.first]];
            pack_2.pos = data.second; grille[data.second];

            updates.push_back(pack_1);
            updates.push_back(pack_2);
        }
    };

    class Juge
    {
        int cpt = 0;
        bool in_function = true;
        
        int nb_actors;
        Scene* scene_ptr;

        actionStream<a_in_stream, action_datatype, 1> action_stream;

    public:
        Juge(Scene* scene, int nb_actors) : nb_actors{ nb_actors }, scene_ptr{ scene },
                                            action_stream(mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD, MPI_INT))
        {
            action_stream.context.count = 1;
            action_stream.context.target = MPI_ANY_SOURCE;
        }

        void listen()
        {
            std::vector<request<action_datatype>>::iterator actions;
            while (in_function && action_stream >> actions)
            {
                if (action_stream.empty()) {
                    int caller;
                    processAction(action_stream.unpack(actions, caller), caller);
                }
                else
                {
                    scene_ptr->propagateUpdate();
                }
            }
            action_stream.clear();
        }

        void fakeListen()
        {
            std::vector<request<action_datatype>>::iterator actions;
            std::cout << "JUGE | setting cannals" << std::endl;
            while (cpt < 40 && in_function && action_stream >> actions)
            {
                std::cout << "JUGE | checking for actions" << std::endl;
                if (!(action_stream.empty())) {
                    int caller = 0;
                    std::cout << "JUGE | checking fake" << std::endl;
                    while (actions != action_stream.begin()) {
                        std::cout << "JUGE | processing fake " << actions - action_stream.begin() << std::endl;
                        processFake(action_stream.unpack(actions, caller), caller);
                        cpt++;
                    }
                }
                std::cout << "JUGE | setting cannals" << std::endl;
                std::this_thread::sleep_for(1000ms);
            }
            action_stream.clear();
            scene_ptr->endGame();
        }

        void processFake(action_datatype action, int caller)
        {
            std::cout << "Juge received movement " << action << " from " << caller << std::endl;
        }

        void processAction(action_datatype action, int caller) const
        {
            if (action == -1) scene_ptr->triggerMeow(caller);
            scene_ptr->updateSelf(std::make_pair(caller, action));
        }

        void close() { in_function = false; }
    };

    class Carte
    {
        Scene scene;
        Juge juge;

    public:

        void initializeActors(unsigned int nb_actor_procs)
        {
            scene.assignRoles(nb_actor_procs);
        }

        Carte() = delete;
        Carte(unsigned int nb_actor_procs, std::vector<char> grille, MPI_Win* end_o_game_w) : scene{ std::move(grille), end_o_game_w }, juge{ &scene, nb_actor_procs }
        {
            
        }

        void startGame()
        {
            juge.listen();
        }

        void fakeStartGame()
        {
            juge.fakeListen();
        }

    };
}

#endif