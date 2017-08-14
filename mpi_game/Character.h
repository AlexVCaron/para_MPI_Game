#ifndef CHARACTER_H
#define CHARACTER_H

class Character
{
protected:
    const char MUR = '#';

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

    std::vector<char> grille;
    int width, height;
    int fearLevel = 0;
    unsigned int position;
public:
    virtual int priseDecision() { return -2; }

    void raiseInFear() { fearLevel = 5; }

    Character() {}

    Character(Character&& oth) : grille{ oth.grille },
        width{ oth.width },
        height{ oth.height },
        position{ oth.position },
        fearLevel{ oth.fearLevel }
    {
        
    }

    Character operator=(Character& oth) {
        return std::move(*this);
    }
protected:

    // A*
    /*
    struct node
    {
        node* parent = nullptr;
        int i,  j;
        float f = 0, g, h;
        node(int i, int j) : i{i}, j{j} {}
        node(int i, int j, node* parent) : parent{ parent }, i{ i }, j{ j } {}
    };

    void insertNode(std::vector<node>& nodes, node& i_node)
    {
        insertNode(nodes, i_node, nodes.begin(), nodes.end(), nodes.begin() + nodes.size() / 2);
    }

    void insertNode(std::vector<node>& nodes, node& i_node, std::vector<node>::iterator beg, std::vector<node>::iterator end, std::vector<node>::iterator hint)
    {
        if ((end - hint) == 0 && (hint - beg) == 0) nodes.insert(hint, i_node);
        else if (hint->f > i_node.f) insertNode(nodes, i_node, hint, end, hint + (end - hint) / 2);
        else if (hint->f < i_node.f) insertNode(nodes, i_node, beg, hint, hint + (hint - beg) / 2);
        else nodes.insert(hint, i_node);
    }

    node& findNode(std::vector<node>& nodes, float f, std::vector<node>::iterator hint)
    {
        return findNode(nodes, f, nodes.begin(), nodes.end(), hint);
    }

    node& findNode(std::vector<node>& nodes, float f, std::vector<node>::iterator beg, std::vector<node>::iterator end, std::vector<node>::iterator hint)
    {
        if ((end - hint) == 0 && (hint - beg) == 0) return *hint;
        if (hint->f < f) findNode(nodes, f, hint, end, hint + (end - hint) / 2);
        else if (hint->f > f) findNode(nodes, f, beg, hint, hint + (hint - beg) / 2);
        return *hint;
    }

    float findNodeIJ(std::vector<node>& nodes, int i, int j)
    {
        std::vector<node>::iterator fn = std::find_if(nodes.begin(), nodes.end(), [&](node& nd)
        {
            return nd.i == i && nd.j == j;
        });
        if (fn == nodes.end()) return -1;
        else return fn->f;
    }

    int searchBestPath(unsigned int goal)
    {
        int g_i = goal % width, g_j = goal / width;

        std::vector<node> o_list;
        std::vector<node> c_list;
        o_list.emplace_back(position % width, position / width);
        while(!o_list.empty())
        {
            node q = o_list.back(); o_list.pop_back();
            if(q.i + 1 < width)
            {
                if (q.i + 1 == g_i && q.j == g_j) break;
                auto n_o = findNodeIJ(o_list, q.i + 1, q.j), n_c = findNodeIJ(c_list, q.i + 1, q.j);
                if(n_o != -1.f && n_o > )
            }
        }
    }
    */


    //

    class AlreadyThereException {};
    int coordX(int posisition) {
        return posisition;
    }

    int coordY(int posisition) {
        return posisition;
    }

    int distEuclidienne(unsigned int position, unsigned int cible) {
        return sqrt(pow(coordX(position) - coordX(cible), 2) + pow(coordY(position) - coordY(cible), 2));
    }

    int meilleureDistanceCible(unsigned int cible) {
        int distCourante;
        int distRetenue = std::numeric_limits<int>::max();
        int posRetenue = -1;
        //Pour chaque cible C dans carte
        for (auto i = grille.begin(); i != grille.end(); ++i) {
            if (*i == cible) {
                distCourante = distEuclidienne(position, cible);
                if (distCourante < distRetenue) {
                    distRetenue = distCourante;
                    posRetenue = position;
                }
            }
        }
        return posRetenue;
    }
};

#endif