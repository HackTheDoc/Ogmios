#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int UI_HEIGHT = 50;

std::vector<std::string> lines;

int cursorX = 0;
int cursorY = 0;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
SDL_Color fontColor = {0, 0, 0, 255};

bool init() {
    bool success = true;
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        std::cout << "Subsystems initialized!..." << std::endl;

        window = SDL_CreateWindow(
            "Ogmios", 
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN
            );
        if (window) {
            std::cout << "Window created!" << std::endl;
        } else {
            success = false;
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            std::cout << "Renderer created!" << std::endl;
        } else {
            success = false;
        }
    } else {
        success = false;
    }
    if (TTF_Init() == -1) {
        std::cout << "Error initializing SDL_TTF!" << std::endl;
        success = false;
    }

    font = TTF_OpenFont("Comfortaa-Regular.ttf", 16);

    SDL_StartTextInput();

    return success;
}

void insertChar(char c) {
    lines[cursorY].insert(cursorX, 1, c);
    cursorX++;
}

void deleteChar() {
    if (cursorX > 0) {
        lines[cursorY].erase(cursorX - 1, 1);
        cursorX--;
    }
}

void moveCursorUp() {
    if (cursorY > 0) {
        cursorY--;
        cursorX = std::min(cursorX, (int)lines[cursorY].size());
    }
}

void moveCursorDown() {
    if (cursorY < (int)(lines.size() - 1) ) {
        cursorY++;
        cursorX = std::min(cursorX, (int)lines[cursorY].size());
    }
}

void renderText() {
    int y = 0;
    for (const std::string& line : lines) {
        std::cout << "Bloup Bloup 0" << std::endl;
        SDL_Surface* s = TTF_RenderText_Solid(font, line.c_str(), fontColor);
        std::cout << "Bloup Bloup 1" << std::endl;
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        std::cout << "Bloup Bloup 2" << std::endl;
        SDL_Rect r = {0, y, s->w, s->h};
        std::cout << "Bloup Bloup 3" << std::endl;
        
        SDL_RenderCopy(renderer, t, nullptr, &r);
        std::cout << "Bloup Bloup 4" << std::endl;
        SDL_FreeSurface(s);
        std::cout << "Bloup Bloup 5" << std::endl;
        SDL_DestroyTexture(t);
        std::cout << "Bloup Bloup 6" << std::endl;

        y += TTF_FontLineSkip(font);
        std::cout << "Bloup Bloup 7" << std::endl;
    }
    std::cout << "Text Rendered" << std::endl;
}

void save(const std::string& filename) {
    std::ofstream out(filename);
    for (const std::string& line : lines) {
        out << line << std::endl;
    }
    out.close();
}

void load(const std::string& filename) {
    lines.clear();
    std::ifstream in(filename);
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    in.close();
}

bool loop() {
    bool looping = true;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                looping = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        moveCursorUp();
                        break;
                    case SDLK_DOWN:
                        moveCursorDown();
                        break;
                    case SDLK_BACKSPACE:
                        deleteChar();
                        break;
                    default:
                        insertChar(event.key.keysym.sym);
                        break;
                }
                break;
            default:
                break;
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    renderText();

    SDL_RenderPresent(renderer);
    
    return looping;
}

void kill() {
    SDL_StopTextInput();
    SDL_DestroyWindow(window);
    window = nullptr;
    renderer = nullptr;
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    if (init()) {
        lines.push_back("");
        while (loop()) {}
        kill();
    } else {
        system("pause");
        return 1;
    }
    return 0;
}

