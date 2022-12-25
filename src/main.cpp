#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "tinyfiledialogs.h"

// Const
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const int TEXT_LEFT_SPAN = 24;
const int LINE_HEIGHT = 22;

const int SCROLL_BAR_WIDTH = 5;
const int SCROLL_SPEED = 1;

// Var
std::vector<std::string> lines;

int cursorX = 0;
int cursorY = 0;
int scrollPosition = 0;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

SDL_Color fontColor = {65, 34, 52, 255};
SDL_Color cursorColor = {204, 0, 153, 255};
SDL_Color UIColor = {50, 26, 40, 255};
SDL_Color textBackgroundColor = {234, 215, 215, 255};
SDL_Color UIBackgroundColor = {194, 173, 207, 255};

SDL_Rect UI = {0, 0, WINDOW_WIDTH, 30};
SDL_Rect viewport = {0, UI.h, WINDOW_WIDTH, WINDOW_HEIGHT - UI.h};
SDL_Rect scrollBar = {WINDOW_WIDTH - SCROLL_BAR_WIDTH, 0, SCROLL_BAR_WIDTH, 0};
SDL_Rect saveButtonBox = {5, 5, 50, UI.h-10};
SDL_Rect loadButtonBox = {5 + saveButtonBox.x + saveButtonBox.w, 5, 50, UI.h-10};


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

void jumpToLineStart() {
    cursorX = 0;
}

void jumpToLineEnd() {
    cursorX = static_cast<int>(lines[cursorY].size());
}

void jumpToFileStart() {
    cursorY = 0;
    jumpToLineStart();
}

void jumpToFileEnd() {
    cursorY = static_cast<int>(lines.size() - 1);
    jumpToLineEnd();
}

void scroll(int y) {
    scrollPosition = y * SCROLL_SPEED;
    scrollPosition = std::max(0, std::min(scrollPosition, static_cast<int>( (lines.size()-1) * LINE_HEIGHT - WINDOW_HEIGHT + UI.h)));
}

void moveCursorUp() {
    if (cursorY > 0) {
        cursorY--;
        cursorX = std::min(cursorX, static_cast<int>(lines[cursorY].size()));

        if ( cursorY  < scrollPosition) {
            scroll(-1);
        }
    }
}

void moveCursorDown() {
    if (cursorY < static_cast<int>(lines.size() - 1) ) {
        cursorY++;
        cursorX = std::min(cursorX, static_cast<int>(lines[cursorY].size()));
        
        if ( (cursorY+1) * LINE_HEIGHT > WINDOW_HEIGHT - UI.h) {
            scroll(1);
        }
    }
}

void moveCursorLeft() {
    if (cursorX == 0) {
        moveCursorUp();
        cursorX = static_cast<int>(lines[cursorY].size());
    }
    else if (cursorX > 0) {
        cursorX--;
    }
}

void moveCursorRight() {
    if (
        cursorX++ > static_cast<int>(lines[cursorY].size()) && 
        cursorY < static_cast<int>(lines.size() - 1)
    ) {
        cursorY++;
        cursorX = 0;
    }
    else {
        cursorX++;
    }
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
        moveCursorUp();
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

    jumpToFileStart();
}


void save() {
    char const * filterPatterns[2] = { "*.txt", "*.text" };
    char* path = tinyfd_saveFileDialog("Save", "./Output/unknow.txt", 2, filterPatterns, NULL);
    
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
    char const * filterPatterns[2] = { "*.txt", "*.text" };
    char* path = tinyfd_openFileDialog("Open", "Output/unknow.txt", 2, filterPatterns, NULL, 0);

    if (path != NULL) {
        lines.clear();
        std::ifstream in(path);
        std::string line;
        while (std::getline(in, line)) {
            lines.push_back(line);
        }
        in.close();

        jumpToFileEnd();
    } else {
        tinyfd_messageBox("Ogmios", "Cannot open the file !", "ok", "error", 1);
    }
}

void updateScrollBar() {
    scrollBar.h = (WINDOW_HEIGHT - UI.h) * (WINDOW_HEIGHT - UI.h) / static_cast<int>(lines.size() * LINE_HEIGHT);
    scrollBar.y = scrollPosition * (WINDOW_HEIGHT - UI.h - scrollBar.h) / static_cast<int>(lines.size() * LINE_HEIGHT);

    viewport.y = -scrollPosition * LINE_HEIGHT + UI.h;
}

std::vector<std::string> splitLine(const std::string& line, TTF_Font* font, int viewportWidth) {
    std::vector<std::string> lines;
    std::string currentLine;
    int currentLineWidth = 0;
    for (char c : line) {
        currentLine += c;

        TTF_SizeText(font, currentLine.c_str(), &currentLineWidth, nullptr);

        if (currentLineWidth > viewportWidth) {
            lines.push_back(currentLine.substr(0, currentLine.length() - 1));
            currentLine = c;
        }
    }
    lines.push_back(currentLine);
    
    return lines;
}

void renderText() {
    SDL_RenderSetViewport(renderer, &viewport);

    int y = 2;
    for (int i = 0; i < static_cast<int>(lines.size()); i++) {
        // Render Line Index
        std::string index = std::to_string(i);
        SDL_Surface* iS = TTF_RenderText_Blended(font, index.c_str(), UIColor);
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
            auto splitLines = splitLine(lines[i], font, WINDOW_WIDTH - TEXT_LEFT_SPAN);
            for (const auto& line : splitLines) {
                SDL_Surface* tS = TTF_RenderText_Blended(font, line.c_str(), fontColor);
                SDL_Texture* tT = SDL_CreateTextureFromSurface(renderer, tS);
                SDL_Rect tR = {TEXT_LEFT_SPAN, y, tS->w, tS->h};

                SDL_RenderCopy(renderer, tT, nullptr, &tR);
                SDL_FreeSurface(tS);
                SDL_DestroyTexture(tT);
                y += LINE_HEIGHT;
            }
        }
        y += LINE_HEIGHT;
    }

    SDL_RenderSetViewport(renderer, nullptr);
}

void renderCursor() {
    SDL_RenderSetViewport(renderer, &viewport);

    int w,h;
    TTF_SizeText(font, lines[cursorY].substr(0, cursorX).c_str(), &w, &h);
    SDL_SetRenderDrawColor(renderer, cursorColor.r, cursorColor.g, cursorColor.b, cursorColor.a);
    SDL_RenderDrawLine(renderer,
        w + TEXT_LEFT_SPAN,
        cursorY * LINE_HEIGHT + 4,
        w + TEXT_LEFT_SPAN,
        (cursorY + 1) * LINE_HEIGHT
        );

    SDL_RenderSetViewport(renderer, nullptr);
}

void renderUI() {
    // Scroll Bar
    SDL_RenderSetViewport(renderer, &viewport);
    SDL_SetRenderDrawColor(renderer, UIColor.r, UIColor.g, UIColor.b, 127);
    SDL_RenderFillRect(renderer, &scrollBar);
    SDL_RenderSetViewport(renderer, nullptr);

    // Background
    SDL_SetRenderDrawColor(renderer, UIBackgroundColor.r, UIBackgroundColor.g, UIBackgroundColor.b, UIBackgroundColor.a);
    SDL_RenderFillRect(renderer, &UI);

    SDL_SetRenderDrawColor(renderer, UIColor.r, UIColor.g, UIColor.b, UIColor.a);

    // Save Button Box
    SDL_RenderDrawRect(renderer, &saveButtonBox);

    // Save Button Label
    SDL_Surface* saveButtonSurface = TTF_RenderText_Blended(font, "Save", UIColor);
    SDL_Texture* saveButtonTexture = SDL_CreateTextureFromSurface(renderer, saveButtonSurface);
    SDL_Rect saveButton = {10, 4, saveButtonSurface->w, saveButtonSurface->h};
    SDL_RenderCopy(renderer, saveButtonTexture, nullptr, &saveButton);
    SDL_FreeSurface(saveButtonSurface);
    SDL_DestroyTexture(saveButtonTexture);

    // Load Button Box
    SDL_RenderDrawRect(renderer, &loadButtonBox);

    // Load Button Label
    SDL_Surface* loadButtonSurface = TTF_RenderText_Blended(font, "Load", UIColor);
    SDL_Texture* loadButtonTexture = SDL_CreateTextureFromSurface(renderer, loadButtonSurface);
    SDL_Rect loadButton = {loadButtonBox.x + 5, 4, loadButtonSurface->w, loadButtonSurface->h};
    SDL_RenderCopy(renderer, loadButtonTexture, nullptr, &loadButton);
    SDL_FreeSurface(loadButtonSurface);
    SDL_DestroyTexture(loadButtonTexture);

    // Draw Editor name
    SDL_Surface* nameSurface = TTF_RenderText_Blended(font, "Ogmios Editor", UIColor);
    SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
    SDL_Rect nameRect = {WINDOW_WIDTH - nameSurface->w - 5, 4, nameSurface->w, nameSurface->h};
    SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
    SDL_FreeSurface(nameSurface);
    SDL_DestroyTexture(nameTexture);

    // Draw UI Border
    SDL_RenderDrawLine(renderer, 0, UI.h, WINDOW_WIDTH, UI.h);
}

void handleTextEditorEvents(SDL_Keycode key) {
    switch (key) {
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
        case SDLK_HOME:
            jumpToLineStart();
            break;
        case SDLK_END:
            jumpToLineEnd();
            break;
        case SDLK_PAGEUP:
            jumpToFileStart();
            break;
        case SDLK_PAGEDOWN:
            jumpToFileEnd();
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
}

void handleUIEvents() {
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
    else if (mousePos.y >= UI.h && mousePos.y < WINDOW_HEIGHT && mousePos.x >= 0 && mousePos.x < WINDOW_WIDTH) {
        int lineIndex = (mousePos.y - UI.h) / LINE_HEIGHT;
        
        if (lineIndex < static_cast<int>(lines.size())) {
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
        else {
            jumpToFileEnd();
        }
        
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
                handleTextEditorEvents(event.key.keysym.sym);
                break;
            case SDL_MOUSEBUTTONUP:
                handleUIEvents();
                break;
            case SDL_MOUSEWHEEL:
                scroll(-event.wheel.y);
                break;
            default:
                break;
        }
    }

    SDL_SetRenderDrawColor(renderer, textBackgroundColor.r, textBackgroundColor.g, textBackgroundColor.b, textBackgroundColor.a);
    SDL_RenderClear(renderer);

    updateScrollBar();
    
    std::cout << cursorY << " " << scrollPosition << std::endl;

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
