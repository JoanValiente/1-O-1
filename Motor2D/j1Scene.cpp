#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Player.h"
#include "j1Collision.h"
#include "j1FadeToBlack.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1Scene.h"

j1Scene::j1Scene() : j1Module()
{
	name.create("scene");
}

// Destructor
j1Scene::~j1Scene()
{}

// Called before render is available
bool j1Scene::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool j1Scene::Start()
{
	if (level == 0) {
		App->map->Load("level1.tmx");
		App->audio->PlayMusic("audio/music/all_of_us.ogg");
	}
	if (level == 1) {
		App->map->Load("level2.tmx");
	}
	if (level == 2) {
		App->map->Load("levelwin.tmx");
	}
	App->map->SetMapLogic();
	App->player->Start();
	if (justloaded == true) {
		App->player->position.x = App->player->loadposition.x;
		App->player->position.y = App->player->loadposition.y;
		justloaded = false;
	}
	
	return true;
}

// Called each loop iteration
bool j1Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		App->LoadGame();

	if (App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		App->SaveGame();

	if (App->input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN) {
		level = 1;
		App->fadetoblack->FadeToBlack((j1Module*)App->scene, (j1Module*)App->scene, 3);
	}
	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) {
		level = 0;
		App->fadetoblack->FadeToBlack((j1Module*)App->scene, (j1Module*)App->scene, 3);
	}
	if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
		App->fadetoblack->FadeToBlack((j1Module*)App->scene, (j1Module*)App->scene, 3);
	}


	//App->render->Blit(img, 0, 0);
	App->map->Draw();

	App->input->GetMousePosition(mouse.x, mouse.y);
	p2SString title("Map:%dx%d Tiles:%dx%d Tilesets:%d TILES: %d, %d",
		App->map->data.width, App->map->data.height,
		App->map->data.tile_width, App->map->data.tile_height,
		App->map->data.tilesets.count(),
		mouse.x, mouse.y);

	App->win->SetTitle(title.GetString());
	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	bool ret = true;

	if(App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool j1Scene::CleanUp()
{
	LOG("Freeing scene");
	App->map->DeleteMap();
	App->player->CleanUp();
	App->collision->CleanUp();
	return true;
}

bool j1Scene::Save(pugi::xml_node& data) const {
	bool ret = false;

	if (data.child("level") == NULL) {
		data.append_child("level").append_attribute("level") = level;
	}
	else {
		data.append_child("level").attribute("level") = level;
	}

	ret = true;
	return ret;
}

// Load
bool j1Scene::Load(pugi::xml_node& data) {

	if (data.child("level") != NULL) {
		if (data.child("level").attribute("level").as_int() != level) {
			level = data.child("level").attribute("level").as_int();
			App->fadetoblack->FadeToBlack((j1Module*)App->scene, (j1Module*)App->scene, 1);
			justloaded = true;
		}
		else {
			App->player->position.x = App->player->loadposition.x;
			App->player->position.y = App->player->loadposition.y;
		}
	}

	return true;
}