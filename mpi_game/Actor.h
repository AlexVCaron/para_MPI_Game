#ifndef ACTOR_H
#define ACTOR_H
#include <vector>
#include "connecteur.h"
#include <mpi.h>
#include "mpi_driver.h"
#include "Action.h"
#include <iostream>

class Actor
{
	using datatype = char;

	actionStream<datatype, 10> action_stream;
	bool in_function = true;

	template <class T>
	using actor_ct = connecteur<canal_acteur<mpi_driver::master_broadcaster_mpi<T>>>;

	void sendMoveRequest(unsigned int move[])
	{
		actor_ct<char> connector;
		auto context = mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD, MPI_INT);
		connector.request<canal_direction::_send>(move, context);
	}

	void processAction(datatype action) const
	{
		switch (action) {
			case '0': 
				break;
			case '1' : 
				break;
		}
	}

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

};

#endif