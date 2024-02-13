#include "Animation.h"

std::pair<int, int>& Animation::getNextSpriteToDraw(const double frameLength)
{
    if (m_framesLeftToDraw <= 0)
    {
        m_framesLeftToDraw = m_animSpeed;
        m_currentAnimStep = (m_currentAnimStep == m_animSteps - 1) ? 0 : m_currentAnimStep + 1;
    }
    m_framesLeftToDraw -= frameLength;
    return m_animSequence[m_currentAnimStep];
}
