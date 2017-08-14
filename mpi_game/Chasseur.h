#ifndef CHASSEUR_H
#define CHASSEUR_H
#include "Character.h"

class Chasseur : public Character
{
    Voisins rechercherChemin(int position, int cible) {
        if (coordX(position) > coordX(cible)) {
            if (position + LEFT == MUR) {
                if (coordY(position) < coordY(cible)) return DOWN;
                else return UP;
            }
            else return LEFT;
        }
        else if (coordX(position) < coordX(cible)) {
            if (position + RIGHT == MUR) {
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

    int priseDecision() {
        int posCible = meilleureDistanceCible('R');
        if (distEuclidienne(position, posCible) < 10) {
            return -1;
        }
        return rechercherChemin(position, posCible);
    }
};

#endif