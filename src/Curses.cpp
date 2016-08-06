#pragma once
#include "Curses.h"
#include <iostream>
#include <list>

Curses::Curses(sf::Vector2i tile_size_, sf::Vector2i grid_dimensions_) : 
	window(sf::VideoMode(tile_size_.x * grid_dimensions_.x, tile_size_.y * grid_dimensions_.y), "SimpleFML Curses"),
	grid_dimensions(grid_dimensions_),
	tile_pixel_dimensions(tile_size_){
	
	font.loadFromFile("unifont.ttf");

	for (int y = 0; y < grid_dimensions_.y; y++) {
		for (int x = 0 ; x < grid_dimensions_.x; x++) {
			tiles.emplace_back(Tile(sf::Vector2i(x, y))); 

			// L'\u0020' = space char
		}
	}


	// Set screen values to increasing unicode chars
	/*wchar_t char_ind = L'\u0041';
	for (int i = 0; i < tiles.size(); i++) {
		tiles.at(i).push_clear(char_ind++, sf::Color::White, sf::Color::Black);
	}*/
}

Curses::~Curses() {
}

void Curses::Update(double delta_time_) {

	for (Tile &tile : tiles) {
		tile.inc_index();
	}

	sf::Event event;
	while (window.pollEvent(event)) {
		if (event.type == sf::Event::Closed)
			window.close();
	}
}

void Curses::Render() {
	window.clear();

	sf::Texture font_texture = font.getTexture(tile_pixel_dimensions.x);

	// Draw text and backfills
	sf::VertexArray backfill_v_arr(sf::Quads, grid_dimensions.x * grid_dimensions.y * 4);
	sf::VertexArray font_v_arr(sf::Quads, grid_dimensions.x * grid_dimensions.y * 4);

	int tile_index = 0;

	for (int i = 0; i < backfill_v_arr.getVertexCount(); i += 4) {

		Tile* tile = &tiles.at(tile_index);
		sf::Glyph glyph = font.getGlyph(tile->current_unicode_value(), tile_pixel_dimensions.x, false);

		// Backfill the tile with the specified backfill color
		backfill_v_arr[i + 0].position = sf::Vector2f(tile->getPosition().x * tile_pixel_dimensions.x, tile->getPosition().y * tile_pixel_dimensions.y);
		backfill_v_arr[i + 1].position = sf::Vector2f(tile->getPosition().x * tile_pixel_dimensions.x + tile_pixel_dimensions.x, tile->getPosition().y * tile_pixel_dimensions.y);
		backfill_v_arr[i + 2].position = sf::Vector2f(tile->getPosition().x * tile_pixel_dimensions.x + tile_pixel_dimensions.x, tile->getPosition().y * tile_pixel_dimensions.y + tile_pixel_dimensions.y);
		backfill_v_arr[i + 3].position = sf::Vector2f(tile->getPosition().x * tile_pixel_dimensions.x, tile->getPosition().y * tile_pixel_dimensions.y + tile_pixel_dimensions.y);
		
		backfill_v_arr[i + 0].color = tile->current_backfill_color();
		backfill_v_arr[i + 1].color = tile->current_backfill_color();
		backfill_v_arr[i + 2].color = tile->current_backfill_color();
		backfill_v_arr[i + 3].color = tile->current_backfill_color();

		// Draw the font with the correct size, texture location, window location, and color 
		font_v_arr[i + 0].position = sf::Vector2f(tile->getPosition().x * tile_pixel_dimensions.x, tile->getPosition().y * tile_pixel_dimensions.y);
		font_v_arr[i + 1].position = sf::Vector2f(tile->getPosition().x * tile_pixel_dimensions.x + glyph.textureRect.width, tile->getPosition().y * tile_pixel_dimensions.y);
		font_v_arr[i + 2].position = sf::Vector2f(tile->getPosition().x * tile_pixel_dimensions.x + glyph.textureRect.width, tile->getPosition().y * tile_pixel_dimensions.y + glyph.textureRect.height);
		font_v_arr[i + 3].position = sf::Vector2f(tile->getPosition().x * tile_pixel_dimensions.x, tile->getPosition().y * tile_pixel_dimensions.y + glyph.textureRect.height);

		// Make the letter appear in the center of the tile by applying an offset
		sf::Vector2f position_offset = sf::Vector2f(tile_pixel_dimensions.x / 4, tile_pixel_dimensions.y / 4); 
		font_v_arr[i + 0].position += position_offset;
		font_v_arr[i + 1].position += position_offset;
		font_v_arr[i + 2].position += position_offset;
		font_v_arr[i + 3].position += position_offset;

		font_v_arr[i + 0].texCoords = sf::Vector2f(glyph.textureRect.left, glyph.textureRect.top);
		font_v_arr[i + 1].texCoords = sf::Vector2f(glyph.textureRect.width + glyph.textureRect.left, glyph.textureRect.top);
		font_v_arr[i + 2].texCoords = sf::Vector2f(glyph.textureRect.width + glyph.textureRect.left, glyph.textureRect.top + glyph.textureRect.height);
		font_v_arr[i + 3].texCoords = sf::Vector2f(glyph.textureRect.left, glyph.textureRect.top + glyph.textureRect.height);

		font_v_arr[i + 0].color = tile->current_font_color();
		font_v_arr[i + 1].color = tile->current_font_color();
		font_v_arr[i + 2].color = tile->current_font_color();
		font_v_arr[i + 3].color = tile->current_font_color();

		tile_index++;
	}
	
	window.draw(backfill_v_arr);
	window.draw(font_v_arr, &font_texture);

	window.display();
}

void Curses::setTile(Tile tile_) {
	tiles.at(multi_to_linear(tile_.getPosition())) = tile_;
}

void Curses::setTiles(std::vector<Tile> tiles_) {
	for (Tile tile: tiles_) {
		tiles.at(multi_to_linear(tile.getPosition())) = tile;
	}
}

void Curses::Clear() {
	for (Tile &tile : tiles) {
		tile.clear();
	}
}

Curses::Tile* Curses::getTile(sf::Vector2i position_) {
	return &tiles.at(multi_to_linear(position_));
}

std::vector<Curses::Tile*> Curses::getTiles(sf::Vector2i start_, sf::Vector2i end_) {
	std::vector<Curses::Tile*> ret;
	for (int i = multi_to_linear(start_); i < multi_to_linear(end_); i++) {
		ret.push_back(&tiles.at(i));
	}
	return ret;
}

void Curses::setScroll(int ratio_, sf::Vector2i start_, std::list<Slot> scroll_text_) {
	// Scrolling and it's scroll ratio is faux implemented by 
	// essentially stacking values and repeating them to slow the scroll down
	
	

	for (int i = 0; i < scroll_text_.size(); i++) {
		append_slots(start_, scroll_text_);
		scroll_text_.push_back(scroll_text_.front());
		scroll_text_.pop_front();
	}


}



void Curses::set_tile_ratio(int ratio_, sf::Vector2i tile_position_) {
	getTile(tile_position_)->set_ratio(ratio_);
}

void Curses::append_slots(sf::Vector2i start_, std::list<Slot> values_)
{
	std::vector<Tile*> tiles = getTiles(start_,
		sf::Vector2i((values_.size() + start_.x) % grid_dimensions.x,
					(values_.size() + start_.x) / grid_dimensions.x + start_.y));
	
	if (tiles.size() != values_.size()) {
		std::cout << "Values did not match up to slots when appending\n";
		return;
	}

	std::list<Slot>::iterator beg_slot_it = values_.begin();
	std::list<Slot>::iterator end_slot_it = values_.end();
	std::list<Slot>::iterator slot_it;
	
	std::vector<Tile*>::iterator beg_tile_it = tiles.begin();
	std::vector<Tile*>::iterator end_tile_it = tiles.end();
	std::vector<Tile*>::iterator tile_it;

	for (slot_it = beg_slot_it, tile_it = beg_tile_it; 
		(slot_it != end_slot_it) && (tile_it != end_tile_it);
		++slot_it, ++tile_it) {

		Tile* t = *tile_it;
		t->push_back(*slot_it);

	}
}

sf::Vector2i Curses::linear_to_multi(int position_) const {
	return sf::Vector2i(position_ % grid_dimensions.x, position_ / grid_dimensions.x);
}
int Curses::multi_to_linear(sf::Vector2i position_) const {
	return position_.y * grid_dimensions.x + position_.x;
}
