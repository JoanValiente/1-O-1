#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "j1Gui.h"
#include "Button.h"
#include "Image.h"
#include "Label.h"

#include "Brofiler/Brofiler.h"


j1Gui::j1Gui() : j1Module()
{
	name.create("gui");
}

// Destructor
j1Gui::~j1Gui()
{}

// Called before render is available
bool j1Gui::Awake(pugi::xml_node& conf)
{
	LOG("Loading GUI atlas");
	bool ret = true;

	atlas_file_name = conf.child("atlas").attribute("file").as_string("");

	return ret;
}

// Called before the first frame
bool j1Gui::Start()
{
	atlas = App->tex->Load(atlas_file_name.GetString());
	debug = false;
	return true;
}

// Update all guis
bool j1Gui::PreUpdate()
{
	return true;
}

bool j1Gui::Update(float dt)
{
	BROFILER_CATEGORY("GUI Update", Profiler::Color::CornflowerBlue);
	if (App->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN) {
		debug = !debug;
	}
	p2List_item<UIElement*> *it = elements.start;

	while (it != nullptr)
	{
		it->data->Draw(dt);
		it->data->Update(dt);
		if (debug) {
			DebugDraw(it->data);
		}
		it = it->next;
	}


	return true;

}

void j1Gui::DebugDraw(UIElement * element)
{
	App->render->DrawQuad({ element->pos.x, element->pos.y, element->current_animation.w,element->current_animation.h}, 0, 255, 0, 255, false);
}

// Called after all Updates
bool j1Gui::PostUpdate()
{
	return true;
}

// Called before quitting
bool j1Gui::CleanUp()
{
	LOG("Freeing GUI");

	elements.clear();

	App->tex->UnLoad(atlas);

	return true;
}
// const getter for atlas
const SDL_Texture* j1Gui::GetAtlas() const
{
	return atlas;
}

// class Gui ---------------------------------------------------

Label* j1Gui::AddLabel(int x, int y, char* text, uint colors, uint fonts, int size, UIElement* parent)
{
	SDL_Color color;

	color = GetColor(colors);
	char* path = GetFont(fonts);
	_TTF_Font* font = App->font->Load(path, size);

	const SDL_Texture* tex = App->font->Print(text, color, font);

	Label* label = new Label(x, y, LABEL, tex, parent);
	elements.add((UIElement*)label);

	return label;
}


Image* j1Gui::AddImage(int x, int y, SDL_Texture* texture, Animation anim, UIElement* parent)
{
	Animation* animation = &anim;
	Image* image = new Image(x, y, IMAGE, texture, animation, parent);
	elements.add((UIElement*)image);

	return image;
}

Button* j1Gui::AddButton(int x, int y, SDL_Texture* texture, Animation anim, j1Module* callback, UIElement* parent)
{
	Animation* animation = &anim;
	Button* button = new Button(x, y, BUTTON, texture, animation, callback, parent);
	elements.add((UIElement*)button);


	return button;
}

void j1Gui::DeleteUI(UIElement * element)
{
	p2List_item<UIElement*>* item = elements.start;

	while (item != nullptr)
	{
		if (item->data == element)
		{
			elements.del(item);
			item->data = nullptr;

		}
		item = item->next;
	}

}



SDL_Color j1Gui::GetColor(int color)
{
	SDL_Color ret;
	ret.a = 255;

	switch (color)
	{
	case BLACK:
		ret.r = 0;
		ret.g = 0;
		ret.b = 0;
		break;
	case WHITE:
		ret.r = 255;
		ret.g = 255;
		ret.b = 255;
		break;
	case RED:
		ret.r = 255;
		ret.g = 0;
		ret.b = 0;
		break;
	case BLUE:
		ret.r = 0;
		ret.g = 0;
		ret.b = 255;
		break;
	case GREEN:
		ret.r = 0;
		ret.g = 255;
		ret.b = 0;
		break;
	case YELLOW:
		ret.r = 218;
		ret.g = 202;
		ret.b = 124;
		break;
	default:
		break;
	}
	return ret;
}

char * j1Gui::GetFont(uint font)
{
	char* path = nullptr;
	switch (font)
	{
	case FREEPIXEL:
		path = "fonts/FreePixel.ttf";
		break;
	case MINECRAFT:
		path = "fonts/Minecraft.ttf";
		break;
	case UPHEAVAL:
		path = "fonts/Upheaval.ttf";
		break;
	default:
		break;
	}
	return path;
}
