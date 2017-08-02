

#include "canal.h"
#include <iostream>
#include <algorithm>
#include "connecteur.h"
#include "mpi_broadcaster_driver.h"


using namespace std;

template<MPI_Datatype mpi_datatype>
using mpi_connector = connecteur<canal_juge<mpi_broadcaster_driver::broadcaster_type<mpi_datatype>>>;

int main()
{
    char c;
    mpi_connector<MPI_CHAR> connector{ };
    auto init_context(mpi_broadcaster_driver::make_mpi_context(
            0, 0, MPI_COMM_WORLD
        ));

    connector.request<canal_direction::_receive>(init_context);

    cout << connector.queue.size() << endl;
   
    for_each(connector.queue.begin(), connector.queue.end(), [](char elem)
    {
        cout << "elem : " << elem << endl;
    });
    cin >> c;
    return 0;
}
