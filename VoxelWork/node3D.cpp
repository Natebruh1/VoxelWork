#include "node3D.h"

void node3D::tick()
{
	
	updateTransform();
	node::tick();
	
}

node3D::~node3D()
{
	node::~node();
}
