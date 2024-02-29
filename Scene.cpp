#include "Scene.h"
#include <iostream>
#include "Entity.h"
#include "GameEngine.h"
#include "AssetManager.h"
#include "Animation.h"
#include "EntityManager.h"
#include "VulkanRenderer.h"

Scene::Scene(GameEngine* ge)
    : m_ge(ge)
{
    m_em = std::make_shared<EntityManager>();
}

void Scene::registerAction(int i, const std::string& name)
{
    m_actionMap[i] = name;
}

void Scene::doAction(Action action)
{
    sDoAction(action);
}

void Scene::sRender()
{
    if (m_ge->isSDL())
    {
        SDL_SetRenderDrawColor(m_ge->renderer(), 0xFF, 0xFF, 0x0, 0x0);
        SDL_RenderClear(m_ge->renderer());
        SDL_SetRenderDrawBlendMode(m_ge->renderer(), SDL_BLENDMODE_BLEND);
    }

    for (auto& entity: m_em->getEntities())
    {
        if (entity->hasComponent<CTransform>())
        {
            if (entity->hasComponent<CText>())
            {
                drawText(entity);
            }
            if (entity->hasComponent<CRectBody>())
            {
                if (entity->hasComponent<CTexture>())
                {
                    drawTexture(entity);
                }
                else if (entity->hasComponent<CSpriteSet>())
                {
                    if (entity->hasComponent<CAnimation>())
                        drawAnimation(entity);
                    else
                        drawSpriteSet(entity);
                }
                else if (entity->hasComponent<CSpriteStack>())
                {
                    drawSpriteStack(entity);
                }
                else if (entity->hasComponent<CVoxel>())
                {
                    drawVoxel(entity);
                }
                else
                {
                    drawRect(entity);
                }
            }
        }
    }

    // render everything at the end of each render loop
    if (m_ge->isSDL())
    {
        SDL_RenderPresent(m_ge->renderer());
    }
    else
    {
        m_ge->vulkanRenderer()->drawFrame();
    }
}

void Scene::drawRect(std::shared_ptr<Entity> &entity)
{
    if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CRectBody>())
        return;
    auto& transform = entity->getComponent<CTransform>();
    auto& body = entity->getComponent<CRectBody>();

    if (m_ge->isSDL())
    {
        SDL_Rect fillRect{
            (int)transform.cameraViewPos.x - body.halfWidth(),
            (int)transform.cameraViewPos.y - body.halfHeight(),
            body.width(),
            body.height()
        };

        SDL_SetRenderDrawColor(
            m_ge->renderer(),
            body.color().x,//r
            body.color().y,//g
            body.color().z,//b
            body.color().w);//alpha

        SDL_RenderFillRect( m_ge->renderer(), &fillRect );
    }
    else
    {
        m_ge->vulkanRenderer()->vulkanRenderRect(entity);
    }
}

void Scene::drawTexture(std::shared_ptr<Entity> &entity)
{
    if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CRectBody>() || !entity->hasComponent<CTexture>())
        return;
    auto& transform = entity->getComponent<CTransform>();
    auto& body = entity->getComponent<CRectBody>();
    auto& texture = entity->getComponent<CTexture>();

    SDL_Rect fillRect{
        (int)transform.cameraViewPos.x - body.halfWidth(),
        (int)transform.cameraViewPos.y - body.halfHeight(),
        body.width(),
        body.height()
        };

    SDL_RenderCopyEx(m_ge->renderer(), m_ge->assetManager()->GetTexture(texture.name), NULL, &fillRect, 0, NULL, SDL_FLIP_NONE);
}

void Scene::drawSpriteSet(std::shared_ptr<Entity> &entity)
{
    if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CRectBody>() || !entity->hasComponent<CSpriteSet>())
        return;

    auto& transform = entity->getComponent<CTransform>();
    auto& body = entity->getComponent<CRectBody>();
    auto& spriteSet = entity->getComponent<CSpriteSet>();

    SDL_Rect fillRect{
        (int)transform.cameraViewPos.x - body.halfWidth(),
        (int)transform.cameraViewPos.y - body.halfHeight(),
        body.width(),
        body.height()
        };
    SDL_Rect cutoutRect{
        spriteSet.columnNumber * spriteSet.w,
        spriteSet.rowNumber * spriteSet.h,
        spriteSet.w,
        spriteSet.h
    };
    SDL_RenderCopyEx(m_ge->renderer(), m_ge->assetManager()->GetTexture(spriteSet.name), &cutoutRect, &fillRect, transform.angle, NULL, SDL_FLIP_NONE);
}

void Scene::drawSpriteStack(std::shared_ptr<Entity>& entity)
{
    if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CRectBody>() || !entity->hasComponent<CSpriteStack>())
        return;
    auto& spriteStack = entity->getComponent<CSpriteStack>();
    auto& transform = entity->getComponent<CTransform>();
    auto& body = entity->getComponent<CRectBody>();

    SDL_Rect cutterRect{ spriteStack.cutoutRect };
    SDL_Rect fillRect{ (int)transform.cameraViewPos.x - body.halfWidth(), (int)transform.cameraViewPos.y - body.halfHeight(), body.width(), body.height() };

    for ( int i = 0; i < spriteStack.rowNumber; i++)
    {
        cutterRect.y = i * spriteStack.h;
        for ( int j = 0; j < spriteStack.columnNumber; j++)
        {
            cutterRect.x = j * spriteStack.w;
            SDL_RenderCopyEx(m_ge->renderer(), m_ge->assetManager()->GetTexture(spriteStack.name), &cutterRect, &fillRect, transform.angle, NULL, SDL_FLIP_NONE);
            fillRect.y -= spriteStack.step;
        }
    }
}

void Scene::drawVoxel(std::shared_ptr<Entity>& entity)
{
    if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CRectBody>() || !entity->hasComponent<CVoxel>())
        return;
    auto& voxel = entity->getComponent<CVoxel>();
    auto& transform = entity->getComponent<CTransform>();
    auto& body = entity->getComponent<CRectBody>();
    SDL_Rect cutterRect{ voxel.cutoutRect };
    SDL_Rect fillRect{ (int)transform.cameraViewPos.x - body.halfWidth(), (int)transform.cameraViewPos.y - body.halfHeight(), body.width(), body.height() };

    for ( int i = voxel.rowNumber - 1; i > 0; i--)
    {
        cutterRect.y = i * voxel.h;
        SDL_RenderCopyEx(m_ge->renderer(), m_ge->assetManager()->GetTexture(voxel.name), &cutterRect, &fillRect, transform.angle - 90, NULL, SDL_FLIP_NONE);
        fillRect.y -= voxel.step;
    }
}

void Scene::drawAnimation(std::shared_ptr<Entity> &entity)
{
    if (!entity->hasComponent<CTransform>() ||
        !entity->hasComponent<CRectBody>() ||
        !entity->hasComponent<CSpriteSet>() ||
        !entity->hasComponent<CAnimation>())
        return;

    auto& spriteSet = entity->getComponent<CSpriteSet>();
    auto& transform = entity->getComponent<CTransform>();
    auto& body = entity->getComponent<CRectBody>();
    auto& anim = entity->getComponent<CAnimation>().anim;

    SDL_Rect fillRect{
        (int)transform.cameraViewPos.x - body.halfWidth(),
        (int)transform.cameraViewPos.y - body.halfHeight(),
        body.width(),
        body.height()
        };
        auto nextSprite = anim->getNextSpriteToDraw(m_ge->getFrameTime());
    SDL_Rect cutoutRect{
        nextSprite.second * spriteSet.w,
        nextSprite.first * spriteSet.h,
        spriteSet.w,
        spriteSet.h
    };
    SDL_RenderCopyEx(m_ge->renderer(), m_ge->assetManager()->GetTexture(spriteSet.name), &cutoutRect, &fillRect, transform.angle, NULL, SDL_FLIP_NONE);
}

void Scene::drawText(std::shared_ptr<Entity> &entity)
{
    if (!entity->hasComponent<CText>() || !entity->hasComponent<CTransform>())
        return;

    auto& transform = entity->getComponent<CTransform>();
    auto& text = entity->getComponent<CText>();

    m_ge->renderText(text.text, text.font, text.color, text.fontSize, transform.cameraViewPos);
}
