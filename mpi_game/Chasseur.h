#ifndef CHASSEUR_H
#define CHASSEUR_H
#include "Character.h"

class Chasseur : public Character
{
public:
    Chasseur(std::vector<char>* grille_p, int width, int height, unsigned int position) : Character(grille_p, width, height, position) {}
private:
    int moew_count = 10;

    int rechercherChemin(int position, int cible) {
        if (coordX(position) > coordX(cible)) {
            if (position - 1 == MUR) {
                if (coordY(position) < coordY(cible)) return position + width;
                else return position - width;
            }
            else return position - 1;
        }
        else if (coordX(position) < coordX(cible)) {
            if (position + 1 == MUR) {
                if (coordY(position) < coordY(cible)) return position + width;
                else return position - width;
            }
            else return position + 1;
        }
        else {
            if (coordY(position) > coordY(cible)) return position - width;
            else if (coordY(position) < coordY(cible)) return position + width;
        }
    }

    virtual std::vector<std::pair<int, int>> createVoisinsOps()
    {
        return {
            std::make_pair(0, 1), std::make_pair(1, 0), std::make_pair(-1, 0), std::make_pair(0, -1)
        };
    }

    virtual bool canMove(char entity)
    {
        return entity != MUR && entity != '+';
    }

    virtual int priseDecision()
    {
        std::cout << "CHAT CHERCHE CIBLE" << std::endl;
        int posCible = meilleureDistanceCible('R');
        if (posCible >= 0 && posCible < grille->size()) {
            if (distEuclidienne(position, posCible) < 3 && moew_count == 10) {
                moew_count = 0;
                return -1;
            }
            moew_count++;
            std::cout << "CHAT CHERCHE CHEMIN" << std::endl;
            node lst = searchBestPath(posCible);
            node* good{ &lst }; node* prv{ &lst };
            while (good->parent != nullptr) { prv = good; good = good->parent; }
            return linear(prv->i, prv->j);
        }
        return position;
    }
};

#endif