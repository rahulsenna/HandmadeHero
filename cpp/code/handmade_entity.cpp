//
// Created by AgentOfChaos on 1/31/21.
//

inline move_spec
DefaultMoveSpec()
{
    move_spec MoveSpec;
    MoveSpec.UnitMaxAccelVector = false;
    MoveSpec.Speed = 1.0f;
    MoveSpec.Drag = 0.0f;
    return (MoveSpec);
}