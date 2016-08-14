#pragma once
#include <SFML/Graphics.hpp>
#include <list>

class Curses {
public:
	
	struct Slot {
		Slot(wchar_t unicode_value_,
			sf::Color font_color_,
			sf::Color backfill_color_) :
			unicode_value(unicode_value_),
			font_color(font_color_),
			backfill_color(backfill_color_)
		{};

		wchar_t unicode_value;
		sf::Color font_color;
		sf::Color backfill_color;
	};

	struct Tile {
	public:
		Tile(sf::Vector2i position_) : 
			blank_standby(L'\u0020', sf::Color::Transparent, sf::Color::Black),
			position(position_)
		{ };

	private:
		Slot blank_standby;
		
		int index = 0;							   // What index in the vector are we. Backbone for blinking and scrolling
		int ratio_counter = 0;					   // Secondary counter to hold index positions for (ratio) length of time
		int ratio_value = 0;
		
		std::vector<Slot> slot_stack;			   // The icon that aligns with the index

		sf::Vector2i position;					   // Position of the text, and backfill
	
	public:
		void set_ratio(int ratio_) {
			ratio_value = ratio_;
		}

		sf::Vector2i getPosition() const {
			return position;
		}

		void push_back(Slot s) {
			slot_stack.push_back(s);
		}
		
		void clear_and_set(Slot s) {
			slot_stack.clear();
			slot_stack.push_back(s);
		}

		void clear() {
			slot_stack.clear();
		}

		sf::Color current_font_color() {
			if (slot_stack.size() > 0)
				return slot_stack.at(index).font_color;
			else
				return blank_standby.font_color;
		}

		sf::Color current_backfill_color() {
			if (slot_stack.size() > 0)
				return slot_stack.at(index).backfill_color;
			else
				return blank_standby.backfill_color;
		}

		wchar_t current_unicode_value() {
			if (slot_stack.size() > 0)
				return slot_stack.at(index).unicode_value;
			else
				return blank_standby.unicode_value;
		}

		void inc_index() {
			if (index >= slot_stack.size() - 1) {
				index = 0;
			}
			else if (ratio_counter == ratio_value) {
				ratio_counter = 0;
				index++;
			}
			else
				ratio_counter++;
		}
	};

	Curses(sf::Vector2i tile_size_, sf::Vector2i grid_dimensions);
	~Curses();


	void Update(double delta_time_);
	void Render();

	
	void setTile(Tile tile_);
	void setTiles(std::vector<Tile> tiles_); // Can be seperate, non-adjacent tiles

	void Clear();

	Tile* getTile(sf::Vector2i position_);
	std::vector<Curses::Tile*> getTiles(sf::Vector2i start_, sf::Vector2i end_);

	void ResizeTiles(sf::Vector2i size_);
	void ResizeTileGrid(sf::Vector2i grid_dimensions_);

	void setBlink(int ratio_, sf::Vector2i position_);
	void setScroll(int ratio_, sf::Vector2i start_, sf::Vector2i end_);
	void setScroll(int ratio_, sf::Vector2i start_, std::list<Slot> tiles_);


private:

	sf::Vector2i grid_dimensions;
	sf::Vector2i tile_pixel_dimensions;

	sf::RenderWindow window;

	std::vector<Tile> tiles;

	sf::Font font;

	int multi_to_linear(sf::Vector2i position_) const;
	sf::Vector2i linear_to_multi(int position_) const;

	void set_tile_ratio(int ratio_, sf::Vector2i tile_position_);
	void append_slots(sf::Vector2i start_, std::list<Slot> values_);

};

