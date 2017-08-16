#include "mpi_interface.h"
#include "Carte.h"
#include "Actor.h"
#include <fstream>


using namespace std;

// copie le contenu d'un fichier dans une string
string readFile(const string & fileName)
{
    string fileContent;
    ifstream i_f{ fileName };
    fileContent.assign((istreambuf_iterator<char>(i_f)), (istreambuf_iterator<char>()));
    i_f.close();
    assert(fileContent.size() > 0);
    return fileContent;
}

// construction d'une ligne de la map
void oneLineMap(string text, vector<char>& grille)
{
    for (string::iterator it = text.begin(); it != text.end(); ++it) grille.push_back(*it);
}

// construction de la carte
void constructMap(const string & text, int& height, int & width, vector<char>& grille)
{
    ifstream file(text);
    string str;
    while (getline(file, str))
    {
        if (width != 0) width = str.size();
        height++;
        oneLineMap(str, grille);
    }
}

std::vector<char> make_grille(string f_name, int& width, int& height)
{
    vector<char> grille{};
    constructMap(f_name, height, width, grille);
    return grille;
}

int main(int argc, char **argv)
{
    int width = 1000, height = 0;
    mpi_interface::MPI_Scope mpi_scope(argc, argv);
    if (mpi_scope.rang() == 0) {
        std::cout << "+-------------------------" << std::endl;
        std::cout << "| HANDSHAKING" << std::endl;
    }
    mpi_interface::realizeInitHandshake(mpi_scope.rang());
    std::vector<char> grille = make_grille(argv[1], width, height);
    bool end_o_game_sig = false;
    MPI_Info info;
    MPI_Info_create(&info);
    MPI_Win end_o_game_w;
    MPI_Win_create(&end_o_game_sig, sizeof(bool), sizeof(bool), info, MPI_COMM_WORLD, &end_o_game_w);
    if(mpi_scope.rang() == 0)
    {
        std::cout << "+------------------------+" << std::endl;
        std::cout << "| Grille                 |" << std::endl;
        std::cout << "+------------------------+" << std::endl;
        carte::Carte ct(mpi_scope.nb_processus() - 1, grille, width, height, &end_o_game_w);
        std::cout << "+------------------------+" << std::endl;
        std::cout << "| Phase d'initialisation |" << std::endl;
        std::cout << "+------------------------+" << std::endl;
        ct.initializeActors(mpi_scope.nb_processus() - 1);
        std::cout << "+-------------------------" << std::endl;
        std::cout << "| Debut de la partie" << std::endl;
        MPI_Barrier(MPI_COMM_WORLD);
        ct.startGame();
        std::cout << "| Fin de la partie      " << std::endl;
        std::cout << "+-------------------------" << std::endl;
    }
    else
    {
        Actor actor(grille, width, height, &end_o_game_sig);
        actor.initialize();
        MPI_Barrier(MPI_COMM_WORLD);
        actor.start();
    }
    MPI_Info_free(&info);
    MPI_Win_free(&end_o_game_w);

    return 0;
}
