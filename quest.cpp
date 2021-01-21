#include "quest.hpp"

Quest::Quest()
{
    name = "Test";
    activations = 0;
    layer = 0;
}

void Quest::applyActivations()
{
    for(auto item : opens)
    {
        item->activations++;
    }
}

void Quest::activate()
{
    if(activations == 0)
    {
        activations = -1;
        for(auto item : opens)
        {
            item->activations--;
        }
    }
}

int Quest::recalculateLayers(int maxLayer)
{
    int depth = layer;
    if(layer >= maxLayer) {return 50;}
    for(auto item : opens)
    {
        item->layer = layer+1;
        int buffer = item->recalculateLayers(maxLayer);
        if(depth < buffer) {depth = buffer;}
    }
    return depth;
}
