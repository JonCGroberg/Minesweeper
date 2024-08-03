#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Window/Mouse.hpp"
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace sf;

// Prints an error message in red
void ErrorMessage(string message) {
  cout << "\033[1;31mError: " + message + "\033[0m\n" << endl;
}

// Returns a tuple with the width, height, and number of bombs
struct Config {
  const int TILE_WIDTH = 32;
  const int TILE_HEIGHT = 32;
  const int FOOTER_HEIGHT = 100;
  const string WINDOW_TITLE = "Minesweeper";
  const string IMAGES_PATH = "images/";
  const string BOARDS_PATH = "boards/";
  const string CONFIG_FILE_PATH = BOARDS_PATH + "/config.cfg";
  const vector<string> textureNames = {
      "number_1",      "number_2",   "number_3",  "number_4", "number_5",
      "number_6",      "number_7",   "number_8",  "digits",   "tile_hidden",
      "tile_revealed", "face_happy", "face_lose", "face_win", "mine",
      "flag",          "debug",      "test_1",    "test_2",   "test_3"};
  int rowLength, widthPixels, colLength, heightPixels, totalCells, numOfFlags,
      numOfRevealed, numOfHidden, numOfMines;

  Config() {
    ifstream config(CONFIG_FILE_PATH);
    if (!config.is_open())
      ErrorMessage("Unable to load " + CONFIG_FILE_PATH);

    string line;
    getline(config, line);
    rowLength = stoi(line);
    widthPixels = rowLength * TILE_WIDTH;
    getline(config, line);
    colLength = stoi(line);
    heightPixels = colLength * TILE_HEIGHT + FOOTER_HEIGHT;
    getline(config, line);
    numOfMines = stoi(line);

    totalCells = rowLength * colLength;
  }
};

class AssetManager {
private:
  string IMAGES_PATH;
  vector<string> textureNames;

public:
  unordered_map<string, Texture> textures;
  AssetManager() {}
  AssetManager(Config &config) {
    IMAGES_PATH = config.IMAGES_PATH;
    textureNames = config.textureNames;
    loadAssets();
  }

  void init(Config &config) {
    IMAGES_PATH = config.IMAGES_PATH;
    textureNames = config.textureNames;
    loadAssets();
  }

  void loadAsset(string textureName) {
    textures[textureName] = Texture();
    textures[textureName].setSmooth(true);
    if (!textures[textureName].loadFromFile(IMAGES_PATH + textureName + ".png"))
      ErrorMessage("Unable to load " + textureName + ".png");
  }

  void loadAssets() {
    for (auto name : textureNames)
      loadAsset(name);
  }
};

class Board {
private:
  class Tile {
  public:
    string hiddenState = "tile_revealed";
    string viewableState = "tile_hidden";
    int numOfMinesAround;
    Tile() { numOfMinesAround = 0; }
    void flip() { viewableState = hiddenState; }
    void toggleFlag() {
      if (viewableState == "tile_hidden")
        viewableState = "flag";
      else if (viewableState == "flag")
        viewableState = "tile_hidden";
    }
  };

public:
  AssetManager assetManager;
  RenderWindow *window;
  Config *config;
  vector<vector<Tile>> tiles;
  bool debugMode = false;

  Board(Config &config) {
    this->assetManager.init(config);
    this->config = &config;
    window =
        new RenderWindow(VideoMode(config.widthPixels, config.heightPixels),
                         config.WINDOW_TITLE);
    for (int r = 0; r < config.rowLength; r++) {
      vector<Tile> row;
      for (int c = 0; c < config.colLength; c++)
        row.push_back(Tile());
      tiles.push_back(row);
    }
  }
  ~Board() { delete window; }

  void fillWithBombs() {
    srand(time(0));
    int bombs = 0;
    while (bombs < config->numOfMines) {
      int r = rand() % config->rowLength;
      int c = rand() % config->colLength;
      if (tiles[r][c].hiddenState != "mine") {
        tiles[r][c].hiddenState = "mine";
        bombs++;
      }
    }
  }

  void fillWithNumbers() {
    for (int r = 0; r < size(tiles); r++)
      for (int c = 0; c < size(tiles[0]); c++) {
        if (tiles[r][c].hiddenState == "mine")
          continue;
        int mines = 0;
        for (int i = -1; i < 2; i++)
          for (int j = -1; j < 2; j++) {
            if (r + i < 0 || r + i >= size(tiles) || c + j < 0 ||
                c + j >= size(tiles[0]))
              continue;
            if (tiles[r + i][c + j].hiddenState == "mine")
              mines++;
          }
        tiles[r][c].numOfMinesAround = mines;
      }
  }

  void drawFooter() {
    // Draw the counter
    Sprite digits(assetManager.textures["digits"]);
    digits.setPosition(config->TILE_WIDTH,
                       config->heightPixels - config->FOOTER_HEIGHT / 2 -
                           config->TILE_HEIGHT + config->TILE_HEIGHT / 2);
    window->draw(digits);
    // Draw the face
    Sprite face(assetManager.textures["face_happy"]);
    face.setPosition(config->widthPixels / 2 - config->TILE_WIDTH / 2,
                     config->heightPixels - config->FOOTER_HEIGHT / 2 -
                         config->TILE_HEIGHT);
    window->draw(face);
    // Draw bebug button
    Sprite debugButton(assetManager.textures["debug"]);
    debugButton.setPosition(config->widthPixels - ((config->TILE_WIDTH * 2) * 4 +(config->TILE_WIDTH)),
                            config->heightPixels - config->FOOTER_HEIGHT / 2 -
                                config->TILE_HEIGHT);
    window->draw(debugButton);
    // Draw test buttons
    for (int i = 3; i > 0; i--) {
      Sprite testButton(assetManager.textures["test_" + to_string(4 - i)]);
      testButton.setPosition(config->widthPixels - config->TILE_WIDTH * 1 -
                                 i * config->TILE_WIDTH * 2,
                             config->heightPixels - config->FOOTER_HEIGHT / 2 -
                                 config->TILE_HEIGHT);
      window->draw(testButton);
    }
  }

  void toggleDebug() {
    for (int r = 0; r < size(tiles); r++)
      for (int c = 0; c < size(tiles[0]); c++) {
        swap(tiles[r][c].hiddenState, tiles[r][c].viewableState);
      }
    debugMode = !debugMode;
  }

  void render() {
    window->clear(Color::White);
    for (int r = 0; r < size(tiles); r++)
      for (int c = 0; c < size(tiles[0]); c++) {
        auto tile = tiles[r][c];
        if (tile.viewableState == "flag") {
          Sprite bg = Sprite(assetManager.textures["tile_hidden"]);
          bg.setPosition(r * config->TILE_HEIGHT, c * config->TILE_WIDTH);
          window->draw(bg);
        } else {
          Sprite bg = Sprite(assetManager.textures["tile_revealed"]);
          bg.setPosition(r * config->TILE_HEIGHT, c * config->TILE_WIDTH);
          window->draw(bg);
        }

        Sprite sprite(assetManager.textures[tile.viewableState]);
        sprite.setPosition(r * config->TILE_HEIGHT, c * config->TILE_WIDTH);
        window->draw(sprite);

        if (tile.viewableState == "tile_revealed" &&
            tile.numOfMinesAround > 0) {
          Sprite number(
              assetManager
                  .textures["number_" + to_string(tile.numOfMinesAround)]);
          number.setPosition(r * config->TILE_HEIGHT, c * config->TILE_WIDTH);
          window->draw(number);
        }
      }
    drawFooter();
    window->display();
  }

  void start() {
    while (window->isOpen()) {
      Event event;
      while (window->pollEvent(event)) {
        if (event.type == Event::Closed)
          window->close();
        else if (event.type == Event::MouseButtonPressed) {
          if (event.mouseButton.button == Mouse::Left) {
            Vector2i mousePos = Mouse::getPosition(*window);

            if (mousePos.x > config->widthPixels - ((config->TILE_WIDTH * 2) * 4 +(config->TILE_WIDTH)) && mousePos.x < config->widthPixels - ((config->TILE_WIDTH * 2) * 3 +(config->TILE_WIDTH))  && mousePos.y > config->heightPixels - config->FOOTER_HEIGHT / 2 - config->TILE_HEIGHT&& mousePos.y < config->heightPixels - config-> TILE_HEIGHT / 2) {
              toggleDebug();
            }

            int r = mousePos.x / config->TILE_WIDTH;
            int c = mousePos.y / config->TILE_HEIGHT;
            if (r < config->rowLength && c < config->colLength) {
              tiles[r][c].flip();
              render();
            }
          }
          if (!debugMode && event.mouseButton.button == Mouse::Right) {
            Vector2i mousePos = Mouse::getPosition(*window);
            int r = mousePos.x / config->TILE_WIDTH;
            int c = mousePos.y / config->TILE_HEIGHT;
            if (r < config->rowLength && c < config->colLength) {
              tiles[r][c].toggleFlag();
              render();
            }
          }
        }
        render();
      }
    }
  }
};

int main() {
  Config config;
  Board board(config);
  board.fillWithBombs();
  board.fillWithNumbers();
  board.start();

  return 0;
}