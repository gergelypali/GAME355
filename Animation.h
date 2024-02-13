#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include <vector>

class Animation
{
public:
    Animation() = delete;
    Animation(int animSpeed, const std::vector<std::pair<int,int>>& sequence) :
        m_animSpeed(animSpeed),
        m_framesLeftToDraw(animSpeed),
        m_animSequence(sequence)
        {
            m_animSteps = m_animSequence.size();
        };

    std::pair<int,int>& getNextSpriteToDraw(const double frameLength);

private:
    int m_animSteps{0};// numbers of sprites on the sequence
    const int m_animSpeed{0};// how many frames each sprite will be drawn
    int m_framesLeftToDraw{0};
    int m_currentAnimStep{0};

    std::vector<std::pair<int, int>> m_animSequence{};// the sequence of the sprites as row/column

};

#endif