#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "tinyfiledialogs.h"

// Const
const int WINDOW_WIDTH_MIN = 384;
const int WINDOW_HEIGHT_MIN = 128;
const int WINDOW_WIDTH_DEFAULT = 800;
const int WINDOW_HEIGHT_DEFAULT = 600;

const int DEFAULT_EDITOR_LEFT_MARGIN = 24;
const int DEFAULT_LINE_HEIGHT = 22;

const int SCROLL_BAR_WIDTH = 5;
const int SCROLL_SPEED = 1;

const int BUTTON_SPAN = 5;
const int BUTTON_WIDTH = 50;

const int DEFAULT_FONT_SIZE = 16;

enum themes { DAY, NIGHT, numberOfThemes };

// Var
int windowWidth;
int windowHeight;

std::vector<std::string> lines;

int cursorX = 0;
int cursorY = 0;
int rCursorX;
int rCursorY;
int scrollPosition = 0;

int currentFontSize;

int editorLeftMargin;
int lineHeight;

int currentTheme;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

SDL_Color fontColor[numberOfThemes];
SDL_Color cursorColor[numberOfThemes];
SDL_Color UIColor[numberOfThemes];
SDL_Color textBackgroundColor[numberOfThemes];
SDL_Color UIBackgroundColor[numberOfThemes];

SDL_Texture* themesIcons[numberOfThemes];

SDL_Rect UI;
SDL_Rect viewport;

SDL_Rect scrollBar;

SDL_Rect saveButtonBox;
SDL_Rect loadButtonBox;
SDL_Rect minusButtonBox;
SDL_Rect sizeButtonBox;
SDL_Rect plusButtonBox;
SDL_Rect themeButtonBox;


SDL_Texture* LoadTexture(const char* fileName) {
    SDL_Surface* tmpSurface = IMG_Load(fileName);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
    SDL_FreeSurface(tmpSurface);
    return texture;
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

void initRects() {
    UI = {0, 0, windowWidth, 30};
    viewport = {0, UI.h, windowWidth, windowHeight - UI.h};

    scrollBar = {windowWidth - SCROLL_BAR_WIDTH, 0, SCROLL_BAR_WIDTH, 0};

    saveButtonBox = {BUTTON_SPAN, BUTTON_SPAN, BUTTON_WIDTH, UI.h-10};
    loadButtonBox = {BUTTON_SPAN + saveButtonBox.x + saveButtonBox.w, BUTTON_SPAN, BUTTON_WIDTH, UI.h-10};
    minusButtonBox = {loadButtonBox.x + loadButtonBox.w + BUTTON_SPAN, BUTTON_SPAN, UI.h - 10, UI.h - 10};
    sizeButtonBox = {minusButtonBox.x + minusButtonBox.w, BUTTON_SPAN, 40, UI.h - 10};
    plusButtonBox = {sizeButtonBox.x + sizeButtonBox.w, BUTTON_SPAN, UI.h - 10, UI.h - 10};
    themeButtonBox = {windowWidth - UI.h + 5, BUTTON_SPAN, UI.h - 10, UI.h - 10};
}

void updateRects() {
    UI.w = windowWidth;
    viewport.w = windowWidth;

    scrollBar.x = windowWidth - SCROLL_BAR_WIDTH;

    themeButtonBox.x = windowWidth - UI.h + 5;
}

bool init() {
    bool success = true;
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        std::cout << "Subsystems initialized!..." << std::endl;

        window = SDL_CreateWindow(
            "Ogmios", 
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
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

    font = TTF_OpenFont("fonts/Nunito-Regular.ttf", DEFAULT_FONT_SIZE);
    currentFontSize = DEFAULT_FONT_SIZE;

    windowWidth = WINDOW_WIDTH_DEFAULT;
    windowHeight = WINDOW_HEIGHT_DEFAULT;
    SDL_SetWindowMinimumSize(window, WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);

    lineHeight = DEFAULT_LINE_HEIGHT;
    editorLeftMargin = DEFAULT_EDITOR_LEFT_MARGIN;

    rCursorX = editorLeftMargin;
    rCursorY = 0;

    initRects();
    updateRects();

    #pragma region INIT THEMES
    //  DAY
    fontColor[DAY] = {65, 34, 52, 255};
    cursorColor[DAY] = {204, 0, 153, 255};
    UIColor[DAY] = {50, 26, 40, 255};
    textBackgroundColor[DAY] = {234, 215, 215, 255};
    UIBackgroundColor[DAY] = {194, 173, 207, 255};
    themesIcons[DAY] = LoadTexture("./icons/sun.png");

    //  NIGHT
    fontColor[NIGHT] = {255, 255, 255, 255};
    cursorColor[NIGHT] = {255, 255, 255, 255};
    UIColor[NIGHT] = {255, 255, 255, 255};
    textBackgroundColor[NIGHT] = {0, 0, 0, 255};
    UIBackgroundColor[NIGHT] = {128, 128, 128, 255};
    themesIcons[NIGHT] = LoadTexture("./icons/moon.png");

    currentTheme = DAY;

    #pragma endregion

    SDL_Surface* icon = IMG_Load("icons/Ogmios.png");
    SDL_SetWindowIcon(window, icon);

    SDL_StartTextInput();

    return success;
}


void updateRenderCursorX() {
    TTF_SizeText(font, lines[cursorY].substr(0, cursorX).c_str(), &rCursorX, nullptr);
    rCursorX += editorLeftMargin;
}

void updateRenderCursorY() {
    rCursorY = cursorY * lineHeight;
}


void jumpToLineStart() {
    cursorX = 0;
    updateRenderCursorX();
}

void jumpToLineEnd() {
    cursorX = static_cast<int>(lines[cursorY].size());
    updateRenderCursorX();
}

void jumpToFileStart() {
    cursorY = 0;
    jumpToLineStart();
    updateRenderCursorY();
}

void jumpToFileEnd() {
    cursorY = static_cast<int>(lines.size() - 1);
    jumpToLineEnd();
    updateRenderCursorY();
}


void scroll(int y) {
    scrollPosition += y;
    scrollPosition = std::max(0, std::min(scrollPosition, static_cast<int>(lines.size()-1)));
}


void moveCursorUp() {
    if (cursorY > 0) {
        cursorY--;
        cursorX = std::min(cursorX, static_cast<int>(lines[cursorY].size()));

        if ( cursorY  < scrollPosition) {
            scroll(-1);
        }

        updateRenderCursorY();
        updateRenderCursorX();
    }
}

void moveCursorDown() {
    if (cursorY < static_cast<int>(lines.size() - 1) ) {
        cursorY++;
        cursorX = std::min(cursorX, static_cast<int>(lines[cursorY].size()));
        
        if ( (cursorY+1) * lineHeight > windowHeight - UI.h) {
            scroll(1);
        }
        
        std::cout << cursorY-1 << " --> " << cursorY << std::endl;

        updateRenderCursorY();
        updateRenderCursorX();
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
    updateRenderCursorX();
}

void moveCursorRight() {
    if (cursorX == static_cast<int>(lines[cursorY].size())) {
        moveCursorDown();
        cursorX = 0;
        updateRenderCursorX();
    }
    else {
        auto sublines = splitLine(lines[cursorY], font, windowWidth - editorLeftMargin);
        
        if (sublines.size()) {
            int y = rCursorY / lineHeight;
            int x = 0;
            for (int i = 0; i <= y-cursorY; i++) {
                x += static_cast<int>(sublines.size() -1);
            }

            TTF_SizeText(font, sublines[y - cursorY].substr(0, x).c_str(), &rCursorX, nullptr);
            rCursorX += editorLeftMargin;

            cursorX++;
        }
        else {
            cursorX++;
            updateRenderCursorX();
        }
    }
}


void insertChar(char c) {
    lines[cursorY].insert(cursorX, 1, c);
    cursorX++;
    updateRenderCursorX();
}

void deletePreviousChar() {
    if (cursorX > 0) {
        lines[cursorY].erase(cursorX - 1, 1);
        cursorX--;
        updateRenderCursorX();
    }
}

void deleteNextChar() {
    if (cursorX < static_cast<int>(lines[cursorY].size())) {
        lines[cursorY].erase(cursorX, 1);
    }
}

void insertTab() {
    lines[cursorY].insert(cursorX, "\t");
    cursorX++;
    updateRenderCursorX();
}

void insertNewLine() {
    std::string newLine = lines[cursorY].substr(cursorX);
    lines[cursorY] = lines[cursorY].substr(0, cursorX);
    lines.insert(lines.begin() + cursorY + 1, newLine);
    moveCursorDown();
    cursorX = 0;
    
    updateRenderCursorX();

    viewport.h += lineHeight;
}

bool deleteCurrentLine() {
    bool deleted = false;

    if (cursorX == 0 && cursorY > 0) {
        std::string oldLine = lines[cursorY];
        lines.erase(lines.begin() + cursorY);
        moveCursorUp();
        cursorX = static_cast<int>(lines[cursorY].size());
        updateRenderCursorX();

        if (oldLine.size()) {
            lines[cursorY].append(oldLine);
        }

        viewport.h -= lineHeight;

        deleted = true;
    }

    return deleted;
}

bool deleteNextLine() {
    bool deleted = false;

    if (cursorX == static_cast<int>(lines[cursorY].size()) &&
        cursorY < static_cast<int>(lines.size() - 1)
    ) {
        std::string oldLine = lines[cursorY + 1];
        lines.erase(lines.begin() + cursorY + 1);
        
        if (oldLine.size()) {
            lines[cursorY].append(oldLine);
        }

        viewport.h -= lineHeight;

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
    scrollBar.h = (windowHeight - UI.h) * (windowHeight - UI.h) / static_cast<int>(lines.size() * lineHeight);
    scrollBar.y = scrollPosition * (windowHeight - UI.h - scrollBar.h) / static_cast<int>(lines.size() * lineHeight);

    viewport.y = -scrollPosition * lineHeight + UI.h;
}

void updateTheme() {
    currentTheme = !currentTheme;
}

void updateFontSize(TTF_Font* f, int s) {
    currentFontSize += s;

    TTF_SetFontSize(f, currentFontSize);

    if (currentFontSize != DEFAULT_FONT_SIZE) {
        lineHeight += s;
        editorLeftMargin += s;
    }

    updateRenderCursorX();
    updateRenderCursorY();
}


void renderText() {
    SDL_RenderSetViewport(renderer, &viewport);

    int y = 2;
    for (int i = 0; i < static_cast<int>(lines.size()); i++) {
        // Render Line Index
        std::string index = std::to_string(i);
        SDL_Surface* iS = TTF_RenderText_Blended(font, index.c_str(), UIColor[currentTheme]);
        SDL_Texture* iT = SDL_CreateTextureFromSurface(renderer, iS);
        SDL_Rect iR = {2, y, iS->w, iS->h};

        SDL_RenderCopy(renderer, iT, nullptr, &iR);
        SDL_FreeSurface(iS);
        SDL_DestroyTexture(iT);

        // Render Separator
        SDL_SetRenderDrawColor(renderer, 51, 51, 51, 255);
        SDL_RenderDrawLine(renderer, editorLeftMargin - 2, iR.y + 1, editorLeftMargin - 2, iR.y + iR.h - 1);

        // Render Line Text
        if (lines[i].size()) {
            auto tempLines = splitLine(lines[i], font, windowWidth - editorLeftMargin);
            for (int j = 0; j < static_cast<int>(tempLines.size()); j++) {
                SDL_Surface* tS = TTF_RenderText_Blended(font, tempLines[j].c_str(), fontColor[currentTheme]);
                SDL_Texture* tT = SDL_CreateTextureFromSurface(renderer, tS);
                SDL_Rect tR = {editorLeftMargin, y, tS->w, tS->h};

                SDL_RenderCopy(renderer, tT, nullptr, &tR);
                SDL_FreeSurface(tS);
                SDL_DestroyTexture(tT);
                
                y += lineHeight;
            }
        } else {
            y += lineHeight;
        }
    }

    SDL_RenderSetViewport(renderer, nullptr);
}

void renderCursor() {
    SDL_RenderSetViewport(renderer, &viewport);

    SDL_SetRenderDrawColor(renderer, cursorColor[currentTheme].r, cursorColor[currentTheme].g, cursorColor[currentTheme].b, cursorColor[currentTheme].a);
    SDL_RenderDrawLine(renderer,
        rCursorX,
        rCursorY + 4,
        rCursorX,
        rCursorY + lineHeight
        );

    SDL_RenderSetViewport(renderer, nullptr);
}

void renderUI() {
    TTF_SetFontSize(font, DEFAULT_FONT_SIZE);

    // Scroll Bar
    SDL_RenderSetViewport(renderer, &viewport);
    SDL_SetRenderDrawColor(renderer, UIColor[currentTheme].r, UIColor[currentTheme].g, UIColor[currentTheme].b, UIColor[currentTheme].a / 2);
    SDL_RenderFillRect(renderer, &scrollBar);
    SDL_RenderSetViewport(renderer, nullptr);

    // Background
    SDL_SetRenderDrawColor(renderer, UIBackgroundColor[currentTheme].r, UIBackgroundColor[currentTheme].g, UIBackgroundColor[currentTheme].b, UIBackgroundColor[currentTheme].a);
    SDL_RenderFillRect(renderer, &UI);

    SDL_SetRenderDrawColor(renderer, UIColor[currentTheme].r, UIColor[currentTheme].g, UIColor[currentTheme].b, UIColor[currentTheme].a);

    #pragma region SAVE BUTTON
    //  Box
    SDL_RenderDrawRect(renderer, &saveButtonBox);

    // Label
    SDL_Surface* saveButtonSurface = TTF_RenderText_Blended(font, "Save", UIColor[currentTheme]);
    SDL_Texture* saveButtonTexture = SDL_CreateTextureFromSurface(renderer, saveButtonSurface);
    SDL_Rect saveButton = {10, 4, saveButtonSurface->w, saveButtonSurface->h};
    SDL_RenderCopy(renderer, saveButtonTexture, nullptr, &saveButton);
    SDL_FreeSurface(saveButtonSurface);
    SDL_DestroyTexture(saveButtonTexture);
    #pragma endregion

    #pragma region LOAD BUTTON
    // Box
    SDL_RenderDrawRect(renderer, &loadButtonBox);

    // Label
    SDL_Surface* loadButtonSurface = TTF_RenderText_Blended(font, "Load", UIColor[currentTheme]);
    SDL_Texture* loadButtonTexture = SDL_CreateTextureFromSurface(renderer, loadButtonSurface);
    SDL_Rect loadButton = {loadButtonBox.x + 5, 4, loadButtonSurface->w, loadButtonSurface->h};
    SDL_RenderCopy(renderer, loadButtonTexture, nullptr, &loadButton);
    SDL_FreeSurface(loadButtonSurface);
    SDL_DestroyTexture(loadButtonTexture);
    #pragma endregion

    #pragma region SIZE BUTTONS
    // Minus Button
    SDL_RenderDrawRect(renderer, &minusButtonBox);

    SDL_Surface* minusButtonSurface = TTF_RenderText_Blended(font, "-", UIColor[currentTheme]);
    SDL_Texture* minusButtonTexture = SDL_CreateTextureFromSurface(renderer, minusButtonSurface);
    SDL_Rect minusButton = {minusButtonBox.x + 5, 4, minusButtonSurface->w, minusButtonSurface->h};
    SDL_RenderCopy(renderer, minusButtonTexture, nullptr, &minusButton);
    SDL_FreeSurface(minusButtonSurface);
    SDL_DestroyTexture(minusButtonTexture);

    // Size Button
    SDL_RenderDrawRect(renderer, &sizeButtonBox);

    SDL_Surface* sizeButtonSurface = TTF_RenderText_Blended(font, "Size", UIColor[currentTheme]);
    SDL_Texture* sizeButtonTexture = SDL_CreateTextureFromSurface(renderer, sizeButtonSurface);
    SDL_Rect sizeButton = {sizeButtonBox.x + 5, 4, sizeButtonSurface->w, sizeButtonSurface->h};
    SDL_RenderCopy(renderer, sizeButtonTexture, nullptr, &sizeButton);
    SDL_FreeSurface(sizeButtonSurface);
    SDL_DestroyTexture(sizeButtonTexture);

    // Plus Button
    SDL_RenderDrawRect(renderer, &plusButtonBox);

    SDL_Surface* plusButtonSurface = TTF_RenderText_Blended(font, "+", UIColor[currentTheme]);
    SDL_Texture* plusButtonTexture = SDL_CreateTextureFromSurface(renderer, plusButtonSurface);
    SDL_Rect plusButton = {plusButtonBox.x + 5, 4, plusButtonSurface->w, plusButtonSurface->h};
    SDL_RenderCopy(renderer, plusButtonTexture, nullptr, &plusButton);
    SDL_FreeSurface(plusButtonSurface);
    SDL_DestroyTexture(plusButtonTexture);

    #pragma endregion

    #pragma region THEME BUTTON
    // Logo
    SDL_RenderCopy(renderer, themesIcons[currentTheme], nullptr, &themeButtonBox);

    //  Box
    SDL_RenderDrawRect(renderer, &themeButtonBox);

    #pragma endregion

    // Draw Editor name
    SDL_Surface* nameSurface = TTF_RenderText_Blended(font, "Ogmios Editor", UIColor[currentTheme]);
    SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
    SDL_Rect nameRect = {themeButtonBox.x - nameSurface->w - 5, 4, nameSurface->w, nameSurface->h};
    SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
    SDL_FreeSurface(nameSurface);
    SDL_DestroyTexture(nameTexture);

    // Draw UI Border
    SDL_RenderDrawLine(renderer, 0, UI.h, windowWidth, UI.h);

    TTF_SetFontSize(font, currentFontSize);
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
            if (!deleteCurrentLine()) {
                deletePreviousChar();
            }
            break;
        case SDLK_DELETE:
            if (!deleteNextLine()) {
                deleteNextChar();
            }
            break;
        case SDLK_RETURN:           // NEW LINE
            insertNewLine();
            break;
        case SDLK_TAB:
            insertTab();
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
        case SDLK_s:
            if (SDL_GetModState() & KMOD_CTRL) {
                save();
            }
            break;
        case SDLK_o:
            if (SDL_GetModState() & KMOD_CTRL) {
                load();
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
    // Minus Button Event
    else if (SDL_PointInRect(&mousePos, &minusButtonBox)) {
        updateFontSize(font, -2);
    }
    // Size Button Event
    else if (SDL_PointInRect(&mousePos, &sizeButtonBox)) {
        updateFontSize(font, DEFAULT_FONT_SIZE-currentFontSize);
    }
    // Plus Button Event
    else if (SDL_PointInRect(&mousePos, &plusButtonBox)) {
        updateFontSize(font, 2);
    }
    // Theme Button Event
    else if (SDL_PointInRect(&mousePos, &themeButtonBox)) {
        updateTheme();
    }
    //  Move mouse in editor
    else if (mousePos.y >= UI.h && mousePos.y < windowHeight && mousePos.x >= 0 && mousePos.x < windowWidth) {
        int lineIndex = (mousePos.y - UI.h) / lineHeight;
        
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

void resizeWindow(int w, int h) {
    windowWidth = w;
    windowHeight = h;

    updateRects();
}

bool loop() {
    bool looping = true;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                looping = false;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    resizeWindow(event.window.data1, event.window.data2);
                }
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

    SDL_SetRenderDrawColor(renderer, textBackgroundColor[currentTheme].r, textBackgroundColor[currentTheme].g, textBackgroundColor[currentTheme].b, textBackgroundColor[currentTheme].a);
    SDL_RenderClear(renderer);

    updateScrollBar();
    
    renderText();
    renderCursor();
    renderUI();
    
    SDL_RenderPresent(renderer);
    
    return looping;
}

void kill() {
    SDL_StopTextInput();

    for (int i = 0; i < numberOfThemes; i++) {
        SDL_DestroyTexture(themesIcons[i]);
    }

    SDL_DestroyWindow(window);
    window = nullptr;
    renderer = nullptr;
    SDL_Quit();

    std::cout << "Window killed!" << std::endl;
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
