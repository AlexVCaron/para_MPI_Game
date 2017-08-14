#ifndef RAT_H
#define RAT_H
#include "Character.h"

class Rat : public Character
{
    virtual int priseDecision() {
        int posCible;
        if (fearLevel != 0) {
            fearLevel--;
            posCible = meilleureDistanceCible('+');
        }
        else posCible = meilleureDistanceCible('F');

        return position + rechercherChemin(posCible);
    }

    Voisins rechercherChemin(int cible) {
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
    
};

#endif