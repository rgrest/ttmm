#include "PoseType.h"

std::ostream &ttmm::operator<<(std::ostream &os, ttmm::PoseType const &pt)
{

    if (((pt & PoseType::TOP_LEFT) == PoseType::TOP_LEFT) && ((pt & PoseType::TOP_RIGHT) == PoseType::TOP_RIGHT))
    {
        os << "top middle";
    }
    else if (((pt & PoseType::BOTTOM_LEFT) == PoseType::BOTTOM_LEFT) && ((pt & PoseType::BOTTOM_RIGHT) == PoseType::BOTTOM_RIGHT))
    {
        os << "bottom middle";
    }
    else if (((pt & PoseType::MIDDLE_LEFT) == PoseType::MIDDLE_LEFT) && ((pt & PoseType::MIDDLE_RIGHT) == PoseType::MIDDLE_RIGHT))
    {
        os << "middle middle";
    }
    else
    {
        switch (pt)
        {
        case PoseType::BOTTOM_LEFT:
            os << "bottom left";
            break;
        case PoseType::BOTTOM_RIGHT:
            os << " bottom right";
            break;
        case PoseType::MIDDLE_LEFT:
            os << "middle left";
            break;
        case PoseType::MIDDLE_RIGHT:
            os << "middle right";
            break;
        case PoseType::TOP_LEFT:
            os << "top left";
            break;
        case PoseType::TOP_RIGHT:
            os << "top right";
            break;
        }
    }
    return os;
}

bool ttmm::leftFootUp(ttmm::PoseType const &pose)
{
    return 0 != (int)(pose & PoseType::TOP_LEFT);
}

bool ttmm::rightFootUp(ttmm::PoseType const &pose)
{
    return 0 != (int)(pose & PoseType::TOP_RIGHT);
}

bool ttmm::leftFootDown(ttmm::PoseType const &pose)
{
    return 0 != (int)(pose & PoseType::BOTTOM_LEFT);
}

bool ttmm::rightFootDown(ttmm::PoseType const &pose)
{
    return 0 != (int)(pose & PoseType::BOTTOM_RIGHT);
}

bool ttmm::bothFeetDown(ttmm::PoseType const &pose)
{
    return leftFootDown(pose) && rightFootDown(pose);
}

bool ttmm::bothFeetUp(ttmm::PoseType const &pose)
{
    return leftFootUp(pose) && rightFootUp(pose);
}