

#include "canal.h"
#include <iostream>
#include <algorithm>
#include "connecteur.h"

using namespace std;

template<class T>
struct BC_boboche
{
    typedef canal_direction::_bi direction;
    typedef T message_type;
    T bf = 'c';
    T resolve() const { return bf; }
    void resolve(T& t) { bf = t; }
};

int main()
{
    using boboche = connecteur<canal_juge<BC_boboche<char>>>;
    char c;
    boboche bb{}; bb.request<canal_direction::_receive>();

    cout << bb.queue.size() << endl;
   
    for_each(bb.queue.begin(), bb.queue.end(), [](char elem)
    {
        cout << "elem : " << elem << endl;
    });
    cin >> c;
    return 0;
}
