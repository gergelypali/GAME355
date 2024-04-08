#ifndef GRID_H
#define GRID_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "Entity.h"

#include <chrono>
#include <random>

class Entity;
class EntityManager;

using intPair = std::pair<int,int>;
using intUMap = std::unordered_map<int, int>;
using entityPtr = std::shared_ptr<Entity>;
using nodes = std::vector<entityPtr>;

class Compare
{
public:
    bool operator() (entityPtr a, entityPtr b)
    {
        return a->getComponent<CNode>().score > b->getComponent<CNode>().score;
    }
};

class Grid
{
private:
    std::mt19937 m_generator{std::chrono::system_clock::now().time_since_epoch().count()};
    std::uniform_real_distribution<float> m_uniformDistribution{0.0f, 1.0f};

    std::vector<nodes> m_grid;
    std::vector<nodes> m_gridJustBricks;
    entityPtr m_startEntity;
    entityPtr m_targetEntity;

    intPair m_dirs[4] = { intPair{1,0}, intPair{0,1}, intPair{-1,0}, intPair{0,-1} };

    std::string m_name{""};
    int m_rowNumber{ 0 };
    int m_columnNumber{ 0 };
    float m_width{ 0.f };
    float m_heigth{ 0.f };
    int m_windowW{ 0 };
    int m_windowH{ 0 };

    int calculateIdx(const intPair& location);
    intPair calculateGridLocation(int idx);

    //pathfinder part
    float herusiticCalculation(entityPtr startEntity, entityPtr targetEntity);
    std::vector<int> generateResultPath(intUMap cameFrom, entityPtr startEntity, entityPtr targetEntity);

    //maze generation methods
    entityPtr getRandomEntityFromGrid(std::vector<nodes>& grid);
    std::string whichWall(entityPtr entity);

public:
    Grid() = delete;
    Grid(const std::string& name, int rowNumber, int columnNumber, int windowWidth, int windowHeigth)
        : m_name(name)
        , m_windowW(windowWidth)
        , m_windowH(windowHeigth)
        , m_rowNumber(rowNumber)
        , m_columnNumber(columnNumber)
    {
        m_width = (float)m_windowW / (float)rowNumber;
        m_heigth = (float)m_windowH / (float)columnNumber;
    };

    void createGrid(std::shared_ptr<EntityManager> entityManager);

    entityPtr getStartEntity() { return m_startEntity; };
    entityPtr getTargetEntity() { return m_targetEntity; };

    void setStartEntity(entityPtr& newStart) { m_startEntity = newStart; };
    void setStartEntity(int x, int y) { m_startEntity = getEntityAt(x, y); };
    void setTargetEntity(entityPtr& newTarget) { m_targetEntity = newTarget; };
    void setTargetEntity(int x, int y) { m_targetEntity = getEntityAt(x, y); };

    void clearStartEntity() { m_startEntity.reset(); };
    void clearTargetEntity() { m_targetEntity.reset(); };

    const std::string& getGridName() { return m_name; };

    entityPtr getEntityAt(int idx) { intPair loc{calculateGridLocation(idx)}; return m_grid[loc.first][loc.second]; };
    entityPtr getEntityAt(intPair location) { return m_grid[location.first][location.second]; };
    entityPtr getEntityAt(int x, int y) { return m_grid[x][y]; };
    entityPtr getEntityAt(float xCoord, float yCoord) { return m_grid[(int)(xCoord / m_width)][(int)(yCoord / m_heigth)]; };
    nodes getEntityAt(const MATH::Vec2& pos);

    std::vector<entityPtr> getNeighbors(int idx);

    void calculateAStar(entityPtr startEntity, entityPtr targetEntity, std::vector<int>& res);

    void generateMaze();

};

#endif