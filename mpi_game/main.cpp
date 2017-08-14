#include "mpi_interface.h"
#include "Carte.h"
#include "Actor.h"


using namespace std;

int main(int argc, char **argv)
{
    mpi_interface::MPI_Scope mpi_scope(argc, argv);
    mpi_interface::realizeInitHandshake(mpi_scope.rang());
    std::vector<char> grille{ '1','R','3','C','5' };
    bool end_o_game_sig = false;
    MPI_Info info;
    MPI_Info_create(&info);
    MPI_Win end_o_game_w;
    MPI_Win_create(&end_o_game_sig, sizeof(bool), sizeof(bool), info, MPI_COMM_WORLD, &end_o_game_w);
    if(mpi_scope.rang() == 0)
    {
        carte::Carte ct(mpi_scope.nb_processus() - 1, grille, &end_o_game_w);
        ct.initializeActors(mpi_scope.nb_processus() - 1);
    }
    else
    {
        Actor actor(&end_o_game_sig);
        actor.initialize();
    }
    MPI_Info_free(&info);
    MPI_Win_free(&end_o_game_w);

    return 0;
}
