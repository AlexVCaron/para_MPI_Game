#ifndef ACTOR_H
#define ACTOR_H
#include <vector>
#include "connecteur.h"
#include <mpi.h>
#include "mpi_driver.h"
#include "Action.h"
#include "Base.h"
#include "Update.h"
#include "Character.h"
#include "Rat.h"
#include "Chasseur.h"


class BadRoleException {};

class Actor
{
	using action_datatype = int;
    using update_datatype = std::vector<std::pair<uint32_t, char>>;
    using update_stream_t = updateStream<in_stream, update_datatype, 1>;

	actionStream<out_stream, action_datatype, 10> action_stream;
    update_stream_t  update_stream;

    Character actor;

	bool in_function = true;

public:

    Actor() : action_stream( mpi_driver::make_mpi_context(0,0,MPI_COMM_WORLD, MPI_INT) ) {  }

	void sendMoveRequest(action_datatype move)
	{
        action_stream << move;
	}

    void initialize()
    {
        using init_connector = mpi_interface::mpi_slave_connector<uint8_t>;

        auto context = mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD, MPI_INT);
        init_connector i_ct; i_ct.request<canal_direction::_receive>(context);

        int role = i_ct.queue.front();
        switch (role) {
        case 0:
            actor = Rat();
            break;
        case 1:
            actor = Chasseur();
            break;
        default:
            throw BadRoleException{};
        }
    }

    action_datatype getNextAction()
    {
        return action_datatype();
    }

    void start ()
	{
        while (in_function) {
            action_stream << getNextAction();
            update();
        }
	}

	void processUpdate(update_datatype update) const { }

	void update()
	{
        update_stream_t::request_it update;
		while (update_stream >> update)
		{
			if (!update_stream.empty()) {
                processUpdate(update_stream.unpack(update));
			}
		}
	}

};

#endif