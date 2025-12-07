# include "Howtoplay.hpp"

Howtoplay::Howtoplay(const InitData& init)
	: IScene{ init }
{
	
}

void Howtoplay::update()
{
	if (MouseL.down()) {
		changeScene(U"Title", 0.3s);
	}
	Cursor::RequestStyle(CursorStyle::Hand);
}

void Howtoplay::draw() const
{
	background.draw();
}
