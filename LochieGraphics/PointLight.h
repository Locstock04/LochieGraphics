#pragma once
#include "Maths.h"
#include <string>

namespace toml {
    inline namespace v3 {
        class table;
    }
}

enum class PointLightEffect
{
    On,
    Off,
    Flickering,
    Explosion,
    SyncsGun,
};

class PointLight {
public:
    float linear = 0;
    float quadratic = 0;
    glm::vec3 colour = {1.0f,1.0f,1.0f};

    bool on = true;
    bool canBeTriggered = false;
    std::string triggerTag = "";

    float range = 1;
    float timeInType = 0.0f;
    PointLightEffect effect = PointLightEffect::On;

    PointLight() {};
    PointLight(toml::table table);
    PointLight(PointLightEffect _effect) : effect(_effect) {};
    void GUI();

    void SetRange(float range);
    void SetRange(float linear, float quadratic);

    toml::table Serialise(unsigned long long guid)const;

    void TriggerCall(std::string tag, bool toggle);
};
