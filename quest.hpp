#pragma once

#include <string>
#include <vector>

class Quest
{
public:
    std::string name;
    std::vector<std::string> description;
    std::vector<Quest*> opens;
    int position;
    int activations;
    int layer;

    void applyActivations();
    void activate();

    int recalculateLayers(int maxLayer);

    Quest();
};
