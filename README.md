# para_MPI_Game

## Game Engine Logic

### Mpi Connections

#### Defining a broadcaster

#### Using contexts and complex MPI_Datatype

#### Predefinable request paramaters

##### Through templating

##### Using canal_types dans canal_traits

    Information to come
    
Naming in the section to come is french, here are traductions of some of the most important of them

- Carte  : map
- Juge   : judge
- Acteur : actor
    
### Game connections

Every **channel** of communication in the game engine is done using a predefined **connector** structure in pair with a 
Mpi **interface** and **driver**. It is used to define a unique **request syntax** when interacting between entites in the 
game, while leveraging Mpi vast and powerful communication architecture, **without the need to have to write the necessary heavy
code**.

#### Streams

Connections can be defined via the usage of streams. A basic stream (action_stream) can be found in **Action.h**. It is
used to define the connection between the **Juge** and its **Acteurs**.

#### Conversation principle 

In the game engine, the **Acteurs** use the stream to send aysnc requests to the **Juge** about their wanted whereabouts around the map. 
The **Juge** then examine the movement request and passes it to the **Carte** if it is juged acceptable. If not it is discarded. The
whole process is done without the **Acteur** knowing if their request has been accepted or not. Knowing this though, they'll continue to
check around, trying to find other things they could do and requesting them too to the **Juge**. In the mean time, if enough modifications
have been done to the **Carte** by the **Juge**, the latter triggers an update on all **Acteurs**, and thus invalidates all the non-processed
requests sent prior to the update time.

#### End of Game Signal

The EOG is handled via a shared buffer between all processes. When a EOG condition is reached, the **Carte** puts a boolean buffer to True. This
buffer is shared through a Mpi_win object (a window on the buffer in the main process). The main process uses Mpi_Put on the window to change the
value through a non-copiable **signal_handle**. The other processes possess the symetrical **read_only_signal**, calling a Mpi_Get on the same window.