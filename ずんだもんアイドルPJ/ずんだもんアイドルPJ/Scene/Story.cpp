# include "Story.hpp"

Story::Story(const InitData& init)
	: IScene{ init }
{
	
}

void Story::update()
{
	if (MouseL.down()) {
		changeScene(U"Scene3", 0.3s);
	}
}

void Story::draw() const
{
	background.draw();
}
