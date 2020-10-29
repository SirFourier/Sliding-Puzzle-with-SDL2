//Using SDL and standard IO
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "tile.h"

bool isEmptyTileInNeighbours(std::vector<std::vector<Tile>>& tiles, const int row, const int col, Tile* emptyTile) {
    // Check for empty tile of neighbours (only in vertical and horizontal direction)
    for (int delta = -1; delta < 2; delta += 2) {
        // If tile is not valid
        if (row + delta < 0 || row + delta > tiles.size()) {
            continue;
        }
        if (emptyTile == &tiles[row + delta][col]) {
            return true;
        }
    }
    for (int delta = -1; delta < 2; delta += 2) {
        // If tile is not valid
        if (col + delta < 0 || col + delta > tiles.size()) {
            continue;
        }
        if (emptyTile == &tiles[row][col + delta]) {
            return true;
        }
    }

    // Empty tile not found
    return false;
}

int main( int argc, char* args[] ) {
    // Screen dimensions
    const unsigned int SCREEN_WIDTH = 600;
    const unsigned int SCREEN_HEIGHT = 600;

    // Tile dimensions based on difficulty
    // Puzzle difficulty proportional to number of tiles e.g. 4 -> 4 x 4 tiles
    // number of tiles = difficulty * difficulty
    const unsigned int DIFFICULTY = 4;
    const unsigned int NUMBER_OF_BORDERS = DIFFICULTY + 1;
    const unsigned int BORDER_THICKNESS = 6;
    const unsigned int TILE_WIDTH = (SCREEN_WIDTH - NUMBER_OF_BORDERS * BORDER_THICKNESS) / DIFFICULTY; 
    const unsigned int TILE_HEIGHT = TILE_WIDTH;

    // Initialise SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL could not initialise! Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Intialise SDL_ttf
    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialise! Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Sliding Puzzle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "SDL could not create window! Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        std::cout << "SDL could not create renderer! Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Load font for text
    const int fontSize = TILE_HEIGHT - 40;
    TTF_Font* font = TTF_OpenFont("assets/octin sports free.ttf", fontSize);
    if (font == nullptr) {
        std::cout << "Failed to load font! Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    // Create vector of tiles
    std::vector<std::vector<Tile>> tiles;
    int startY = 0;
    for (int row = 0; row < DIFFICULTY; ++row) {
        std::vector<Tile> tileRow;
        startY += BORDER_THICKNESS;
        int startX = 0;
        for (int col = 0; col < DIFFICULTY; ++col) {

            startX += BORDER_THICKNESS;
            SDL_Rect rect = {startX, startY, TILE_WIDTH, TILE_HEIGHT};
            // Make final tile "empty"
            SDL_Color colour;
            if (row == DIFFICULTY - 1 && col == DIFFICULTY - 1) {
                colour = {0, 0, 0, 255};
            } else {
                colour = {255, 123, 43, 255};
            }
            // convert number to string
            int number = row * (DIFFICULTY) + col + 1;
            std::string numberStr = std::to_string(number);

            Tile tile(rect, colour, numberStr.c_str(), font, renderer);
            tileRow.push_back(tile);
            
            startX += TILE_WIDTH;
        }
        startY += TILE_HEIGHT;
        tiles.push_back(tileRow);
    }

    // frame rate parameters
    const unsigned int FPS = 60;
    const float milliSecondsPerFrame = 1000 / FPS;
    float lastTimeRendered = SDL_GetTicks();
    float deltaTimeRendered;

    // Animation speed parameters
    const unsigned int pixelsPerSecond = 1000;
    const float milliSecondsPerPixel = 1000 / pixelsPerSecond;
    float lastTimeMoved = SDL_GetTicks();
    float deltaTimeMoved;
    
    // Track the "moving" and "empty" tile
    Tile* movingTile = nullptr;
    bool doneMoving = true;
    Tile* emptyTile = &tiles[DIFFICULTY - 1][DIFFICULTY - 1];
    bool selected = false;
    int oldXPosition;
    int oldYPosition;

    // Shuffle tiles
    


    // Game loop variables
    bool stop = false;
    SDL_Event event;

    // Game loop
    while (!stop) {
        // Handle events
        while (SDL_PollEvent(&event) != 0) {
            // Handle quiting
            if (event.type == SDL_QUIT) {
                stop = true;
            }
            // Handle mouse down
            if (doneMoving) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    for (int row = 0; row < tiles.size(); ++row) {
                        for (int col = 0; col < tiles[row].size(); ++col) {
                            if (tiles[row][col].isMouseInside(x, y)) {
                                if (isEmptyTileInNeighbours(tiles, row, col, emptyTile)) {
                                    movingTile = &tiles[row][col];
                                    selected = true;
                                    doneMoving = false;
                                }
                            }
                        }
                    }
                }
            }
        }

        // If a tile is selected (moving) save position for new empty tile position
        if (selected && movingTile != nullptr) {
            oldXPosition = movingTile->getXPosition();
            oldYPosition = movingTile->getYPosition();
            selected = false;
        }

        // Move tile into empty tile and control animation speed (indepedant of frame rate)
        if (movingTile != nullptr) {
            deltaTimeMoved = SDL_GetTicks() - lastTimeMoved;
            if (deltaTimeMoved > milliSecondsPerPixel) {
                doneMoving = movingTile->moveTo(emptyTile->getXPosition(), emptyTile->getYPosition());
                if (doneMoving) {
                    emptyTile->setPositionTo(oldXPosition, oldYPosition);
                    std::iter_swap(movingTile, emptyTile);
                    emptyTile = movingTile;
                    movingTile = nullptr;
                }
                lastTimeMoved = SDL_GetTicks();
            }
        }
        
        // Control frame rate (rendering rate)
        deltaTimeRendered = SDL_GetTicks() - lastTimeRendered;
        if (deltaTimeRendered > milliSecondsPerFrame) {
            // Clear screen and render tiles
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            
            // Render all tiles
            for (int row = 0; row < tiles.size(); ++row) {
                for (int col = 0; col < tiles[row].size(); ++col) {
                    // Don't render empty tile
                    if (emptyTile == &tiles[row][col]) {
                        continue;
                    }
                    tiles[row][col].render(renderer);
                }
            }

            // Update screen from backbuffer and clear backbuffer
            SDL_RenderPresent(renderer);
            lastTimeRendered = SDL_GetTicks();
        }

        // Add overall delay to slow program down
        // SDL_Delay(5);
    }

    // Free textures
    for (int row = 0; row < tiles.size(); ++row) {
        for (int col = 0; col < tiles[row].size(); ++col) {
            tiles[row][col].free();
        }
    }

    // Destroy renderer and window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = nullptr;
    window = nullptr;

    // Free font
    TTF_CloseFont(font);
    font = nullptr;

    // Quit SDL
    SDL_Quit();

    // Exit program
    std::cout << "Exiting program..." << std::endl;
    return 0;
}   