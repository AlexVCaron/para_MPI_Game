#ifndef ACTOR_H
#define ACTOR_H
#include <vector>
#include "connecteur.h"
#include <mpi.h>
#include <algorithm>
#include "mpi_driver.h"
#include "Action.h"
#include "Base.h"
#include "Update.h"
#include "Character.h"
#include "Rat.h"
#include "Chasseur.h"
#include <thread>


// Pour la recherche de chemin, si c'est un mur dans un sens, aller a la perpendiculaire....

using namespace std;

class BadRoleException {};

class Actor
{
    struct update_pack { int pos; char val; };
	using action_datatype = int;
    using update_datatype = update_pack;
    using update_metatype = int;
    using update_meta_stream_t = updateStream<in_stream, update_metatype, 1>;
    using update_data_stream_t = updateStream<in_stream, update_datatype, 1>;

    std::vector<char> grille;

	actionStream<out_stream, action_datatype, 10> action_stream;
    update_meta_stream_t update_m_stream;
    update_data_stream_t update_d_stream;

    Character* actor;

	bool* end_o_game_flag;

public:

    Actor(bool* end_o_game_flag) : end_o_game_flag{ end_o_game_flag }, action_stream(mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD, MPI_INT)),
                                                                       update_m_stream(mpi_driver::make_mpi_context(0,0, MPI_COMM_WORLD, MPI_INT)),
                                                                       update_d_stream(mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD))
    { 
        action_stream.context.count = 1;
        update_m_stream.context.count = 2;
    }

	void sendMoveRequest(action_datatype move)
	{
        action_stream << &move;
	}

    void initialize()
    {
        using init_connector = mpi_interface::mpi_slave_connector<char>;
        auto context = mpi_driver::make_mpi_context(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_CHAR);
        context.count = 1;
        init_connector i_ct; i_ct.request<canal_direction::_receive>(context);
        char role = *(i_ct.queue.front());
        switch (role) {
        case '0':
            std::cout << "I am an actor playing a rat !" << std::endl;
            actor = new Rat();
            break;
        case '1':
            std::cout << "I am an actor playing a cat !" << std::endl;
            actor = new Chasseur();
            break;
        default:
            throw BadRoleException{};
        }
    }

    void start ()
	{
        while (!(*end_o_game_flag)) {
            int decision = actor->priseDecision();
            action_stream << &decision;
            std::this_thread::sleep_for(1000ms);
            update();
        }
	}

    void fakeStart()
    {
        while (!(*end_o_game_flag)) {
            int decision = actor->priseDecision();
            action_stream << &decision;
            std::this_thread::sleep_for(1000ms);
            update();
        }
    }

	void processUpdate(update_datatype* update, int count) {
        std::for_each(update, update + count, [&](update_datatype& upd) {
            grille[upd.pos] = upd.val;
        });
    }

    void processScream() {
        actor->raiseInFear();
    }

	void update()
	{
        update_meta_stream_t::request_it m_update;
        update_data_stream_t::request_it d_update;
        std::cout << "actor getting an update" << std::endl;
        update_m_stream >> m_update;
        std::cout << "actor testing an update" << std::endl;
        int caller;
        int* meta = update_m_stream.unpack(m_update, caller);
        if (meta[0] == -1) processScream();
        else if (meta[1] > 0) {
            std::cout << "updating" << std::endl;
            update_d_stream.context.count = meta[1];
            update_d_stream >> d_update;
            std::cout << "got update " << meta[0] << " " << meta[1] << std::endl;
            processUpdate(update_d_stream.unpack(d_update, caller), meta[1]);
        }
	}

    ~Actor() { delete actor; }
};

#endif