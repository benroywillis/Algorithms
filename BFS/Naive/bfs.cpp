
#include <stdint.h>
#include "TimingLib.h"
#include "graph_ge.h"

// size is the number of vertices in the graph and half the number of edges
#ifndef SIZE
#define SIZE 	128
#endif
#define EDGES SIZE*(SIZE-1)/3

#if PRECISION == 0
#define TYPE double
#elif PRECISION == 1
#define TYPE float
#elif PRECISION == 2
#define TYPE long
#else
#define TYPE int
#endif

#ifndef TARGET
#define TARGET (TYPE)(SIZE-1)
#endif

struct Node {
	struct Node** children;
	int no_children;
	TYPE payload;
};

int main( int argc, char** argv ) {
	// holds all nodes in the graph
	// this creates a single-connected graph in which nodes may have multiple parents and multiple children
    // - for some reason it doesn't do anything with node 0, so an entrypoint into the graph should be graph[1]
	Node* graph = (Node*)malloc( SIZE*sizeof(Node) );
	int* adj_matrix = random_connected_graph( SIZE, EDGES, 1, 0, nullptr );
	if( !adj_matrix ) { return 1; }
    for ( int i = 1; i < SIZE; i++ ) {
		graph[i].payload = (TYPE)i;
		graph[i].no_children = 0;
		Node* node_children[SIZE];
     	for ( int j = i + 1; j <= SIZE; j++ ) {
            int index = ( i - 1 ) * SIZE + j - 1;
			if( adj_matrix[ index ] ) {
				node_children[ graph[i].no_children ] = &graph[j];
				graph[i].no_children++;
			}
		}
		graph[i].children = (Node**)malloc( graph[i].no_children*sizeof(Node*) );
		for( unsigned j = 0; j < graph[i].no_children; j++ ) {
			graph[i].children[j] = node_children[j];
		}
	}
	// used to perform the bfs through the graph
	Node* queue[EDGES];
	// queue pointers
	int qs_front = 0;
	int qs_back = 0;
	// the node that has our target
	Node* match = nullptr;
	__TIMINGLIB_benchmark( [&]() {
		// bfs kernel
		queue[qs_front] = &graph[1];
		++qs_back;
		while( ((qs_back - qs_front) > 0) && (qs_back < EDGES) ) {
			printf("On node %d\n", queue[qs_front]->payload);
			if( queue[qs_front]->payload == TARGET ) {
				match = queue[qs_front];
				break;
			}
			else {
				for( unsigned i = 0; i < queue[qs_front]->no_children; i++ ) {
					queue[qs_back] = queue[qs_front]->children[i];
					++qs_back;
				}
			}
			// pop the front of the queue
			++qs_front;
		}
	});
	if( match ) { printf("The matching node had payload %d\n", match->payload); }
	else        { printf("The target node was not found!\n"); }

	/*for( unsigned i = 0; i < SIZE; i++ ) {
		free(graph[i].children);
	}*/
	free(graph);
	free(adj_matrix);
	return 0;
}
