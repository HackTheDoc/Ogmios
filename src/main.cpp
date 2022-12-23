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
        if (line.size()) {
            SDL_Surface* s = TTF_RenderText_Solid(font, line.c_str(), fontColor);
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);

            SDL_Rect r = {0, y, s->w, s->h};
            
            SDL_RenderCopy(renderer, t, nullptr, &r);
            SDL_FreeSurface(s);
            SDL_DestroyTexture(t);
        }

        y += TTF_FontLineSkip(font);
    }
}

void renderCursor() {
    int w,h;
    TTF_SizeText(font, lines[cursorY].substr(0, cursorX).c_str(), &w, &h);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    int lineSkip = TTF_FontLineSkip(font);
    SDL_RenderDrawLine(renderer, w, cursorY * lineSkip, w, (cursorY + 1) * lineSkip);
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
            case SDL_TEXTINPUT:
                insertChar(*event.text.text);
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_UP:               // CURSOR UP
                        moveCursorUp();
                        break;
                    case SDLK_DOWN:             // CURSOR DOWN
                        moveCursorDown();
                        break;
                    case SDLK_BACKSPACE:        // SUPPR CHAR
                        deleteChar();
                        break;
                    case SDLK_RETURN:           // NEW LINE
                        lines.push_back("");
                        moveCursorDown();
                        break;
                    case SDLK_c:                // COPY
                        if (SDL_GetModState() & KMOD_CTRL) {
                            SDL_SetClipboardText(lines[cursorY].c_str());
                        }
                        break;
                    case SDLK_v:                // PASTE
                        if (SDL_GetModState() & KMOD_CTRL) {
                            lines[cursorY].append(SDL_GetClipboardText());
                        }
                        break;
                    default:
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
    renderCursor();

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
