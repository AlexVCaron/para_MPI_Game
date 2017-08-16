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
        struct update_pack
        {
            int posA; int posB; 
            update_pack() { }
            update_pack(const update_pack& oth) : posA{ oth.posA }, posB{ oth.posB } { }
            update_pack(const update_pack&& oth) noexcept : posA{ oth.posA }, posB{ oth.posB } { }
            update_pack& operator=(const update_pack& oth) { posA = oth.posA; posB = oth.posB; return *this; }
        };
        using update_datatype = update_pack;
        using update_metatype = int;
        using update_meta_stream_t = updateStream<out_stream, update_metatype, 1>;
        using update_data_stream_t = updateStream<out_stream, update_datatype, 1>;



        int nb_actors;
        mpi_interface::signal_handle end_o_game_signal;
        unsigned int nb_rat = 0, nb_chasseurs = 0, nb_fromages = 0;
        std::vector<datatype> grille; int width; int height;
        unsigned int present_rats;
        
        update_meta_stream_t update_m_stream;
        update_data_stream_t update_d_stream;
        

        void countGridElements()
        {
            for (size_t i = 0; i < grille.size(); ++i)
            {
                if (grille[i] == 'C') {
                    ++nb_chasseurs; actor_pos.push_back(i);
                }
                else if (grille[i] == 'R') {
                    ++nb_rat; actor_pos.push_front(i);
                }
                else if (grille[i] == 'F') nb_fromages++;
            }
            assert(nb_rat + nb_chasseurs == nb_actors);
            present_rats = nb_rat;
        }

        std::vector<update_pack> updates;

    public:

        std::vector<char> actor_roles;
        std::deque<unsigned int> actor_pos;
        std::vector<bool> actor_fear;

        char grid(int i) { return grille[i]; }

        void printMap()
        {
            int cpt = 0;
            for (size_t i = 0; i < grille.size(); i++)
            {
                if (cpt == width)
                {
                    cpt = 0;
                    cout << endl;
                }
                cout << grille[i];
                cpt++;
            }
            cout << endl;
        }

        Scene(std::vector<datatype>&& grille, int width, int height, MPI_Win* end_o_game_w, int nb_actors) : nb_actors{nb_actors}, end_o_game_signal { end_o_game_w }, nb_rat{ 0 }, nb_chasseurs{ 0 }, nb_fromages{ 0 }, 
                                                                                                             grille{ grille }, width{ width }, height{ height },
                                                                                                             update_m_stream(mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD, MPI_INT)),
                                                                                                             update_d_stream(mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD))
        {
            std::cout << "CART | taille totale " << grille.size() << std::endl;
            std::cout << width << " " << height << std::endl;
            update_m_stream.context.count = 2;
            countGridElements();
            actor_roles.resize(nb_actors);
            actor_fear.resize(nb_actors);
            printMap();
        }

        void endGame()
        {
            for (unsigned int i = 1; i < nb_chasseurs + nb_rat; ++i) end_o_game_signal.put<MPI_C_BOOL>(true, i);
        }

        void removeRat(int caller)
        {
            bool end{ true };
            if (grille[actor_pos[caller - 1]] == 'R') grille[actor_pos[caller - 1]] = ' ';
            --present_rats; if (present_rats == 0) endGame();
            end_o_game_signal.put<MPI_C_BOOL>(true, caller);
        }

        void eatCheese(int position)
        {
            if (grille[position] == 'F') grille[position] = ' ';
            --nb_fromages; if (nb_fromages == 0) endGame();
        }

        void assignRoles(unsigned int nb_actor_procs)
        {
            mpi_interface::mpi_main_connector<int> connector;
            auto context = mpi_driver::make_mpi_context(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_INT);
            context.count = 1;
            for (unsigned int i = 1; i <= nb_actor_procs; ++i)
            {
                int pos = actor_pos[i - 1];
                context.target = i;
                actor_roles[i - 1] = grille[pos];
                connector.request<canal_direction::_send>(context, &pos);
            }
        }

        void triggerMeow(int actor) 
        {
            for (int i = 0; i < nb_rat + nb_chasseurs; ++i)
            {
                if(actor_roles[i] == 'R' && abs(int(actor_pos[i]) - int(actor_pos[actor - 1])) <= 3)
                {
                    actor_fear[i] = true;
                }
            }
        }

        void propagateUpdate()
        {
            int meta[2]; meta[0] = 1; meta[1] = updates.size() + 1;
            update_d_stream.context.count = updates.size() + 1;
            update_m_stream.context.count = 2;
            update_pack* buff = static_cast<update_pack*>(malloc(sizeof(update_pack) * (updates.size() + 1)));
            for (int i = 0; i < updates.size(); ++i)
            {
                buff[i] = updates[i];
            }
            buff[updates.size()].posA = -1; buff[updates.size()].posB = 0;
            for (int i = 1; i <= nb_chasseurs + nb_rat; ++i) {
                if(actor_fear[i - 1])
                {
                    buff[updates.size()].posB = 1;
                    actor_fear[i - 1] = false;
                }
                update_m_stream.context.target = i;
                update_d_stream.context.target = i;
                update_m_stream << meta;
                if(meta[1] > 0) update_d_stream << buff;
            }
            updates.clear();
            free(buff);
        }

        void updateSelf(std::pair<int,action_datatype> data)
        {
            std::cout << "SCEN | " << actor_pos[data.first] << " " << data.second << std::endl;

            char tmp = grille[actor_pos[data.first]];
            grille[actor_pos[data.first]] = grille[data.second];
            grille[data.second] = tmp;


            update_pack pack_1;
            pack_1.posA = actor_pos[data.first]; pack_1.posB = data.second;

            actor_pos[data.first] = data.second;

            updates.push_back(pack_1);

            printMap();
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
            int cpt = nb_actors, cptg = 0;
            std::vector<request_t<action_datatype*>>::iterator actions;
            while (cptg == 5 || (in_function && action_stream >> actions))
            {
                --cpt;
                int caller = 1000;
                action_datatype* action = action_stream.unpack(actions, caller);
                processAction(action, caller);
                if (cpt == 0) {
                    cpt = nb_actors;
                    scene_ptr->propagateUpdate();
                    cptg++;
                }
            }
            scene_ptr->endGame();
        }

        void fakeListen()
        {
            std::vector<request_t<action_datatype*>>::iterator actions;
            while (cpt < 40 && in_function && action_stream >> actions)
            {
                int caller = 0;
                processAction(action_stream.unpack(actions, caller), caller);
                cpt++;
            }
            std::this_thread::sleep_for(1000ms);
            scene_ptr->endGame();
        }

        void processFake(action_datatype* action, int caller)
        {
        }

        bool processAction(action_datatype* action, int caller) const
        {
            char cnd;
            if (*action == -1) {
                scene_ptr->triggerMeow(caller);
                return false;
            }
            else {
                if (scene_ptr->grid(*action) == '#') {
                    return false;
                }
                if (scene_ptr->grid(*action) == '+' && scene_ptr->actor_roles[caller - 1] == 'R') {
                    scene_ptr->removeRat(caller);
                    return true;
                }
                if (scene_ptr->grid(*action) == 'F' && scene_ptr->actor_roles[caller - 1] == 'R') {
                    scene_ptr->eatCheese(scene_ptr->actor_pos[caller - 1]);
                }
                if (scene_ptr->grid(*action) == 'R' && scene_ptr->actor_roles[caller - 1] == 'C') {
                    scene_ptr->removeRat(caller);
                }
                scene_ptr->updateSelf(std::make_pair(caller - 1, *action));
            }
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
        Carte(unsigned int nb_actor_procs, std::vector<char> grille, int width, int height, MPI_Win* end_o_game_w) : scene{ std::move(grille), width, height, end_o_game_w , nb_actor_procs }, juge{ &scene, nb_actor_procs }
        {
            
        }

        void startGame()
        {
            juge.listen();
        }

        void fakeStartGame()
        {
            juge.listen();
        }

    };
}

#endif