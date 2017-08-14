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
#include <limits>
#include <cmath>

enum Voisins {
	DOWN = 0,
	UP = 1,
	LEFT = 2,
	RIGHT = 3,
	DIAG_UP_RIGHT = 4,
	DIAG_UP_LEFT = 5,
	DIAG_DOWN_RIGHT = 6,
	DIAG_DOWN_LEFT = 7
};

class BadRoleException {};
class AlreadyThereException {};
int coordX(int posisition) {
	return posisition;
}

int coordY(int posisition) {
	return posisition;
}

int distEuclidienne(int position, int cible) {
	return sqrt(pow(coordX(position) - coordX(cible), 2) + pow(coordY(position) - coordY(cible), 2));
}

int meilleureDistanceCible(int position, Carte carte, texte cible) {
	int distCourante;
	int distRetenue = std::numeric_limits<int>::max();
	int posRetenue = -1;
	//Pour chaque cible C dans carte
	for (auto i = carte.begin(); i != carte.end(); i++) {
		if (*i == cible) {
			distCourante = distEuclidienne(position, cible.position());
			if (distCourante < distRetenue) {
				distRetenue = distCourante;
				posRetenue = cible.position();
			}
		}
	}
}

// Pour la recherche de chemin, si c'est un mur dans un sens, aller a la perpendiculaire....
Voisins rechercherCheminRat(int position, int cible, char texte[]) {
	if (coordX(position) > coordX(cible)) {
		if (coordY(position) > coordY(cible)) return DIAG_UP_LEFT;
		else if (coordY(position) < coordY(cible)) return DIAG_DOWN_LEFT;
		else return LEFT;
	}
	else if (coordX(position) < coordX(cible)) {
		if (coordY(position) > coordY(cible)) return DIAG_UP_RIGHT;
		else if (coordY(position) < coordY(cible)) return DIAG_DOWN_RIGHT;
		else return RIGHT;
	}
	else {
		if (coordY(position) > coordY(cible)) return UP;
		else if (coordY(position) < coordY(cible)) return DOWN;
		else throw AlreadyThereException{};
	}
}

Voisins rechercherCheminChasseur(int position, int cible) {
	if (coordX(position) > coordX(cible)) {
		if (position + LEFT == mur) {
			if (coordY(position) < coordY(cible)) return DOWN;
			else return UP;
		} else return LEFT;
	}
	else if (coordX(position) < coordX(cible)) {
		if (position + RIGHT == mur) {
			if (coordY(position) < coordY(cible)) return DOWN;
			else return UP;
		}
		else return RIGHT;
	}
	else {
		if (coordY(position) > coordY(cible)) return UP;
		else if (coordY(position) < coordY(cible)) return DOWN;
		else throw AlreadyThereException{};
	}
}

void priseDecisionChasseur() {
	int posCible = meilleureDistanceCible(this.position, this.carte, "rat");
	if (distEuclidienne(this.position, posCible) < 10) {
		// Voit un rat
		//// Meow, bitch
	}
	else {
		Voisins moveTo = rechercherCheminChasseur(this.position, posCible)
	}
}

void priseDecisionRat() {
	if (entendUnMeow) {
		this.fearLevel = 5;
		posCible = meilleureDistanceCible(this.position, this.carte, "porte");
	}
	else if (this.fearLevel != 0) {
		this.fearLevel--;
		posCible = meilleureDistanceCible(this.position, this.carte, "porte");
	}
	else posCible = meilleureDistanceCible(this.position, this.carte, "fromage");
	
	Voisins moveTo = rechercherCheminRat(this.position, posCible)
}
class Actor
{
    struct update_pack { int pos; char val; };
	using action_datatype = int;
    using update_datatype = update_pack;
    using update_metatype = int*;
    using update_meta_stream_t = updateStream<in_stream, update_metatype, 1>;
    using update_data_stream_t = updateStream<in_stream, update_datatype, 1>;

    std::vector<char> grille;

	actionStream<out_stream, action_datatype, 10> action_stream;
    update_meta_stream_t update_m_stream;
    update_data_stream_t update_d_stream;

    Character actor;

	bool* end_o_game_flag;

public:

    Actor(bool* end_o_game_flag) : end_o_game_flag{ end_o_game_flag }, action_stream(mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD, MPI_INT)),
                                                                       update_m_stream(mpi_driver::make_mpi_context(0,0, MPI_COMM_WORLD, MPI_INT)),
                                                                       update_d_stream(mpi_driver::make_mpi_context(0, 0, MPI_COMM_WORLD, MPI_NONE))
    { 
        update_m_stream.context.count = 2;
    }

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
        while (!(*end_o_game_flag)) {
            action_stream << getNextAction();
            update();
        }
	}   

	void processUpdate(update_datatype update, int count) const {
        std::for_each(&update, &update + count, [&](update_datatype& upd) {
            grille[upd.pos] = upd.val;
        });
    }

    void processScream() {

    }

	void update()
	{
        update_meta_stream_t::request_it m_update;
        update_data_stream_t::request_it d_update;
        update_m_stream >> m_update;
        if (!update_m_stream.empty()) {
            int * meta = update_m_stream.unpack(m_update);
            if (meta[0] == -1) processScream();
            else {
                update_d_stream.context.count = meta[1];
                update_d_stream >> d_update;
                if (!update_d_stream.empty()) {
                    processUpdate(update_d_stream.unpack(d_update), meta[1]);
                }
            }
        }
	}
};

#endif