#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include "LuaCPP/LuaCpp.hpp"
#include "globals.h"

#include <algorithm>
template <typename Key, typename Value, std::size_t size>
struct Map
{
	std::array<std::pair<Key, Value>, size>data;
	[[nodiscard]] constexpr Value at(const Key& key) const
	{
		const auto itr =
			std::find_if(begin(data), end(data),
				[&key](const auto& v) {return v.first == key; });
		if (itr != end(data))
		{
			return itr->second;
		}
		else
		{
			throw std::range_error("Index out of Bounds");
		}
	}
};



//Temp
#include<iostream>
class camera;
class LightManager;
class node
{
public:
	node();
	virtual ~node();
	virtual void tick();
	virtual void render(camera& currentCamera);
	virtual node& addChild(node& newChild);
	virtual node& removeChild(node& childToRemove);
	void setVisible(bool newVis) { visible = newVis; };

	static float deltaTime;
public:
	//LUA
	void attachScript(std::string attachedScript);
	virtual void RegisterFunctions();

protected:
	bool visible = true;
	std::vector<node*> children;

	node* parent = nullptr;

	//LUA 
	lua_State* L;
	//--Callbacks--
	int tickFunctionRef = LUA_NOREF; // Store the function reference
	static int lua_tick(lua_State* L)
	{
		node** sNodePtr = static_cast<node**>(lua_touserdata(L, lua_upvalueindex(1)));
		if (!sNodePtr || !(*sNodePtr))
		{
			return luaL_error(L, "Node instance is null");
		}

		node* sNode = *sNodePtr;

		if (!lua_isfunction(L, 1))
		{
			return luaL_error(L, "Expected a function");
		}

		lua_pushvalue(L, 1); // Copy function reference
		sNode->tickFunctionRef = luaL_ref(L, LUA_REGISTRYINDEX);
		return 0;
	}
	static int lua_AttachScript(lua_State* L)
	{
		node** sNodePtr = static_cast<node**>(lua_touserdata(L, lua_upvalueindex(1)));
		if (!sNodePtr || !(*sNodePtr))
		{
			return luaL_error(L, "Noode instance is null");

		}
		node* sNode = *sNodePtr;
		if (!lua_isstring(L, 1))
		{
			return luaL_error(L, "Expected a string");
		}
		std::string scriptPathRef = lua_tostring(L, 1);
		sNode->attachScript(scriptPathRef);

		return 0; //We're not returning anything from the lua functon
	}

	void runLuaScript(lua_State* L, std::string script);
	bool scriptIsAttached = false;

	static void printLuaStack(lua_State* L) {
		int top = lua_gettop(L);
		std::cout << "Lua Stack (top=" << top << "):\n";
		for (int i = 1; i <= top; i++) {
			int t = lua_type(L, i);
			switch (t) {
			case LUA_TSTRING: std::cout << i << ": string -> " << lua_tostring(L, i) << "\n"; break;
			case LUA_TBOOLEAN: std::cout << i << ": boolean -> " << (lua_toboolean(L, i) ? "true" : "false") << "\n"; break;
			case LUA_TNUMBER: std::cout << i << ": number -> " << lua_tonumber(L, i) << "\n"; break;
			case LUA_TTABLE: std::cout << i << ": table\n"; break;
			default: std::cout << i << ": other -> " << lua_typename(L, t) << "\n"; break;
			}
		}
	}
};

