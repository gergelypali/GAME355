#include "VulkanScene1.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Logger.h"
#include "Grid.h"

void VulkanScene1::init()
{
    registerAction(SDL_SCANCODE_W, "UP");
    registerAction(SDL_SCANCODE_S, "DOWN");
    registerAction(SDL_SCANCODE_A, "LEFT");
    registerAction(SDL_SCANCODE_D, "RIGHT");
    registerAction(SDL_SCANCODE_R, "FINDPATH");
    registerAction(SDL_SCANCODE_F, "GENERATEMAZE");
    registerAction(SDL_BUTTON_LEFT, "MOUSECLICK");
    registerAction(SDL_MOUSEMOTION, "MOUSEMOTION");

    SDL_GetWindowSize(m_ge->window(), &windowX, &windowY);

    m_map = m_em->addEntity("map");
    m_map->addComponent<CTransform>(MATH::Vec2{windowX/2, windowY/2});
    m_map->addComponent<CState>();
    m_map->addComponent<CAABB>(windowX, windowY);

    m_grid = std::make_shared<Grid>("grid1", mazeX, mazeY, windowX, windowY);//1736 block
    m_grid->createGrid(m_em);
    m_grid->setTargetEntity(mazeX - 1, mazeY - 1);

    m_player = m_em->addEntity("Player");

    int playerWidth{25}, playerHeight{25};
    m_player->addComponent<CTransform>(m_grid->getEntityAt(0, 0)->getComponent<CTransform>().pos, MATH::Vec2(0.f, 0.f), 0, 90, 150);
    m_player->addComponent<CRectBody>(playerWidth, playerHeight, MATH::Vec4{1.f, 0.f, 1.f, 0.f});
    m_player->addComponent<CState>();
    m_player->addComponent<CAABB>(playerWidth, playerHeight);
    m_player->addComponent<CShape2d>("rectangleVertex", "triangleIndex");

    m_grid->generateMaze();

}

void VulkanScene1::endScene()
{

}

void VulkanScene1::update()
{
    checkEndMap();
    for (auto& entity: m_em->getEntities())
    { checkEntityLifetime(entity); }
    m_em->update();
    playerPhysicsUpdate();
    reactToMapBorder();
    checkWalls();
    sMovement();
    sRender();
    m_currentFrame++;
}

void VulkanScene1::sDoAction(const Action& action)
{
    if (!m_player->hasComponent<CTransform>() || !m_player->hasComponent<CState>())
        return;
    auto& transform = m_player->getComponent<CTransform>();
    auto& state = m_player->getComponent<CState>();

    if (action.name() == "UP")
    {
        transform.vel.y = (action.type() == "START") ? -1.f : 0.f;
    }
    else if (action.name() == "DOWN")
    {
        transform.vel.y = (action.type() == "START") ? 1.f : 0.f;
    }
    else if (action.name() == "LEFT")
    {
        transform.vel.x = (action.type() == "START") ? -1.f : 0.f;
    }
    else if (action.name() == "RIGHT")
    {
        transform.vel.x = (action.type() == "START") ? 1.f : 0.f;
    }
    else if (action.name() == "MOUSECLICK" && action.event().type == SDL_MOUSEBUTTONDOWN)
    {
        spawnEnemy(action.event().button.x, action.event().button.y);
    }
    else if (action.name() == "MOUSEMOTION")
    {
        //spawnEnemy(action.event().button.x, action.event().button.y);
    }
    else if (action.type() == "START" && action.name() == "FINDPATH")
    {
        std::vector<int> path{};
        m_grid->calculateAStar(m_grid->getEntityAt(m_player->getComponent<CTransform>().pos)[0], m_grid->getTargetEntity(), path);

        int lifetime{120};
        for (int i = 1; i < path.size() - 1; i++)
        {
            std::string markerName{};
            int dir{path[i + 1] - path[i]};
            if (dir == 1)
            { markerName = "rightArrow"; }
            else if (dir == -1)
            { markerName = "leftArrow"; }
            else if (dir == mazeX)
            { markerName = "downArrow"; }
            else if (dir == -mazeX)
            { markerName = "upArrow"; }
            auto node = m_grid->getEntityAt(path[i])->getComponent<CTransform>().pos;
            spawnMarker(node.x, node.y, lifetime, markerName);
            lifetime +=1;
        }
    }
    else if (action.type() == "START" && action.name() == "GENERATEMAZE")
    {
        generateMaze();
    }

    state.moving = (MATH::VMath::mag(transform.vel) != 0) ? true : false;
}

void VulkanScene1::sMovement()
{
    for (auto& entity: m_em->getEntities())
    {
        if (entity->hasComponent<CTransform>() && entity->hasComponent<CState>())
        {
            auto& transform = entity->getComponent<CTransform>();
            auto& state = entity->getComponent<CState>();

            if (state.moving)
                transform.pos = transform.pos + transform.vel * transform.moveSpeed;

            if (state.turning)
                transform.angle = fmod(transform.angle + transform.turnDirection * transform.turnSpeed, 360);

        }
    }
}

void VulkanScene1::playerPhysicsUpdate()
{
    if (m_player->hasComponent<CTransform>())
    {
        auto& transform = m_player->getComponent<CTransform>();

        transform.moveSpeed = (float)transform.maxMoveSpeed / m_ge->getFPS();
        transform.turnSpeed = transform.maxTurnSpeed / m_ge->getFPS();

        auto currentRad = atan2f(transform.vel.y, transform.vel.x);
        auto currentDeg = currentRad * 180 / M_PI;
        transform.angle = currentDeg;
    }
}

void VulkanScene1::reactToMapBorder()
{
    // check every enemy
    for ( auto& enemy: m_em->getEntities("Enemy"))
    {
        std::pair<bool, bool> checkRes{checkInsideEntity(enemy, m_map)};
        if (checkRes.first)
            enemy->getComponent<CTransform>().vel.x *= -1;
        if (checkRes.second)
            enemy->getComponent<CTransform>().vel.y *= -1;
    }

    // player part of the checking
    std::pair<bool, bool> playerCheckRes{checkInsideEntity(m_player, m_map)};
    if (playerCheckRes.first)
        m_player->getComponent<CTransform>().vel.x = 0;
    if (playerCheckRes.second)
        m_player->getComponent<CTransform>().vel.y = 0;
}

void VulkanScene1::checkWalls()
{
    if(!m_player->getComponent<CState>().moving)
        return;

    auto playerMovespeed = m_player->getComponent<CTransform>().moveSpeed;
    auto playerPos = m_player->getComponent<CTransform>().pos;
    auto playerVel = m_player->getComponent<CTransform>().vel * playerMovespeed;
    auto playerHW = m_player->getComponent<CAABB>().halfWidth();
    auto playerHH = m_player->getComponent<CAABB>().halfHeight();
    auto nodEntity = m_grid->getEntityAt(m_player->getComponent<CTransform>().pos);
    auto nodePos = nodEntity[0]->getComponent<CTransform>().pos;
    auto nodeHW = nodEntity[0]->getComponent<CAABB>().halfWidth();
    auto nodeHH = nodEntity[0]->getComponent<CAABB>().halfHeight();
    auto& nodeWalls = nodEntity[0]->getComponent<CWalls>();
    std::vector<std::shared_ptr<Entity>> eastNeighborEntities(5, nullptr);
    if (nodEntity[1])
        eastNeighborEntities = m_grid->getEntityAt(nodEntity[1]->getComponent<CTransform>().pos);
    std::vector<std::shared_ptr<Entity>> southNeighborEntities(5, nullptr);
    if (nodEntity[2])
        southNeighborEntities = m_grid->getEntityAt(nodEntity[2]->getComponent<CTransform>().pos);
    auto directNeighborEntities = m_grid->getEntityAt(nodePos);

    MATH::Vec2 relPos{playerPos - nodePos};
    if  ((playerVel.x > 0
        && relPos.x + playerHW + playerVel.x > nodeHW
        && (abs(relPos.y - playerHH) > nodeHH && (nodEntity[1] && nodEntity[1]->getComponent<CWalls>().north || eastNeighborEntities[4]->getComponent<CWalls>().west) // missing one check, the east -> north node's west wall
        || relPos.y + playerHH > nodeHH && eastNeighborEntities[2] && (eastNeighborEntities[2]->getComponent<CWalls>().north || eastNeighborEntities[2]->getComponent<CWalls>().west)
        || nodEntity[1] && nodEntity[1]->getComponent<CWalls>().west)
        ) || (
        playerVel.x < 0
        && abs(relPos.x - playerHW + playerVel.x) > nodeHW
        && (abs(relPos.y - playerHH) > nodeHH && (directNeighborEntities[4] && directNeighborEntities[4]->getComponent<CWalls>().west || directNeighborEntities[3] && directNeighborEntities[3]->getComponent<CWalls>().north)
        || relPos.y + playerHH > nodeHH && (directNeighborEntities[2] && directNeighborEntities[2]->getComponent<CWalls>().west || southNeighborEntities[3] && southNeighborEntities[3]->getComponent<CWalls>().north)
        || nodeWalls.west)
        ))
    { m_player->getComponent<CTransform>().vel.x = 0; }
    else if ((playerVel.y > 0
        && relPos.y + playerHH + playerVel.y > nodeHH
        && (relPos.x + playerHW > nodeHW && southNeighborEntities[1] && (southNeighborEntities[1]->getComponent<CWalls>().north || southNeighborEntities[1]->getComponent<CWalls>().west)
        || abs(relPos.x - playerHW) > nodeHW && (southNeighborEntities[0] && southNeighborEntities[0]->getComponent<CWalls>().west || southNeighborEntities[3] && southNeighborEntities[3]->getComponent<CWalls>().north)
        || southNeighborEntities[0] && southNeighborEntities[0]->getComponent<CWalls>().north)
    ) || (
        playerVel.y < 0
        && abs(relPos.y - playerHH + playerVel.y) > nodeHH
        && (relPos.x + playerHW > nodeHW && (eastNeighborEntities[4] && eastNeighborEntities[4]->getComponent<CWalls>().west || eastNeighborEntities[0] && eastNeighborEntities[0]->getComponent<CWalls>().north)
        || abs(relPos.x - playerHW) > nodeHW && (directNeighborEntities[4] && directNeighborEntities[4]->getComponent<CWalls>().west || directNeighborEntities[3] && directNeighborEntities[3]->getComponent<CWalls>().north)
        || nodeWalls.north)
    ))
    { m_player->getComponent<CTransform>().vel.y = 0; }
}

void VulkanScene1::spawnEnemy(const float& x, const float& y)
{
    auto enemy = m_em->addEntity("Enemy");
    enemy->addComponent<CTransform>(MATH::Vec2{x, y}, MATH::Vec2(0.f, 0.f), 0, 90, 5);
    enemy->addComponent<CRectBody>(25, 25);
    enemy->addComponent<CState>(false, false);
    enemy->addComponent<CAABB>(25, 25);
    enemy->addComponent<CShape2d>("rectangleVertex", "rectangleIndex");
    enemy->addComponent<CTexture>("plane");
    enemy->addComponent<CLifetime>(120, m_currentFrame);
}

void VulkanScene1::spawnMarker(float x, float y, int lifetime, std::string markerName)
{
    auto marker = m_em->addEntity("Marker");
    marker->addComponent<CTransform>(MATH::Vec2{x, y}, MATH::Vec2(0.f, 0.f), 0, 90, 5);
    marker->addComponent<CRectBody>(25, 25, MATH::Vec4{0,0,1,0});
    marker->addComponent<CState>();
    marker->addComponent<CShape2d>("xarrowVertex", markerName);
    marker->addComponent<CLifetime>(lifetime, m_currentFrame);
}

void VulkanScene1::generateMaze()
{
    for (auto entity: m_em->getEntities(m_grid->getGridName()))
    {
        entity->destroy();
    }
    for (auto entity: m_em->getEntities(m_grid->getGridName() + "bricks"))
    {
        entity->destroy();
    }
    m_grid.reset();
    m_grid = std::make_shared<Grid>("grid1", mazeX, mazeY, windowX, windowY);
    m_grid->createGrid(m_em);
    m_grid->generateMaze();
    m_player->getComponent<CTransform>().pos = m_grid->getEntityAt(0, 0)->getComponent<CTransform>().pos;
    m_grid->setTargetEntity(mazeX - 1, mazeY - 1);
}

void VulkanScene1::checkEndMap()
{
    if (m_grid->getEntityAt(m_player->getComponent<CTransform>().pos)[0]->getComponent<CNode>().id == mazeX * mazeY - 1)
        m_ge->changeScene("VulkanSceneMenu");
}
