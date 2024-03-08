#ifndef COMPONENT_H
#define COMPONENT_H

#include "Vector.h"
#include "VMath.h"
#include <SDL.h>
#include <memory>
#include <SDL_ttf.h>
#include <cstring>
#include <vulkan/vulkan.h>

class Animation;

class Component
{
public:
//just an interface class, nothing more
    bool added{false};
};

class CTransform : public Component
{
public:
    CTransform() {};
    CTransform(MATH::Vec2 startingPos) : pos(startingPos), cameraViewPos(startingPos) {};
    CTransform(MATH::Vec2 startingPos, MATH::Vec2 startingVel, double angleInit, double turnSpeedInit, int moveSpeedInit)
        : pos(startingPos), cameraViewPos(startingPos), vel(startingVel), angle(angleInit), turnSpeed(turnSpeedInit), maxTurnSpeed(turnSpeedInit), moveSpeed(moveSpeedInit), maxMoveSpeed(moveSpeedInit) {};

    MATH::Vec2 pos{0.f, 0.f};
    MATH::Vec2 cameraViewPos{0.f, 0.f};
    MATH::Vec2 vel{0.f, 0.f};
    double angle{0};
    int turnDirection{1};
    double turnSpeed{0};
    double maxTurnSpeed{0};
    int moveSpeed{0};
    int maxMoveSpeed{0};

};

class CAABB : public Component
{
public:
    CAABB() {};
    CAABB(int w, int h) :
        width(w),
        height(h),
        halfWidth(w / 2),
        halfHeight(h / 2)
    {};

    int width{0};
    int height{0};
    int halfWidth{0};
    int halfHeight{0};

};

class CState : public Component
{
public:
    CState() {};
    CState(bool turn, bool move) : turning(turn), moving(move) {};

    bool turning{false};
    bool moving{false};
    bool cameraIndependent{false};

};

class CInput : public Component
{
public:

};

class CRectBody : public Component
{
private:
    int m_width{0};
    int m_height{0};
    int m_halfWidth{0};
    int m_halfHeight{0};
    MATH::Vec4 m_color{};

public:
    CRectBody() {};
    CRectBody(int width, int height) : m_width(width), m_height(height), m_halfWidth(width / 2), m_halfHeight(height / 2) {};
    CRectBody(int width, int height, MATH::Vec4 color) : m_width(width), m_height(height), m_halfWidth(width / 2), m_halfHeight(height / 2), m_color(color) {};

    int width() { return m_width; };
    int height() { return m_height; };
    int halfWidth() { return m_halfWidth; };
    int halfHeight() { return m_halfHeight; };
    MATH::Vec4& color() { return m_color; };

};

class CLifetime : public Component
{
public:
    CLifetime() {};
    CLifetime(int lifetime, int start): maxLifetime(lifetime), startFrame(start) {};

    int maxLifetime{0}; // number of frames;
    int startFrame{0};

};

class CScore : public Component
{
public:
    CScore() {};

    int score{0};

};

class CTexture : public Component
{
public:
    CTexture() {};
    CTexture(const std::string& theName) : name(theName) {};

    std::string name{""};

};

class CSpriteSet : public Component
{
public:
    CSpriteSet() {};
    CSpriteSet(
        const std::string& theName,
        int rowNbr,
        int columnNbr,
        int initW,
        int initH,
        int drawRow,
        int drawColumn
        )
        :
        name(theName),
        w(initW / columnNbr),
        h(initH / rowNbr),
        rowNumber(drawRow),
        maxRowNumber(rowNbr),
        columnNumber(drawColumn),
        maxColumnNumber(columnNbr
        ) {};
    CSpriteSet(
        const std::string& theName,
        int rowNbr,
        int columnNbr,
        int initW,
        int initH
        )
        :
        name(theName),
        w(initW / columnNbr),
        h(initH / rowNbr),
        maxRowNumber(rowNbr),
        maxColumnNumber(columnNbr
        ) {};

    std::string name{""};
    int w{0}, h{0};
    int rowNumber{0};
    int columnNumber{0};
    int maxRowNumber{};
    int maxColumnNumber{};

};

class CSpriteStack : public Component
{
public:
    CSpriteStack() {};
    CSpriteStack(const std::string& theName, int rowNbr, int columnNbr, int initW, int initH, int initStep)
        : name(theName), w(initW / columnNbr), h(initH / rowNbr), rowNumber(rowNbr), columnNumber(columnNbr), step(initStep) {
            cutoutRect.w = w;
            cutoutRect.h = h;
        };

    std::string name{""};
    int w{0}, h{0};
    SDL_Rect cutoutRect{0, 0, 0, 0};
    int rowNumber{};
    int columnNumber{};
    int step{};

};

class CVoxel : public Component
{
public:
    CVoxel() {};
    CVoxel(const std::string& theName, int rowNbr, int columnNbr, int initW, int initH, int playerHeight)
        : name(theName), w(initW / columnNbr), h(initH / rowNbr), rowNumber(rowNbr), columnNumber(columnNbr) {
            cutoutRect.x = 0;
            cutoutRect.y = (rowNbr - 1) * h;
            cutoutRect.w = w;
            cutoutRect.h = h;
            step = playerHeight / h;
        };

    std::string name{""};
    int w{0}, h{0};
    SDL_Rect cutoutRect{0, 0, 0, 0};
    int rowNumber{};
    int columnNumber{};
    int step{};
};

class CAnimation : public Component
{
public:
    CAnimation() {};
    CAnimation(std::shared_ptr<Animation> newAnim) : anim(newAnim) {};

    std::shared_ptr<Animation> anim{nullptr};

};

class CText : public Component
{
public:
    CText() {};
    CText(std::string textInit, TTF_Font* fontInit, const SDL_Color& colorInit, int fontSizeInit) : text(textInit), font(fontInit), color(colorInit), fontSize(fontSizeInit) {};
    CText(int textInit, TTF_Font* fontInit, const SDL_Color& colorInit, int fontSizeInit) : text(std::to_string(textInit)), font(fontInit), color(colorInit), fontSize(fontSizeInit) {};

    std::string text{""};
    TTF_Font* font{nullptr};
    SDL_Color color{};
    int fontSize{0};

};

#endif