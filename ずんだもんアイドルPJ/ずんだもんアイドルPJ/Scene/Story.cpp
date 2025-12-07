# include "Story.hpp"

Story::Story(const InitData& init)
	: IScene{ init }
{
	
}

void Story::update()
{
	if (MouseL.down()) {
		changeScene(U"Title", 0.3s);
	}
	Cursor::RequestStyle(CursorStyle::Hand);
}

void Story::draw() const
{
	background.draw();
}
