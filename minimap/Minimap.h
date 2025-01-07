#pragma once

class Minimap {
	public:
		Minimap() {
			x = 0;
			y = 0;
			height = 100;
			width = 100;
		}
		Minimap(int x, int y, int width, int height) {
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
		}
		~Minimap();
	public:
		int x;
		int y;
		int width;
		int height;
};
