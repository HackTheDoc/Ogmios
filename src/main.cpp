#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "tinyfiledialogs.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int UI_HEIGHT = 30;
const int TEXT_LEFT_SPAN = 24;

std::vector<std::string> lines;

int cursorX = 0;
int cursorY = 0;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
SDL_Color fontColor = {0, 0, 0, 255};
SDL_Color indexColor = {51, 51, 51, 255};

SDL_Rect saveButtonBox = {5, 5, 50, UI_HEIGHT-10};
SDL_Rect loadButtonBox = {5 + saveButtonBox.x + saveButtonBox.w, 5, 50, UI_HEIGHT-10};


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

    font = TTF_OpenFont("fonts/Nunito-Regular.ttf", 16);

    SDL_StartTextInput();

    return success;
}

void moveCursorUp() {
    if (cursorY > 0) {
        cursorY--;
        cursorX = std::min(cursorX, static_cast<int>(lines[cursorY].size()));
    }
}

void moveCursorDown() {
    if (cursorY < static_cast<int>(lines.size() - 1) ) {
        cursorY++;
        cursorX = std::min(cursorX, static_cast<int>(lines[cursorY].size()));
    }
}

void moveCursorLeft() {
    if (cursorX > 0) {
        cursorX--;
    }
}

void moveCursorRight() {
    cursorX++;
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

void insertNewLine() {
    std::string newLine = lines[cursorY].substr(cursorX);
    lines[cursorY] = lines[cursorY].substr(0, cursorX);
    lines.insert(lines.begin() + cursorY + 1, newLine);
    moveCursorDown();
    cursorX = 0;
}

bool deleteLine() {
    bool deleted = false;

    if (cursorX == 0 && cursorY > 0) {
        std::string oldLine = lines[cursorY];
        lines.erase(lines.begin() + cursorY);
        cursorY--;
        cursorX = static_cast<int>(lines[cursorY].size());

        if (oldLine.size()) {
            lines[cursorY].append(oldLine);
        }
        deleted = true;
    }

    return deleted;
}

void clearEditor() {
    lines.clear();
    lines.push_back("");

    cursorY = 0;
    cursorX = 0;
}

void save() {
    char* path = tinyfd_saveFileDialog("Save", "Output/unknown.txt", 0, NULL, NULL);
    
    if (path != NULL) {
        std::ofstream out(path);
        for (const std::string& line : lines) {
            out << line << std::endl;
        }
        out.close();

        clearEditor();
    } else {
        tinyfd_messageBox("Ogmios", "Cannot save the file !", "ok", "error", 1);
    }
}

void load() {
    char* path = tinyfd_openFileDialog("Open", "/Output/", 0, NULL, NULL, 0);

    if (path != NULL) {
        lines.clear();
        std::ifstream in(path);
        std::string line;
        while (std::getline(in, line)) {
            lines.push_back(line);
        }
        in.close();

        cursorY = static_cast<int>(lines.size() - 1);
        cursorX = static_cast<int>(lines[cursorY].size());
    } else {
        tinyfd_messageBox("Ogmios", "Cannot open the file !", "ok", "error", 1);
    }
}

void renderText() {
    int y = UI_HEIGHT + 2;
    for (int i = 0; i < static_cast<int>(lines.size()); i++) {
        // Render Line Index
        std::string index = std::to_string(i);
        SDL_Surface* iS = TTF_RenderText_Blended(font, index.c_str(), indexColor);
        SDL_Texture* iT = SDL_CreateTextureFromSurface(renderer, iS);
        SDL_Rect iR = {2, y, iS->w, iS->h};

        SDL_RenderCopy(renderer, iT, nullptr, &iR);
        SDL_FreeSurface(iS);
        SDL_DestroyTexture(iT);

        // Render Separator
        SDL_SetRenderDrawColor(renderer, 51, 51, 51, 255);
        SDL_RenderDrawLine(renderer, TEXT_LEFT_SPAN - 2, iR.y + 1, TEXT_LEFT_SPAN - 2, iR.y + iR.h - 1);

        // Render Line Text
        if (lines[i].size()) {
            SDL_Surface* tS = TTF_RenderText_Blended(font, lines[i].c_str(), fontColor);
            SDL_Texture* tT = SDL_CreateTextureFromSurface(renderer, tS);
            SDL_Rect tR = {TEXT_LEFT_SPAN, y, tS->w, tS->h};

            SDL_RenderCopy(renderer, tT, nullptr, &tR);
            SDL_FreeSurface(tS);
            SDL_DestroyTexture(tT);
        }

        y += 22;
    }
}

void renderCursor() {
    int w,h;
    TTF_SizeText(font, lines[cursorY].substr(0, cursorX).c_str(), &w, &h);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(renderer,
        w + TEXT_LEFT_SPAN,
        cursorY * 22 + UI_HEIGHT + 4,
        w + TEXT_LEFT_SPAN,
        (cursorY + 1) * 22 + UI_HEIGHT
        );
}

void renderUI() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 127);

    // Save Button Box
    SDL_RenderDrawRect(renderer, &saveButtonBox);

    // Save Button Label
    SDL_Surface* saveButtonSurface = TTF_RenderText_Blended(font, "Save", fontColor);
    SDL_Texture* saveButtonTexture = SDL_CreateTextureFromSurface(renderer, saveButtonSurface);
    SDL_Rect saveButton = {10, 4, saveButtonSurface->w, saveButtonSurface->h};
    SDL_RenderCopy(renderer, saveButtonTexture, nullptr, &saveButton);
    SDL_FreeSurface(saveButtonSurface);
    SDL_DestroyTexture(saveButtonTexture);

    // Load Button Box
    SDL_RenderDrawRect(renderer, &loadButtonBox);

    // Load Button Label
    SDL_Surface* loadButtonSurface = TTF_RenderText_Blended(font, "Load", fontColor);
    SDL_Texture* loadButtonTexture = SDL_CreateTextureFromSurface(renderer, loadButtonSurface);
    SDL_Rect loadButton = {loadButtonBox.x + 5, 4, loadButtonSurface->w, loadButtonSurface->h};
    SDL_RenderCopy(renderer, loadButtonTexture, nullptr, &loadButton);
    SDL_FreeSurface(loadButtonSurface);
    SDL_DestroyTexture(loadButtonTexture);

    // Draw Editor name
    SDL_Surface* nameSurface = TTF_RenderText_Blended(font, "Ogmios Editor", fontColor);
    SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
    SDL_Rect nameRect = {WINDOW_WIDTH - nameSurface->w - 5, 4, nameSurface->w, nameSurface->h};
    SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
    SDL_FreeSurface(nameSurface);
    SDL_DestroyTexture(nameTexture);

    // Draw UI Border
    SDL_RenderDrawLine(renderer, 0, UI_HEIGHT, WINDOW_WIDTH, UI_HEIGHT);
}

void handleUIEvent(const SDL_Event& event) {
    SDL_Point mousePos;
    SDL_GetMouseState(&mousePos.x, &mousePos.y);

    // Save Button Event
    if (SDL_PointInRect(&mousePos, &saveButtonBox)) {
        save();
    }
    // Load Button Event
    else if (SDL_PointInRect(&mousePos, &loadButtonBox)) {
        load();
    }
    //  Move mouse in editor
    else if (mousePos.y >= UI_HEIGHT && mousePos.y < WINDOW_HEIGHT && mousePos.x >= 0 && mousePos.x < WINDOW_WIDTH) {
        int lineIndex = (mousePos.y - UI_HEIGHT) / 22;
        
        int w,h;
        TTF_SizeText(font, lines[lineIndex].c_str(), &w, &h);

        int charPos = 0;
        int width = 0;
        for (char c : lines[lineIndex]) {
            int charW, charH;
            TTF_SizeText(font, &c, &charW, &charH);
            
            if (width + charW / 2 > mousePos.x) {
                break;
            }

            width += charW;
            charPos++;
        }

        cursorX = charPos;
        cursorY = lineIndex;
    }
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
                    case SDLK_UP:
                        moveCursorUp();
                        break;
                    case SDLK_DOWN:
                        moveCursorDown();
                        break;
                    case SDLK_LEFT:
                        moveCursorLeft();
                        break;
                    case SDLK_RIGHT:
                        moveCursorRight();
                        break;   
                    case SDLK_BACKSPACE:        // SUPPR CHAR
                        if (!deleteLine()) {
                            deleteChar();
                        }
                        break;
                    case SDLK_RETURN:           // NEW LINE
                        insertNewLine();
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
            case SDL_MOUSEBUTTONUP:
                handleUIEvent(event);
                break;
            default:
                break;
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    renderText();
    renderCursor();
    renderUI();
    
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

