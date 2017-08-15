#ifndef CHASSEUR_H
#define CHASSEUR_H
#include "Character.h"

class Chasseur : public Character
{
    int moew_count = 10;

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

    virtual int priseDecision() {
        int posCible = meilleureDistanceCible('R');
        if (distEuclidienne(position, posCible) < 3 && moew_count == 10) {
            moew_count = 0;
            return -1;
        }
        moew_count++;
        return rechercherChemin(position, posCible);
    }
};

#endif