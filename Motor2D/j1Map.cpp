#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Map.h"
#include "j1Collision.h"
#include "j1Player.h"
#include <math.h>

j1Map::j1Map() : j1Module(), map_loaded(false)
{
	name.create("map");
}

// Destructor
j1Map::~j1Map()
{}

// Called before render is available
bool j1Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	folder.create(config.child("folder").child_value());

	return ret;
}

void j1Map::SetMapLogic() {
	if (map_loaded == false)
		return;
	
	p2List_item<TileSet*>* item_tileset = data.tilesets.start;
	logic_layer = data.layers.start;

	bool ret = false;
	
	while (ret == false)
	{
		if (logic_layer != nullptr) {
			if (logic_layer->data->logic == true) {
				ret = true;
				LOG("Loading logic layer");
				break;
			}
			else if (logic_layer->next == nullptr) {
				ret = true;
				LOG("Error loading logic layer");
			}
			else  {
				logic_layer = logic_layer->next;
			}
		}
		else {
			ret = true;
			LOG("Error loading logic layer");
		}
	}

	if (logic_layer->data->logic == true) {
		for (int x = 0; logic_layer->data->width > x; x++)
		{
			for (int y = 0; logic_layer->data->height > y; y++)
			{
				int gid = logic_layer->data->Get(x, y);
				if (gid != 0) {
					iPoint pos = MapToWorld(x, y);
					SDL_Rect col = item_tileset->data->GetTileRect(gid);
					col.x = pos.x;
					col.y = pos.y;

					if (gid == 19) {
						App->collision->AddCollider(col, COLLIDER_WALL);
					}
					if (gid == 4) {
						App->collision->AddCollider(col, COLLIDER_KILL);
					}				
					if (gid == 9) {
						App->player->spawn.x = pos.x;
						App->player->spawn.y = pos.y;
					}
				
				}
				
			}
		}
	}
}

void j1Map::Draw()
{
	if (map_loaded == false)
		return;
	float speed = 0;

	p2List_item<TileSet*>* item_tileset = data.tilesets.end;

	// TODO 5: Prepare the loop to draw all tilesets + Blit
	while (item_tileset != NULL) {
		p2List_item<MapLayer*>* item_layer = data.layers.start;
		while (item_layer != NULL) {
			for (int x = 0; item_layer->data->width > x; x++)
			{
				for (int y = 0; item_layer->data->height > y; y++)
				{
					int tileid = 0;

					if (x > item_layer->data->width || y > item_layer->data->height)
					{
						tileid = 0;
					}
					else
					{
						tileid = item_layer->data->Get(x, y);
					}
					if (tileid > 0) {
						iPoint pos = MapToWorld(x, y);
						SDL_Rect rect = item_tileset->data->GetTileRect(tileid);

						if (item_layer->data->name == "map") {
							speed = 1.0f;
							App->render->Blit(item_tileset->data->texture, pos.x, pos.y, &rect, speed);
						}

						if (item_layer->data->name == "parallax1") {
							speed = 0.6f;
							App->render->Blit(item_tileset->data->texture, pos.x, pos.y, &rect, speed);
						}

						if (item_layer->data->name == "parallax2") {
							speed = 0.4f;
							App->render->Blit(item_tileset->data->texture, pos.x, pos.y, &rect, speed);
						}

					}

				}

			}
			item_layer = item_layer->next;
		}
		item_tileset = item_tileset->prev;

	}
	// TODO 9: Complete the draw function

}

iPoint j1Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	if (data.type == MAPTYPE_ORTHOGONAL) {
		ret.x = x * data.tile_width;
		ret.y = y * data.tile_height;
	}

	if (data.type == MAPTYPE_ISOMETRIC) {
		ret.x = (x - y)* data.tile_width / 2;
		ret.y = (x + y)* data.tile_height / 2;
	}


	return ret;
}

iPoint j1Map::WorldToMap(int x, int y) const
{
	iPoint ret;

	if (data.type == MAPTYPE_ORTHOGONAL) {
		ret.x = x / data.tile_width;
		ret.y = y / data.tile_height;
	}

	if (data.type == MAPTYPE_ISOMETRIC) {
		ret.x = (x/data.tile_width) + (y / data.tile_height);
		ret.y = (y / data.tile_height) - (x / data.tile_width);
	}

	return ret;
}

SDL_Rect TileSet::GetTileRect(int id) const
{
	int relative_id = id - firstgid;
	SDL_Rect rect;
	rect.w = tile_width;
	rect.h = tile_height;
	rect.x = margin + ((rect.w + spacing) * (relative_id % num_tiles_width));
	rect.y = margin + ((rect.h + spacing) * (relative_id / num_tiles_width));
	return rect;
}

// Called before quitting
bool j1Map::CleanUp()
{
	LOG("Unloading map");

	// Remove all tilesets
	p2List_item<TileSet*>* item;
	item = data.tilesets.start;

	while(item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	data.tilesets.clear();

	// Remove all layers
	p2List_item<MapLayer*>* i;
	i = data.layers.start;

	while (i != NULL)
	{
		RELEASE(i->data);
		i = i->next;
	}
	data.layers.clear();

	// Clean up the pugui tree
	map_file.reset();

	return true;
}

// Load new map
bool j1Map::Load(const char* file_name)
{
	bool ret = true;
	p2SString tmp("%s%s", folder.GetString(), file_name);

	pugi::xml_parse_result result = map_file.load_file(tmp.GetString());

	if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", file_name, result.description());
		ret = false;
	}

	// Load general info ----------------------------------------------
	if(ret == true)
	{
		ret = LoadMap();
	}

	// Load all tilesets info ----------------------------------------------
	pugi::xml_node tileset;
	for(tileset = map_file.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if(ret == true)
		{
			ret = LoadTilesetDetails(tileset, set);
		}

		if(ret == true)
		{
			ret = LoadTilesetImage(tileset, set);
		}

		data.tilesets.add(set);
	}

	// Load layer info ----------------------------------------------
	for (pugi::xml_node layerset = map_file.child("map").child("layer"); layerset && ret; layerset = layerset.next_sibling("layer"))
	{
		MapLayer* layer = new MapLayer();

		if (ret == true)
		{
			ret = LoadLayer(layerset, layer);
		}
		data.layers.add(layer);
	}

	if(ret == true)
	{
		LOG("Successfully parsed map XML file: %s", file_name);
		LOG("width: %d height: %d", data.width, data.height);
		LOG("tile_width: %d tile_height: %d", data.tile_width, data.tile_height);

		p2List_item<TileSet*>* item = data.tilesets.start;
		while(item != NULL)
		{
			TileSet* s = item->data;
			LOG("Tileset ----");
			LOG("name: %s firstgid: %d", s->name.GetString(), s->firstgid);
			LOG("tile width: %d tile height: %d", s->tile_width, s->tile_height);
			LOG("spacing: %d margin: %d", s->spacing, s->margin);
			item = item->next;
		}

		p2List_item<MapLayer*>* item_layer = data.layers.start;
		while(item_layer != NULL)
		{
			MapLayer* l = item_layer->data;
			LOG("Layer ----");
			LOG("name: %s", l->name.GetString());
			LOG("tile width: %d tile height: %d", l->width, l->height);
			item_layer = item_layer->next;
		}
	}

	map_loaded = ret;

	return ret;
}

// Load map general properties
bool j1Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = map_file.child("map");

	if(map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		data.width = map.attribute("width").as_int();
		data.height = map.attribute("height").as_int();
		data.tile_width = map.attribute("tilewidth").as_int();
		data.tile_height = map.attribute("tileheight").as_int();
		p2SString bg_color(map.attribute("backgroundcolor").as_string());

		data.background_color.r = 0;
		data.background_color.g = 0;
		data.background_color.b = 0;
		data.background_color.a = 0;

		if(bg_color.Length() > 0)
		{
			p2SString red, green, blue;
			bg_color.SubString(1, 2, red);
			bg_color.SubString(3, 4, green);
			bg_color.SubString(5, 6, blue);

			int v = 0;

			sscanf_s(red.GetString(), "%x", &v);
			if(v >= 0 && v <= 255) data.background_color.r = v;

			sscanf_s(green.GetString(), "%x", &v);
			if(v >= 0 && v <= 255) data.background_color.g = v;

			sscanf_s(blue.GetString(), "%x", &v);
			if(v >= 0 && v <= 255) data.background_color.b = v;
		}

		p2SString orientation(map.attribute("orientation").as_string());

		if(orientation == "orthogonal")
		{
			data.type = MAPTYPE_ORTHOGONAL;
		}
		else if(orientation == "isometric")
		{
			data.type = MAPTYPE_ISOMETRIC;
		}
		else if(orientation == "staggered")
		{
			data.type = MAPTYPE_STAGGERED;
		}
		else
		{
			data.type = MAPTYPE_UNKNOWN;
		}
	}

	return ret;
}

bool j1Map::DeleteMap() {
	LOG("Unloading map");

	// Remove all tilesets
	p2List_item<TileSet*>* item;
	item = data.tilesets.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	data.tilesets.clear();

	// Remove all layers
	p2List_item<MapLayer*>* i;
	i = data.layers.start;

	while (i != NULL)
	{
		RELEASE(i->data);
		i = i->next;
	}
	data.layers.clear();

	// Clean up the pugui tree
	map_file.reset();
	return true;
}

bool j1Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	set->name.create(tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tile_width = tileset_node.attribute("tilewidth").as_int();
	set->tile_height = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if(offset != NULL)
	{
		set->offset_x = offset.attribute("x").as_int();
		set->offset_y = offset.attribute("y").as_int();
	}
	else
	{
		set->offset_x = 0;
		set->offset_y = 0;
	}

	return ret;
}

bool j1Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if(image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		set->texture = App->tex->Load(PATH(folder.GetString(), image.attribute("source").as_string()));
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);
		set->tex_width = image.attribute("width").as_int();

		if(set->tex_width <= 0)
		{
			set->tex_width = w;
		}

		set->tex_height = image.attribute("height").as_int();

		if(set->tex_height <= 0)
		{
			set->tex_height = h;
		}

		set->num_tiles_width = set->tex_width / set->tile_width;
		set->num_tiles_height = set->tex_height / set->tile_height;
	}

	return ret;
}

bool j1Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	if (node == NULL) {
		LOG("Error loading layer");
		ret = false;
	}
	layer->name = node.attribute("name").as_string();
	layer->height = node.attribute("height").as_int();
	layer->width = node.attribute("width").as_int();
	layer->logic = node.child("properties").child("property").attribute("value").as_bool();

	layer->gid = new uint[layer->width * layer->height];

	memset(layer->gid, 0, layer->width * layer->height);

	int i = 0;
	

	for (pugi::xml_node dataLayer = node.child("data").child("tile"); dataLayer; dataLayer = dataLayer.next_sibling("tile"))
	{
		layer->gid[i++] = dataLayer.attribute("gid").as_int();
		LOG("Layer %i", dataLayer.attribute("gid").as_int());
	}
	

	return ret;
}
inline uint MapLayer::Get(int x, int y) const
{
	return gid[(y*width + x)];
}