#ifndef RAT_H
#define RAT_H
#include "Character.h"

class Rat : public Character
{
public:
    Rat(std::vector<char>* grille_p, int width, int height, unsigned int position) : Character( grille_p,  width,  height, position) {}
private:
    virtual int priseDecision()
    {
        int posCible;
        if (fearLevel != 0) {
            fearLevel--;
            posCible = meilleureDistanceCible('+');
        }
        else posCible = meilleureDistanceCible('F');
        if (posCible > 0 && posCible < grille->size()) {
            node lst = searchBestPath(posCible);
            node* good{ &lst }; node* prv{ &lst };
            while (good->parent != nullptr) { std::cout << "unpacking nodes"; prv = good; good = good->parent; }
            return linear(good->i, good->j);
        }
        return position;
    }

    virtual std::vector<std::pair<int, int>> createVoisinsOps()
    {
        return {
            std::make_pair(0, 1), std::make_pair(1, 0), std::make_pair(-1, 0), std::make_pair(0, -1),
            std::make_pair(-1, 1), std::make_pair(1, -1), std::make_pair(-1, -1), std::make_pair(1, 1)
        };
    }

    virtual bool canMove(char entity)
    {
        return entity != MUR && entity != 'C';
    }

    int rechercherChemin(int cible) {
        if (coordX(position) > coordX(cible)) {
            if (coordY(position) > coordY(cible)) return position - (width + 1);
            else if (coordY(position) < coordY(cible)) return position + (width - 1);
            else return position - 1;
        }
        else if (coordX(position) < coordX(cible)) {
            if (coordY(position) > coordY(cible)) return position - width + 1;
            else if (coordY(position) < coordY(cible)) return position + width + 1;
            else return position + 1;
        }
        else {
            if (coordY(position) > coordY(cible)) return position - width;
            else if (coordY(position) < coordY(cible)) return position + width;
        }
    }
    
};

#endif