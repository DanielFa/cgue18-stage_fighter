//
// Created by Raphael on 22.04.2018.
//

#ifndef STAGE_FIGHTER_DEBUGTEXTHUD_H
#define STAGE_FIGHTER_DEBUGTEXTHUD_H

#include <memory>
#include <functional>

#include "../helper/Logger.h"
#include "Widget.h"

#include "Label.h"

/**
 * Debug Text HUD does handle all the F_ Keys which should display some debug information:
 * F1 - Help (if available)
 * F2 - Frame Time on/off
 * F3 - Wire Frame on/off
 * F4-F7 - Enable/Disable effect (if necessary, see Effects)
 * F8 - View-frustum Culling on/off
 */
class DebugTextHud : public Widget, public Logger {

private:
    double refreshTimer;

    bool wireFrameMode = false;
    bool viewFrustumCulling = false;

    bool effect[4] = {true, true, true, true};

    std::shared_ptr<Label> fpsLabel;
    char fpsBuffer[64];
    double lastDelta;
    double highestDelta;
    double accumDelta;

    std::function<void(int,int,int,int)> keyCallback;
public:
    DebugTextHud();

    void toggleEffect(int id);
    bool showFrameTime = false;

    void update(std::chrono::duration<double, std::milli> delta);
    void render(const glm::mat4 &projection) override;
    void resize(float x, float y) override;

    std::function<void(int,int,int,int)> &getKeyCallback();
};


#endif //STAGE_FIGHTER_DEBUGTEXTHUD_H