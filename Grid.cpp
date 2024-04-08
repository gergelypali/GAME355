#include "Grid.h"
#include "EntityManager.h"
#include <algorithm>
#include <queue>
#include "Logger.h"

int Grid::calculateIdx(const intPair& location)
{
    return location.first + location.second * m_rowNumber;
}

intPair Grid::calculateGridLocation(int idx)
{
    int x = idx % m_rowNumber;
    int y = idx / m_rowNumber;

    return intPair(x, y);
}

float Grid::herusiticCalculation(entityPtr startEntity, entityPtr targetEntity)
{
    auto& start = startEntity->getComponent<CNode>();
    auto& target = targetEntity->getComponent<CNode>();
    return abs(start.row - target.row) + abs(start.column - target.column); // Manhattan distance; usable for perfect maze solving, other one can stuck at points
    //return sqrtf(powf(abs(start.row - target.row), 2) + powf(abs(start.column - target.column), 2)); // Euclidean distance; for better, more straight pathfinding
}

std::vector<int> Grid::generateResultPath(intUMap cameFrom, entityPtr startEntity, entityPtr targetEntity)
{
    std::vector<int> res{};

    auto& start = startEntity->getComponent<CNode>();
    auto& target = targetEntity->getComponent<CNode>();

    int current = target.id;
    if(cameFrom.count(current) == 0)
        return res;

    while (current != start.id)
    {
        res.push_back(current);
        current = cameFrom[current];
    }
    res.push_back(start.id);
    std::reverse(res.begin(), res.end());

    return res;
}

void Grid::createGrid(std::shared_ptr<EntityManager> entityManager)
{
    int id{0};

    m_grid.resize(m_rowNumber, std::vector<entityPtr>(m_columnNumber));
    m_gridJustBricks.resize(m_rowNumber, std::vector<entityPtr>(m_columnNumber));
    float halfW = m_width/2.f;
    float halfH = m_heigth/2.f;

    for (int i = 0; i < m_columnNumber; i++)
    {
        for (int j = 0; j < m_rowNumber; j++)
        {
            // textures only
            auto brickNode = entityManager->addEntity(m_name + "bricks");

            brickNode->addComponent<CTransform>(MATH::Vec2{j*m_width + halfW, i*m_heigth + halfH});
            brickNode->addComponent<CRectBody>(m_width, m_heigth);
            brickNode->addComponent<CState>();
            brickNode->addComponent<CShape2d>("rectangleVertex", "rectangleIndex");
            brickNode->addComponent<CTexture>("brick");
            m_gridJustBricks[j][i] = brickNode;

            auto oneNode = entityManager->addEntity(m_name);

            oneNode->addComponent<CTransform>(MATH::Vec2{j*m_width + halfW, i*m_heigth + halfH});
            oneNode->addComponent<CRectBody>(m_width, m_heigth, MATH::Vec4{0,0,0,0});
            oneNode->addComponent<CAABB>(m_width, m_heigth);
            oneNode->addComponent<CState>();
            oneNode->addComponent<CNode>(j, i, id);
            oneNode->addComponent<CShape2d>("wallsVertex", "wallsNoneIndex");
            oneNode->addComponent<CWalls>();

            m_grid[j][i] = oneNode;

            id++;
        }
    }
}

nodes Grid::getEntityAt(const MATH::Vec2 &pos)
{
    nodes res{};
    res.resize(5, nullptr);

    int x = (int)(pos.x / m_width);
    int y = (int)(pos.y / m_heigth);
    res[0] = (m_grid[x][y]);
    if(x + 1 < m_rowNumber)
        res[1] = (m_grid[x + 1][y]);
    if(y + 1 < m_columnNumber)
        res[2] = (m_grid[x][y + 1]);
    if(x - 1 >= 0)
        res[3] = (m_grid[x - 1][y]);
    if(y - 1 >= 0)
        res[4] = (m_grid[x][y - 1]);

    return res;
}

std::vector<entityPtr> Grid::getNeighbors(int idx)
{
    std::vector<entityPtr> res{};

    intPair loc{calculateGridLocation(idx)};
    for (auto const& dir : m_dirs)
    {
        int neighborX{loc.first + dir.first};
        int neighborY{loc.second + dir.second};

        if (neighborX < 0 ||
            neighborX >= m_rowNumber ||
            neighborY < 0 ||
            neighborY >= m_columnNumber)
            continue;

        res.push_back(m_grid[neighborX][neighborY]);
    }

    return res;
}

void Grid::calculateAStar(entityPtr startEntity, entityPtr targetEntity, std::vector<int> &res)
{
    Logger::Instance()->logVerbose("calculateAStar start");
    auto& start = startEntity->getComponent<CNode>();
    auto& target = targetEntity->getComponent<CNode>();

    std::priority_queue<entityPtr, std::vector<entityPtr>, Compare> frontier;
    frontier.push(startEntity);
    intUMap cameFrom{};
    cameFrom[start.id] = -1;
    intUMap costSoFar{};
    costSoFar[start.id] = 0;

    while (!frontier.empty())
    {
        Logger::Instance()->logVerbose("calculateAStar frontier while start");
        auto currentEntity = frontier.top();
        frontier.pop();
        auto& currentNode = currentEntity->getComponent<CNode>();
        Logger::Instance()->logVerbose("calculateAStar frontier ID: " + std::to_string(currentNode.id));
        Logger::Instance()->logVerbose("calculateAStar target ID: " + std::to_string(target.id));

        if (currentNode.id == target.id)
            break;

        for (auto neighborEntity : getNeighbors(currentNode.id))
        {
            Logger::Instance()->logVerbose("calculateAStar neighbor start");
            auto& neighborNode = neighborEntity->getComponent<CNode>();
            auto dirX = neighborNode.row - currentNode.row + 1;
            auto dirY = neighborNode.column - currentNode.column + 1;
            int newCost = costSoFar[currentNode.id] + currentNode.scores[dirX][dirY];
            Logger::Instance()->logVerbose("calculateAStar neighbor ID: " + std::to_string(neighborNode.id));
            if (costSoFar.count(neighborNode.id) == 0 || newCost < costSoFar[neighborNode.id])
            {
                Logger::Instance()->logVerbose("calculateAStar neighbor new path piece");
                costSoFar[neighborNode.id] = newCost;
                neighborNode.score = newCost + herusiticCalculation(targetEntity, neighborEntity);
                frontier.push(neighborEntity);
                cameFrom[neighborNode.id] = currentNode.id;
            }
        }
        Logger::Instance()->logVerbose("calculateAStar frontier while end");
    }

    res = generateResultPath(cameFrom, startEntity, targetEntity);
    Logger::Instance()->logVerbose("calculateAStar return");
}

void Grid::generateMaze()
{
    m_grid[m_rowNumber - 1][m_columnNumber - 1]->addComponent<CMaze>();
    entityPtr currentEntity = getRandomEntityFromGrid(m_grid);
    entityPtr startEntity = currentEntity;

    while (currentEntity)
    {
        while (!currentEntity->hasComponent<CMaze>())
        {
            auto neighbors = getNeighbors(currentEntity->getComponent<CNode>().id);
            // pick one from the neighbors randomly
            auto nextEntity = neighbors[std::min((int)(neighbors.size() - 1), (int)(neighbors.size() * m_uniformDistribution(m_generator)))];

            // calculate the direction towards the chosen node and store it
            int xDir = calculateGridLocation(nextEntity->getComponent<CNode>().id).first - calculateGridLocation(currentEntity->getComponent<CNode>().id).first;
            int yDir = calculateGridLocation(nextEntity->getComponent<CNode>().id).second - calculateGridLocation(currentEntity->getComponent<CNode>().id).second;
            currentEntity->getComponent<CNode>().xDir = xDir;
            currentEntity->getComponent<CNode>().yDir = yDir;

            currentEntity = nextEntity;
        }

        // create maze path from the picked nodes
        while (startEntity->getComponent<CNode>().id != currentEntity->getComponent<CNode>().id)
        {
            startEntity->addComponent<CMaze>();

            auto& walls = startEntity->getComponent<CWalls>();
            auto& node = startEntity->getComponent<CNode>();
            auto& shape = startEntity->getComponent<CShape2d>();
            if (node.xDir == -1)
            {
                walls.west = false;
                shape.indexName = whichWall(startEntity);
                node.scores[0][1] = 0;
            }
            else if (node.yDir == -1)
            {
                walls.north = false;
                shape.indexName = whichWall(startEntity);
                node.scores[1][0] = 0;
            }
            else if (node.xDir == 1)
            {
                auto nextEntity = getEntityAt(node.row + 1, node.column);
                nextEntity->getComponent<CWalls>().west = false;
                nextEntity->getComponent<CShape2d>().indexName = whichWall(nextEntity);
                node.scores[2][1] = 0;
            }
            else if (node.yDir == 1)
            {
                auto nextEntity = getEntityAt(node.row, node.column + 1);
                nextEntity->getComponent<CWalls>().north = false;
                nextEntity->getComponent<CShape2d>().indexName = whichWall(nextEntity);
                node.scores[1][2] = 0;
            }

            startEntity = getEntityAt(node.xDir + node.row, node.yDir + node.column);
        }

        currentEntity = getRandomEntityFromGrid(m_grid);
        startEntity = currentEntity;
    }
    //delete the maze component so we can generate a new one next time
    for (auto& row: m_grid)
    {
        for (auto& entity: row)
        {
            entity->removeComponent<CMaze>();
        }
    }
}

entityPtr Grid::getRandomEntityFromGrid(std::vector<nodes>& grid)
{
    for (auto& row: grid)
    {
        for (auto& entity: row)
        {
            if (!entity->hasComponent<CMaze>())
                return entity;
        }
    }
    return nullptr;
}

std::string Grid::whichWall(entityPtr entity)
{
    auto& walls = entity->getComponent<CWalls>();

    if (!walls.north)
    {
        if (!walls.west)
        {
            return "wallsNorthWestIndex";
        }
        return "wallsNorthIndex";
    } else if (!walls.west)
    {
        return "wallsWestIndex";
    }
}
