#include "node.h";
#include "camera.h";
#include "LightManager.h";
//#include "camera.h";
float node::deltaTime = 0.f;

node::node()
{
	
}

node::~node()
{
	
	for (int i = 0;i<children.size();i++)
	{
		if (children[0] != nullptr)
		{
			delete children[0];
		}
	}
	children.clear();
	if (parent != nullptr)
	{
		parent->removeChild(*this);
	}
}

void node::tick()
{
	for (auto& child : children)
	{
		
		if (child!=nullptr&&child->visible) //Only render visible children
		{
			child->tick();
		}

	}

	//Script Tick
	if (tickFunctionRef != LUA_NOREF) //If the script function refeerence is set
	{
		//Push Function onto stack
		lua_rawgeti(L, LUA_REGISTRYINDEX, tickFunctionRef);
		//Push Deltatime as argument
		lua_pushnumber(L, deltaTime);

		if (lua_pcall(L, 1, 0, 0) != LUA_OK) //Current State on stack, 1 argument, 0 results
		{
			std::cerr << "Error in Lua tick function: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}
	}
}

void node::render(camera& currentCamera)
{
	for (auto& child : children)
	{
		if (child->visible) //Only render visible children
		{
			child->render(currentCamera);
			
		}
		
	}

	

}

node& node::addChild(node& newChild)
{
	if (auto search = std::find(children.begin(), children.end(), &newChild); search == children.end()) //If we can't find the child in our tree.
	{
		children.push_back(&newChild);
		//Remove our newChild from its old parent
		if (newChild.parent != nullptr)
		{
			newChild.parent->removeChild(newChild);
		}
		newChild.parent = this;
		
	}
	return newChild;
}

node* node::addChild(node* newChild)
{
	if (!newChild) return nullptr;
	if (auto search = std::find(children.begin(), children.end(), newChild); search == children.end()) //If we can't find the child in our tree.
	{
		children.push_back(newChild);
		//Remove our newChild from its old parent
		if (newChild->parent != nullptr)
		{
			newChild->parent->removeChild(*newChild);
		}
		newChild->parent = this;

	}
	return newChild;
}

node& node::removeChild(node& childToRemove)
{
	if (auto search = std::find(children.begin(), children.end(), &childToRemove); search != children.end()) //If we can find the child we want to remove
	{
		childToRemove.parent = nullptr;
		children.erase(search); //Erase via the iterator
	}
	return childToRemove;
}

void node::attachScript(std::string attachedScript)
{
	L = luaL_newstate(); //Set the lua state
	luaL_openlibs(L); //Open standard lua libraries
	
	RegisterFunctions();
	luaL_unref(L, LUA_REGISTRYINDEX, tickFunctionRef); //Unbind Registered Tick Function
	tickFunctionRef = LUA_NOREF;
	runLuaScript(L, attachedScript);
	scriptIsAttached = true;
}

void node::RegisterFunctions()
{
	lua_newtable(L); //Create Node table

	//Push userdata (node* instance) as an upvalue
	node** ptr = static_cast<node**>(lua_newuserdata(L, sizeof(node*)));
	*ptr = this;

	lua_pushcclosure(L, lua_tick, 1); //Bind `lua_tick` with this object as an upvalue
	lua_setfield(L, -2, "tick");

	ptr = static_cast<node**>(lua_newuserdata(L, sizeof(node*)));
	*ptr = this;  // Store this instance

	lua_pushcclosure(L, lua_AttachScript, 1);
	lua_setfield(L, -2, "AttachScript");
	//lua_setfield(L, -2, "ChangeScript");

	lua_setglobal(L, "Node"); //Set table as global "Node"
}



void node::runLuaScript(lua_State* L, std::string script)
{
	if (luaL_dofile(L, script.c_str()) != LUA_OK)
	{
		std::cerr << "Lua Error: " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1);
	}
}
