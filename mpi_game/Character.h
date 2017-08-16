#ifndef CHARACTER_H
#define CHARACTER_H

class Character
{
    virtual std::vector<std::pair<int, int>> createVoisinsOps() {
        return std::vector<std::pair<int, int>>();
    }
    virtual bool canMove(char entity) { return false; }
protected:
    const char MUR = '#';

    std::vector<char>* grille;
    int width, height;
    int fearLevel = 0;
    
public:
    virtual int priseDecision() { return -2; }
    unsigned int position;
    void raiseInFear() { fearLevel = 5; }

    Character(std::vector<char>* grille_p, int width, int height, unsigned int position) : grille(grille_p), width{ width }, height{ height }, position{ position } {}

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
    
    struct node
    {
        node* parent = nullptr;
        int i,  j;
        float f = 0, g=0, h=0;
        node(int i, int j) : i{i}, j{j} {}
        node(std::pair<int,int> coords) : i{ coords.first }, j{ coords.second } {}
        node(std::pair<int, int> coords, node* parent) : parent{ parent }, i { coords.first }, j{ coords.second } {}
        node(int i, int j, node* parent) : parent{ parent }, i{ i }, j{ j } {}
        node(const node& oth) : parent{ oth.parent }, i{ oth.i }, j{ oth.j }, f{ oth.f }, g{ oth.g }, h { oth.h } {}
        node(const node&& oth) noexcept : parent{ oth.parent }, i{ oth.i }, j{ oth.j }, f{ oth.f }, g{ oth.g }, h{ oth.h } {}
        node& operator=(const node& oth) {
            parent = oth.parent;
            i = oth.i; j = oth.j;
            f = oth.f; g = oth.g; h = oth.h;
            return *this;
        }
    };

    int linear(int i, int j)
    {
        return i * width + j;
    }



    std::vector<node> fetchNeigs(node* parent)
    {
        std::vector<node> voisins;
        auto voisin_ops = createVoisinsOps();
        for_each(voisin_ops.begin(), voisin_ops.end(), [&](auto op)
        {
            int  i = parent->i + op.first, j = parent->j + op.second;
            int pt = linear(i, j);
            if(i >= 0 && i < height && j >= 0 && j < width)
            {
                if (canMove((*grille)[pt])) {
                    voisins.emplace_back(i, j, parent);
                }
            }
        });
        return std::move(voisins);
    }

    struct {
        bool operator() (node& a, node& b) {
            return a.f < b.f;
        }
    } lowest_node;

    node searchBestPath(int goal)
    {
        int g_i = coordX(goal), g_j = coordY(goal);
        std::vector<node> o_list;
        std::vector<node> c_list;
        std::cout << position << std::endl;
        o_list.emplace_back(position / width, position % width);
        o_list.front().h = distEuclidienne(position, goal);
        std::cout << o_list.front().i << " " << o_list.front().j << std::endl;
        while(!o_list.empty())
        {
            std::vector<node>::iterator q_it;
            if (o_list.size() > 1) {
                q_it = std::min_element(o_list.begin(), o_list.end(), lowest_node);
            } else q_it = o_list.begin();
            int q_pos = q_it - o_list.begin();
            node q{ *q_it };
            std::vector<node> neigs = fetchNeigs(&q);
            auto neig_it = find_if(neigs.begin(), neigs.end(), [&](node& vs)
            {
                int lin_pos = linear(vs.i, vs.j);
                if (lin_pos == goal) return true;
                vs.g = q.g + 1;
                vs.h = distEuclidienne(lin_pos, goal);
                vs.f = vs.g + vs.h;
                auto lower_o = find_if(o_list.begin(), o_list.end(), [&](node& o_el)
                {
                    return linear(o_el.i, o_el.j) == lin_pos && o_el.f < vs.f;
                });
                auto lower_c = find_if(c_list.begin(), c_list.end(), [&](node& c_el)
                {
                    return linear(c_el.i, c_el.j) == lin_pos && c_el.f < vs.f;
                });
                if (lower_c == c_list.end() && lower_o == o_list.end()) {
                    o_list.push_back(vs);
                }
            });
            if (neig_it != neigs.end()) break;
            c_list.push_back(o_list[q_pos]);
            o_list.erase(o_list.begin() + q_pos);
        }
        return c_list.back();
    }
    


    //

    class AlreadyThereException {};
    int coordX(int posisition) {
        return posisition / width;
    }

    int coordY(int posisition) {
        return posisition % width;
    }

    int distEuclidienne(int position, int cible) {
        return sqrt(pow(coordX(position) - coordX(cible), 2) + pow(coordY(position) - coordY(cible), 2));
    }

    int meilleureDistanceCible(char cible) {
        int distCourante;
        int distRetenue = std::numeric_limits<int>::max();
        int posRetenue = -1;
        //Pour chaque cible C dans carte
        for (auto i = grille->begin(); i != grille->end(); ++i) {
            if (*i == cible) {
                distCourante = distEuclidienne(position, i - grille->begin());
                if (distCourante < distRetenue) {
                    distRetenue = distCourante;
                    posRetenue = i - grille->begin();
                }
            }
        }
        return posRetenue;
    }
};

#endif