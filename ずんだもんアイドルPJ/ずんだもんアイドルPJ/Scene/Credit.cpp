# include "Credit.hpp"

Credit::Credit(const InitData& init)
	: IScene{ init }
{
	
}

void Credit::update()
{
	if (MouseL.down()) {
		changeScene(U"Scene3", 0.3s);
	}
	Cursor::RequestStyle(CursorStyle::Hand);
}

void Credit::draw() const
{
	background.draw();
}
