

#include "canal.h"
#include <iostream>
#include <algorithm>
#include "connecteur.h"
#include "mpi_driver.h"
#include "mpi_interface.h"


using namespace std;



int main(int argc, char **argv)
{
    mpi_interface::MPI_Scope mpi_scope(argc, argv);
    mpi_interface::realizeInitHandshake(mpi_scope.rang());
    return 0;
}
