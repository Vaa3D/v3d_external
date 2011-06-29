#include "../component_tree.h"
#include "../myalgorithms.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char * argv[])
{
	ComponentTree tree;
	tree.load(argv[1]);
	tree.printTree();
	tree.printReverseAlphaMapping();
}
