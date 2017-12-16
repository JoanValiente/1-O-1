#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"
#include "Animation.h"



enum Levels
{
	MENU,
	OPTIONS,
	MENU_PLAY,
	level_1,
	level_2,
	hidden_level,
	congrats,
};

enum InGameUI
{
	NO_UI,
	LIFES,
	URN_UI,
	AMMO,
};

struct SDL_Texture;

class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before render is available
	bool Awake(pugi::xml_node& node);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Save & Load
	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);

	void GetKeys();
	void ButtonInteractions();

	void LoadLevelUI();
	void UnLoadLevelUI();

	void Timer();

	uint MiliToSeconds(uint time);

	void CheckChange();

public:
	int level = 0;
	bool justloaded = false;
	bool changeMap;

	uint level_change_fx;

	int player_lifes = 5;
	int ammo = 10;
	int urns = 0;
	float time = 0;
	int score = 0;


private:
	pugi::xml_node level_name;
	SDL_Texture* ui_texture;
	Animation lifes_anim, urn_anim, ammo_anim;

	pugi::xml_document	animation_file;
	pugi::xml_node animations;
	pugi::xml_node attributes;
	pugi::xml_node rect;

	uint current;
	Animation * load_anim;
	uint total_time;
	uint start_time;

};

#endif // __j1SCENE_H__