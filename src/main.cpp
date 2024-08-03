#include "SFML/Graphics/Texture.hpp"
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>

using namespace std;
using namespace sf;

const string WINDOW_TITLE = "Minesweeper";
const string IMAGES_PATH = "images/";
const string BOARDS_PATH = "boards/";
const string CONFIG_FILE_PATH = BOARDS_PATH + "/config.cfg";
const int SQUARE_WIDTH = 32;
const int SQUARE_HEIGHT = 32;
const int FOOTER_HEIGHT = 100;
const string textureNames[] = {
    "number_1",      "number_2",   "number_3",  "number_4", "number_5",
    "number_6",      "number_7",   "number_8",  "digits",   "tile_hidden",
    "tile_revealed", "face_happy", "face_lose", "face_win", "mine",
    "flag",          "debug",      "test_1",    "test_2",   "test_3"};

// Prints an error message in red
void ErrorMessage(string message) {
  cout << "\033[1;31mError: " + message + "\033[0m\n" << endl;
}
// Returns a tuple with the width, height, and number of bombs
tuple<int, int, int> loadConfig() {
  int width, height, numOfBombs;
  ifstream config(CONFIG_FILE_PATH);

  if (!config.is_open()) {
    ErrorMessage("Unable to load " + CONFIG_FILE_PATH);
    return make_tuple(0, 0, 0);
  }

  string line;
  getline(config, line);
  width = stoi(line) * SQUARE_WIDTH;
  getline(config, line);
  height = stoi(line) * SQUARE_HEIGHT + FOOTER_HEIGHT;
  getline(config, line);
  numOfBombs = stoi(line);

  config.close();
  return make_tuple(width, height, numOfBombs);
}

void loadAssets(unordered_map<string, Texture>& textures) {
  for (auto name : textureNames) {
    textures[name] = Texture();
    textures[name].setSmooth(true);
    if (!textures[name].loadFromFile(IMAGES_PATH + name + ".png"))
      ErrorMessage("Unable to load " + name + ".png");
  }
}

int main() {

  // Load the configuration file
  auto [width, height, numOfBombs] = loadConfig();
  // Load the assets
  unordered_map<string, Texture> textures;
  loadAssets(textures);

  // Create the window
  RenderWindow window(VideoMode(width, height), WINDOW_TITLE);
  CircleShape shape(100.f);
  shape.setFillColor(Color::Green);

  while (window.isOpen()) {
    Event event;
    while (window.pollEvent(event)) {
      if (event.type == Event::Closed)
        window.close();
    }

    window.clear();
    window.draw(shape);
    window.draw(shape);
    window.display();
  }

  return 0;
}